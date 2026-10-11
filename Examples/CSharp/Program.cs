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
if (System.Runtime.InteropServices.Marshal.SizeOf<BreakEvent>() != 88 ||
    System.Runtime.InteropServices.Marshal.SizeOf<MassProperties>() != 56)
  throw new Exception("Extension ABI layout mismatch");
var settings = World.Defaults(3);
settings.Gravity = new Vec3(0, -10, 0);
settings.LinearDamping = 0;
settings.FixedDt = 0.1;
using var physics = new World(settings);
desc = World.BodyDefaults(); desc.Shape = 1; desc.Mass = 2; desc.Layer = 0;
var body = physics.CreateBody(desc);
var props = physics.MassProperties(body);
if (props.Volume != 1 || props.Weight != 20 || props.WeightForce.Y != -20)
  throw new Exception("Mass/weight properties failed");
physics.SetDensity(body, 3); physics.SetMass(body, 2);
physics.SetFriction(body, 0.8);
if (physics.Material(body).Friction != 0.8) throw new Exception("Friction failed");
physics.SetGravity(new Vec3(0,0,0));
physics.ApplyThrust(body, new Vec3(10,0,0),10); physics.Step(0.1);
if (Math.Abs(physics.State(body).Velocity.X-0.5)>1e-10) throw new Exception("Thrust failed");
desc = World.BodyDefaults(); desc.Layer=0; desc.IsStatic=1;
var anchor=physics.CreateBody(desc);
physics.SetPosition(body,new Vec3(1,0,0)); physics.SetVelocity(body,new Vec3(0,0,0));
var link=physics.Distance(anchor,body,1);
physics.SetElasticity(link,10); physics.SetDistanceStrength(link,0.1);
if (physics.DistanceProperties(link).Compliance!=0.1) throw new Exception("Elasticity failed");
physics.ApplyForce(body,new Vec3(10,0,0)); physics.Step(0.1);
if (physics.BreakEvents().Length!=1 || physics.BreakEvents()[0].Source!=link ||
    physics.BreakEvents()[0].Kind!=BreakKind.DistanceBroken)
  throw new Exception("Link break event failed");
var fracture=physics.Fracture(body);
if (fracture.Fragments.Length!=8) throw new Exception("Fracture failed");
double mass=0;
foreach (var child in fracture.Fragments) mass+=physics.MassProperties(child).Mass;
if (Math.Abs(mass-2)>1e-10) throw new Exception("Mass conservation failed");
desc=World.BodyDefaults(); desc.Layer=0; desc.Position=new Vec3(20,0,0);
body=physics.CreateBody(desc); physics.SetStrength(body,1);
physics.ApplyThrust(body,new Vec3(1,0,0),2); physics.Step(0.1);
if (physics.BreakEvents().Length!=1 || physics.BreakEvents()[0].Source!=body ||
    physics.BreakEvents()[0].Kind!=BreakKind.BodyFractured)
  throw new Exception("Automatic fracture failed");
Console.WriteLine("C# mass, thrust, materials and fracture passed");
