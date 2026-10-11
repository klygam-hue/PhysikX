// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

#include <PhysikX/physikx.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
static_assert(sizeof(px_world_desc) == 64 && sizeof(px_body_desc) == 128,
              "Existing ABI v1 descriptors changed");
static_assert(sizeof(px_mass_properties) == 56 && sizeof(px_material) == 24 &&
              sizeof(px_distance_properties) == 48 && sizeof(px_break_event) == 88,
              "Extension layout mismatch");
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
    test("volume density weight and mass mutation", [] {
      for (uint32_t dim : {2u, 3u}) {
        auto settings = zero(dim);
        settings.gravity = {3, -4, 0};
        PhysikX::World w(settings);
        auto d = ball(); d.shape = PX_BOX; d.half_extents = {2, 3, 4}; d.mass = 12;
        auto id = w.create_body(d);
        auto p = w.mass_properties(id);
        near(p.volume, dim == 2 ? 24 : 192, 1e-10, "box volume");
        near(p.weight, 60, 1e-10, "weight magnitude");
        near(p.weight_force.x, 36, 1e-10, "weight direction");
        w.set_density(id, 2);
        near(w.mass_properties(id).mass, p.volume * 2, 1e-10, "density mass");
        w.set_mass(id, 2);
        w.impulse(id, {4, 0, 0});
        near(w.state(id).velocity.x, 2, 1e-12, "new inverse mass");
        d = ball(); d.radius = 2;
        p = w.mass_properties(w.create_body(d));
        near(p.volume, dim == 2 ? 4 * std::acos(-1.0) : 32 * std::acos(-1.0) / 3,
             1e-10, "sphere volume");
      }
    });
    test("thrust normalization and validation", [] {
      auto settings = zero(2); settings.fixed_dt = 0.1;
      PhysikX::World w(settings);
      auto d = ball(); d.mass = 2;
      auto id = w.create_body(d);
      w.apply_thrust(id, {3, 4, 999}, 10);
      w.step(0.1);
      near(w.state(id).velocity.x, 0.3, 1e-12, "thrust X");
      near(w.state(id).velocity.y, 0.4, 1e-12, "thrust Y");
      require(px_body_apply_thrust(w.native_handle(), id, {0,0,1}, 5) == PX_INVALID_ARGUMENT,
              "2D zero direction");
      require(px_body_set_mass(w.native_handle(), id, -1) == PX_INVALID_ARGUMENT, "negative mass");
      require(px_body_set_density(w.native_handle(), id, INFINITY) == PX_INVALID_ARGUMENT, "infinite density");
      require(px_body_set_friction(w.native_handle(), id, -1) == PX_INVALID_ARGUMENT, "negative friction");
      require(px_body_set_strength(w.native_handle(), id, NAN) == PX_INVALID_ARGUMENT, "NaN strength");
      near(w.mass_properties(id).mass, 2, 1e-12, "failed mass mutation");
      w.set_friction(id, 0.75);
      near(w.material(id).friction, 0.75, 1e-12, "friction readback");
    });
    test("runtime friction affects contact", [] {
      auto simulate = [](double friction) {
        auto settings = zero(2); settings.gravity = {0,-9.81,0};
        PhysikX::World w(settings);
        auto d = px_body_defaults(); d.shape = PX_BOX; d.is_static = 1;
        d.position = {0,-0.5,0}; d.half_extents = {10,0.5,1}; d.restitution = 0;
        auto floor = w.create_body(d); w.set_friction(floor, friction);
        d = px_body_defaults(); d.position = {0,0.49,0}; d.velocity = {5,-1,0}; d.restitution = 0;
        auto id = w.create_body(d); w.set_friction(id, friction);
        w.step(settings.fixed_dt);
        return w.state(id).velocity.x;
      };
      near(simulate(0), 5, 1e-12, "zero friction");
      require(simulate(1) < 4, "new friction ignored by solver");
    });
    test("elasticity and strength control links", [] {
      auto settings = zero(); settings.fixed_dt = 0.1;
      PhysikX::World w(settings);
      auto d = ball(); d.is_static = 1;
      auto anchor = w.create_body(d), id = w.create_body(ball({1,0,0}));
      auto link = w.distance({anchor,id,1,0});
      w.set_elasticity(link, 10);
      near(w.distance_properties(link).compliance, 0.1, 1e-12, "stiffness conversion");
      w.force(id, {10,0,0}); w.step(0.1);
      near(w.state(id).position.x, 1 + 0.1 / 1.1, 1e-10, "elastic extension");
      near(w.distance_properties(link).force, 10 * (0.1 / 1.1), 1e-10, "reaction force");
      w.set_distance_strength(link, 0.1);
      w.force(id, {10,0,0}); w.step(0.1);
      require(w.stats().constraints == 0, "link did not break");
      auto events = w.break_events();
      require(events.size() == 1 && events[0].kind == PX_DISTANCE_BROKEN && events[0].source == link,
              "link break event");
      require(events[0].count == 0 && events[0].force > 0.1, "link event load");
    });
    test("fracture preserves volume mass momentum and center", [] {
      for (uint32_t dim : {2u,3u}) for (uint32_t shape : {0u,1u}) {
        PhysikX::World w(zero(dim));
        auto d = ball({3,4,0}); d.shape = shape; d.mass = 16;
        d.velocity = {2,-1,0}; d.half_extents = {2,3,4}; d.radius = 2; d.user_data = 99;
        auto id = w.create_body(d), other = w.create_body(ball({20,0,0}));
        w.distance({id,other,10,0});
        auto before = w.mass_properties(id);
        auto event = w.fracture(id);
        require(event.count == (1u << dim) && event.source == id, "fragment count");
        double mass=0, v=0, momentum=0, center=0;
        for (uint32_t i=0; i<event.count; ++i) {
          auto child=event.fragments[i]; auto p=w.mass_properties(child); auto state=w.state(child);
          mass+=p.mass; v+=p.volume; momentum+=p.mass*state.velocity.x; center+=p.mass*state.position.x;
          require(state.user_data == 99, "fragment tag");
          near(w.material(child).breaking_force, 0, 1e-12, "cascade prevention");
        }
        near(mass,before.mass,1e-10,"mass conservation");
        near(v,before.volume,1e-10,"volume conservation");
        near(momentum,32,1e-10,"momentum conservation");
        near(center/mass,3,1e-10,"center conservation");
        require(w.stats().constraints == 0, "fracture orphaned link");
        px_body_state out;
        require(px_body_get_state(w.native_handle(),id,&out) == PX_NOT_FOUND,"parent remains alive");
      }
    });
    test("automatic body strength and event lifetime", [] {
      PhysikX::World w(zero());
      auto id = w.create_body(ball()); w.set_strength(id, 5); w.force(id,{10,0,0});
      w.step(1.0/120);
      auto events=w.break_events();
      require(events.size()==1 && events[0].source==id && events[0].kind==PX_BODY_FRACTURED,
              "force fracture event");
      near(events[0].force,10,1e-12,"fracture load");
      require(w.stats().bodies==8,"automatic fragment count");
      w.step(1.0/240); require(w.break_events().size()==1,"event lost without substep");
      w.step(1.0/240); require(w.break_events().empty(),"event not cleared next step");
      require(w.stats().bodies==8,"recursive fracture");
      id=w.create_body(ball()); w.set_strength(id, 5); w.impulse(id,{1,0,0}); w.step(1.0/120);
      require(!w.break_events().empty(),"impulse did not fracture");
    });
    test("collision load triggers fracture", [] {
      PhysikX::World w(zero(2));
      auto d=px_body_defaults(); d.shape=PX_BOX; d.is_static=1; d.half_extents={5,0.5,1};
      w.create_body(d);
      d=px_body_defaults(); d.position={0,0.9,0}; d.velocity={0,-10,0};
      auto id=w.create_body(d); w.set_strength(id,1); w.step(1.0/120);
      require(w.break_events().size()==1 && w.break_events()[0].source==id,"impact fracture");
    });
    test("fracture rejects invalid request without mutation", [] {
      PhysikX::World w(zero());
      auto d=ball(); d.mass=1e-9; auto id=w.create_body(d);
      px_break_event event;
      require(px_body_fracture(w.native_handle(),id,&event)==PX_INVALID_ARGUMENT,"tiny fragment mass");
      require(w.stats().bodies==1 && w.break_events().empty(),"failed fracture mutated world");
      d=ball(); d.is_static=1; id=w.create_body(d);
      require(px_body_set_strength(w.native_handle(),id,1)==PX_INVALID_ARGUMENT,"static strength");
      require(px_body_fracture(w.native_handle(),id,&event)==PX_INVALID_ARGUMENT,"static fracture");
      id=w.create_body(ball());
      require(px_body_fracture(w.native_handle(),id,nullptr)==PX_INVALID_ARGUMENT,"null output");
      require(w.stats().bodies==3,"null output removed body");
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
