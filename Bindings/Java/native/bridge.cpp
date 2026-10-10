// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

#include <PhysikX/physikx.h>
#include <cstdint>
#include <jni.h>
static px_world *world(jlong handle) {
  return reinterpret_cast<px_world *>(static_cast<uintptr_t>(handle));
}
static bool check(JNIEnv *env, px_status status) {
  if (status == PX_OK)
    return true;
  auto klass = env->FindClass("java/lang/IllegalStateException");
  if (klass)
    env->ThrowNew(klass, px_last_error());
  return false;
}
extern "C" {
JNIEXPORT jlong JNICALL Java_physikx_World_create(JNIEnv *env, jclass,
                                                  jint dim) {
  auto desc = px_world_defaults(static_cast<uint32_t>(dim));
  auto *w = px_world_create(&desc);
  if (!w) {
    check(env, PX_INVALID_ARGUMENT);
    return 0;
  }
  return static_cast<jlong>(reinterpret_cast<uintptr_t>(w));
}
JNIEXPORT void JNICALL Java_physikx_World_destroy(JNIEnv *, jclass, jlong w) {
  px_world_destroy(world(w));
}
JNIEXPORT jlong JNICALL Java_physikx_World_body(
    JNIEnv *env, jclass, jlong w, jint shape, jdouble x, jdouble y, jdouble z,
    jdouble hx, jdouble hy, jdouble hz, jdouble mass, jboolean fixed) {
  auto d = px_body_defaults();
  d.shape = shape;
  d.position = {x, y, z};
  d.half_extents = {hx, hy, hz};
  d.radius = hx;
  d.mass = mass;
  d.is_static = fixed ? 1 : 0;
  px_id id = 0;
  check(env, px_body_create(world(w), &d, &id));
  return static_cast<jlong>(id);
}
JNIEXPORT void JNICALL Java_physikx_World_stepNative(JNIEnv *env, jclass,
                                                     jlong w, jdouble elapsed) {
  check(env, px_world_step(world(w), elapsed));
}
JNIEXPORT jdoubleArray JNICALL Java_physikx_World_stateNative(JNIEnv *env,
                                                              jclass, jlong w,
                                                              jlong id) {
  px_body_state state{};
  if (!check(env, px_body_get_state(world(w), static_cast<px_id>(id), &state)))
    return nullptr;
  auto out = env->NewDoubleArray(6);
  if (!out)
    return nullptr;
  jdouble data[] = {state.position.x, state.position.y, state.position.z,
                    state.velocity.x, state.velocity.y, state.velocity.z};
  env->SetDoubleArrayRegion(out, 0, 6, data);
  return out;
}
JNIEXPORT void JNICALL Java_physikx_World_vector(JNIEnv *env, jclass, jlong w,
                                                 jlong id, jint op, jdouble x,
                                                 jdouble y, jdouble z) {
  px_vec3 v{x, y, z};
  px_status result = PX_INVALID_ARGUMENT;
  switch (op) {
  case 0:
    result = px_world_set_gravity(world(w), v);
    break;
  case 1:
    result = px_body_set_position(world(w), id, v);
    break;
  case 2:
    result = px_body_set_velocity(world(w), id, v);
    break;
  case 3:
    result = px_body_apply_force(world(w), id, v);
    break;
  case 4:
    result = px_body_apply_impulse(world(w), id, v);
    break;
  }
  check(env, result);
}
JNIEXPORT void JNICALL Java_physikx_World_removeBody(JNIEnv *env, jclass,
                                                     jlong w, jlong id) {
  check(env, px_body_destroy(world(w), id));
}
JNIEXPORT jlong JNICALL Java_physikx_World_distanceNative(JNIEnv *env, jclass,
                                                          jlong w, jlong a,
                                                          jlong b,
                                                          jdouble length,
                                                          jdouble compliance) {
  px_distance_desc d{static_cast<px_id>(a), static_cast<px_id>(b), length,
                     compliance};
  px_id id = 0;
  check(env, px_distance_create(world(w), &d, &id));
  return static_cast<jlong>(id);
}
JNIEXPORT void JNICALL Java_physikx_World_removeDistance(JNIEnv *env, jclass,
                                                         jlong w, jlong id) {
  check(env, px_distance_destroy(world(w), id));
}
}
