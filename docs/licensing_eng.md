# Licensing and source-publication policy

Updated: 2026-11-08 | [한국어](licensing_kor.md)

## Scope

This fork publishes modified CANgaroo source, tests, scripts and documentation at
https://github.com/ungkyu/CANgaroo. The maintainer does not intend to publish this
fork's EXEs, installers, runtime DLL packages, or vendor SDKs. Local compilation
does not mean a binary release is being offered. Links to upstream releases in
the inherited README identify upstream artifacts, not this fork's releases.

Source publication still requires compliance with the source files' licenses.
This policy does not waive obligations for anyone who later distributes binaries.

## Existing and new code

- Preserve the root [LICENSE](../LICENSE), original copyright notices, author
  credits, and component-specific terms. Do not label the entire repository MIT,
  proprietary, or exclusively owned by the fork maintainer.
- Many inherited application files specify GPLv2-or-later; root LICENSE contains
  the GPLv2 text. The root text alone is not evidence that every file is GPLv2-only.
- New fork-authored application, test and build code is marked
  `SPDX-License-Identifier: GPL-2.0-or-later` where added. This is not a license
  declaration over separately licensed third-party code.
- Modified application/build/test files carry dated fork-change notices linking
  to the development history. Original copyright is retained; no ownership of
  inherited code is claimed.
- Locally authored documentation follows the fork's GPLv2-or-later source
  publication policy. Quoted third-party notices and license texts retain their
  own terms. The [Korean](windows-development_kor.md) and
  [English](windows-development_eng.md) histories describe the modifications.

## Qt VectorCAN code and Qt Charts

The four vendored Vector backend files come from Qt Serial Bus v6.12.0. Their
original copyright/SPDX alternatives remain intact. The earlier local README
selection of GPLv2-only has been corrected: select the offered GPLv3 alternative
for the combined configuration using open-source Qt Charts, which is GPLv3.
This selects an upstream permission; it does not relicense upstream Qt code.

The fork's GPLv2-or-later additions can be used under GPLv3 in that configuration.
Do not claim that the entire Charts-linked application is distributable under
GPLv2-only. This is not a declaration that every inherited/optional repository
component has been audited for compatibility.

Qt licensing: https://doc.qt.io/qt-6/qtcharts-index.html

## Included license texts and notices

- [Third-party inventory](../THIRD_PARTY_NOTICES.md)
- [GPLv2](../LICENSES/GPL-2.0-only.txt)
- [GPLv3](../LICENSES/GPL-3.0-only.txt)
- [LGPLv3](../LICENSES/LGPL-3.0-only.txt)
- [Existing Candle API license](../src/driver/CandleApiDriver/api/LICENSE)
- [Vector provenance and modifications](../src/driver/VectorDriver/qtvectorcan/README.md)

GPLv2 was copied unchanged from root LICENSE; GPLv3 and LGPLv3 were copied
unchanged from the installed Qt 6.12.0 source license collection. Text filenames
identify canonical versions; file notices determine whether later versions are
also permitted. No Qt commercial license is asserted by including upstream SPDX
alternatives, and proprietary vendor license texts are not reproduced here.

## Maintenance and limitations

- Keep origin, version, license, authorship and modification dates when importing
  code. Include the applicable license text before publishing new third-party
  source. Do not treat “public on GitHub” as a license grant.
- Do not commit local `.deps/`, build outputs, proprietary DLLs/SDKs, or confidential
  DBC/LDF files without separate review of their rights and confidentiality.
- Firmware and optional hardware SDKs have not received an exhaustive audit in
  this change. Existing file-specific notices continue to govern them.
- No conclusion is made here that proprietary Vector XL or other vendor library
  combinations are GPL-compatible. Users building those configurations must
  review the relevant vendor terms and applicable GPL exceptions/compatibility;
  source-only hosting does not grant those permissions.
- The repository's previously published commits remain in history. These notices
  clarify current source publication; they do not certify retroactive legal
  compliance or guarantee absence of claims.
- If binary distribution policy ever changes, separately assess corresponding
  source, dependency notices, redistribution rights and other applicable terms.

This document is a maintenance record, not a legal opinion or an exhaustive audit.
