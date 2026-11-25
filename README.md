# occ-c-bridge

A pure C API over [OpenCASCADE Technology (OCCT)](https://dev.opencascade.org/),
designed for stable FFI and easy dynamic loading (`dlopen` / `LoadLibrary`).

`occ-c-bridge` isolates OCCT behind an `extern "C"` boundary:

- all exported symbols use the C ABI (`cdecl`) and the `ocb_` prefix;
- OCCT C++ types never cross the API boundary — objects are opaque
  `uint64_t` handles managed inside the library;
- value types (`OcbPnt`, `OcbVec`, `OcbTrsf`, `OcbBox`, ...) are plain POD
  structs independent of OCCT;
- no C++ exception escapes the boundary; every call returns an `OcbStatus`
  and `ocb_last_error()` provides details.

## Status

**v1.5.0**. The API covers the surface an assembly/CAD tool needs:

| Area | Examples |
|---|---|
| Core / lifecycle | `ocb_version`, `ocb_last_status`, `ocb_last_error`, `ocb_live_count` |
| Value geometry | `ocb_point_*`, `ocb_trsf_*` (translation/rotation/compose/invert/apply), `ocb_box_*` |
| BRep shapes | primitives, boolean ops, bbox, volume, distance, copy/move/location, explorers |
| Topology queries | solid/face/edge/wire counts and traversal, orientation, outer wire |
| Meshing | triangulation parameters and mesh iteration |
| STEP I/O | `ocb_stepreader_*` parameters, import/export |
| OCAF / XCAF | `ocb_app_*`, `ocb_doc_*`, `ocb_shapetool_*`, `ocb_label_*`, locations |
| Viewer / AIS | `ocb_viewer_*`, `ocb_ais_*`, interactive context, selection, lights |
| Wire/jumper geometry | arc/segment construction used by the consuming tool |

See [`docs/api-reference.md`](docs/api-reference.md) for the full list and
[`docs/api-test-coverage.md`](docs/api-test-coverage.md) for coverage.

## Layout

```
include/
  occ_c_types.h      POD types, handles, status enum
  occ_c_bridge.h     extern "C" API
  occ_c_guard.h      handle/RAII helpers for C++ consumers
src/
  handle_table.h     uint64_t -> OCCT object registry
  occ_c_bridge.cpp   core + boxes
  occ_c_shape.cpp    BRep / topology / meshing
  occ_c_io.cpp       STEP I/O
  occ_c_ocaf.cpp     OCAF / XCAF
  occ_c_viewer.cpp   viewer / AIS
tests/               doctest suite (test_*.cpp)
examples/minimal-consumer/   compiles against the installed package only
```

## Build

Requirements: CMake 3.20+, MSVC 2019/2022 (or any C++17 compiler),
OpenCASCADE 7.7.

### Dynamic OCCT (default)

The produced `occ_c_bridge.dll` depends on `TK*.dll` (plus TBB/FreeType in
the default OCCT build):

```powershell
cmake -S . -B build -DOCB_OCCT_ROOT=C:/OpenCASCADE-7.7.0-vc14-64/opencascade-7.7.0
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

### Static OCCT (no `TK*.dll`)

Link a statically built OCCT (`BUILD_LIBRARY_TYPE=Static`) to get a
self-contained DLL that depends only on system / VC-runtime DLLs:

```powershell
cmake -S . -B build-static -A x64 `
  -DOCB_OCC_STATIC=ON `
  -DOCB_OCCT_STATIC_ROOT=<static-occt-build-tree> `
  -DOCB_BUILD_TESTS=OFF
cmake --build build-static --config Release
```

Verify the result:

```powershell
dumpbin /DEPENDENTS build-static/Release/occ_c_bridge.dll   # no TK*.dll
```

`OCB_OCCT_STATIC_ROOT` must contain `inc/` and `win64/vc14/lib/`
(`lib/` on non-Windows).

## Installing

```bat
cmake --install build --config Release --prefix <install-dir>
```

The install tree contains `bin/`, `lib/`, `include/`, the CMake package
config, and `share/doc/occ_c_bridge/` with `LICENSE`, `NOTICE`,
`THIRD_PARTY_NOTICES.md`, and the third-party license texts. Maintainers'
prebuilt `win64` / `win64-static` archives include the same notice files.

## Quick start (C)

```c
#include "occ_c_bridge.h"

int main(void) {
    OcbPnt origin = ocb_point_xyz(0, 0, 0);

    OcbShape box = OCB_INVALID;
    if (ocb_shape_make_box(origin, 10, 20, 30, &box) != OCB_OK) {
        return 1;
    }

    OcbBox bounds;
    ocb_shape_bounding_box(box, &bounds);

    double volume = 0;
    ocb_shape_volume(box, &volume);   /* 6000 */

    ocb_shape_release(box);
    return 0;
}
```

## Consuming from another language

The C ABI is designed for FFI (cgo, ctypes, JNI, ...). See
[`docs/consumer-integration.md`](docs/consumer-integration.md) for the
load/version/handle/error conventions, and
[`examples/minimal-consumer`](examples/minimal-consumer) for a build that
uses only the installed package (`find_package(occ_c_bridge CONFIG)`).

## Testing / TDD

Tests use [doctest](https://github.com/doctest/doctest) (vendored single
header in `tests/third_party/doctest.h`) and are organized by area
(`tests/test_*.cpp`). Run:

```powershell
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

See [`docs/TDD.md`](docs/TDD.md) for the workflow and per-API test matrix.

## Design notes

- **Handles are IDs, not pointers.** `HandleTable` maps `uint64_t` to a
  `shared_ptr` holding the OCCT object. Invalid or released handles are
  detected (`OCB_ERR_INVALID_HANDLE`) instead of crashing.
- **Errors do not throw across the boundary.** OCCT exceptions are caught
  inside the implementation and reported through `OcbStatus`.
- **Threading.** OCCT is not thread-safe. Call all `ocb_*` functions from a
  single thread (normally the UI thread).
- **Versioning.** The `OcbTrsf` layout is fixed (row-major 3x4). Releases
  follow semantic versioning; the ABI is append-only within a major version.

## License

- `occ-c-bridge` itself: **Apache-2.0** (see `LICENSE`).
- Third-party components:
  - OCCT 7.7.0 — LGPL-2.1 with the OCCT exception;
  - oneTBB 2021.5 — Apache-2.0;
  - FreeType 2.5.5 — FreeType License (FTL);
  - doctest 2.5.0 — MIT (vendored under `tests/third_party/`).

  Full texts and attribution: `THIRD_PARTY_NOTICES.md` and
  `third_party/licenses/`. Prebuilt static packages embed OCCT/oneTBB object
  code and bundle `freetype.dll`; distribute these notices with them.

## Copyright

Copyright (c) 2026 Yonghua Liao
