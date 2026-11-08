// SPDX-License-Identifier: GPL-2.0-or-later
// Fork changes (2026-11-08): Windows build / CAN timing / Tx Generator development.
// See docs/windows-development_eng.md for the change record.
#include <QtTest>
#include "window/TxGeneratorWindow/TxGeneratorListUtils.h"

class TxGeneratorListTest : public QObject {
    Q_OBJECT
private slots:
    void search() {
        const QStringList fields{"0x123", "EngineStatus", "CAN Channel 1", "291"};
        QVERIFY(txGeneratorMatchesSearch("", fields));
        QVERIFY(txGeneratorMatchesSearch("  ", fields));
        QVERIFY(txGeneratorMatchesSearch("engine", fields));
        QVERIFY(txGeneratorMatchesSearch("0X123", fields));
        QVERIFY(txGeneratorMatchesSearch("291", fields));
        QVERIFY(txGeneratorMatchesSearch("engine 123 channel", fields));
        QVERIFY(txGeneratorMatchesSearch("engine\tchannel", fields));
        QVERIFY(!txGeneratorMatchesSearch("engine brake", fields));
        QVERIFY(!txGeneratorMatchesSearch("0x124", fields));
    }
    void preserveDifferentConfigurations() {
        const auto key = txGeneratorDuplicateKey("id:1", 0x123, false, false, false, false, 100, QByteArray::fromHex("0011"));
        QCOMPARE(key, txGeneratorDuplicateKey("id:1", 0x123, false, false, false, false, 100, QByteArray::fromHex("0011")));
        QVERIFY(key != txGeneratorDuplicateKey("id:2", 0x123, false, false, false, false, 100, QByteArray::fromHex("0011")));
        QVERIFY(key != txGeneratorDuplicateKey("id:1", 0x124, false, false, false, false, 100, QByteArray::fromHex("0011")));
        QVERIFY(key != txGeneratorDuplicateKey("id:1", 0x123, true, false, false, false, 100, QByteArray::fromHex("0011")));
        QVERIFY(key != txGeneratorDuplicateKey("id:1", 0x123, false, true, false, false, 100, QByteArray::fromHex("0011")));
        QVERIFY(key != txGeneratorDuplicateKey("id:1", 0x123, false, false, true, false, 100, QByteArray::fromHex("0011")));
        QVERIFY(key != txGeneratorDuplicateKey("id:1", 0x123, false, false, false, true, 100, QByteArray::fromHex("0011")));
        QVERIFY(key != txGeneratorDuplicateKey("id:1", 0x123, false, false, false, false, 0, QByteArray::fromHex("0011")));
        QVERIFY(key != txGeneratorDuplicateKey("id:1", 0x123, false, false, false, false, 100, QByteArray::fromHex("0012")));
        QVERIFY(key != txGeneratorDuplicateKey("id:1", 0x123, false, false, false, false, 100, QByteArray::fromHex("001100")));
        QVERIFY(txGeneratorDuplicateKey("unresolved:0:A", 1, false, false, false, false, 10, {})
             != txGeneratorDuplicateKey("unresolved:0:B", 1, false, false, false, false, 10, {}));
    }
};
QTEST_APPLESS_MAIN(TxGeneratorListTest)
#include "TxGeneratorListTest.moc"
