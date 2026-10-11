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
    try (World w = new World(3)) {
      long b=w.box(0,0,0,0.5,0.5,0.5,2,false);
      if (w.massProperties(b).volume()!=1 || Math.abs(w.massProperties(b).weight()-19.62)>1e-10)
        throw new AssertionError("Mass properties failed");
      w.setDensity(b,3); if (w.massProperties(b).mass()!=3) throw new AssertionError("Density failed");
      w.setMass(b,2); w.setFriction(b,0.8);
      if (w.material(b).friction()!=0.8) throw new AssertionError("Friction failed");
      w.setGravity(0,0,0); w.applyThrust(b,10,0,0,10); w.step(1.0/120);
      if (w.state(b)[3]<=0) throw new AssertionError("Thrust failed");
      long a=w.sphere(0,0,0,0.1,1,true);
      w.setPosition(b,1,0,0); w.setVelocity(b,0,0,0);
      long link=w.distance(a,b,1,0);
      w.setElasticity(link,10); w.setDistanceStrength(link,0.0001);
      if (w.distanceProperties(link).compliance()!=0.1 || w.distanceProperties(link).a()!=a)
        throw new AssertionError("Elasticity failed");
      w.applyForce(b,10,0,0); w.step(1.0/120);
      World.BreakEvent[] events=w.breakEvents();
      if (events.length!=1 || events[0].source()!=link || events[0].kind()!=World.DISTANCE_BROKEN)
        throw new AssertionError("Link break event failed");
      World.BreakEvent event=w.fracture(b);
      if (event.fragments().length!=8) throw new AssertionError("Fracture failed");
      double mass=0;
      for (long child:event.fragments()) mass+=w.massProperties(child).mass();
      if (Math.abs(mass-2)>1e-10) throw new AssertionError("Mass conservation failed");
      b=w.sphere(20,0,0,0.5,1,false); w.setStrength(b,1); w.applyThrust(b,1,0,0,2); w.step(1.0/120);
      events=w.breakEvents();
      if (events.length!=1 || events[0].source()!=b || events[0].kind()!=World.BODY_FRACTURED)
        throw new AssertionError("Automatic fracture failed");
    }
    System.out.println("Java JNI mass, thrust, materials and fracture passed");
  }
}
