# Copyright (c) 2026 Mistrul.
# SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
# See the accompanying LICENSE for permitted use.

"""PhysikX native physics library. Importing does not load native code until World()."""

import ctypes as C
import math
import os
from pathlib import Path
import sys

__version__ = "0.2.0b1"
SPHERE, BOX = 0, 1
DISTANCE_BROKEN, BODY_FRACTURED = 0, 1


class Vec3(C.Structure):
    _fields_ = [("x", C.c_double), ("y", C.c_double), ("z", C.c_double)]

    def __iter__(self):
        return iter((self.x, self.y, self.z))

    def __repr__(self):
        return f"Vec3({self.x:g}, {self.y:g}, {self.z:g})"


class WorldDesc(C.Structure):
    _fields_ = (
        [
            (k, C.c_uint32)
            for k in ("dimension", "iterations", "max_substeps", "reserved")
        ]
        + [(k, C.c_double) for k in ("fixed_dt", "linear_damping", "max_speed")]
        + [("gravity", Vec3)]
    )


class BodyDesc(C.Structure):
    _fields_ = (
        [(k, C.c_uint32) for k in ("shape", "is_static", "layer", "mask")]
        + [(k, Vec3) for k in ("position", "velocity", "half_extents")]
        + [(k, C.c_double) for k in ("radius", "mass", "restitution", "friction")]
        + [("user_data", C.c_uint64)]
    )


class BodyState(C.Structure):
    _fields_ = [("position", Vec3), ("velocity", Vec3), ("user_data", C.c_uint64)]


class DistanceDesc(C.Structure):
    _fields_ = [
        ("a", C.c_uint64),
        ("b", C.c_uint64),
        ("rest_length", C.c_double),
        ("compliance", C.c_double),
    ]


class Contact(C.Structure):
    _fields_ = [
        ("a", C.c_uint64),
        ("b", C.c_uint64),
        ("normal", Vec3),
        ("penetration", C.c_double),
    ]


class RayHit(C.Structure):
    _fields_ = [
        ("body", C.c_uint64),
        ("point", Vec3),
        ("normal", Vec3),
        ("distance", C.c_double),
    ]


class Stats(C.Structure):
    _fields_ = [
        (k, C.c_uint64)
        for k in ("bodies", "constraints", "candidates", "contacts", "steps")
    ] + [("dropped_time", C.c_double)]


class MassProperties(C.Structure):
    _fields_ = [(k, C.c_double) for k in ("volume", "density", "mass", "weight")] + [("weight_force", Vec3)]


class Material(C.Structure):
    _fields_ = [(k, C.c_double) for k in ("friction", "restitution", "breaking_force")]


class DistanceProperties(C.Structure):
    _fields_ = [("a", C.c_uint64), ("b", C.c_uint64)] + [
        (k, C.c_double) for k in ("rest_length", "compliance", "breaking_force", "force")
    ]


class BreakEvent(C.Structure):
    _fields_ = [("kind", C.c_uint32), ("count", C.c_uint32), ("source", C.c_uint64),
                ("fragments", C.c_uint64 * 8), ("force", C.c_double)]

    @property
    def fragment_ids(self):
        return list(self.fragments[:self.count])


_lib = None


def _load():
    global _lib
    if _lib is not None:
        return _lib
    name = (
        "PhysikX.dll"
        if sys.platform == "win32"
        else ("libPhysikX.dylib" if sys.platform == "darwin" else "libPhysikX.so")
    )
    explicit = os.environ.get("PHYSIKX_LIBRARY")
    candidates = [explicit] if explicit else [str(Path(__file__).parent / name), name]
    errors = []
    for candidate in candidates:
        try:
            lib = C.CDLL(candidate)
            break
        except OSError as exc:
            errors.append(str(exc))
    else:
        raise OSError(
            "Build PhysikX and set PHYSIKX_LIBRARY to its absolute path. "
            + "; ".join(errors)
        )
    signatures = {
        "px_last_error": (C.c_char_p, []),
        "px_version": (C.c_char_p, []),
        "px_world_defaults": (WorldDesc, [C.c_uint32]),
        "px_body_defaults": (BodyDesc, []),
        "px_world_create": (C.c_void_p, [C.POINTER(WorldDesc)]),
        "px_world_destroy": (None, [C.c_void_p]),
        "px_world_step": (C.c_int, [C.c_void_p, C.c_double]),
        "px_world_set_gravity": (C.c_int, [C.c_void_p, Vec3]),
        "px_world_get_stats": (C.c_int, [C.c_void_p, C.POINTER(Stats)]),
        "px_body_create": (
            C.c_int,
            [C.c_void_p, C.POINTER(BodyDesc), C.POINTER(C.c_uint64)],
        ),
        "px_body_destroy": (C.c_int, [C.c_void_p, C.c_uint64]),
        "px_body_get_state": (C.c_int, [C.c_void_p, C.c_uint64, C.POINTER(BodyState)]),
        "px_distance_create": (
            C.c_int,
            [C.c_void_p, C.POINTER(DistanceDesc), C.POINTER(C.c_uint64)],
        ),
        "px_distance_destroy": (C.c_int, [C.c_void_p, C.c_uint64]),
        "px_world_contacts": (C.c_size_t, [C.c_void_p, C.POINTER(Contact), C.c_size_t]),
        "px_world_query_aabb": (
            C.c_size_t,
            [C.c_void_p, Vec3, Vec3, C.POINTER(C.c_uint64), C.c_size_t],
        ),
        "px_world_raycast": (
            C.c_int,
            [C.c_void_p, Vec3, Vec3, C.c_double, C.c_uint32, C.POINTER(RayHit)],
        ),
    }
    for method in ("set_position", "set_velocity", "apply_force", "apply_impulse"):
        signatures["px_body_" + method] = (C.c_int, [C.c_void_p, C.c_uint64, Vec3])
    for name, data in (("px_body_get_mass_properties", MassProperties),
                       ("px_body_get_material", Material),
                       ("px_distance_get_properties", DistanceProperties),
                       ("px_body_fracture", BreakEvent)):
        signatures[name] = (C.c_int, [C.c_void_p, C.c_uint64, C.POINTER(data)])
    for name in ("px_body_set_mass", "px_body_set_density", "px_body_set_friction",
                 "px_body_set_strength", "px_distance_set_elasticity", "px_distance_set_strength"):
        signatures[name] = (C.c_int, [C.c_void_p, C.c_uint64, C.c_double])
    signatures["px_body_apply_thrust"] = (C.c_int, [C.c_void_p, C.c_uint64, Vec3, C.c_double])
    signatures["px_world_break_events"] = (C.c_size_t, [C.c_void_p, C.POINTER(BreakEvent), C.c_size_t])
    for key, (result, args) in signatures.items():
        getattr(lib, key).restype, getattr(lib, key).argtypes = result, args
    _lib = lib
    return lib


def _check(code):
    if code:
        raise RuntimeError(_load().px_last_error().decode("utf-8"))


def _vec(value):
    return value if isinstance(value, Vec3) else Vec3(*value)


class World:
    """Owns a native world; use a context manager or close explicitly. Single-threaded."""

    def __init__(self, dimension=3, **settings):
        self._lib = _load()
        self._handle = None
        desc = self._lib.px_world_defaults(dimension)
        allowed = {
            "iterations",
            "max_substeps",
            "fixed_dt",
            "linear_damping",
            "max_speed",
            "gravity",
        }
        for key, value in settings.items():
            if key not in allowed:
                raise TypeError(f"Unknown world setting: {key}")
            setattr(desc, key, _vec(value) if key == "gravity" else value)
        self._handle = self._lib.px_world_create(C.byref(desc))
        if not self._handle:
            raise RuntimeError(self._lib.px_last_error().decode())

    @property
    def _ptr(self):
        if not self._handle:
            raise RuntimeError("World is closed")
        return self._handle

    def close(self):
        if self._handle:
            self._lib.px_world_destroy(self._handle)
            self._handle = None

    def __del__(self):
        if getattr(self, "_handle", None):
            self.close()

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()

    def create_body(self, **settings):
        desc = self._lib.px_body_defaults()
        allowed = {key for key, _ in desc._fields_}
        for key, value in settings.items():
            if key not in allowed:
                raise TypeError(f"Unknown body setting: {key}")
            setattr(
                desc,
                key,
                (
                    _vec(value)
                    if key in {"position", "velocity", "half_extents"}
                    else value
                ),
            )
        result = C.c_uint64()
        _check(self._lib.px_body_create(self._ptr, C.byref(desc), C.byref(result)))
        return result.value

    def state(self, body):
        result = BodyState()
        _check(self._lib.px_body_get_state(self._ptr, body, C.byref(result)))
        return result

    def step(self, elapsed):
        _check(self._lib.px_world_step(self._ptr, elapsed))

    def set_gravity(self, value):
        _check(self._lib.px_world_set_gravity(self._ptr, _vec(value)))

    def destroy_body(self, body):
        _check(self._lib.px_body_destroy(self._ptr, body))

    def set_position(self, body, value):
        _check(self._lib.px_body_set_position(self._ptr, body, _vec(value)))

    def set_velocity(self, body, value):
        _check(self._lib.px_body_set_velocity(self._ptr, body, _vec(value)))

    def apply_force(self, body, value):
        _check(self._lib.px_body_apply_force(self._ptr, body, _vec(value)))

    def apply_impulse(self, body, value):
        _check(self._lib.px_body_apply_impulse(self._ptr, body, _vec(value)))

    def distance(self, a, b, rest_length, compliance=0):
        desc = DistanceDesc(a, b, rest_length, compliance)
        result = C.c_uint64()
        _check(self._lib.px_distance_create(self._ptr, C.byref(desc), C.byref(result)))
        return result.value

    def destroy_distance(self, constraint):
        _check(self._lib.px_distance_destroy(self._ptr, constraint))

    def stats(self):
        result = Stats()
        _check(self._lib.px_world_get_stats(self._ptr, C.byref(result)))
        return result

    def contacts(self):
        count = self._lib.px_world_contacts(self._ptr, None, 0)
        result = (Contact * count)()
        self._lib.px_world_contacts(self._ptr, result, count)
        return list(result)

    def query_aabb(self, minimum, maximum):
        lo, hi = _vec(minimum), _vec(maximum)
        if any(a > b for a, b in zip(lo, hi)) or not all(
            math.isfinite(x) and abs(x) <= 1e9 for x in (*lo, *hi)
        ):
            raise ValueError("Invalid AABB")
        count = self._lib.px_world_query_aabb(self._ptr, lo, hi, None, 0)
        result = (C.c_uint64 * count)()
        self._lib.px_world_query_aabb(self._ptr, lo, hi, result, count)
        return list(result)

    def raycast(self, origin, direction, max_distance=1000, mask=0xFFFFFFFF):
        result = RayHit()
        code = self._lib.px_world_raycast(
            self._ptr,
            _vec(origin),
            _vec(direction),
            max_distance,
            mask,
            C.byref(result),
        )
        if code == 2:
            return None
        _check(code)
        return result

    def mass_properties(self, body):
        result = MassProperties()
        _check(self._lib.px_body_get_mass_properties(self._ptr, body, C.byref(result)))
        return result

    def set_mass(self, body, mass):
        _check(self._lib.px_body_set_mass(self._ptr, body, mass))

    def set_density(self, body, density):
        _check(self._lib.px_body_set_density(self._ptr, body, density))

    def apply_thrust(self, body, direction, force):
        _check(self._lib.px_body_apply_thrust(self._ptr, body, _vec(direction), force))

    def material(self, body):
        result = Material()
        _check(self._lib.px_body_get_material(self._ptr, body, C.byref(result)))
        return result

    def set_friction(self, body, friction):
        _check(self._lib.px_body_set_friction(self._ptr, body, friction))

    def set_strength(self, body, force):
        _check(self._lib.px_body_set_strength(self._ptr, body, force))

    def distance_properties(self, link):
        result = DistanceProperties()
        _check(self._lib.px_distance_get_properties(self._ptr, link, C.byref(result)))
        return result

    def set_elasticity(self, link, stiffness):
        _check(self._lib.px_distance_set_elasticity(self._ptr, link, stiffness))

    def set_distance_strength(self, link, force):
        _check(self._lib.px_distance_set_strength(self._ptr, link, force))

    def fracture(self, body):
        result = BreakEvent()
        _check(self._lib.px_body_fracture(self._ptr, body, C.byref(result)))
        return result

    def break_events(self):
        count = self._lib.px_world_break_events(self._ptr, None, 0)
        result = (BreakEvent * count)()
        self._lib.px_world_break_events(self._ptr, result, count)
        return list(result)
