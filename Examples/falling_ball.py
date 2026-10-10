# Copyright (c) 2026 Mistrul.
# SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
# See the accompanying LICENSE for permitted use.

import PhysikX

with PhysikX.World(dimension=3) as world:
    world.create_body(
        shape=PhysikX.BOX,
        is_static=1,
        position=(0, -0.5, 0),
        half_extents=(10, 0.5, 10),
    )
    ball = world.create_body(position=(0, 5, 0), restitution=0.6)
    for frame in range(180):
        world.step(1 / 60)
        if frame % 30 == 0:
            print(world.state(ball).position)
