// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

#include <PhysikX/physikx.hpp>
#include <chrono>
#include <iostream>
int main() {
  auto desc = px_world_defaults(3);
  desc.gravity = {0, 0, 0};
  PhysikX::World w(desc);
  for (int i = 0; i < 10000; ++i) {
    auto b = px_body_defaults();
    b.position = {double(i) * 2, 0, 0};
    w.create_body(b);
  }
  for (int i = 0; i < 10; ++i)
    w.step(desc.fixed_dt);
  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < 120; ++i)
    w.step(desc.fixed_dt);
  double ms = std::chrono::duration<double, std::milli>(
                  std::chrono::steady_clock::now() - start)
                  .count() /
              120;
  std::cout << "scenario=sparse bodies=10000 ms_per_step=" << ms
            << " candidates=" << w.stats().candidates << "\n";
}
