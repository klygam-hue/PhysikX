// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

//! PhysikX owning wrapper. A World deliberately cannot be shared across threads.
use std::{ffi::CStr, marker::PhantomData, os::raw::c_char, ptr::NonNull, rc::Rc};
#[repr(C)]
#[derive(Clone, Copy, Default, Debug)]
pub struct Vec3 {
    pub x: f64,
    pub y: f64,
    pub z: f64,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct WorldDesc {
    pub dimension: u32,
    pub iterations: u32,
    pub max_substeps: u32,
    pub reserved: u32,
    pub fixed_dt: f64,
    pub linear_damping: f64,
    pub max_speed: f64,
    pub gravity: Vec3,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct BodyDesc {
    pub shape: u32,
    pub is_static: u32,
    pub layer: u32,
    pub mask: u32,
    pub position: Vec3,
    pub velocity: Vec3,
    pub half_extents: Vec3,
    pub radius: f64,
    pub mass: f64,
    pub restitution: f64,
    pub friction: f64,
    pub user_data: u64,
}
#[repr(C)]
#[derive(Clone, Copy, Default, Debug)]
pub struct BodyState {
    pub position: Vec3,
    pub velocity: Vec3,
    pub user_data: u64,
}
#[repr(C)]
pub struct DistanceDesc {
    pub a: u64,
    pub b: u64,
    pub rest_length: f64,
    pub compliance: f64,
}
#[repr(C)]
#[derive(Default, Debug)]
pub struct Stats {
    pub bodies: u64,
    pub constraints: u64,
    pub candidates: u64,
    pub contacts: u64,
    pub steps: u64,
    pub dropped_time: f64,
}
#[repr(C)]
struct NativeWorld {
    _private: [u8; 0],
}
extern "C" {
    fn px_last_error() -> *const c_char;
    fn px_world_defaults(dimension: u32) -> WorldDesc;
    fn px_body_defaults() -> BodyDesc;
    fn px_world_create(desc: *const WorldDesc) -> *mut NativeWorld;
    fn px_world_destroy(world: *mut NativeWorld);
    fn px_world_step(world: *mut NativeWorld, elapsed: f64) -> i32;
    fn px_world_set_gravity(world: *mut NativeWorld, value: Vec3) -> i32;
    fn px_world_get_stats(world: *const NativeWorld, result: *mut Stats) -> i32;
    fn px_body_create(world: *mut NativeWorld, desc: *const BodyDesc, id: *mut u64) -> i32;
    fn px_body_destroy(world: *mut NativeWorld, id: u64) -> i32;
    fn px_body_get_state(world: *const NativeWorld, id: u64, state: *mut BodyState) -> i32;
    fn px_body_set_position(world: *mut NativeWorld, id: u64, value: Vec3) -> i32;
    fn px_body_set_velocity(world: *mut NativeWorld, id: u64, value: Vec3) -> i32;
    fn px_body_apply_force(world: *mut NativeWorld, id: u64, value: Vec3) -> i32;
    fn px_body_apply_impulse(world: *mut NativeWorld, id: u64, value: Vec3) -> i32;
    fn px_distance_create(world: *mut NativeWorld, desc: *const DistanceDesc, id: *mut u64) -> i32;
    fn px_distance_destroy(world: *mut NativeWorld, id: u64) -> i32;
}
fn error() -> String {
    unsafe {
        CStr::from_ptr(px_last_error())
            .to_string_lossy()
            .into_owned()
    }
}
fn check(status: i32) -> Result<(), String> {
    if status == 0 {
        Ok(())
    } else {
        Err(error())
    }
}
impl WorldDesc {
    pub fn defaults(dimension: u32) -> Self {
        unsafe { px_world_defaults(dimension) }
    }
}
impl Default for BodyDesc {
    fn default() -> Self {
        unsafe { px_body_defaults() }
    }
}
pub struct World {
    handle: NonNull<NativeWorld>,
    _single_thread: PhantomData<Rc<()>>,
}
impl World {
    pub fn new(desc: WorldDesc) -> Result<Self, String> {
        let handle = NonNull::new(unsafe { px_world_create(&desc) }).ok_or_else(error)?;
        Ok(Self {
            handle,
            _single_thread: PhantomData,
        })
    }
    pub fn create_body(&mut self, desc: BodyDesc) -> Result<u64, String> {
        let mut id = 0;
        check(unsafe { px_body_create(self.handle.as_ptr(), &desc, &mut id) })?;
        Ok(id)
    }
    pub fn state(&self, id: u64) -> Result<BodyState, String> {
        let mut state = BodyState::default();
        check(unsafe { px_body_get_state(self.handle.as_ptr(), id, &mut state) })?;
        Ok(state)
    }
    pub fn step(&mut self, elapsed: f64) -> Result<(), String> {
        check(unsafe { px_world_step(self.handle.as_ptr(), elapsed) })
    }
    pub fn set_gravity(&mut self, v: Vec3) -> Result<(), String> {
        check(unsafe { px_world_set_gravity(self.handle.as_ptr(), v) })
    }
    pub fn set_position(&mut self, id: u64, v: Vec3) -> Result<(), String> {
        check(unsafe { px_body_set_position(self.handle.as_ptr(), id, v) })
    }
    pub fn set_velocity(&mut self, id: u64, v: Vec3) -> Result<(), String> {
        check(unsafe { px_body_set_velocity(self.handle.as_ptr(), id, v) })
    }
    pub fn apply_force(&mut self, id: u64, v: Vec3) -> Result<(), String> {
        check(unsafe { px_body_apply_force(self.handle.as_ptr(), id, v) })
    }
    pub fn apply_impulse(&mut self, id: u64, v: Vec3) -> Result<(), String> {
        check(unsafe { px_body_apply_impulse(self.handle.as_ptr(), id, v) })
    }
    pub fn destroy_body(&mut self, id: u64) -> Result<(), String> {
        check(unsafe { px_body_destroy(self.handle.as_ptr(), id) })
    }
    pub fn distance(&mut self, desc: DistanceDesc) -> Result<u64, String> {
        let mut id = 0;
        check(unsafe { px_distance_create(self.handle.as_ptr(), &desc, &mut id) })?;
        Ok(id)
    }
    pub fn destroy_distance(&mut self, id: u64) -> Result<(), String> {
        check(unsafe { px_distance_destroy(self.handle.as_ptr(), id) })
    }
    pub fn stats(&self) -> Result<Stats, String> {
        let mut out = Stats::default();
        check(unsafe { px_world_get_stats(self.handle.as_ptr(), &mut out) })?;
        Ok(out)
    }
}
impl Drop for World {
    fn drop(&mut self) {
        unsafe { px_world_destroy(self.handle.as_ptr()) }
    }
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn fall_and_lifetime() {
        let mut w = World::new(WorldDesc::defaults(2)).unwrap();
        let id = w
            .create_body(BodyDesc {
                position: Vec3 {
                    x: 0.,
                    y: 3.,
                    z: 7.,
                },
                ..BodyDesc::default()
            })
            .unwrap();
        w.step(1. / 120.).unwrap();
        let s = w.state(id).unwrap();
        assert!(s.position.y < 3.);
        assert_eq!(s.position.z, 0.);
        w.destroy_body(id).unwrap();
        assert!(w.state(id).is_err());
    }
    #[test]
    fn invalid_dimension() {
        assert!(World::new(WorldDesc::defaults(4)).is_err());
    }
}
