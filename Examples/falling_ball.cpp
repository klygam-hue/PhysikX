// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

#include <PhysikX/physikx.hpp>
#include <iostream>
int main() {
  PhysikX::World world;
  auto floor = px_body_defaults();
  floor.shape = PX_BOX;
  floor.is_static = 1;
  floor.position = {0, -.5, 0};
  floor.half_extents = {10, .5, 10};
  world.create_body(floor);
  auto ball = px_body_defaults();
  ball.position = {0, 5, 0};
  auto id = world.create_body(ball);
  for (int frame = 0; frame < 180; ++frame) {
    world.step(1. / 60);
    if (frame % 30 == 0)
      std::cout << "time=" << frame / 60. << " y=" << world.state(id).position.y
                << '\n';
  }
}
