// SPDX-License-Identifier: GPL-2.0-or-later
// Fork changes (2026-11-08): Windows build / CAN timing / Tx Generator development.
// See docs/windows-development_eng.md for the change record.
#include <QtTest>
#include <QLineEdit>
#include <QTreeWidget>
#include "window/TxGeneratorWindow/TxSignalListWidget.h"
#include "db/model/CanDbMessage.h"
#include "db/model/CanDbSignal.h"

class TxSignalListTest : public QObject {
    Q_OBJECT
private slots:
    void findsOwningMessagesAndDeduplicatesSelection() {
        CanDb db;
        CanDbMessage engine(&db), wheels(&db);
        engine.setName("EngineStatus"); engine.setRaw_id(0x123);
        wheels.setName("WheelStatus"); wheels.setRaw_id(0x124);
        CanDbSignal speed(&engine), rpm(&engine), wheelSpeed(&wheels);
        speed.setName("VehicleSpeed"); rpm.setName("EngineRPM"); wheelSpeed.setName("VehicleSpeed");
        engine.addSignal(&speed); engine.addSignal(&rpm); wheels.addSignal(&wheelSpeed);
        TxSignalListWidget widget;
        widget.setMessages({{&engine, "engine.dbc"}, {&wheels, "wheel.dbc"}});
        auto *tree = widget.findChild<QTreeWidget *>("signalResults");
        auto *search = widget.findChild<QLineEdit *>("signalSearch");
        QVERIFY(tree); QVERIFY(search);
        QCOMPARE(tree->topLevelItemCount(), 3);
        QCOMPARE(tree->topLevelItem(0)->text(1), QString("EngineStatus"));
        QCOMPARE(tree->topLevelItem(2)->text(1), QString("WheelStatus"));
        search->setText("vehiclespeed");
        QVERIFY(!tree->topLevelItem(0)->isHidden());
        QVERIFY(tree->topLevelItem(1)->isHidden());
        QVERIFY(!tree->topLevelItem(2)->isHidden());
        tree->topLevelItem(0)->setSelected(true);
        tree->topLevelItem(2)->setSelected(true);
        QCOMPARE(widget.selectedMessages().size(), 2);
        search->clear();
        tree->clearSelection();
        tree->topLevelItem(0)->setSelected(true);
        tree->topLevelItem(1)->setSelected(true);
        QCOMPARE(widget.selectedMessages(), QList<CanDbMessage *>{&engine});
        search->setText("EngineRPM");
        QCOMPARE(widget.selectedMessages(), QList<CanDbMessage *>{&engine});
        search->setText("no such signal");
        QVERIFY(widget.selectedMessages().isEmpty());
        QVERIFY(widget.currentMessage() == nullptr);
    }
    void refreshPreservesFilterAndClearsOldSelections() {
        CanDb db;
        CanDbMessage message(&db);
        message.setName("EngineStatus"); message.setRaw_id(0x80000123u);
        CanDbSignal speed(&message); speed.setName("VehicleSpeed"); message.addSignal(&speed);
        TxSignalListWidget widget;
        widget.setMessages({{&message, "test.dbc"}});
        auto *tree = widget.findChild<QTreeWidget *>("signalResults");
        auto *search = widget.findChild<QLineEdit *>("signalSearch");
        QCOMPARE(tree->topLevelItem(0)->text(2), QString("0X123"));
        search->setText("speed EngineStatus 291");
        QVERIFY(!tree->topLevelItem(0)->isHidden());
        tree->setCurrentItem(tree->topLevelItem(0));
        QCOMPARE(widget.currentMessage(), &message);
        search->setText("missing");
        widget.setMessages({{&message, "test.dbc"}});
        QCOMPARE(search->text(), QString("missing"));
        QVERIFY(tree->topLevelItem(0)->isHidden());
        QVERIFY(widget.selectedMessages().isEmpty());
        widget.setMessages({});
        QCOMPARE(tree->topLevelItemCount(), 0);
        QVERIFY(widget.currentMessage() == nullptr);
    }
    void doubleClickRequestsMessageAddition() {
        CanDb db;
        CanDbMessage message(&db); message.setName("EngineStatus"); message.setRaw_id(0x123);
        CanDbSignal speed(&message); speed.setName("VehicleSpeed"); message.addSignal(&speed);
        TxSignalListWidget widget;
        widget.resize(640, 300);
        widget.setMessages({{&message, "test.dbc"}});
        widget.show();
        QCoreApplication::processEvents();
        auto *tree = widget.findChild<QTreeWidget *>("signalResults");
        tree->setCurrentItem(tree->topLevelItem(0));
        QSignalSpy activated(&widget, &TxSignalListWidget::messageActivated);
        const QPoint position = tree->visualItemRect(tree->topLevelItem(0)).center();
        QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, position);
        QTest::mouseDClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, position);
        QCOMPARE(activated.count(), 1);
        QCOMPARE(widget.selectedMessages(), QList<CanDbMessage *>{&message});
    }
};
QTEST_MAIN(TxSignalListTest)
#include "TxSignalListTest.moc"
