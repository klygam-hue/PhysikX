// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

package physikx;
/**
 * Owns a native world. Use try-with-resources. All methods are
 * single-threaded.
 */
public final class World implements AutoCloseable {
  static { System.loadLibrary("PhysikXJNI"); }
  private long handle;
  public World(int dimension) { handle = create(dimension); }
  private long checked() {
    if (handle == 0)
      throw new IllegalStateException("World is closed");
    return handle;
  }
  @Override
  public void close() {
    if (handle != 0) {
      destroy(handle);
      handle = 0;
    }
  }
  public long sphere(double x, double y, double z, double radius, double mass,
                     boolean isStatic) {
    return body(checked(), 0, x, y, z, radius, radius, radius, mass, isStatic);
  }
  public long box(double x, double y, double z, double hx, double hy, double hz,
                  double mass, boolean isStatic) {
    return body(checked(), 1, x, y, z, hx, hy, hz, mass, isStatic);
  }
  public void step(double elapsed) { stepNative(checked(), elapsed); }
  /** Returns x,y,z,vx,vy,vz. */
  public double[] state(long body) { return stateNative(checked(), body); }
  public void setGravity(double x, double y, double z) {
    vector(checked(), 0, 0, x, y, z);
  }
  public void setPosition(long id, double x, double y, double z) {
    vector(checked(), id, 1, x, y, z);
  }
  public void setVelocity(long id, double x, double y, double z) {
    vector(checked(), id, 2, x, y, z);
  }
  public void applyForce(long id, double x, double y, double z) {
    vector(checked(), id, 3, x, y, z);
  }
  public void applyImpulse(long id, double x, double y, double z) {
    vector(checked(), id, 4, x, y, z);
  }
  public void destroyBody(long id) { removeBody(checked(), id); }
  public long distance(long a, long b, double length, double compliance) {
    return distanceNative(checked(), a, b, length, compliance);
  }
  public void destroyDistance(long id) { removeDistance(checked(), id); }
  private static native long create(int dimension);
  private static native void destroy(long world);
  private static native long body(long world, int shape, double x, double y,
                                  double z, double hx, double hy, double hz,
                                  double mass, boolean isStatic);
  private static native void stepNative(long world, double elapsed);
  private static native double[] stateNative(long world, long body);
  private static native void vector(long world, long body, int operation,
                                    double x, double y, double z);
  private static native void removeBody(long world, long body);
  private static native long distanceNative(long world, long a, long b,
                                            double length, double compliance);
  private static native void removeDistance(long world, long constraint);
}
