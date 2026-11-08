# Windows Build Environment and Development History

This document tracks CANgaroo's Windows build environment, setup, verification scope, and future features. Update procedures and verification results together when the environment or functionality changes.

- Created / last updated: 2026-11-08 / 2026-11-08
- Repository: `E:\MyWorkspaces\CANgaroo`
- Current branch: `develop`; the initial `build/qt-20261108` branch was created from `master`.
- Target: Windows x64 / Qt 6.12.0 / MinGW / Release
- Language: English | [한국어](windows-development_kor.md)

## 1. Current status

| Item | Status | Evidence and scope |
| --- | --- | --- |
| Branch and remote tracking | Complete | `develop` tracks `origin/develop`; initial environment commit pushed |
| Qt Serial Port / Serial Bus | Complete | Installed by the user; both module files verified |
| pybind11 | Complete | Version 3.1.0 in `.deps/python/`; import verified with MinGW Python |
| Python development files | Complete | Python 3.9 headers and import library bundled with Qt tools |
| Release compilation and linking | Complete | Standard `cangaroo.exe` rebuilt after the 80% feature changes |
| Runtime deployment | Complete | windeployqt run; Python DLL and standard library copied |
| Process startup | Partially verified | Hidden process responded 15 seconds after launch |
| GUI display and clean exit | Not verified | No direct GUI interaction verified by the agent |
| Embedded Python scripts | Not verified | In-app script execution still needs testing |
| CAN / LIN communication | Not verified | Physical hardware tests required |
| Feature unit / mock API tests | Passed | CandleTimingTest 4/4; VectorTimingTest 9/9, including initialization/cleanup; existing full suite not run |
| Vector / Candlelight nominal 80% | Implemented | GUI presets and actual configuration path added; hardware validation pending |

Compilation success and feature validation are separate. Early limited process queries did not show the hidden test processes, leading to an incorrect assumption that they had exited. A later privileged query found those processes still running; they were then stopped. Process responsiveness does not establish successful GUI operation. The user subsequently closed the existing application, and the standard executable was rebuilt.

## 2. Installed environment and paths

| Component | Version / location | Purpose |
| --- | --- | --- |
| Qt | `E:\Qt\6.12.0\mingw_64` | Build and Qt runtime |
| MinGW | GCC 13.1.0 / `E:\Qt\Tools\mingw1310_64` | C/C++ compilation and linking |
| qmake | `E:\Qt\6.12.0\mingw_64\bin\qmake.exe` | Generate Makefiles |
| make | `E:\Qt\Tools\mingw1310_64\bin\mingw32-make.exe` | Parallel build |
| Build Python | 3.9.10 / `E:\Qt\Tools\mingw1310_64\opt\bin\python3.exe` | MinGW-compatible Python integration |
| Python headers | `E:\Qt\Tools\mingw1310_64\opt\include\python3.9` | Includes `Python.h` |
| Python import library | `E:\Qt\Tools\mingw1310_64\opt\lib\libpython3.9.dll.a` | MinGW linking |
| Python runtime | `E:\Qt\Tools\mingw1310_64\opt\bin\libpython3.9.dll` | Application runtime |
| Package installer Python | `C:\Users\hanun\AppData\Local\Programs\Python\Python315\python.exe` | pip download and installation |
| pybind11 | 3.1.0 / `.deps/python/pybind11` | C++ / Python binding headers |

Python 3.15 installs the header package; the application links against Python 3.9. To change the linked Python version, match headers, import library, DLL, standard library, and architecture, then rebuild.

Qt modules used include Core, Gui, Widgets, Xml, Charts, Svg, SerialPort, SerialBus, Network, and OpenGL-related modules. Optional PEAK, Kvaser, and ZsCanFd SDK configurations were not enabled. Vector and TinyCAN are included by default; hardware use may require additional vendor drivers. Vector requires the XL driver library at runtime.

## 3. Setup and troubleshooting history

| Issue | Applied resolution | Remaining work |
| --- | --- | --- |
| Missing Qt Serial Port / Serial Bus | User installed both Qt modules | Recheck after Qt updates |
| Missing pybind11 | Installed with pip `--target`, then moved from build to `.deps/python/` | Restore dependencies on a new clone |
| `python3-embed development package not found` | Disabled pkg-config integration through qmake arguments; supplied headers and library explicitly | Generalize Windows Python discovery |
| Python include auto-detection `SyntaxError` | Passed the pybind11 include directory explicitly | Original detection expression can still print a diagnostic |
| Cygwin sh / test selected during build | Restricted build-process PATH to Qt, MinGW, and Windows | System PATH unchanged |
| Candle `_snwprintf` compilation failure | Added `DEFINES+=STRSAFE_NO_DEPRECATE` | Consider replacing the deprecated API |
| Missing runtime DLLs next to EXE | Ran windeployqt and copied Python runtime files | Test on a clean Windows machine |
| Existing EXE locked during linking | Built a temporary executable; stopped remaining test processes after user closed the app; rebuilt standard EXE | Close the application before rebuilding |

Initial environment setup did not modify application sources or `.pro` files. FEAT-001 subsequently changed Vector/Candlelight sources, Vector qmake configuration, and test projects.

## 4. Reproduction steps

Run in PowerShell. Adjust machine-specific Qt, MinGW, Python, and repository paths on another PC. Check each command's exit status before proceeding.

### 4.1 Check branch and modules

```powershell
Set-Location E:\MyWorkspaces\CANgaroo
git status --short --branch
& E:\Qt\6.12.0\mingw_64\bin\qmake.exe -v
Get-ChildItem E:\Qt\6.12.0\mingw_64\modules -Filter '*Serial*'
```

Future features are intended to be developed on `develop`. Record feature scope and verification evidence below.

### 4.2 Install or restore pybind11

```powershell
.\scripts\windows\install-deps.ps1 -PythonExe 'C:\Users\hanun\AppData\Local\Programs\Python\Python315\python.exe'
```

The script uses `scripts/windows/requirements.txt` (`pybind11==3.1.0`) and installs into `.deps/python/` with `--upgrade`. The bundled MinGW Python has no pip, so another Python interpreter installs this header package. Equivalent direct installation:

```powershell
& 'C:\Users\hanun\AppData\Local\Programs\Python\Python315\python.exe' -m pip install --target E:\MyWorkspaces\CANgaroo\.deps\python pybind11==3.1.0
```

### 4.3 Build

```powershell
& E:\MyWorkspaces\CANgaroo\scripts\windows\build-qt.ps1
```

The tracked script automatically locates the repository, creates the build directory, and accepts `-QtDir`, `-MinGWDir`, and `-Jobs`. Python linking currently remains fixed to version 3.9. The initial script was committed in `04d5b59`.

Dependencies live under `.deps/python/`; generated output lives under `build/`. Both are ignored by Git. Deleting only build preserves dependencies. After a new clone or deletion of `.deps/`, reinstall dependencies first.

The equivalent low-level configuration is:

```powershell
$env:PATH = 'E:\Qt\6.12.0\mingw_64\bin;E:\Qt\Tools\mingw1310_64\bin;E:\Qt\Tools\mingw1310_64\opt\bin;C:\Windows\System32;C:\Windows'
$env:PYTHONPATH = 'E:\MyWorkspaces\CANgaroo\.deps\python'
New-Item -ItemType Directory -Force E:\MyWorkspaces\CANgaroo\build\qt-mingw-release\src | Out-Null
Set-Location E:\MyWorkspaces\CANgaroo\build\qt-mingw-release\src
& E:\Qt\6.12.0\mingw_64\bin\qmake.exe E:\MyWorkspaces\CANgaroo\src\src.pro `
    'CONFIG+=release' 'CONFIG+=c++20' 'CONFIG-=debug_and_release' `
    -after 'CONFIG-=link_pkgconfig' 'PKGCONFIG=' `
    'INCLUDEPATH+=E:/Qt/Tools/mingw1310_64/opt/include/python3.9' `
    'INCLUDEPATH+=E:/MyWorkspaces/CANgaroo/.deps/python/pybind11/include' `
    'LIBS+=E:/Qt/Tools/mingw1310_64/opt/lib/libpython3.9.dll.a' `
    'DEFINES+=STRSAFE_NO_DEPRECATE'
if ($LASTEXITCODE -ne 0) { throw 'qmake failed' }
& E:\Qt\Tools\mingw1310_64\bin\mingw32-make.exe -j8
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
```

The app was built directly from `src/src.pro`. A complete root `cangaroo.pro` build with all tests has not been verified. Environment changes affect only the current process; the build script restores its PATH/PYTHONPATH afterward.

### 4.4 Prepare the runtime directory

```powershell
$env:PATH = 'E:\Qt\6.12.0\mingw_64\bin;E:\Qt\Tools\mingw1310_64\bin;' + $env:PATH
$binDir = 'E:\MyWorkspaces\CANgaroo\build\qt-mingw-release\bin'
& E:\Qt\6.12.0\mingw_64\bin\windeployqt.exe --release --no-translations "$binDir\cangaroo.exe"
if ($LASTEXITCODE -ne 0) { throw 'Qt deployment failed' }
Copy-Item E:\Qt\Tools\mingw1310_64\opt\bin\libpython3.9.dll $binDir -Force
New-Item -ItemType Directory -Force "$binDir\lib" | Out-Null
Copy-Item E:\Qt\Tools\mingw1310_64\opt\lib\python3.9 "$binDir\lib" -Recurse -Force
```

Keep the entire bin directory. Copying only the EXE omits Qt/MinGW DLLs, platform plugins, and Python runtime files. The application locates bundled Python using `lib/python3.9/os.py`.

windeployqt reported missing Direct3D 12 `dxcompiler.dll` / `dxil.dll`; effects on features using those components have not been tested. Qt translations were excluded with `--no-translations`.

## 5. Artifacts and logs

| Item | Repository-relative path |
| --- | --- |
| Standard executable | `build/qt-mingw-release/bin/cangaroo.exe` |
| Temporary feature executable | `build/qt-mingw-release/bin/cangaroo-samplepoint80.exe` |
| Build script | `scripts/windows/build-qt.ps1` |
| Dependency installer / version pin | `scripts/windows/install-deps.ps1`, `scripts/windows/requirements.txt` |
| pybind11 files | `.deps/python/` |
| Initial compiler log | `build/qt-mingw-release/src/build.log` |
| Compatibility rebuild log | `build/rebuild.log` |
| Startup stderr / stdout | `build/startup-error.log`, `build/startup-output.log` |
| Feature final build log | `build/sample-point-final-build.log` |
| Standard EXE rebuild log | `build/sample-point-standard-build.log` |
| Candle test results | `build/tests/candle-timing-results.txt` |
| Vector test results | `build/tests/vector-timing-results.txt` |

Logs are local and may be overwritten. Summarize significant failures and verification evidence in the history below.

## 6. Follow-up work

- [ ] Verify visible main window, menus, and clean application shutdown.
- [ ] Run embedded Python scripts and import standard library modules.
- [ ] Verify CAN communication and logging using virtual or physical adapters.
- [ ] Verify DBC/LDF loading and Trace/Graph behavior.
- [ ] Run the deployed bin directory on Windows without Qt installed.
- [x] Build and run the new Candle / Vector feature tests individually.
- [ ] Run the entire existing Windows unit test suite.
- [x] Add repository discovery and Qt/MinGW path and Jobs parameters to the build script.
- [ ] Generalize Python 3.9 paths and automate runtime deployment.
- [ ] Improve Windows Python auto-detection and pkg-config handling.
- [ ] Consider replacing the deprecated Candle formatting function.
- [ ] Complete physical Vector/Candlelight tests for FEAT-001.

## 7. Feature tracker

| ID | Work | Status | Branch | Evidence |
| --- | --- | --- | --- | --- |
| ENV-001 | Windows Qt environment | Build complete; functional verification pending | Initial `build/qt-20261108`, then `develop` | Release linking and partial process startup check |
| FEAT-001 | Vector / Candlelight nominal 80% | Implemented; hardware verification pending | `develop` | Calculation/mock tests and Release build; details below |

Use Planned, In progress, Verification pending, Complete, or On hold. Record evidence before marking a feature complete.

## 8. Feature record template

### FEAT-XXX: Feature name

- Created / updated: YYYY-MM-DD / YYYY-MM-DD
- Status: Planned
- Branch:
- Issue / commit:
- Purpose and user scenario:
- Scope:
- Changed files:
- Environment / dependency changes:
- UI / settings / file-format changes:
- Hardware or driver requirements:
- Effects on existing functionality:

#### Implementation

- [ ] Agree on requirements and acceptance criteria.
- [ ] Implement.
- [ ] Update documentation.
- [ ] Verify and record results.

#### Verification

| Date | Environment / equipment | Procedure or command | Expected | Actual | Result / log |
| --- | --- | --- | --- | --- | --- |
| YYYY-MM-DD | | | | | Not run |

#### Known issues and follow-up

- Issue / reproduction conditions:
- Impact:
- Workaround:
- Follow-up:

#### Rollback

- Code rollback procedure:
- Settings / data compatibility considerations:

## 9. Environment and development history

| Date | ID | Change | Outcome / remaining work |
| --- | --- | --- | --- |
| 2026-11-08 | ENV-001 | Created initial branch; installed/checked modules; linked MinGW Python and pybind11; added compatibility flags | Release EXE generated |
| 2026-11-08 | ENV-001 | Deployed Qt/MinGW and Python runtime files | Process responsive at 15 seconds; GUI/functionality not verified |
| 2026-11-08 | ENV-001 | Moved build script to `scripts/windows/`; created build directory automatically | Script survives build deletion |
| 2026-11-08 | ENV-002 | Moved pybind11 into `.deps/python/`; pinned version and added installation script | Included in initial environment commit |
| 2026-11-08 | GIT-001 | Created develop; configured local author; committed and pushed `04d5b59` | Complete |
| 2026-11-08 | DOC-001 | Created build documentation and feature template | Verification scope recorded |
| 2026-11-08 | FEAT-001 | Added Vector/Candlelight nominal 80% and tests | Build and tests passed; hardware verification pending |
| 2026-11-08 | BUILD-002 | Built temporary EXE, then standard cangaroo.exe | Complete |
| 2026-11-08 | DOC-002 | Renamed Korean document with `_kor`, filled gaps, created `_eng` counterpart | Versioned with feature changes |

## 10. References

- [Project README](../README.md)
- [Root qmake project](../cangaroo.pro)
- [App qmake project](../src/src.pro)
- [Python integration](../src/core/PythonEngine.cpp)
- [Test projects](../tests/tests.pro)
- [Detailed 80% feature record, Korean](can-sample-point.md)
- [Vector backend origin, modifications, and license](../src/driver/VectorDriver/qtvectorcan/README.md)

## 11. Dependency directory separation

- `.deps/python/`: installed pybind11 files moved from `build/python-deps`; ignored by Git.
- `scripts/windows/requirements.txt`: tracked version pin.
- `scripts/windows/install-deps.ps1`: tracked installation/restoration procedure.
- `scripts/windows/build-qt.ps1`: tracked build procedure; repository root detected automatically.
- Python development files remain in the Qt installation. Deployment copies the Python DLL and standard library beside the output executable.
- Deleting build requires rebuilding and redeploying runtime files. `git clean -fdx` also removes ignored `.deps/`; restore it with the dependency installer.
- Update requirements and this document when adding, removing, or changing dependencies. `--upgrade` installs the version specified in requirements into the target directory.

## 12. Git state and 80% feature details

### 12.1 Repository and commits

- Origin fetch/push: `git@github.com:ungkyu/CANgaroo.git`
- GitHub: https://github.com/ungkyu/CANgaroo
- Current branch / upstream: `develop` / `origin/develop`
- Initial environment commit: `04d5b59` — `build: add Windows Qt build scripts and dependency setup`
- That pushed commit contains `.gitignore`, the original Korean document, build/installation scripts, and requirements.
- Repository-local author: `ungkyu.han <hanungkyu@gmail.com>`.
- Follow-up change set: 80% support, tests, and bilingual documentation. Commit title: `feat: add 80% CAN sample point support and bilingual docs`. Check Git for the actual commit ID and remote synchronization status.
- `windows-development.md` was renamed to `windows-development_kor.md`; this file is its English counterpart. Maintain both together.

### 12.2 FEAT-001: nominal sample point 80%

**Vector:** added nominal 80.0% alongside 87.5%. An app-local backend derived from Qt Serial Bus v6.12.0 replaces the stock plugin for this integration; the installed Qt plugin is unchanged. Classic CAN uses `xlCanSetChannelParams`; CAN FD uses `xlCanFdSetConfiguration`. Selected timing is applied before channel activation, without a later bitrate setter overwriting it.

Missing initialization access, XL configuration errors, and FD sample points that cannot be represented exactly cause connection failure. Vector FD data 5/10 Mbps has 16/8 TQ with the current 80 MHz timing clock, so those data presets expose 87.5% rather than inexact 80%. Old saved 80% settings for those combinations must be reselected. Existing FD nominal combinations such as 800 kbps / 87.5% can also be inexact and are rejected.

**Candlelight / gs_usb:** calculate exact nominal 80% entries using the device's BRP range/increment and TSEG1/TSEG2 limits while preserving the original bit period. This also runs when applying saved settings without opening the GUI. Existing presets remain; the Candle FD data table is unchanged.

170 MHz / 500 kbps and 48 MHz / 800 kbps can represent 80%. A 48 MHz / 500 kbps bit occupies 96 clock cycles, so exact 80% is impossible; the GUI does not offer it. Exact 80% cannot be forced on every hardware/bitrate combination without changing the clock or bitrate.

### 12.3 Changed file groups

| Group | Main paths | Purpose |
| --- | --- | --- |
| Initial setup, committed | `.gitignore`, `scripts/windows/*.ps1`, `scripts/windows/requirements.txt` | Dependency separation, version pinning, install/build |
| Candlelight | `src/driver/CandleApiDriver/CandleApiInterface.*`, `CandleApiTiming.*` | Capability-constrained 80% calculation, presets, actual configuration |
| Vector | `src/driver/VectorDriver/VectorInterface.*`, `VectorDriver.cpp`, `VectorDriver.pri`, `VectorTiming.h` | Pass GUI values, integrate backend, calculate integer TQ segments |
| Qt-derived backend | `src/driver/VectorDriver/qtvectorcan/` | Extended XL configuration; provenance/modifications/license recorded in README |
| Tests | `tests/candle_timing/`, `tests/vector_timing/`, `tests/tests.pro` | Calculation and mock XL coverage; Candle test registered on Windows |
| Documentation | `docs/windows-development_kor.md`, `docs/windows-development_eng.md`, `docs/can-sample-point.md` | Progress, reproduction, detailed feature history |

### 12.4 Verification and artifacts

- CandleTimingTest: 4 passed / 0 failed. VectorTimingTest: 9 passed / 0 failed. Totals include Qt Test initialization and cleanup entries.
- Vector tests use mocked XL APIs, covering selected timing delivery, permission/API failure before activation, prevention of bitrate overwrites, exact integer TQ, and FD segment bounds.
- Test executables: `build/tests/candle_timing_test.exe`, `build/tests/vector_timing_test.exe`.
- Each new `tests/<name>/<name>.pro` project was built individually with qmake and mingw32-make. Include Qt/MinGW bin directories in PATH when running tests.
- A running standard EXE initially blocked linking, so `cangaroo-samplepoint80.exe` was generated temporarily.
- After the user closed the app and the remaining hidden test processes were stopped, the standard build script compiled/linked `cangaroo.exe` successfully.
- Latest standard executable: `build/qt-mingw-release/bin/cangaroo.exe`, build verified at 18:30 KST on 2026-11-08.
- Manual selection of the new GUI presets, settings save/reload, and physical Vector/Candlelight communication remain unverified.

## 13. FEAT-002: Initialize Tx Generator intervals from DBC cycle times (2026-11-08)

- Parse per-message `BA_ "GenMsgCycleTime" BO_ <ID> <ms>;` and use it as the initial interval when adding a DBC message.
- If the message has no explicit value, use `BA_DEF_DEF_ "GenMsgCycleTime" <ms>;` when present. If neither is defined, register **0 ms**.
- Explicit values override defaults, including explicit zero. Negative, fractional, and out-of-int-range message values are treated as zero.
- Store the cycle time in the message model and copy it during database updates. Extended-ID DBC message attributes are supported.
- Newly added DBC messages no longer receive a hardcoded 100 ms interval. Existing generator rows and saved user intervals are not automatically overwritten.
- Allow zero in the Interval spin box and expand its maximum to 2,147,483,647 ms, preventing large DBC intervals from being truncated when selected.
- **0 ms means unspecified / no cyclic transmission.** It is not converted into a 1 ms transmit loop. Starting cyclic transmission at zero is rejected; changing a running row to zero stops its cyclic transmission. Send Once remains available.
- Manual message addition still uses the Interval control, initially 100 ms.
- DBC parser tests: 37 passed / 0 failed, including initialization/cleanup. Coverage includes missing/explicit/default values, precedence, zero, negative/fractional/overflow values, unrelated IDs/scopes, large intervals, extended IDs, and database updates.
- Test results: `build/tests/dbc-cycle-time-results.txt`. Application build log: `build/dbc-cycle-time-build.log`.
- GUI interaction and hardware transmission timing require separate verification. Commit title: `feat: initialize Tx intervals from DBC cycle times`. Check Git for the actual commit ID and remote synchronization status.
- Release compilation/linking completed: FEAT-002 is included in the standard `build/qt-mingw-release/bin/cangaroo.exe`.

## 14. FEAT-003: Tx Generator search and duplicate removal (2026-11-08)

- Added a search box above the transmission list. Search message names, hexadecimal/decimal CAN IDs, and interfaces without case sensitivity. All whitespace-separated terms must match, potentially across different columns.
- Filtering affects visibility only: hidden messages continue transmitting. Reapply the filter after list rebuilds, additions, and row updates.
- Hidden rows are deselected and a hidden current item is cleared. Select All selects visible rows only, preventing hidden rows from remaining targets for bulk transmission, edits, or removal.
- Reapply the existing Available Messages search after refreshing the left-hand DBC list, and clear hidden selections there as well.
- The Remove Duplicates button examines the entire transmission list, including filtered-out rows.
- Duplicate identity: **same interface + CAN ID + Standard/Extended + Classic/FD + BRS + RTR + payload (including length) + interval**. Preserve different channels, data, or intervals even when the CAN ID matches.
- Display names and RUN/STOP states are excluded from the key. Preserve the first registered row's configuration/state and remove later duplicates. Saved interface names distinguish unresolved interfaces.
- The result dialog reports total rows removed and each removed row's original row number, name, CAN ID, interface, interval, payload, and whether it was transmitting. Show the first eight entries in the summary and the complete list under Details. Also report when no duplicates exist.
- Refresh the send schedule after removal. Removed running rows stop generating future repetitions; frames already queued in the driver cannot be recalled.
- Main files: `TxGeneratorWindow.cpp/.h/.ui/.pri`, `TxGeneratorListUtils.h`, `tests/tx_generator_list/`, `tests/tests.pro`.
- TxGeneratorListTest: 4 passed / 0 failed, including initialization/cleanup. Covered whitespace/case/multi-term/ID searches and duplicate keys that preserve differences in interface, frame format, payload, and interval.
- Test results: `build/tests/tx-generator-list-results.txt`. Application build log: `build/tx-generator-list-build.log`.
- Manual GUI interaction and hardware transmission behavior remain unverified. Commit title: `feat: add Tx Generator search and duplicate removal`. Check Git for the actual commit ID and remote synchronization status.
- Standard cangaroo.exe Release compilation/linking completed after retrying an initial output-file access denial.
