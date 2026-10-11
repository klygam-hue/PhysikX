# Copyright (c) 2026 Mistrul.
# SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
# See the accompanying LICENSE for permitted use.

import math
import unittest
import PhysikX as P


class BindingTests(unittest.TestCase):
    def test_fall(self):
        with P.World(2) as w:
            b = w.create_body(position=(0, 5, 20))
            w.step(1 / 120)
            self.assertLess(w.state(b).position.y, 5)
            self.assertEqual(w.state(b).position.z, 0)

    def test_close(self):
        w = P.World()
        w.close()
        w.close()
        with self.assertRaises(RuntimeError):
            w.step(1 / 120)

    def test_invalid(self):
        with self.assertRaises(RuntimeError):
            P.World(7)
        with self.assertRaises(TypeError):
            P.World(wrong=1)
        with P.World() as w:
            with self.assertRaises(RuntimeError):
                w.create_body(mass=0)
            with self.assertRaises(RuntimeError):
                w.step(float("nan"))

    def test_raycast(self):
        with P.World(gravity=(0, 0, 0)) as w:
            b = w.create_body(position=(3, 0, 0), user_data=42)
            self.assertEqual(w.state(b).user_data, 42)
            self.assertEqual(w.raycast((0, 0, 0), (1, 0, 0)).body, b)
            self.assertAlmostEqual(w.raycast((0, 0, 0), (1, 0, 0)).distance, 2.5)
            self.assertIsNone(w.raycast((0, 3, 0), (1, 0, 0)))
            with self.assertRaises(RuntimeError):
                w.raycast((0, 0, 0), (0, 0, 0))

    def test_query(self):
        with P.World() as w:
            b = w.create_body()
            self.assertEqual(w.query_aabb((-1, -1, -1), (1, 1, 1)), [b])
            with self.assertRaises(ValueError):
                w.query_aabb((2, 0, 0), (1, 1, 1))

    def test_links_and_cleanup(self):
        with P.World(gravity=(0, 0, 0), linear_damping=0) as w:
            a = w.create_body(is_static=1, layer=0)
            b = w.create_body(position=(2, 0, 0), layer=0)
            link = w.distance(a, b, 1)
            w.step(1 / 120)
            self.assertAlmostEqual(w.state(b).position.x, 1)
            w.destroy_distance(link)
            w.distance(a, b, 1)
            w.destroy_body(a)
            self.assertEqual(w.stats().constraints, 0)

    def test_contacts(self):
        with P.World(gravity=(0, 0, 0)) as w:
            a = w.create_body(position=(-0.4, 0, 0))
            b = w.create_body(position=(0.4, 0, 0))
            w.step(1 / 120)
            self.assertEqual(len(w.contacts()), 1)
            self.assertEqual({w.contacts()[0].a, w.contacts()[0].b}, {a, b})
            w.destroy_body(a)
            self.assertEqual(w.contacts(), [])

    def test_force_impulse(self):
        with P.World(gravity=(0, 0, 0), linear_damping=0) as w:
            b = w.create_body(mass=2)
            w.apply_impulse(b, (2, 0, 0))
            self.assertAlmostEqual(w.state(b).velocity.x, 1)
            w.apply_force(b, (2, 0, 0))
            w.step(1 / 120)
            self.assertAlmostEqual(w.state(b).velocity.x, 1 + 1 / 120)
            w.set_position(b, (7, 8, 9))
            w.set_velocity(b, (0, 0, 0))
            w.set_gravity((0, 0, 0))
            self.assertEqual(tuple(w.state(b).position), (7, 8, 9))

    def test_long_run(self):
        with P.World() as w:
            w.create_body(
                shape=P.BOX,
                is_static=1,
                position=(0, -0.5, 0),
                half_extents=(10, 0.5, 10),
            )
            b = w.create_body(position=(0, 8, 0), restitution=0)
            for _ in range(10000):
                w.step(1 / 120)
            self.assertTrue(all(math.isfinite(x) for x in w.state(b).position))
            self.assertAlmostEqual(w.state(b).position.y, 0.5, delta=0.001)

    def test_layout(self):
        import ctypes

        self.assertEqual(ctypes.sizeof(P.WorldDesc), 64)
        self.assertEqual(ctypes.sizeof(P.BodyDesc), 128)
        self.assertEqual(ctypes.sizeof(P.BodyState), 56)
        self.assertEqual(ctypes.sizeof(P.MassProperties), 56)
        self.assertEqual(ctypes.sizeof(P.Material), 24)
        self.assertEqual(ctypes.sizeof(P.DistanceProperties), 48)
        self.assertEqual(ctypes.sizeof(P.BreakEvent), 88)

    def test_mass_density_weight_thrust(self):
        with P.World(gravity=(0,-10,0), fixed_dt=0.1, linear_damping=0) as w:
            b=w.create_body(shape=P.BOX, half_extents=(0.5,0.5,0.5), mass=2, layer=0)
            p=w.mass_properties(b)
            self.assertEqual((p.volume,p.density,p.mass,p.weight), (1,2,2,20))
            self.assertEqual(tuple(p.weight_force), (0,-20,0))
            w.set_density(b,3); self.assertEqual(w.mass_properties(b).mass,3)
            w.set_mass(b,2); w.set_gravity((0,0,0))
            w.apply_thrust(b,(10,0,0),10); w.step(0.1)
            self.assertAlmostEqual(w.state(b).velocity.x,0.5)
            w.set_friction(b,0.8); self.assertEqual(w.material(b).friction,0.8)
            with self.assertRaises(RuntimeError): w.set_mass(b,float('nan'))
            with self.assertRaises(RuntimeError): w.apply_thrust(b,(0,0,0),10)

    def test_elastic_link_break(self):
        with P.World(gravity=(0,0,0), linear_damping=0, fixed_dt=0.1) as w:
            a=w.create_body(is_static=1, layer=0)
            b=w.create_body(position=(1,0,0),layer=0)
            link=w.distance(a,b,1)
            w.set_elasticity(link,10); w.set_distance_strength(link,0.1)
            p=w.distance_properties(link)
            self.assertEqual((p.a,p.b,p.compliance,p.breaking_force),(a,b,0.1,0.1))
            w.apply_force(b,(10,0,0)); w.step(0.1)
            event=w.break_events()[0]
            self.assertEqual((event.kind,event.source,event.fragment_ids),(P.DISTANCE_BROKEN,link,[]))
            self.assertGreater(event.force,0.1)
            with self.assertRaises(RuntimeError): w.distance_properties(link)

    def test_manual_and_automatic_fracture(self):
        with P.World(2,gravity=(0,0,0),linear_damping=0) as w:
            b=w.create_body(shape=P.BOX,mass=8,velocity=(2,0,0),layer=0)
            event=w.fracture(b)
            self.assertEqual((event.kind,event.source,event.count),(P.BODY_FRACTURED,b,4))
            self.assertAlmostEqual(sum(w.mass_properties(c).mass for c in event.fragment_ids),8)
            self.assertTrue(all(w.state(c).velocity.x==2 for c in event.fragment_ids))
            self.assertEqual(w.break_events()[0].fragment_ids,event.fragment_ids)
            with self.assertRaises(RuntimeError): w.state(b)
            b=w.create_body(position=(10,0,0),layer=0)
            w.set_strength(b,5); self.assertEqual(w.material(b).breaking_force,5)
            w.apply_thrust(b,(1,0,0),10); w.step(1/120)
            events=w.break_events()
            self.assertEqual(len(events),1)
            self.assertEqual(events[0].source,b)
            self.assertTrue(all(w.material(c).breaking_force==0 for c in events[0].fragment_ids))

    def test_failed_fracture_keeps_body(self):
        with P.World() as w:
            b=w.create_body(mass=1e-9)
            with self.assertRaises(RuntimeError): w.fracture(b)
            self.assertEqual(w.stats().bodies,1)
            self.assertEqual(w.break_events(),[])
            self.assertEqual(w.mass_properties(b).mass,1e-9)


if __name__ == "__main__":
    unittest.main()
