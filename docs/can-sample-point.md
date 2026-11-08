# CAN sample point 80% 지원

- 작업일: 2026-11-08
- 브랜치: `develop`
- 범위: Windows Vector 및 Candlelight / gs_usb의 nominal sample point

## 변경 내용

### Vector

기존 nominal 87.5% 목록에 80.0%를 추가했다. Qt 기본 VectorCAN 플러그인은
sample point 설정 키를 제공하지 않으므로, Qt Serial Bus v6.12.0의 Vector 백엔드를
`src/driver/VectorDriver/qtvectorcan/`에 포함하고 확장했다. 설치된 Qt 플러그인은 변경하지 않는다.

- GUI의 선택값을 MeasurementInterface에서 백엔드로 전달한다.
- Classic CAN: `xlCanSetChannelParams`로 bitrate, SJW, TSEG1, TSEG2를 전달한다.
- CAN FD: `xlCanFdSetConfiguration`으로 nominal/data 타이밍을 전달한다.
- 타이밍은 채널 활성화 전에 설정하며, 활성화 후 bitrate를 다시 적용하지 않는다.
- 초기화 권한을 다른 프로그램이 보유하거나 XL API가 타이밍을 거부하면 연결에 실패하고 원인을 기록한다.
- 80 MHz 기준 CAN FD 정수 TQ로 요청 sample point를 정확하게 표현할 수 없으면 임의 반올림하지 않고 실패한다.
- FD data 5/10 Mbps는 각각 16/8 TQ이므로 80%를 정확히 표현할 수 없다. 해당 data 목록은 87.5%로 제공한다. 기존 작업 파일에 이 조합이 80%로 저장되어 있으면 다시 선택해야 한다.
- Windows가 아닌 환경에서는 Vector XL 백엔드를 빌드하지 않는다.

장치와 드라이버에 따라 지원 가능한 타이밍이 다르므로 GUI 선택만으로 실제 장비 호환성을 보장하지 않는다.
일부 기존 조합(예: 80 MHz 기준 FD nominal 800 kbps / 87.5%)도 정수 TQ로 정확히 표현되지 않아 오류가 발생할 수 있다. 80% 조합을 사용하거나 별도 장비 검증이 필요하다.

### Candlelight / gs_usb

장치가 보고한 BRP 증가 단위/범위 및 TSEG1/TSEG2 범위 안에서 기존 bit period를
유지하는 **정확한 80% nominal 타이밍**을 계산해 프리셋에 추가한다.

- 기존 bitrate 및 sample point 프리셋은 유지한다.
- 실제 가능한 조합에만 80.0%를 추가한다.
- 저장한 설정을 GUI를 열지 않고 적용할 때도 같은 계산을 수행한다.
- 80%에 해당하는 BRP/TSEG를 실제 USB 설정 요청으로 전달한다.
- CAN FD data 타이밍 테이블은 변경하지 않는다.

예: 170 MHz / 500 kbps는 80% 타이밍을 만들 수 있다. 48 MHz / 800 kbps도 가능하다.
반면 48 MHz / 500 kbps는 한 비트가 96 clock cycles여서 정확한 80%를 만들 수 없다.
이 경우 GUI에 80%를 표시하지 않는다. 클럭이나 bitrate를 변경하지 않고 모든 장치에 80%를 강제할 수는 없다.

## 검증

- CandleTimingTest: 기존 bit period 유지, 80% 비율, 장치 segment 제한 및 잘못된 BRP 증가 단위 검증.
- VectorTimingTest: Classic/FD 설정 전달, bitrate 덮어쓰기 방지, 권한 부족/XL 오류/정수 TQ 불일치 시 채널 활성화 방지 검증. XL API는 모의 함수로 대체한다.
- Windows Qt Release 앱 컴파일 및 링크 확인.
- 실제 Vector/Candlelight 장치 통신과 GUI 수동 조작은 미검증.

실행 중인 기존 `cangaroo.exe`가 출력 파일을 잠가 이번 검증 EXE는
`build/qt-mingw-release/bin/cangaroo-samplepoint80.exe`로 생성한다.
이후 실행 프로세스를 종료하고 일반 빌드 스크립트로 `cangaroo.exe` 재빌드도 완료했다. 최신 기본 실행 파일에도 동일한 기능이 포함되어 있다.

## 장비 검증 체크리스트

- [ ] Vector 설정 화면에서 nominal 80.0% 선택 및 저장/재로드
- [ ] Classic CAN 500 kbps / 80%에서 장치별 XL API 적용 및 송수신 확인
- [ ] CAN FD nominal 500 kbps / 80%, data 2 Mbps / 80% 송수신 확인
- [ ] 다른 프로그램이 초기화 권한을 보유한 경우 명확한 오류 확인
- [ ] Candlelight 장치별 clock/capability를 확인하고 80% 표시 조건 확인
- [ ] Candlelight 기존 sample point와 새 80% 설정 송수신 비교
- [x] 기존 실행 파일을 종료한 뒤 일반 이름으로 재빌드

## 참고

- [Qt VectorCAN 설정 API](https://doc.qt.io/qt-6/qtserialbus-vectorcan-overview.html)
- [기반 Qt 소스와 변경 내역](../src/driver/VectorDriver/qtvectorcan/README.md)
- [Windows 개발 문서](windows-development_kor.md)
