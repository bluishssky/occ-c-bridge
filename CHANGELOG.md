<!-- SPDX-FileCopyrightText: 2026 Yonghua Liao -->
<!-- SPDX-License-Identifier: Apache-2.0 -->

# Changelog

All notable changes to this project are documented in this file.
The project follows [Semantic Versioning](https://semver.org/); within a major
version the C ABI is append-only.

## Unreleased

### Fixed

- Invalid or released handles no longer crash `ocb_explorer_*` / `ocb_gprops_*`:
  they now return `OCB_ERR_INVALID_HANDLE` (`ocb_explorer_next` is a no-op)
  instead of dereferencing a null table entry.
- `ocb_gprops_volume/linear/surface` compute properties before registering the
  handle, so a failed computation no longer leaves a dead handle in the table.
- Regression tests added for invalid / released / wrong-type handles and null
  out-pointers.

## 1.5.0 — Initial open-source release (2026-11-24)

> Versions 1.0 through 1.4 were internal iterations and were never published.
> This is the first public release.

### Added

- `AIS_Trihedron` API family (10 functions): origin / local trihedron objects —
  datum display mode, draw arrows, per-part colors, labels, text color, size,
  infinite state, line style, zoom persistence.
- `AIS_ViewCube` API family (10 functions): navigation cube — colors, size,
  axis labels and radius, transparency, decor, font, per-face side labels,
  triedron persistence.

### Compatibility

- Append-only ABI; compatible with internal v1.4.2.
- `ocb_version()` returns `"1.5.0"`.

### Packaging

- `win64-static` variant: statically linked OCCT, no `TK*.dll` dependency;
  requires `freetype.dll` for text rendering.
- `win64` variant: bundles OCCT / oneTBB / FreeType runtime DLLs.

### Quality

- 144 unit-test cases, 0 failures (new tests cover the 20 new APIs).
