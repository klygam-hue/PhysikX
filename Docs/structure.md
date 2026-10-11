# Repository structure

| Path | Responsibility |
| --- | --- |
| `Code/include/PhysikX/physikx.h` | exported C ABI and descriptor layouts |
| `Code/include/PhysikX/physikx.hpp` | C++ RAII convenience wrapper |
| `Code/src/physikx.cpp` | world storage, integration, collision and constraints |
| `Bindings/Python/` | ctypes package imported as PhysikX |
| `Bindings/CSharp/` | .NET 8 P/Invoke wrapper |
| `Bindings/Rust/` | safe ownership wrapper, native link settings and tests |
| `Bindings/Java/` | AutoCloseable Java world and JNI bridge |
| `Examples/` | C++, Python, desktop 2D, C# and Java usage |
| `Tests/` | native regression suite, pure C and Python ABI checks |
| `Benchmarks/` | sparse 10,000-body CPU benchmark |
| `Docs/licensing.md` | usage restrictions, license history and publication guidance |
| `Docs/` | API contracts, architecture, roadmap, bindings and validation |
| `tools/build.py` | compiler-only fallback build and tests |
| `cmake/` | installed find_package configuration |
| `.github/workflows/` | platform builds and binding CI |
| `CMakeLists.txt` | portable build, testing and installation |
| `AGENTS.md` | contributor rules and ABI/validation requirements |
| `README.md` | overview plus the final command guide: six language references, settings, complete examples, loader setup, query results and common mistakes |
| `CHANGELOG.md`, `LICENSE` | version history and proprietary evaluation terms |

Generated build directories, binaries, caches and development-tool downloads
are excluded from source control. Release packages may include platform-specific
native binaries under an explicitly named directory.
