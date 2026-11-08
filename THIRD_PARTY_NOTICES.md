# Source and dependency notices

Updated: 2026-11-08. This fork publishes source code, tests, build scripts and
documentation. It does not offer its own executables, installers, bundled runtime
libraries or vendor SDKs. Local build outputs and `.deps/` are ignored by Git.

This is an inventory of the application components reviewed for this fork's
changes, not a replacement for individual file notices or a complete audit of
every inherited file, firmware dependency, or optional driver.

| Component | Location / source | Applicable notice |
| --- | --- | --- |
| Inherited CANgaroo | Existing repository and source headers; upstream links retained in README | Root LICENSE contains GPLv2; many application headers permit GPLv2 or later. Preserve file-specific notices and original authorship. |
| New fork-authored application/test/build code | Files with new SPDX headers | GPL-2.0-or-later. This permits choosing GPLv3 for compatible combined configurations; it does not change upstream component terms. |
| Qt VectorCAN backend, v6.12.0 | `src/driver/VectorDriver/qtvectorcan/`; https://github.com/qt/qtserialbus/tree/v6.12.0/src/plugins/canbus/vectorcan | Copyright (C) 2017 Denis Shienkov. Original SPDX expression: `LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only`. Preserve all alternatives. Use the GPLv3 alternative for the combined open-source Qt Charts configuration. See the local backend README for modifications. |
| Candle Windows API | `src/driver/CandleApiDriver/api/` | Existing headers specify LGPLv3-or-later and identify Hubert Denkmair / Schildkroet where applicable. Preserve its existing LICENSE and all notices. |
| Qt modules, including Charts | External build/runtime dependencies, not bundled here | Qt Charts is commercial or GPLv3. Other modules and their third-party dependencies have their own terms. See https://doc.qt.io/qt-6/qtcharts-index.html and https://doc.qt.io/qt-6/licensing.html. |
| pybind11, pinned 3.1.0 | Installed locally by `scripts/windows/install-deps.ps1`, not vendored | BSD-3-Clause; retain the exact installed package's LICENSE if its source/header files are redistributed. https://github.com/pybind/pybind11 |
| Python 3.9 runtime/development files | External Qt/MinGW installation, not bundled here | PSF licensing plus notices for incorporated software. https://docs.python.org/3.9/license.html |
| MinGW / GCC runtimes | External toolchain, not bundled here | Component-specific licenses and applicable runtime exceptions; not one project-wide license. |
| Vector XL and optional adapter SDKs | Installed separately by users; no redistribution permission asserted | Vendor terms apply. Availability of a Qt backend does not grant rights to proprietary SDKs/DLLs or establish GPL compatibility of every configuration. |

License texts are provided in `LICENSES/`. Root `LICENSE` and the Candle API's
existing license file remain unchanged. The GPL text itself is shared by the
SPDX `-only` and `-or-later` identifiers; the applicable choice is specified in
the source notice, not by the text file's name.

See [한국어](docs/licensing_kor.md) / [English](docs/licensing_eng.md) for this
fork's source-publication policy and remaining limitations.
