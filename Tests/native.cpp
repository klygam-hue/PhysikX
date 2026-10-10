// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

#include <PhysikX/physikx.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
static void require(bool pass, const char *what) {
  if (!pass)
    throw std::runtime_error(what);
}
static void near(double a, double b, double eps, const char *what) {
  require(std::abs(a - b) < eps, what);
}
static px_world_desc zero(uint32_t dim = 3) {
  auto d = px_world_defaults(dim);
  d.gravity = {0, 0, 0};
  d.linear_damping = 0;
  return d;
}
static px_body_desc ball(px_vec3 p = {0, 0, 0}) {
  auto d = px_body_defaults();
  d.position = p;
  d.layer = 0;
  return d;
}
int main() {
  int passed = 0;
  auto test = [&](const char *name, auto fn) {
    try {
      fn();
      ++passed;
      std::cout << "PASS " << name << '\n';
    } catch (const std::exception &e) {
      std::cerr << "FAIL " << name << ": " << e.what() << '\n';
      throw;
    }
  };
  try {
    test("invalid descriptors", [] {
      auto d = zero(4);
      require(px_world_create(&d) == nullptr, "dimension accepted");
      d = zero();
      d.fixed_dt = std::numeric_limits<double>::quiet_NaN();
      require(px_world_create(&d) == nullptr, "NaN accepted");
      require(px_world_step(nullptr, 1) == PX_INVALID_ARGUMENT,
              "null accepted");
    });
    test("analytical free fall", [] {
      auto d = zero();
      d.gravity = {0, -9.81, 0};
      PhysikX::World w(d);
      auto id = w.create_body(ball({0, 10, 0}));
      for (int i = 0; i < 120; ++i)
        w.step(d.fixed_dt);
      auto s = w.state(id);
      near(s.velocity.y, -9.81, 1e-10, "velocity");
      near(s.position.y, 10 - 9.81 * d.fixed_dt * d.fixed_dt * 120 * 121 / 2,
           1e-10, "semi-implicit trajectory");
    });
    test("2D plane", [] {
      PhysikX::World w(zero(2));
      auto d = ball({1, 2, 30});
      d.velocity = {1, 2, 9};
      auto id = w.create_body(d);
      w.force(id, {0, 0, 100});
      w.impulse(id, {0, 0, 100});
      w.step(1. / 120);
      auto s = w.state(id);
      near(s.position.z, 0, 1e-12, "z position");
      near(s.velocity.z, 0, 1e-12, "z velocity");
    });
    test("impulse mass", [] {
      PhysikX::World w(zero());
      auto d = ball();
      d.mass = 2;
      auto id = w.create_body(d);
      w.impulse(id, {10, 0, 0});
      near(w.state(id).velocity.x, 5, 1e-12, "mass scaling");
    });
    test("force accumulation across substeps", [] {
      auto d = zero();
      PhysikX::World w(d);
      auto id = w.create_body(ball());
      w.force(id, {12, 0, 0});
      w.step(d.fixed_dt / 2);
      near(w.state(id).velocity.x, 0, 1e-12, "partial time");
      w.step(d.fixed_dt * 2.5);
      near(w.state(id).velocity.x, 0.3, 1e-12, "force lost");
      w.step(d.fixed_dt);
      near(w.state(id).velocity.x, 0.3, 1e-12, "force not cleared");
    });
    test("stale handles and swap removal", [] {
      PhysikX::World w(zero());
      auto a = w.create_body(ball()), b = w.create_body(ball()),
           c = w.create_body(ball());
      w.destroy_body(b);
      require(w.state(c).user_data == 0, "swapped body lost");
      px_body_state s;
      require(px_body_get_state(w.native_handle(), b, &s) == PX_NOT_FOUND,
              "stale id reused");
      w.destroy_body(a);
      auto next = w.create_body(ball());
      require(next != b && next != a, "handle reuse");
    });
    test("constraint cleanup", [] {
      PhysikX::World w(zero());
      auto a = w.create_body(ball()), b = w.create_body(ball({2, 0, 0}));
      w.distance({a, b, 1, 0});
      w.destroy_body(a);
      require(w.stats().constraints == 0, "orphan link");
      w.step(1. / 120);
    });
    test("XPBD distance", [] {
      PhysikX::World w(zero());
      auto a = ball();
      a.is_static = 1;
      auto id = w.create_body(a), b = w.create_body(ball({2, 0, 0}));
      w.distance({id, b, 1, 0});
      w.step(1. / 120);
      near(w.state(b).position.x, 1, 1e-9, "distance not satisfied");
    });
    test("coincident constraint", [] {
      PhysikX::World w(zero());
      auto a = w.create_body(ball()), b = w.create_body(ball());
      w.distance({a, b, 1, 0});
      w.step(1. / 120);
      near(std::abs(w.state(a).position.x - w.state(b).position.x), 1, 1e-9,
           "coincident distance");
    });
    test("elastic collision momentum", [] {
      PhysikX::World w(zero());
      auto a = ball({-0.49, 0, 0}), b = ball({0.49, 0, 0});
      a.layer = b.layer = 1;
      a.restitution = b.restitution = 1;
      a.friction = b.friction = 0;
      a.velocity = {1, 0, 0};
      b.velocity = {-1, 0, 0};
      auto x = w.create_body(a), y = w.create_body(b);
      w.step(1. / 120);
      near(w.state(x).velocity.x, -1, 1e-9, "a rebound");
      near(w.state(y).velocity.x, 1, 1e-9, "b rebound");
    });
    test("sphere inside box", [] {
      PhysikX::World w(zero());
      auto b = ball();
      b.shape = PX_BOX;
      b.is_static = 1;
      b.layer = 1;
      b.half_extents = {2, 2, 2};
      w.create_body(b);
      auto d = ball({1.9, 0, 0});
      d.layer = 1;
      auto id = w.create_body(d);
      w.step(1. / 120);
      require(w.state(id).position.x > 2.49, "sphere stayed inside");
    });
    test("resting floor", [] {
      auto d = px_world_defaults(3);
      PhysikX::World w(d);
      auto floor = px_body_defaults();
      floor.shape = PX_BOX;
      floor.is_static = 1;
      floor.position = {0, -.5, 0};
      floor.half_extents = {50, .5, 50};
      floor.restitution = 0;
      w.create_body(floor);
      auto b = px_body_defaults();
      b.position = {0, 5, 0};
      b.restitution = 0;
      auto id = w.create_body(b);
      for (int i = 0; i < 2400; ++i)
        w.step(d.fixed_dt);
      auto s = w.state(id);
      near(s.position.y, .5, 2e-4, "floor penetration");
      near(s.velocity.y, 0, 1e-9, "resting velocity");
    });
    test("box box contact", [] {
      PhysikX::World w(zero(2));
      auto a = px_body_defaults();
      a.shape = PX_BOX;
      a.is_static = 1;
      w.create_body(a);
      a.is_static = 0;
      a.position = {.8, 0, 0};
      auto id = w.create_body(a);
      w.step(1. / 120);
      require(w.state(id).position.x > .99, "box penetration");
    });
    test("collision masks", [] {
      PhysikX::World w(zero());
      auto a = px_body_defaults();
      a.mask = 0;
      auto x = w.create_body(a), y = w.create_body(a);
      w.step(1. / 120);
      require(w.stats().contacts == 0, "mask ignored");
      near(w.state(x).position.x, w.state(y).position.x, 1e-12,
           "masked separation");
    });
    test("ray and AABB", [] {
      PhysikX::World w(zero());
      auto d = px_body_defaults();
      d.position = {3, 0, 0};
      auto id = w.create_body(d);
      px_ray_hit hit;
      PhysikX::check(px_world_raycast(w.native_handle(), {0, 0, 0}, {2, 0, 0},
                                      10, 1, &hit));
      require(hit.body == id, "ray body");
      near(hit.distance, 2.5, 1e-12, "ray distance");
      require(px_world_query_aabb(w.native_handle(), {2, -1, -1}, {4, 1, 1},
                                  nullptr, 0) == 1,
              "query count");
      PhysikX::check(px_world_raycast(w.native_handle(), {3, 0, 0}, {1, 0, 0},
                                      10, 1, &hit));
      near(hit.distance, 0, 1e-12, "inside ray");
    });
    test("box ray parallel miss", [] {
      PhysikX::World w(zero());
      auto d = px_body_defaults();
      d.shape = PX_BOX;
      w.create_body(d);
      px_ray_hit hit;
      require(px_world_raycast(w.native_handle(), {2, 2, 0}, {1, 0, 0}, 10, 1,
                               &hit) == PX_NOT_FOUND,
              "parallel ray");
    });
    test("time budget and speed clamp", [] {
      auto d = zero();
      d.max_speed = 5;
      d.max_substeps = 2;
      PhysikX::World w(d);
      auto id = w.create_body(ball());
      w.impulse(id, {1000, 0, 0});
      w.step(10);
      near(w.state(id).velocity.x, 5, 1e-12, "speed limit");
      require(w.stats().steps == 2, "substep limit");
      require(w.stats().dropped_time > 9.9, "dropped time");
    });
    test("invalid state mutation", [] {
      PhysikX::World w(zero());
      auto id = w.create_body(ball());
      require(px_body_set_position(w.native_handle(), id, {INFINITY, 0, 0}) ==
                  PX_INVALID_ARGUMENT,
              "infinite position");
      near(w.state(id).position.x, 0, 1e-12, "state corrupted");
      require(px_world_step(w.native_handle(), -1) == PX_INVALID_ARGUMENT,
              "negative time");
    });
    test("static invariance", [] {
      PhysikX::World w;
      auto d = ball({0, 3, 0});
      d.is_static = 1;
      auto id = w.create_body(d);
      w.step(.1);
      near(w.state(id).position.y, 3, 1e-12, "static movement");
      require(px_body_apply_impulse(w.native_handle(), id, {1, 0, 0}) ==
                  PX_INVALID_ARGUMENT,
              "static impulse");
    });
    test("repeatability", [] {
      auto simulate = []() {
        PhysikX::World w;
        auto b = px_body_defaults();
        b.position = {1, 10, 2};
        auto id = w.create_body(b);
        for (int i = 0; i < 200; ++i)
          w.step(1. / 120);
        return w.state(id).position.y;
      };
      require(simulate() == simulate(), "same-build nondeterminism");
    });
  } catch (...) {
    return 1;
  }
  std::cout << passed << " native regression tests passed\n";
  return 0;
}
