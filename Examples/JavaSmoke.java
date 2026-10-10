// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

import physikx.World;
public final class JavaSmoke {
  public static void main(String[] args) {
    try (World w = new World(2)) {
      long b = w.sphere(0, 5, 7, .5, 1, false);
      w.step(1.0 / 120);
      if (w.state(b)[1] >= 5 || w.state(b)[2] != 0)
        throw new AssertionError("Gravity/2D test failed");
      w.applyImpulse(b, 2, 0, 0);
      if (w.state(b)[3] <= 0)
        throw new AssertionError("Impulse failed");
      w.destroyBody(b);
      try {
        w.state(b);
        throw new AssertionError("Stale handle accepted");
      } catch (IllegalStateException expected) {
      }
    }
    System.out.println("Java JNI binding passed");
  }
}
