// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

fn main() {
    println!("cargo:rerun-if-env-changed=PHYSIKX_LIB_DIR");
    if let Ok(path) = std::env::var("PHYSIKX_LIB_DIR") {
        println!("cargo:rustc-link-search=native={path}");
    }
    println!("cargo:rustc-link-lib=dylib=PhysikX");
}
