// SPDX-License-Identifier: GPL-2.0-or-later
// Fork changes (2026-11-10): scoped signal edits and multi-message panel tests.
#include <QtTest>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTreeWidget>
#include <limits>
#include "window/TxGeneratorWindow/TxSignalPanel.h"

struct Fixture {
    CanDb db;
    CanDbMessage first{&db}, second{&db};
    CanDbSignal speed{&first}, temperature{&first}, otherSpeed{&second};
    QList<TxPanelMessage> messages;
    Fixture() {
        first.setName("Engine"); first.setRaw_id(0x123);
        second.setName("OtherEngine"); second.setRaw_id(0x123);
        for (auto *signal : {&speed, &temperature, &otherSpeed}) {
            signal->setName(signal == &temperature ? "Temperature" : "Speed");
            signal->setLength(8); signal->setUnsigned(true);
            signal->setMinimumValue(0); signal->setMaximumValue(255);
        }
        temperature.setStartBit(8);
        first.addSignal(&speed); first.addSignal(&temperature); second.addSignal(&otherSpeed);
        BusMessage a, b;
        a.setId(0x123); a.setLength(2); a.setDataAt(0, 1); a.setDataAt(1, 42);
        b.setId(0x123); b.setLength(2); b.setDataAt(0, 9); b.setDataAt(1, 77);
        messages = {{QUuid::createUuid(), "Engine [#1]", "CAN1", a, &first, 100, false},
                    {QUuid::createUuid(), "Engine [#2]", "CAN2", b, &second, 20, true}};
    }
};

class TxSignalPanelTest : public QObject {
    Q_OBJECT
private slots:
    void changesOnlySignalBits() {
        Fixture f;
        f.speed.setLength(4); f.speed.setMaximumValue(15);
        auto frame = f.messages[0].frame;
        frame.setDataAt(0, 0xA1);
        QVERIFY(applyTxPanelSignal(frame, f.speed, 3));
        QCOMPARE(frame.getByte(0), uint8_t(0xA3));
        QCOMPARE(frame.getByte(1), uint8_t(42));
        QCOMPARE(frame.getId(), 0x123u);
        QCOMPARE(frame.getLength(), uint8_t(2));
        QVERIFY(!applyTxPanelSignal(frame, f.speed, 16));
        QVERIFY(!applyTxPanelSignal(frame, f.speed, std::numeric_limits<double>::infinity()));
        QCOMPARE(frame.getByte(0), uint8_t(0xA3));
    }
    void editsBindToMessageUuidAndPreserveRunState() {
        Fixture f;
        TxSignalPanel panel;
        panel.synchronize(f.messages);
        auto *available = panel.findChild<QTreeWidget *>("panelAvailable");
        auto *table = panel.findChild<QTableWidget *>("panelSelected");
        available->topLevelItem(0)->setSelected(true);
        available->topLevelItem(1)->setSelected(true);
        available->topLevelItem(2)->setSelected(true);
        panel.findChild<QPushButton *>("panelAdd")->click();
        panel.findChild<QPushButton *>("panelAdd")->click();
        QCOMPARE(table->rowCount(), 3); // Never duplicate a pinned signal.
        QSignalSpy edits(&panel, &TxSignalPanel::signalEdited);
        connect(&panel, &TxSignalPanel::signalEdited, &panel,
            [&](const QUuid &id, const QString &name, double value) {
                for (auto &message : f.messages)
                    if (message.id == id)
                        QVERIFY(applyTxPanelSignal(message.frame, *message.databaseMessage->getSignalByName(name), value));
                panel.synchronize(f.messages);
            });
        table->item(2, 4)->setText("23");
        QCOMPARE(edits.count(), 1);
        QCOMPARE(edits.first().first().toUuid(), f.messages[1].id);
        QCOMPARE(f.messages[1].frame.getByte(0), uint8_t(23));
        QCOMPARE(f.messages[0].frame.getByte(0), uint8_t(1));
        table->item(0, 4)->setText("11");
        QCOMPARE(f.messages[0].frame.getByte(0), uint8_t(11));
        QCOMPARE(f.messages[0].frame.getByte(1), uint8_t(42));
        QVERIFY(!f.messages[0].transmitting);
        QVERIFY(f.messages[1].transmitting);
        QCOMPARE(f.messages[0].interval, 100);
        QCOMPARE(f.messages[1].interval, 20);
        QCOMPARE(table->item(1, 4)->text(), QString("42"));
        table->item(0, 4)->setText("999");
        QCOMPARE(edits.count(), 2); // Invalid input is not published.
        QCOMPARE(table->item(0, 4)->text(), QString("11"));
        panel.close();
        panel.show();
        QCOMPARE(table->rowCount(), 3);
        table->selectRow(1);
        panel.findChild<QPushButton *>("panelRemove")->click();
        QCOMPARE(table->rowCount(), 2);
        QCOMPARE(f.messages.size(), 2); // Removing a pin never removes the CAN message.
        f.messages.removeAt(0);
        panel.synchronize(f.messages);
        QCOMPARE(table->rowCount(), 1);
        QCOMPARE(table->item(0, 4)->text(), QString("23"));
        panel.synchronize({});
        QCoreApplication::processEvents(); // Pending post-edit refreshes must be safe after removal.
        QCOMPARE(table->rowCount(), 0);
    }
    void validatesScaledSignedAnd64BitValues() {
        Fixture f;
        auto frame = f.messages[0].frame;
        f.speed.setUnsigned(false); f.speed.setFactor(0.5); f.speed.setOffset(-10);
        f.speed.setMinimumValue(-74); f.speed.setMaximumValue(53.5);
        QVERIFY(applyTxPanelSignal(frame, f.speed, -15));
        QCOMPARE(frame.getByte(0), uint8_t(0xF6)); // raw -10
        QCOMPARE(frame.getByte(1), uint8_t(42));
        QVERIFY(!applyTxPanelSignal(frame, f.speed, 54));
        f.speed.setUnsigned(true); f.speed.setFactor(1); f.speed.setOffset(0);
        f.speed.setMinimumValue(0); f.speed.setMaximumValue(0);
        f.speed.setLength(64); frame.setLength(8);
        QVERIFY(!applyTxPanelSignal(frame, f.speed, -1));
        QVERIFY(!applyTxPanelSignal(frame, f.speed, std::ldexp(1.0, 64)));
        QVERIFY(applyTxPanelSignal(frame, f.speed, 123));
        QCOMPARE(frame.getByte(0), uint8_t(123));
    }
    void filtersCandidatesAndRejectsInactiveSignals() {
        Fixture f;
        f.first.setMuxer(&f.speed);
        f.temperature.setIsMuxed(true); f.temperature.setMuxValue(2);
        TxSignalPanel panel;
        panel.synchronize(f.messages);
        auto *available = panel.findChild<QTreeWidget *>("panelAvailable");
        auto *table = panel.findChild<QTableWidget *>("panelSelected");
        available->topLevelItem(1)->setSelected(true);
        panel.findChild<QPushButton *>("panelAdd")->click();
        QVERIFY(!(table->item(0, 4)->flags() & Qt::ItemIsEditable));
        QSignalSpy edits(&panel, &TxSignalPanel::signalEdited);
        table->item(0, 4)->setText("55");
        QCOMPARE(edits.count(), 0);
        auto *search = panel.findChild<QLineEdit *>("panelSearch");
        search->setText("speed CAN2");
        QVERIFY(available->topLevelItem(0)->isHidden());
        QVERIFY(!available->topLevelItem(2)->isHidden());
        QVERIFY(!available->topLevelItem(1)->isSelected());
        f.messages[0].frame.setDataAt(0, 2);
        panel.synchronize(f.messages);
        QVERIFY(table->item(0, 4)->flags() & Qt::ItemIsEditable);
    }
};
QTEST_MAIN(TxSignalPanelTest)
#include "TxSignalPanelTest.moc"
