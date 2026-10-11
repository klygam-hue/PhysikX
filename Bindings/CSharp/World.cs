// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

using System;
using System.Runtime.InteropServices;
namespace PhysikX;
[StructLayout(LayoutKind.Sequential)]
public struct Vec3 {
  public double X, Y, Z;
  public Vec3(double x, double y, double z = 0) {
    X = x;
    Y = y;
    Z = z;
  }
}
[StructLayout(LayoutKind.Sequential)]
public struct WorldDesc {
  public uint Dimension, Iterations, MaxSubsteps, Reserved;
  public double FixedDt, LinearDamping, MaxSpeed;
  public Vec3 Gravity;
}
[StructLayout(LayoutKind.Sequential)]
public struct BodyDesc {
  public uint Shape, IsStatic, Layer, Mask;
  public Vec3 Position, Velocity, HalfExtents;
  public double Radius, Mass, Restitution, Friction;
  public ulong UserData;
}
[StructLayout(LayoutKind.Sequential)]
public struct BodyState {
  public Vec3 Position, Velocity;
  public ulong UserData;
}
[StructLayout(LayoutKind.Sequential)]
public struct DistanceDesc {
  public ulong A, B;
  public double RestLength, Compliance;
}
[StructLayout(LayoutKind.Sequential)]
public struct Stats {
  public ulong Bodies, Constraints, Candidates, Contacts, Steps;
  public double DroppedTime;
}
[StructLayout(LayoutKind.Sequential)]
public struct MassProperties {
  public double Volume, Density, Mass, Weight;
  public Vec3 WeightForce;
}
[StructLayout(LayoutKind.Sequential)]
public struct Material {
  public double Friction, Restitution, BreakingForce;
}
[StructLayout(LayoutKind.Sequential)]
public struct DistanceProperties {
  public ulong A, B;
  public double RestLength, Compliance, BreakingForce, Force;
}
public enum BreakKind : uint { DistanceBroken = 0, BodyFractured = 1 }
[StructLayout(LayoutKind.Sequential)]
public struct BreakEvent {
  public BreakKind Kind;
  public uint Count;
  public ulong Source;
  private ulong f0, f1, f2, f3, f4, f5, f6, f7;
  public double Force;
  public ulong[] Fragments {
    get {
      var all = new[] { f0, f1, f2, f3, f4, f5, f6, f7 };
      Array.Resize(ref all, (int)Math.Min(Count, 8u));
      return all;
    }
  }
}
internal static class Native {
  private const string Lib = "PhysikX";
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern IntPtr px_last_error();
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern WorldDesc px_world_defaults(uint dimension);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern BodyDesc px_body_defaults();
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern IntPtr px_world_create(in WorldDesc desc);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern void px_world_destroy(IntPtr world);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_world_step(IntPtr world, double elapsed);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_world_set_gravity(IntPtr world, Vec3 v);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_world_get_stats(IntPtr world, out Stats result);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_create(IntPtr world, in BodyDesc desc,
                                            out ulong id);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_destroy(IntPtr world, ulong id);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_get_state(IntPtr world, ulong id,
                                               out BodyState state);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_set_position(IntPtr world, ulong id,
                                                  Vec3 v);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_set_velocity(IntPtr world, ulong id,
                                                  Vec3 v);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_apply_force(IntPtr world, ulong id,
                                                 Vec3 v);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_apply_impulse(IntPtr world, ulong id,
                                                   Vec3 v);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int
  px_distance_create(IntPtr world, in DistanceDesc desc, out ulong id);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_distance_destroy(IntPtr world, ulong id);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_get_mass_properties(IntPtr world, ulong id, out MassProperties value);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_set_mass(IntPtr world, ulong id, double value);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_set_density(IntPtr world, ulong id, double value);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_apply_thrust(IntPtr world, ulong id, Vec3 direction, double force);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_get_material(IntPtr world, ulong id, out Material value);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_set_friction(IntPtr world, ulong id, double value);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_set_strength(IntPtr world, ulong id, double value);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_distance_get_properties(IntPtr world, ulong id, out DistanceProperties value);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_distance_set_elasticity(IntPtr world, ulong id, double value);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_distance_set_strength(IntPtr world, ulong id, double value);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern int px_body_fracture(IntPtr world, ulong id, out BreakEvent value);
  [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
  internal static extern nuint px_world_break_events(IntPtr world, [Out] BreakEvent[]? output, nuint capacity);
}
/// <summary>A single-threaded native physics world. Always dispose
/// it.</summary>
public sealed class World : IDisposable {
  private IntPtr handle;
  private IntPtr Handle =>
      handle != IntPtr.Zero ? handle
                            : throw new ObjectDisposedException(nameof(World));
  private static void Check(int status) {
    if (status != 0)
      throw new InvalidOperationException(
          Marshal.PtrToStringUTF8(Native.px_last_error()));
  }
  public static WorldDesc
  Defaults(uint dimension = 3) => Native.px_world_defaults(dimension);
  public static BodyDesc BodyDefaults() => Native.px_body_defaults();
  public World(uint dimension = 3) : this(Defaults(dimension)) {}
  public World(WorldDesc desc) {
    handle = Native.px_world_create(in desc);
    if (handle == IntPtr.Zero)
      throw new InvalidOperationException(
          Marshal.PtrToStringUTF8(Native.px_last_error()));
  }
  public void Dispose() {
    if (handle != IntPtr.Zero) {
      Native.px_world_destroy(handle);
      handle = IntPtr.Zero;
    }
    GC.SuppressFinalize(this);
  }
  ~World() {
    if (handle != IntPtr.Zero)
      Native.px_world_destroy(handle);
  }
  public ulong CreateBody(BodyDesc desc) {
    Check(Native.px_body_create(Handle, in desc, out var id));
    return id;
  }
  public void DestroyBody(ulong id) => Check(Native.px_body_destroy(Handle,
                                                                    id));
  public BodyState State(ulong id) {
    Check(Native.px_body_get_state(Handle, id, out var state));
    return state;
  }
  public void Step(double elapsed) => Check(Native.px_world_step(Handle,
                                                                 elapsed));
  public void
  SetGravity(Vec3 value) => Check(Native.px_world_set_gravity(Handle, value));
  public void SetPosition(ulong id, Vec3 value) =>
      Check(Native.px_body_set_position(Handle, id, value));
  public void SetVelocity(ulong id, Vec3 value) =>
      Check(Native.px_body_set_velocity(Handle, id, value));
  public void ApplyForce(ulong id, Vec3 value) =>
      Check(Native.px_body_apply_force(Handle, id, value));
  public void ApplyImpulse(ulong id, Vec3 value) =>
      Check(Native.px_body_apply_impulse(Handle, id, value));
  public ulong Distance(ulong a, ulong b, double restLength,
                        double compliance = 0) {
    var desc = new DistanceDesc { A = a, B = b, RestLength = restLength,
                                  Compliance = compliance };
    Check(Native.px_distance_create(Handle, in desc, out var id));
    return id;
  }
  public void
  DestroyDistance(ulong id) => Check(Native.px_distance_destroy(Handle, id));
  public Stats Statistics() {
    Check(Native.px_world_get_stats(Handle, out var value));
    return value;
  }
  public MassProperties MassProperties(ulong id) {
    Check(Native.px_body_get_mass_properties(Handle, id, out var value));
    return value;
  }
  public void SetMass(ulong id, double value) => Check(Native.px_body_set_mass(Handle, id, value));
  public void SetDensity(ulong id, double value) => Check(Native.px_body_set_density(Handle, id, value));
  public void ApplyThrust(ulong id, Vec3 direction, double force) => Check(Native.px_body_apply_thrust(Handle, id, direction, force));
  public Material Material(ulong id) {
    Check(Native.px_body_get_material(Handle, id, out var value));
    return value;
  }
  public void SetFriction(ulong id, double value) => Check(Native.px_body_set_friction(Handle, id, value));
  public void SetStrength(ulong id, double value) => Check(Native.px_body_set_strength(Handle, id, value));
  public DistanceProperties DistanceProperties(ulong id) {
    Check(Native.px_distance_get_properties(Handle, id, out var value));
    return value;
  }
  public void SetElasticity(ulong id, double value) => Check(Native.px_distance_set_elasticity(Handle, id, value));
  public void SetDistanceStrength(ulong id, double value) => Check(Native.px_distance_set_strength(Handle, id, value));
  public BreakEvent Fracture(ulong id) {
    Check(Native.px_body_fracture(Handle, id, out var value));
    return value;
  }
  public BreakEvent[] BreakEvents() {
    nuint count = Native.px_world_break_events(Handle, null, 0);
    var result = new BreakEvent[checked((int)count)];
    Native.px_world_break_events(Handle, result, count);
    return result;
  }
}
