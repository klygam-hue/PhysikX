#!/usr/bin/env python3
# Copyright (c) 2026 Mistrul.
# SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
# See the accompanying LICENSE for permitted use.

"""Dependency-free Linux/macOS development build (CMake is preferred)."""

from pathlib import Path
import os
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
build = root / "build"
build.mkdir(exist_ok=True)
if sys.platform == "win32":
    raise SystemExit("Use CMake with Visual Studio on Windows.")
ext = "dylib" if sys.platform == "darwin" else "so"
lib = build / f"libPhysikX.{ext}"
flags = [
    "-std=c++17",
    "-O2",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Werror",
    "-I" + str(root / "Code/include"),
]
cxx = os.environ.get("CXX", "c++")
subprocess.run(
    [
        cxx,
        *flags,
        "-fPIC",
        "-shared",
        "-DPX_BUILD",
        str(root / "Code/src/physikx.cpp"),
        "-o",
        str(lib),
    ],
    check=True,
)
for name, source in [
    ("native", "Tests/native.cpp"),
    ("demo", "Examples/falling_ball.cpp"),
    ("benchmark", "Benchmarks/sparse.cpp"),
]:
    subprocess.run(
        [
            cxx,
            *flags,
            str(root / source),
            "-L" + str(build),
            "-lPhysikX",
            "-Wl,-rpath," + str(build),
            "-o",
            str(build / name),
        ],
        check=True,
    )
subprocess.run(
    [
        os.environ.get("CC", "cc"),
        "-std=c11",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-I" + str(root / "Code/include"),
        str(root / "Tests/c_smoke.c"),
        "-L" + str(build),
        "-lPhysikX",
        "-Wl,-rpath," + str(build),
        "-o",
        str(build / "c_test"),
    ],
    check=True,
)
for binary in ["native", "c_test"]:
    subprocess.run([str(build / binary)], check=True)
env = dict(
    os.environ, PHYSIKX_LIBRARY=str(lib), PYTHONPATH=str(root / "Bindings/Python")
)
subprocess.run(
    [sys.executable, str(root / "Tests/test_python.py"), "-v"], env=env, check=True
)
print("Build and regression tests completed.")
