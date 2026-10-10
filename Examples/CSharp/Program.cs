// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

using PhysikX;
using var world = new World(2);
var desc = World.BodyDefaults();
desc.Position = new Vec3(0, 5, 0);
var ball = world.CreateBody(desc);
world.Step(1.0 / 120);
if (world.State(ball).Position.Y >= 5)
  throw new Exception("Gravity test failed");
world.DestroyBody(ball);
try {
  world.State(ball);
  throw new Exception("Stale handle accepted");
} catch (InvalidOperationException) {
}
Console.WriteLine("C# binding passed");
