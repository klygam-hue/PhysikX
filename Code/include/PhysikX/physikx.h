// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

#ifndef PHYSIKX_H
#define PHYSIKX_H
#include <stddef.h>
#include <stdint.h>
#if defined(_WIN32)
#if defined(PX_BUILD)
#define PX_API __declspec(dllexport)
#else
#define PX_API __declspec(dllimport)
#endif
#else
#define PX_API __attribute__((visibility("default")))
#endif
#ifdef __cplusplus
extern "C" {
#endif
/* ABI v1: structs use natural alignment; scalars are fixed-width. SI units. */
typedef struct px_world px_world;
typedef uint64_t px_id;
typedef struct {
  double x, y, z;
} px_vec3;
typedef enum {
  PX_OK = 0,
  PX_INVALID_ARGUMENT = 1,
  PX_NOT_FOUND = 2,
  PX_OUT_OF_MEMORY = 3,
  PX_INTERNAL_ERROR = 4
} px_status;
typedef enum { PX_SPHERE = 0, PX_BOX = 1 } px_shape;
typedef struct {
  uint32_t dimension, iterations, max_substeps, reserved;
  double fixed_dt, linear_damping, max_speed;
  px_vec3 gravity;
} px_world_desc;
typedef struct {
  uint32_t shape, is_static, layer, mask;
  px_vec3 position, velocity, half_extents;
  double radius, mass, restitution, friction;
  uint64_t user_data;
} px_body_desc;
typedef struct {
  px_vec3 position, velocity;
  uint64_t user_data;
} px_body_state;
typedef struct {
  px_id a, b;
  double rest_length, compliance;
} px_distance_desc;
typedef struct {
  px_id a, b;
  px_vec3 normal;
  double penetration;
} px_contact;
typedef struct {
  px_id body;
  px_vec3 point, normal;
  double distance;
} px_ray_hit;
typedef struct {
  uint64_t bodies, constraints, candidates, contacts, steps;
  double dropped_time;
} px_stats;
PX_API const char *px_version(void);
PX_API const char *px_last_error(void); /* thread-local; never free */
PX_API px_world_desc px_world_defaults(uint32_t dimension);
PX_API px_body_desc px_body_defaults(void);
PX_API px_world *px_world_create(const px_world_desc *desc);
PX_API void px_world_destroy(px_world *world);
PX_API px_status px_world_step(px_world *world, double elapsed);
PX_API px_status px_world_set_gravity(px_world *world, px_vec3 gravity);
PX_API px_status px_world_get_stats(const px_world *world, px_stats *out);
PX_API px_status px_body_create(px_world *world, const px_body_desc *desc,
                                px_id *out);
PX_API px_status px_body_destroy(px_world *world, px_id body);
PX_API px_status px_body_get_state(const px_world *world, px_id body,
                                   px_body_state *out);
PX_API px_status px_body_set_position(px_world *world, px_id body,
                                      px_vec3 value);
PX_API px_status px_body_set_velocity(px_world *world, px_id body,
                                      px_vec3 value);
PX_API px_status px_body_apply_force(px_world *world, px_id body,
                                     px_vec3 value);
PX_API px_status px_body_apply_impulse(px_world *world, px_id body,
                                       px_vec3 value);
PX_API px_status px_distance_create(px_world *world,
                                    const px_distance_desc *desc, px_id *out);
PX_API px_status px_distance_destroy(px_world *world, px_id constraint);
/* Contacts describe the final substep. On insufficient capacity, return total
 * count. */
PX_API size_t px_world_contacts(const px_world *world, px_contact *out,
                                size_t capacity);
PX_API size_t px_world_query_aabb(const px_world *world, px_vec3 min,
                                  px_vec3 max, px_id *out, size_t capacity);
/* Direction need not be normalized. A ray starting inside a body hits at
 * distance zero. */
PX_API px_status px_world_raycast(const px_world *world, px_vec3 origin,
                                  px_vec3 direction, double max_distance,
                                  uint32_t mask, px_ray_hit *out);
#ifdef __cplusplus
}
#endif
#endif
