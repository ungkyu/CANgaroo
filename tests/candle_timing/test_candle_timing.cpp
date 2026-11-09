// SPDX-License-Identifier: GPL-2.0-or-later
// Fork changes (2026-11-08): Windows build / CAN timing / Tx Generator development.
// See docs/windows-development_eng.md for the change record.
#include <QtTest>
#include "driver/CandleApiDriver/CandleApiTiming.h"

class CandleTimingTest : public QObject {
    Q_OBJECT
private slots:
    void fd75Percent_data() {
        QTest::addColumn<unsigned>("cycles");
        QTest::addColumn<bool>("possible");
        for (unsigned cycles : {160u, 80u, 40u, 32u, 20u, 48u, 24u, 16u, 12u, 8u})
            QTest::newRow(qPrintable(QString::number(cycles))) << cycles << true;
        for (unsigned cycles : {170u, 85u, 34u, 17u, 125u, 50u, 25u, 10u})
            QTest::newRow(qPrintable(QString::number(cycles))) << cycles << false;
    }
    void fd75Percent() {
        QFETCH(unsigned, cycles);
        QFETCH(bool, possible);
        candle_bittiming_t result{};
        // Only the bit period matters; the replacement must meet FD segment limits.
        const candle_bittiming_t original{1, cycles - 3, 1, 1, 1};
        QCOMPARE(CandleApiTiming::with75PercentData(original, result), possible);
        if (!possible) return;
        const unsigned tq = 1 + result.prop_seg + result.phase_seg1 + result.phase_seg2;
        QCOMPARE(tq * result.brp, cycles);
        QCOMPARE((1 + result.prop_seg + result.phase_seg1) * 4, tq * 3);
        QVERIFY(result.prop_seg + result.phase_seg1 <= 31);
        QVERIFY(result.phase_seg2 >= 1 && result.phase_seg2 <= 16);
        QVERIFY(result.brp >= 1 && result.brp <= 32);
    }
    void exactTimingAndLimits() {
        candle_capability_t caps{};
        caps.brp_min = 1; caps.brp_max = 1024; caps.brp_inc = 1;
        caps.tseg1_min = 1; caps.tseg1_max = 16;
        caps.tseg2_min = 1; caps.tseg2_max = 8; caps.sjw_max = 4;
        candle_bittiming_t result{};
        // 48 MHz / 500 kbps = 96 clock cycles: no exact 80% solution.
        QVERIFY(!CandleApiTiming::with80Percent(caps, {1, 12, 2, 1, 6}, result));
        // 48 MHz / 800 kbps = 60 cycles: 20 TQ, BRP=3, SP=16/20.
        QVERIFY(CandleApiTiming::with80Percent(caps, {1, 11, 2, 1, 4}, result));
        const unsigned tq = 1 + result.prop_seg + result.phase_seg1 + result.phase_seg2;
        QCOMPARE(tq * result.brp, 60u);
        QCOMPARE((1 + result.prop_seg + result.phase_seg1) * 5, tq * 4);
        QVERIFY(result.phase_seg1 + result.prop_seg <= caps.tseg1_max);
        QVERIFY(result.phase_seg2 <= caps.tseg2_max);
        caps.brp_inc = 0;
        QVERIFY(!CandleApiTiming::with80Percent(caps, {1, 11, 2, 1, 4}, result));
    }
    void fdControllerNominalTiming() {
        candle_capability_t caps{};
        caps.brp_min = 1; caps.brp_max = 512; caps.brp_inc = 1;
        caps.tseg1_min = 2; caps.tseg1_max = 256;
        caps.tseg2_min = 2; caps.tseg2_max = 128; caps.sjw_max = 128;
        for (unsigned brp : {1u, 2u, 4u, 8u, 10u}) {
            candle_bittiming_t result{};
            QVERIFY(CandleApiTiming::with80Percent(caps, {1, 147, 21, 1, brp}, result));
            const unsigned tq = 1 + result.prop_seg + result.phase_seg1 + result.phase_seg2;
            QCOMPARE(tq * result.brp, 170u * brp);
            QCOMPARE((1 + result.prop_seg + result.phase_seg1) * 5, tq * 4);
        }
    }
};
QTEST_APPLESS_MAIN(CandleTimingTest)
#include "test_candle_timing.moc"
