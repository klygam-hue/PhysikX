// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

use physikx::{BodyDesc, Vec3, World, WorldDesc};
fn main() -> Result<(), String> {
    let mut world = World::new(WorldDesc::defaults(3))?;
    let ball = world.create_body(BodyDesc {
        position: Vec3 {
            x: 0.,
            y: 5.,
            z: 0.,
        },
        ..BodyDesc::default()
    })?;
    for _ in 0..120 {
        world.step(1. / 120.)?;
    }
    println!("{:?}", world.state(ball)?.position);
    Ok(())
}
