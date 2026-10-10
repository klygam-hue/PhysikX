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


if __name__ == "__main__":
    unittest.main()
