# Windows 빌드 환경 및 개발 이력

이 문서는 CANgaroo의 Windows 빌드 환경, 설정 과정, 실행 확인 범위와 앞으로 추가할 기능을 관리한다. 환경이나 기능이 바뀌면 관련 절차와 검증 결과를 함께 갱신한다.

- 최초 작성: 2026-11-08
- 마지막 갱신: 2026-11-08
- 작업 저장소: `E:\MyWorkspaces\CANgaroo`
- 현재 브랜치: `develop` (최초 작업 브랜치 `build/qt-20261108`은 `master`에서 생성)
- 빌드 대상: Windows x64 / Qt 6.12.0 / MinGW / Release
- 언어: 한국어 | [English](windows-development_eng.md)

## 1. 현재 상태

| 항목 | 상태 | 근거 및 범위 |
| --- | --- | --- |
| 작업 브랜치 및 원격 등록 | 완료 | `develop` 생성, `origin/develop` 추적 및 초기 환경 커밋 푸시 완료 |
| Qt Serial Port / Serial Bus | 완료 | 사용자 설치 후 두 모듈 파일 확인 |
| pybind11 설치 | 완료 | 프로젝트 `.deps/python`에 3.1.0 설치, MinGW Python에서 import 확인 |
| Python 개발 파일 연결 | 완료 | Qt 도구에 포함된 Python 3.9 헤더와 import library 사용 |
| Release 컴파일 및 링크 | 완료 | `cangaroo.exe` 생성, 빌드 로그에서 오류 없음 확인 |
| Qt 및 Python 런타임 배치 | 완료 | windeployqt 실행, Python DLL 및 표준 라이브러리 복사 |
| 실행 프로세스 확인 | 부분 확인 | 숨김 실행 후 15초 시점에 프로세스 응답 확인 |
| 화면 표시 및 정상 종료 | 미검증 | GUI를 직접 조작해 확인하지 않음 |
| Python 스크립트 기능 | 미검증 | 앱 내부에서 스크립트 실행 확인 필요 |
| CAN / LIN 하드웨어 통신 | 미검증 | 실제 장비를 연결한 송수신 검증 필요 |
| 기능 단위 / 모의 API 테스트 | 통과 | CandleTimingTest 4/4, VectorTimingTest 9/9 (초기화/정리 포함); 전체 기존 테스트는 미실행 |
| Vector / Candlelight 80% | 구현 완료 | nominal sample point 선택 및 실제 설정 경로 구현; 실장비 검증 대기 |

컴파일 성공과 기능 검증 완료는 별도로 기록한다. 최초 테스트 프로세스가 일반 조회에서 보이지 않아 종료된 것으로 추정했으나, 이후 권한이 있는 조회에서 숨김 테스트 프로세스가 남아 있음을 확인하고 종료했다. 프로세스 응답만으로 정상 GUI 구동을 단정하지 않는다. 사용자는 기존 EXE 종료를 알렸으며, 이후 기본 이름으로 재빌드를 완료했다.

## 2. 설치 환경과 경로

| 구성 요소 | 버전 / 경로 | 용도 |
| --- | --- | --- |
| Qt | `E:\Qt\6.12.0\mingw_64` | 앱 빌드 및 Qt 런타임 |
| MinGW | GCC 13.1.0 / `E:\Qt\Tools\mingw1310_64` | C/C++ 컴파일 및 링크 |
| qmake | `E:\Qt\6.12.0\mingw_64\bin\qmake.exe` | Makefile 생성 |
| make | `E:\Qt\Tools\mingw1310_64\bin\mingw32-make.exe` | 병렬 빌드 |
| 빌드용 Python | 3.9.10 / `E:\Qt\Tools\mingw1310_64\opt\bin\python3.exe` | MinGW용 Python 연동 |
| Python 헤더 | `E:\Qt\Tools\mingw1310_64\opt\include\python3.9` | `Python.h` 등 |
| Python import library | `E:\Qt\Tools\mingw1310_64\opt\lib\libpython3.9.dll.a` | MinGW 링크 |
| Python 런타임 | `E:\Qt\Tools\mingw1310_64\opt\bin\libpython3.9.dll` | 실행 시 필요 |
| 패키지 설치용 Python | `C:\Users\hanun\AppData\Local\Programs\Python\Python315\python.exe` | pip를 통한 pybind11 다운로드 및 설치 |
| pybind11 | 3.1.0 / `.deps\python\pybind11` | C++ / Python 바인딩 헤더 |

패키지 설치에 사용한 Python 3.15와 앱에 연결한 Python 3.9는 역할이 다르다. 현재 앱은 Python 3.9에 링크되어 있다. Python 버전을 바꾸려면 헤더, 링크 라이브러리, DLL, 표준 라이브러리를 같은 버전과 아키텍처로 맞추고 재빌드한다.

Qt 모듈은 Core, Gui, Widgets, Xml, Charts, Svg, SerialPort, SerialBus, Network 및 OpenGL 관련 모듈을 사용한다. 선택 하드웨어 SDK인 PEAK, Kvaser, ZsCanFd는 이번 빌드에서 활성화하지 않았다. Vector와 TinyCAN은 프로젝트의 기본 설정에 포함되지만 실제 사용에는 별도 장치 드라이버가 필요할 수 있다.

## 3. 진행한 설정과 문제 해결

| 문제 | 적용한 조치 | 남은 사항 |
| --- | --- | --- |
| Qt Serial Port / Serial Bus 누락 | 사용자가 Qt 모듈 추가 설치 | Qt 업데이트 후 재확인 |
| pybind11 누락 | pip `--target`으로 최초 build 폴더에 설치한 뒤 `.deps/python/`으로 이동 | 다른 PC에서 다시 설치 필요 |
| `python3-embed development package not found` | 로컬 qmake 인자로 pkg-config 연동을 해제하고 Python include / library 직접 지정 | 범용 Windows 빌드 설정은 추후 정리 가능 |
| qmake의 Python include 자동 탐지에서 `SyntaxError` | pybind11 include 경로를 qmake 인자로 직접 추가 | 원래 탐지 구문은 수정하지 않아 메시지가 남을 수 있음 |
| Cygwin sh / test 선택으로 빌드 실패 | 빌드 프로세스의 PATH를 Qt, MinGW, Windows 경로로 제한 | 시스템 PATH 자체는 변경하지 않음 |
| Candle 드라이버 `_snwprintf` 컴파일 오류 | `DEFINES+=STRSAFE_NO_DEPRECATE` 적용 | 기존 API 허용을 위한 호환성 옵션이며 소스 개선은 별도 검토 |
| EXE 폴더에 런타임 DLL 누락 | windeployqt 및 Python 런타임 복사 | 깨끗한 Windows 환경에서 실행 검증 필요 |

초기 환경 구성 단계에서는 앱 소스 및 `.pro` 파일을 수정하지 않았다. 이후 FEAT-001 구현에서 Vector/Candlelight 소스, Vector qmake 구성 및 테스트 프로젝트를 수정했다.

## 4. 재현 절차

아래 명령은 현재 PC의 경로를 기준으로 PowerShell에서 실행한다. 다른 PC에서는 Qt, MinGW, Python, 저장소 경로를 조정한다. 각 단계가 실패하면 다음 단계로 진행하지 않고 로그를 확인한다.

### 4.1 브랜치와 의존성 확인

```powershell
Set-Location E:\MyWorkspaces\CANgaroo
git status --short --branch
& E:\Qt\6.12.0\mingw_64\bin\qmake.exe -v
Get-ChildItem E:\Qt\6.12.0\mingw_64\modules -Filter '*Serial*'
```

현재 개발 브랜치는 `develop`이며 앞으로 기능 추가도 이 브랜치에 반영할 예정이다. 기능별 작업과 검증 근거를 아래 이력에 기록한다.

### 4.2 pybind11 설치

```powershell
.\scripts\windows\install-deps.ps1 -PythonExe 'C:\Users\hanun\AppData\Local\Programs\Python\Python315\python.exe'
```

MinGW에 포함된 Python에는 pip가 없어서 별도 Python의 pip로 헤더 패키지를 설치했다. 재설치 시 기존 대상 폴더 상태를 확인한다.

### 4.3 빌드

현재 PC에서는 로컬 스크립트를 실행할 수 있다.

```powershell
& E:\MyWorkspaces\CANgaroo\scripts\windows\build-qt.ps1
```

스크립트는 `scripts/windows/`에 두어 Git으로 관리한다(초기 환경 커밋 `04d5b59`에 포함). 의존성은 `.deps/python/`, 결과물은 `build/`에 분리한다. 두 경로 모두 Git에서 제외하며, build 폴더만 삭제할 때는 의존성이 유지된다. 새 clone이나 .deps 삭제 후에는 4.2절의 설치가 필요하다. 스크립트는 빌드 디렉터리를 자동 생성한다. 아래 명령으로도 동일한 설정을 재현할 수 있다.

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

이번에는 `src/src.pro`를 직접 빌드했다. 루트의 `cangaroo.pro`를 통한 테스트 포함 빌드는 검증하지 않았다. 환경 변수 변경은 해당 PowerShell 프로세스에만 적용된다.

### 4.4 실행 파일 배포 준비

```powershell
$env:PATH = 'E:\Qt\6.12.0\mingw_64\bin;E:\Qt\Tools\mingw1310_64\bin;' + $env:PATH
$binDir = 'E:\MyWorkspaces\CANgaroo\build\qt-mingw-release\bin'

& E:\Qt\6.12.0\mingw_64\bin\windeployqt.exe --release --no-translations "$binDir\cangaroo.exe"
if ($LASTEXITCODE -ne 0) { throw 'Qt deployment failed' }
Copy-Item E:\Qt\Tools\mingw1310_64\opt\bin\libpython3.9.dll $binDir -Force
New-Item -ItemType Directory -Force "$binDir\lib" | Out-Null
Copy-Item E:\Qt\Tools\mingw1310_64\opt\lib\python3.9 "$binDir\lib" -Recurse -Force
```

`bin` 폴더 전체를 유지한다. EXE만 복사하면 Qt DLL, MinGW 런타임, 플랫폼 플러그인, Python DLL이 누락된다. 앱은 `lib/python3.9/os.py`를 기준으로 함께 배치된 Python 홈을 탐색한다.

windeployqt 실행 시 Direct3D 12 관련 `dxcompiler.dll`, `dxil.dll` 탐색 경고가 있었다. 해당 기능 사용 시 영향은 미검증이다. 번역 파일은 `--no-translations`로 배포 대상에서 제외했다.

## 5. 결과물과 로그

| 항목 | 저장소 기준 경로 |
| --- | --- |
| 실행 파일 | `build/qt-mingw-release/bin/cangaroo.exe` |
| 재빌드 스크립트 | `scripts/windows/build-qt.ps1` |
| pybind11 설치 위치 | `.deps/python/` |
| 최초 컴파일 로그 | `build/qt-mingw-release/src/build.log` |
| 호환성 옵션 적용 후 빌드 로그 | `build/rebuild.log` |
| 실행 확인 stderr | `build/startup-error.log` |
| 실행 확인 stdout | `build/startup-output.log` |
| 80% 기능 최종 빌드 로그 | `build/sample-point-final-build.log` |
| 기본 EXE 재빌드 로그 | `build/sample-point-standard-build.log` |
| Candle 테스트 결과 | `build/tests/candle-timing-results.txt` |
| Vector 테스트 결과 | `build/tests/vector-timing-results.txt` |

로그는 로컬 참고 자료이며 다음 실행에서 덮어쓸 수 있다. 중요한 실패나 배포 검증 결과는 아래 이력에 요약해 남긴다.

## 6. 다음 확인 및 개선 항목

- [ ] 일반 실행으로 메인 창 표시, 메뉴 동작, 정상 종료 확인
- [ ] Python 창에서 기본 스크립트 실행 및 표준 라이브러리 import 확인
- [ ] 가상 CAN 또는 실제 어댑터를 이용한 송수신 및 로깅 확인
- [ ] DBC / LDF 로딩과 Trace / Graph 기본 동작 확인
- [ ] Qt가 설치되지 않은 Windows PC에서 `bin` 폴더 실행 확인
- [x] 새 Candle / Vector 기능 테스트 개별 빌드 및 실행
- [ ] Windows 전체 기존 단위 테스트 실행
- [x] 빌드 스크립트에 저장소 자동 탐지와 Qt/MinGW 경로 및 Jobs 매개변수 추가
- [ ] Python 3.9 고정 경로 일반화 및 배포 자동화 검토
- [ ] Windows Python 자동 탐지와 pkg-config 의존 방식 개선 검토
- [ ] Candle 드라이버의 deprecated 함수 교체 검토

위 항목은 후보 작업이며, 완료되기 전에는 기능이 검증된 것으로 표시하지 않는다.

## 7. 기능 추가 현황

새 기능마다 한 행을 추가하고 상세 기록은 다음 절의 양식을 복사한다.

| ID | 기능 / 작업 | 상태 | 브랜치 | 검증 결과 | 상세 기록 |
| --- | --- | --- | --- | --- | --- |
| ENV-001 | Windows Qt 빌드 환경 구성 | 빌드 완료 / 기능 검증 대기 | `build/qt-20261108` | Release 링크 성공, 실행 프로세스 부분 확인 | 본 문서 1~5절 |
| FEAT-001 | Vector / Candlelight nominal sample point 80% | 구현 / 장비 검증 대기 | `develop` | 계산 및 모의 XL API 테스트, Release 빌드 | [상세 기록](can-sample-point.md) |

상태 예시: 계획, 진행 중, 검증 대기, 완료, 보류. 기능 완료 시 관련 검증 근거를 기록한다.

## 8. 기능 추가 기록 템플릿

### FEAT-XXX: 기능명

- 작성일 / 갱신일: YYYY-MM-DD / YYYY-MM-DD
- 상태: 계획
- 작업 브랜치:
- 관련 이슈 / 커밋:
- 목적과 사용자 시나리오:
- 구현 범위:
- 변경 파일:
- 환경 / 의존성 변경:
- UI / 설정 / 저장 형식 변경:
- 하드웨어 또는 드라이버 요구사항:
- 기존 기능에 미치는 영향:

#### 구현 항목

- [ ] 요구사항 및 완료 기준 확정
- [ ] 구현
- [ ] 필요한 문서 갱신
- [ ] 검증 및 결과 기록

#### 검증 기록

| 날짜 | 환경 / 장비 | 절차 또는 명령 | 기대 결과 | 실제 결과 | 판정 / 로그 |
| --- | --- | --- | --- | --- | --- |
| YYYY-MM-DD | | | | | 미실행 |

#### 알려진 문제 및 후속 작업

- 문제 / 재현 조건:
- 영향 범위:
- 임시 대응:
- 후속 작업:

#### 되돌리기

- 코드 되돌리기 방법:
- 설정 / 데이터 호환성 주의사항:

## 9. 환경 및 개발 변경 이력

| 날짜 | ID | 변경 내용 | 검증 결과 / 남은 사항 |
| --- | --- | --- | --- |
| 2026-11-08 | ENV-001 | 새 브랜치 생성, Qt 모듈 확인, pybind11 설치, MinGW Python 연결, 빌드 호환성 옵션 적용 | Release EXE 생성 |
| 2026-11-08 | ENV-001 | Qt / MinGW 런타임, Python DLL 및 표준 라이브러리 배치 | 실행 후 15초 시점 프로세스 응답 확인; GUI와 기능 미검증 |
| 2026-11-08 | ENV-001 | 재빌드 스크립트를 `scripts/windows/`로 이동, 빌드 디렉터리 자동 생성 추가 | build 폴더 삭제 시 스크립트 유지; 의존성은 현재 .deps로 분리 |
| 2026-11-08 | DOC-001 | 빌드 절차와 기능 이력 템플릿 작성 | 실행 확인 범위와 후속 검증 항목 명시 |

## 10. 참고

- [프로젝트 README](../README.md)
- [메인 qmake 프로젝트](../cangaroo.pro)
- [앱 qmake 프로젝트](../src/src.pro)
- [Python 연동 구현](../src/core/PythonEngine.cpp)
- [테스트 프로젝트](../tests/tests.pro)


## 11. 의존성 디렉터리 분리 (2026-11-08)

- `.deps/python/`: pybind11 실제 설치 파일. 기존 `build/python-deps`에서 이동. Git 제외.
- `scripts/windows/requirements.txt`: 버전 고정 파일. Git 관리 대상.
- `scripts/windows/install-deps.ps1`: 의존성 설치 및 복원. Git 관리 대상.
- `scripts/windows/build-qt.ps1`: 저장소 경로를 자동 탐지하며 `-QtDir`, `-MinGWDir`, `-Jobs`로 도구 경로와 병렬 작업 수 지정 가능. Python 링크 버전은 현재 3.9로 고정.
- Qt에 포함된 Python 개발 파일은 기존 Qt 설치 위치를 유지한다. 배포용 Python DLL/표준 라이브러리는 여전히 빌드 결과물 옆에 복사한다.
- `build/` 전체 삭제 후에는 빌드와 런타임 배포를 다시 수행한다. `.deps/`도 `git clean -fdx`에는 삭제되므로 아래 명령으로 복원한다.

```powershell
# 저장소 루트에서 실행. pip가 설치된 Python 경로를 지정한다.
.\scripts\windows\install-deps.ps1 -PythonExe 'C:\Users\hanun\AppData\Local\Programs\Python\Python315\python.exe'
.\scripts\windows\build-qt.ps1
```

설치 스크립트의 `--upgrade`는 버전 고정 파일에 명시된 버전으로 설치 디렉터리를 갱신한다. 패키지 삭제/추가 시에는 버전 고정 파일과 본 문서의 환경 및 변경 이력을 함께 갱신한다.

## 12. Git 상태 및 80% 기능 진행 내역

### 12.1 저장소와 커밋

- 원격 `origin` (fetch/push): `git@github.com:ungkyu/CANgaroo.git`
- GitHub: https://github.com/ungkyu/CANgaroo
- 현재 브랜치 / upstream: `develop` / `origin/develop`
- 초기 환경 커밋: `04d5b59` — `build: add Windows Qt build scripts and dependency setup`
- 위 커밋에 `.gitignore`, 최초 한국어 문서, 빌드/설치 스크립트, requirements 파일을 포함하여 푸시했다.
- 작성자 정보는 이 저장소 로컬 설정에 적용했다: `ungkyu.han <hanungkyu@gmail.com>`.
- 후속 변경 묶음: 80% 기능, 테스트 및 한·영 문서 분리. 커밋 제목: `feat: add 80% CAN sample point support and bilingual docs`. 실제 커밋 ID와 원격 동기화 상태는 Git에서 확인한다.
- 기존 문서 `windows-development.md`는 `windows-development_kor.md`로 이름을 변경했다. 영어본은 `windows-development_eng.md`이다. 두 문서는 함께 갱신한다.

### 12.2 FEAT-001: nominal sample point 80%

- Vector: nominal 목록에 80.0% 추가. Qt v6.12.0 백엔드를 앱 내부에 포함하여 확장했다. 설치된 Qt 플러그인은 그대로 유지한다.
- Classic CAN은 `xlCanSetChannelParams`, CAN FD는 `xlCanFdSetConfiguration`으로 선택한 타이밍을 채널 활성화 전에 전달한다. 이후 bitrate 재적용으로 sample point를 덮어쓰지 않는다.
- 초기화 권한 부족, XL API 오류, 정확하게 표현할 수 없는 FD 타이밍은 연결 실패로 처리한다.
- Vector FD data 5/10 Mbps는 80 MHz 기준 16/8 TQ여서 정확한 80%가 불가능하므로 87.5%를 표시한다. 예전 설정에서 해당 data 조합을 80%로 저장했다면 다시 선택해야 한다.
- FD nominal 800 kbps / 87.5%처럼 기존 목록에 있어도 정수 TQ로 표현할 수 없는 조합은 오류로 처리한다.
- Candlelight / gs_usb: 장치 capability의 BRP 범위·증가 단위, TSEG1/TSEG2 제한에 맞춰 기존 bit period를 유지하는 정확한 80% nominal 타이밍만 추가한다.
- 저장 설정을 GUI 없이 적용할 때도 Candle 타이밍을 계산한다. Candle FD data 테이블은 변경하지 않았다.
- 170 MHz / 500 kbps와 48 MHz / 800 kbps는 80% 계산이 가능하다. 48 MHz / 500 kbps는 96 clock cycles라 정확한 80%를 만들 수 없어 표시하지 않는다.
- 상세 구현 및 장비 검증 체크리스트: [80% 기능 기록](can-sample-point.md).

### 12.3 변경 파일 그룹

| 그룹 | 주요 경로 | 역할 |
| --- | --- | --- |
| 초기 환경 (커밋 완료) | `.gitignore`, `scripts/windows/*.ps1`, `scripts/windows/requirements.txt` | 의존성 분리, 버전 고정, 설치 및 빌드 |
| Candlelight | `src/driver/CandleApiDriver/CandleApiInterface.*`, `CandleApiTiming.*` | 장치 제한을 반영한 80% 계산, 목록 및 실제 적용 |
| Vector | `src/driver/VectorDriver/VectorInterface.*`, `VectorDriver.cpp`, `VectorDriver.pri`, `VectorTiming.h` | UI 선택값 전달, 백엔드 통합, 정수 TQ 계산 |
| Qt 기반 Vector 백엔드 | `src/driver/VectorDriver/qtvectorcan/` | XL 설정 확장; 원본 출처·변경 내역·라이선스는 해당 README에 기록 |
| 테스트 | `tests/candle_timing/`, `tests/vector_timing/`, `tests/tests.pro` | 계산 및 모의 XL API 검증; Candle 테스트는 Windows에서 등록 |
| 문서 | `docs/windows-development_kor.md`, `docs/windows-development_eng.md`, `docs/can-sample-point.md` | 진행 현황, 재현 절차, 기능 상세 기록 |

### 12.4 검증과 결과물

- CandleTimingTest: 4 passed / 0 failed. VectorTimingTest: 9 passed / 0 failed. 합계에는 Qt Test 초기화/정리 항목도 포함된다.
- Vector 테스트는 모의 XL API를 사용한다. 설정 전달, 권한 부족 및 API 오류 시 활성화 방지, 타이밍 덮어쓰기 방지, FD 정수 TQ와 segment 범위를 확인했다.
- 테스트 실행 파일: `build/tests/candle_timing_test.exe`, `build/tests/vector_timing_test.exe`.
- 새 테스트는 각 `tests/<name>/<name>.pro`를 qmake와 mingw32-make로 빌드했다. 실행 시 Qt/MinGW의 bin을 PATH에 포함한다.
- 기존 `cangaroo.exe`가 실행 중이어서 임시로 `cangaroo-samplepoint80.exe`를 생성했다.
- 사용자 앱 종료 및 남아 있던 숨김 테스트 프로세스 종료 후 `scripts/windows/build-qt.ps1`로 기본 `cangaroo.exe`의 컴파일/링크를 완료했다.
- 최신 기본 EXE: `build/qt-mingw-release/bin/cangaroo.exe` (2026-11-08 18:30 KST 빌드 확인).
- 실제 GUI에서 새 항목 선택·설정 저장/복원 및 Vector/Candlelight 장비 송수신은 아직 검증하지 않았다.

### 12.5 추가 이력

| 날짜 | ID | 변경 내용 | 상태 |
| --- | --- | --- | --- |
| 2026-11-08 | ENV-002 | pybind11을 `.deps/python/`으로 분리, 버전 고정 및 설치 스크립트 추가 | 초기 환경 커밋에 포함 |
| 2026-11-08 | GIT-001 | `develop` 생성, 작성자 로컬 설정, `04d5b59` 커밋 및 원격 푸시 | 완료 |
| 2026-11-08 | FEAT-001 | Vector / Candlelight nominal 80% 구현, 테스트 추가 | 빌드·테스트 완료, 장비 검증 대기 |
| 2026-11-08 | BUILD-002 | 임시 EXE 생성 후 기본 `cangaroo.exe`로 재빌드 | 완료 |
| 2026-11-08 | DOC-002 | 한국어 문서 이름에 `_kor` 추가, 누락 내역 보완, `_eng` 영어본 생성 | 기능 변경과 함께 Git 관리 |