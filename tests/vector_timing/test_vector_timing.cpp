// SPDX-License-Identifier: GPL-2.0-or-later
// Fork changes (2026-11-08): Windows build / CAN timing / Tx Generator development.
// See docs/windows-development_eng.md for the change record.
#include <QtTest>
#include "driver/VectorDriver/VectorTiming.h"

#ifdef Q_OS_WIN
// Compile the backend in this TU so its dynamically loaded XL function pointers
// can be replaced with mocks. No Vector driver or hardware is used by this test.
#include "../../src/driver/VectorDriver/qtvectorcan/vectorcanbackend.cpp"
namespace {
bool allowInitialization = true;
int activationCalls = 0;
int bitrateReplayCalls = 0;
int timingCalls = 0;
XLstatus timingResult = XL_SUCCESS;
XLchipParams classicTiming{};
XLcanFdConf fdTiming{};
HANDLE notification = nullptr;
XLstatus WINAPI driverOk() { return XL_SUCCESS; }
XLstatus WINAPI configMock(XLdriverConfig *config) {
    *config = {};
    config->channelCount = 1;
    config->channel[0].channelMask = 1;
    config->channel[0].channelCapabilities = XL_CHANNEL_FLAG_CANFD_SUPPORT;
    return XL_SUCCESS;
}
XLaccess WINAPI channelMaskMock(int, int, int) { return 1; }
XLstatus WINAPI openMock(XLportHandle *port, char *, XLaccess, XLaccess *permission,
                         quint32, quint32, quint32) {
    *port = 1; *permission = allowInitialization ? 1 : 0; return XL_SUCCESS;
}
XLstatus WINAPI closeMock(XLportHandle) {
    if (notification) { CloseHandle(notification); notification = nullptr; }
    return XL_SUCCESS;
}
XLstatus WINAPI deactivateMock(XLportHandle, XLaccess) { return XL_SUCCESS; }
XLstatus WINAPI activateMock(XLportHandle, XLaccess, quint32, quint32) {
    ++activationCalls; return XL_SUCCESS;
}
XLstatus WINAPI notificationMock(XLportHandle, XLhandle *handle, int) {
    notification = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    *handle = notification; return XL_SUCCESS;
}
XLstatus WINAPI classicMock(XLportHandle, XLaccess, XLchipParams *timing) {
    ++timingCalls; classicTiming = *timing; return timingResult;
}
XLstatus WINAPI fdMock(XLportHandle, XLaccess, XLcanFdConf *timing) {
    ++timingCalls; fdTiming = *timing; return timingResult;
}
XLstatus WINAPI bitrateMock(XLportHandle, XLaccess, quint32) {
    ++bitrateReplayCalls; return XL_SUCCESS;
}
char * WINAPI errorMock(XLstatus) { static char message[] = "mock XL error"; return message; }
}
#endif

class VectorTimingTest : public QObject {
    Q_OBJECT
private slots:
#ifdef Q_OS_WIN
    void init() {
        allowInitialization = true; timingResult = XL_SUCCESS;
        activationCalls = bitrateReplayCalls = timingCalls = 0;
        xlOpenDriver = driverOk; xlCloseDriver = driverOk;
        xlGetDriverConfig = configMock; xlGetChannelMask = channelMaskMock;
        xlOpenPort = openMock; xlClosePort = closeMock;
        xlActivateChannel = activateMock; xlDeactivateChannel = deactivateMock;
        xlSetNotification = notificationMock; xlCanSetChannelParams = classicMock;
        xlCanFdSetConfiguration = fdMock; xlCanSetChannelBitrate = bitrateMock;
        xlGetErrorString = errorMock;
    }
    void classicAppliedBeforeActivation() {
        CangarooVectorCanBackend device("can0");
        device.setConfigurationParameter(QCanBusDevice::BitRateKey, 500000);
        device.setConfigurationParameter(CangarooVectorCanBackend::NominalSamplePointKey, 800);
        QVERIFY(device.connectDevice());
        QCOMPARE(classicTiming.bitRate, 500000u);
        QCOMPARE(classicTiming.tseg1, quint8(15));
        QCOMPARE(classicTiming.tseg2, quint8(4));
        QCOMPARE(timingCalls, 1); QCOMPARE(activationCalls, 1);
        QCOMPARE(bitrateReplayCalls, 0);
        device.disconnectDevice();
    }
    void fdApplied() {
        CangarooVectorCanBackend device("can0");
        device.setConfigurationParameter(QCanBusDevice::BitRateKey, 500000);
        device.setConfigurationParameter(QCanBusDevice::CanFdKey, true);
        device.setConfigurationParameter(QCanBusDevice::DataBitRateKey, 2000000);
        device.setConfigurationParameter(CangarooVectorCanBackend::NominalSamplePointKey, 800);
        device.setConfigurationParameter(CangarooVectorCanBackend::DataSamplePointKey, 800);
        QVERIFY(device.connectDevice());
        QCOMPARE((1 + fdTiming.tseg1Abr) * 5, (1 + fdTiming.tseg1Abr + fdTiming.tseg2Abr) * 4);
        QCOMPARE((1 + fdTiming.tseg1Dbr) * 5, (1 + fdTiming.tseg1Dbr + fdTiming.tseg2Dbr) * 4);
        QCOMPARE(bitrateReplayCalls, 0);
        device.disconnectDevice();
    }
    void refusesMissingInitializationAccess() {
        allowInitialization = false;
        CangarooVectorCanBackend device("can0");
        device.setConfigurationParameter(CangarooVectorCanBackend::NominalSamplePointKey, 800);
        QVERIFY(!device.connectDevice());
        QCOMPARE(timingCalls, 0); QCOMPARE(activationCalls, 0);
    }
    void refusesRejectedTiming() {
        timingResult = XL_ERR_INVALID_ACCESS;
        CangarooVectorCanBackend device("can0");
        device.setConfigurationParameter(CangarooVectorCanBackend::NominalSamplePointKey, 800);
        QVERIFY(!device.connectDevice());
        QCOMPARE(timingCalls, 1); QCOMPARE(activationCalls, 0);
    }
    void refusesInexactFdSamplePoint() {
        CangarooVectorCanBackend device("can0");
        device.setConfigurationParameter(QCanBusDevice::CanFdKey, true);
        device.setConfigurationParameter(QCanBusDevice::DataBitRateKey, 5000000);
        device.setConfigurationParameter(CangarooVectorCanBackend::NominalSamplePointKey, 800);
        device.setConfigurationParameter(CangarooVectorCanBackend::DataSamplePointKey, 800);
        QVERIFY(!device.connectDevice());
        QCOMPARE(timingCalls, 0); QCOMPARE(activationCalls, 0);
    }
#endif
    void segments() {
        uint32_t tseg1 = 0, tseg2 = 0;
        QVERIFY(vectorTimingSegments(20, 800, tseg1, tseg2));
        QCOMPARE(tseg1, 15u); QCOMPARE(tseg2, 4u);
        QVERIFY(vectorTimingSegments(16, 875, tseg1, tseg2));
        QCOMPARE(tseg1, 13u); QCOMPARE(tseg2, 2u);
        QVERIFY(vectorTimingSegments(80, 800, tseg1, tseg2));
        QCOMPARE(tseg1, 63u); QCOMPARE(tseg2, 16u);
        // Inexact requests must not be silently rounded.
        QVERIFY(!vectorTimingSegments(16, 800, tseg1, tseg2));
        QVERIFY(!vectorTimingSegments(8, 800, tseg1, tseg2));
        QVERIFY(!vectorTimingSegments(20, 0, tseg1, tseg2));
        QVERIFY(!vectorTimingSegments(20, 1000, tseg1, tseg2));
    }
#ifdef Q_OS_WIN
    void fdDataSegmentLimits() {
        for (int rate : {500000, 1000000, 2000000, 4000000, 8000000}) {
            const auto timing = xlCanFdConfInit(500000, rate, 800, 800);
            QCOMPARE(timing.dataBitRate, unsigned(rate));
            QVERIFY(timing.tseg1Dbr <= 32);
            QVERIFY(timing.tseg2Dbr <= 16);
            QCOMPARE((1 + timing.tseg1Dbr) * 5, (1 + timing.tseg1Dbr + timing.tseg2Dbr) * 4);
        }
    }
#endif
};
QTEST_GUILESS_MAIN(VectorTimingTest)
#include "test_vector_timing.moc"
