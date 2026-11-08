// SPDX-License-Identifier: GPL-2.0-or-later
// Fork changes (2026-11-08): Windows build / CAN timing / Tx Generator development.
// See docs/windows-development_eng.md for the change record.
#include <QtTest>
#include "window/TxGeneratorWindow/TxLiveEditUtils.h"

class TxLiveEditTest : public QObject {
    Q_OBJECT
private slots:
    void payloadPreservesIdentity() {
        BusMessage target, edited;
        target.setId(0x123); target.setLength(2); target.setInterfaceId(7);
        target.setExtended(false); target.setFD(false); target.setRTR(false); target.setBRS(false);
        edited.setId(0x456); edited.setLength(8); edited.setInterfaceId(9);
        edited.setExtended(true); edited.setFD(true); edited.setBRS(true);
        edited.setDataAt(0, 0x12); edited.setDataAt(1, 0xAB);
        applyTxLivePayload(target, edited);
        QCOMPARE(target.getByte(0), uint8_t(0x12));
        QCOMPARE(target.getByte(1), uint8_t(0xAB));
        QCOMPARE(target.getId(), 0x123u);
        QCOMPARE(target.getLength(), uint8_t(2));
        QCOMPARE(target.getInterfaceId(), BusInterfaceId(7));
        QVERIFY(!target.isExtended()); QVERIFY(!target.isFD());
        QVERIFY(!target.isRTR()); QVERIFY(!target.isBRS());
    }
    void stoppedEditsNeverStartTransmission() {
        int interval = 100; bool enabled = false;
        applyTxInterval(interval, enabled, 20);
        QCOMPARE(interval, 20); QVERIFY(!enabled);
        applyTxInterval(interval, enabled, 0);
        QCOMPARE(interval, 0); QVERIFY(!enabled);
        applyTxInterval(interval, enabled, 50);
        QCOMPARE(interval, 50); QVERIFY(!enabled);
    }
    void runningEditsPreserveStateUntilZero() {
        int interval = 100; bool enabled = true;
        applyTxInterval(interval, enabled, 20);
        QCOMPARE(interval, 20); QVERIFY(enabled);
        applyTxInterval(interval, enabled, 0);
        QCOMPARE(interval, 0); QVERIFY(!enabled);
        applyTxInterval(interval, enabled, 100);
        QVERIFY(!enabled);
    }
};
QTEST_APPLESS_MAIN(TxLiveEditTest)
#include "TxLiveEditTest.moc"
