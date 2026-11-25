<!-- SPDX-FileCopyrightText: 2026 Yonghua Liao -->
<!-- SPDX-License-Identifier: Apache-2.0 -->

# Third-Party Notices

`occ-c-bridge` itself is licensed under the Apache License 2.0 (see `LICENSE`).
It uses the following third-party components. Verbatim copies of their license
texts are provided in `third_party/licenses/`.

| Component | Version | License | How it is used |
|---|---|---|---|
| Open CASCADE Technology (OCCT) | 7.7.0 | LGPL-2.1 with the OCCT exception | Geometry kernel; linked into `occ_c_bridge` (static or dynamic) |
| oneAPI Threading Building Blocks (oneTBB) | 2021.5 | Apache-2.0 | Parallel runtime used by OCCT |
| FreeType | 2.5.5 | FreeType License (FTL) | Font rendering used by OCCT |
| doctest | 2.5.0 | MIT | Unit-test framework, vendored in `tests/third_party/` |

Qt is not a dependency of `occ-c-bridge`.

## Open CASCADE Technology (OCCT) 7.7.0

OCCT is licensed under the GNU Lesser General Public License version 2.1
with the Open CASCADE exception.

**This software is based in part on the work of Open CASCADE Technology.**

License texts:

- [GNU LGPL 2.1](third_party/licenses/LGPL-2.1.txt)
- [Open CASCADE exception](third_party/licenses/OCCT_LGPL_EXCEPTION.txt)

The OCCT exception permits distribution of object code that incorporates
material from OCCT header files under terms of your choice, provided that
prominent notice is given that the object code makes use of, or is based on,
facilities provided by Open CASCADE Technology.

## oneAPI Threading Building Blocks (oneTBB) 2021.5

Licensed under the Apache License, Version 2.0.

Copyright (c) Intel Corporation.

Full license text: [TBB-LICENSE.txt](third_party/licenses/TBB-LICENSE.txt).
Intel, the Intel logo, and oneAPI are trademarks of Intel Corporation or its
subsidiaries in the U.S. and/or other countries.

## FreeType 2.5.5

Licensed under the FreeType License (FTL).

Full license text: [FTL.txt](third_party/licenses/FTL.txt).

Portions of this software are copyright © 2006 The FreeType Project
(https://freetype.org). All rights reserved.

## doctest 2.5.0

Licensed under the MIT License.

Copyright (c) 2016-2023 Viktor Kirilov.

Full license text: [LICENSE.doctest.txt](tests/third_party/LICENSE.doctest.txt).

## Distribution note

The source repository does not contain OCCT, oneTBB, or FreeType source code.
When distributing prebuilt packages that embed or bundle these binaries (for
example the `win64-static` and `win64` variants), ship this file together with
`third_party/licenses/` and `LICENSES/`.
