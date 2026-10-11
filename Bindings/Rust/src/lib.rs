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
#[derive(Clone, Copy, Default, Debug)]
pub struct MassProperties {
    pub volume: f64,
    pub density: f64,
    pub mass: f64,
    pub weight: f64,
    pub weight_force: Vec3,
}
#[repr(C)]
#[derive(Clone, Copy, Default, Debug)]
pub struct Material {
    pub friction: f64,
    pub restitution: f64,
    pub breaking_force: f64,
}
#[repr(C)]
#[derive(Clone, Copy, Default, Debug)]
pub struct DistanceProperties {
    pub a: u64,
    pub b: u64,
    pub rest_length: f64,
    pub compliance: f64,
    pub breaking_force: f64,
    pub force: f64,
}
pub const DISTANCE_BROKEN: u32 = 0;
pub const BODY_FRACTURED: u32 = 1;
#[repr(C)]
#[derive(Clone, Copy, Default, Debug)]
pub struct BreakEvent {
    pub kind: u32,
    pub count: u32,
    pub source: u64,
    pub fragments: [u64; 8],
    pub force: f64,
}
impl BreakEvent {
    pub fn fragment_ids(&self) -> &[u64] {
        &self.fragments[..(self.count as usize).min(8)]
    }
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
    fn px_body_get_mass_properties(world: *const NativeWorld, id: u64, out: *mut MassProperties) -> i32;
    fn px_body_set_mass(world: *mut NativeWorld, id: u64, value: f64) -> i32;
    fn px_body_set_density(world: *mut NativeWorld, id: u64, value: f64) -> i32;
    fn px_body_apply_thrust(world: *mut NativeWorld, id: u64, direction: Vec3, force: f64) -> i32;
    fn px_body_get_material(world: *const NativeWorld, id: u64, out: *mut Material) -> i32;
    fn px_body_set_friction(world: *mut NativeWorld, id: u64, value: f64) -> i32;
    fn px_body_set_strength(world: *mut NativeWorld, id: u64, value: f64) -> i32;
    fn px_distance_get_properties(world: *const NativeWorld, id: u64, out: *mut DistanceProperties) -> i32;
    fn px_distance_set_elasticity(world: *mut NativeWorld, id: u64, value: f64) -> i32;
    fn px_distance_set_strength(world: *mut NativeWorld, id: u64, value: f64) -> i32;
    fn px_body_fracture(world: *mut NativeWorld, id: u64, out: *mut BreakEvent) -> i32;
    fn px_world_break_events(world: *const NativeWorld, out: *mut BreakEvent, capacity: usize) -> usize;
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
    pub fn mass_properties(&self, id: u64) -> Result<MassProperties, String> {
        let mut out = MassProperties::default();
        check(unsafe { px_body_get_mass_properties(self.handle.as_ptr(), id, &mut out) })?;
        Ok(out)
    }
    pub fn set_mass(&mut self, id: u64, value: f64) -> Result<(), String> {
        check(unsafe { px_body_set_mass(self.handle.as_ptr(), id, value) })
    }
    pub fn set_density(&mut self, id: u64, value: f64) -> Result<(), String> {
        check(unsafe { px_body_set_density(self.handle.as_ptr(), id, value) })
    }
    pub fn apply_thrust(&mut self, id: u64, direction: Vec3, force: f64) -> Result<(), String> {
        check(unsafe { px_body_apply_thrust(self.handle.as_ptr(), id, direction, force) })
    }
    pub fn material(&self, id: u64) -> Result<Material, String> {
        let mut out = Material::default();
        check(unsafe { px_body_get_material(self.handle.as_ptr(), id, &mut out) })?;
        Ok(out)
    }
    pub fn set_friction(&mut self, id: u64, value: f64) -> Result<(), String> {
        check(unsafe { px_body_set_friction(self.handle.as_ptr(), id, value) })
    }
    pub fn set_strength(&mut self, id: u64, value: f64) -> Result<(), String> {
        check(unsafe { px_body_set_strength(self.handle.as_ptr(), id, value) })
    }
    pub fn distance_properties(&self, id: u64) -> Result<DistanceProperties, String> {
        let mut out = DistanceProperties::default();
        check(unsafe { px_distance_get_properties(self.handle.as_ptr(), id, &mut out) })?;
        Ok(out)
    }
    pub fn set_elasticity(&mut self, id: u64, value: f64) -> Result<(), String> {
        check(unsafe { px_distance_set_elasticity(self.handle.as_ptr(), id, value) })
    }
    pub fn set_distance_strength(&mut self, id: u64, value: f64) -> Result<(), String> {
        check(unsafe { px_distance_set_strength(self.handle.as_ptr(), id, value) })
    }
    pub fn fracture(&mut self, id: u64) -> Result<BreakEvent, String> {
        let mut out = BreakEvent::default();
        check(unsafe { px_body_fracture(self.handle.as_ptr(), id, &mut out) })?;
        Ok(out)
    }
    pub fn break_events(&self) -> Vec<BreakEvent> {
        let count = unsafe { px_world_break_events(self.handle.as_ptr(), std::ptr::null_mut(), 0) };
        let mut out = vec![BreakEvent::default(); count];
        unsafe { px_world_break_events(self.handle.as_ptr(), out.as_mut_ptr(), count); }
        out
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
    #[test]
    fn properties_and_fracture_ffi() -> Result<(), String> {
        assert_eq!(std::mem::size_of::<BreakEvent>(),88);
        assert_eq!(std::mem::size_of::<MassProperties>(),56);
        let mut settings=WorldDesc::defaults(3);
        settings.gravity=Vec3::default(); settings.linear_damping=0.0; settings.fixed_dt=0.1;
        let mut w=World::new(settings)?;
        let id=w.create_body(BodyDesc{shape:1,mass:2.0,layer:0,..BodyDesc::default()})?;
        assert_eq!(w.mass_properties(id)?.volume,1.0);
        w.set_density(id,3.0)?; assert_eq!(w.mass_properties(id)?.mass,3.0);
        w.set_mass(id,2.0)?; w.set_friction(id,0.8)?;
        assert_eq!(w.material(id)?.friction,0.8);
        w.apply_thrust(id,Vec3{x:10.0,y:0.0,z:0.0},10.0)?; w.step(0.1)?;
        assert!((w.state(id)?.velocity.x-0.5).abs()<1e-10);
        let event=w.fracture(id)?;
        assert_eq!(event.kind,BODY_FRACTURED); assert_eq!(event.fragment_ids().len(),8);
        let mass: f64=event.fragment_ids().iter().map(|&c| w.mass_properties(c).unwrap().mass).sum();
        assert!((mass-2.0).abs()<1e-10);
        assert_eq!(w.break_events()[0].source,id); assert!(w.state(id).is_err());
        let b=w.create_body(BodyDesc{layer:0,position:Vec3{x:10.0,y:0.0,z:0.0},..BodyDesc::default()})?;
        w.set_strength(b,1.0)?; w.apply_thrust(b,Vec3{x:1.0,y:0.0,z:0.0},2.0)?; w.step(0.1)?;
        assert_eq!(w.break_events()[0].source,b);
        Ok(())
    }
    #[test]
    fn elastic_link_strength_ffi() -> Result<(), String> {
        let mut settings=WorldDesc::defaults(3);
        settings.gravity=Vec3::default(); settings.linear_damping=0.0; settings.fixed_dt=0.1;
        let mut w=World::new(settings)?;
        let a=w.create_body(BodyDesc{is_static:1,layer:0,..BodyDesc::default()})?;
        let b=w.create_body(BodyDesc{layer:0,position:Vec3{x:1.0,y:0.0,z:0.0},..BodyDesc::default()})?;
        let link=w.distance(DistanceDesc{a,b,rest_length:1.0,compliance:0.0})?;
        w.set_elasticity(link,10.0)?; w.set_distance_strength(link,0.1)?;
        assert_eq!(w.distance_properties(link)?.compliance,0.1);
        w.apply_force(b,Vec3{x:10.0,y:0.0,z:0.0})?; w.step(0.1)?;
        assert_eq!(w.break_events()[0].kind,DISTANCE_BROKEN);
        assert_eq!(w.break_events()[0].source,link); assert!(w.distance_properties(link).is_err());
        Ok(())
    }
}
