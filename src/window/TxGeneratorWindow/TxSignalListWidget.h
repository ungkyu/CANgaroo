// SPDX-License-Identifier: GPL-2.0-or-later
// Fork changes (2026-11-08): Windows build / CAN timing / Tx Generator development.
// See docs/windows-development_eng.md for the change record.
#pragma once
#include <QWidget>
#include <QList>
#include <QString>

class CanDbMessage;
class QTreeWidget;
class QLineEdit;

struct TxSignalMessageEntry {
    CanDbMessage *message;
    QString database;
};

class TxSignalListWidget : public QWidget {
    Q_OBJECT
public:
    explicit TxSignalListWidget(QWidget *parent = nullptr);
    void setMessages(const QList<TxSignalMessageEntry> &entries);
    QList<CanDbMessage *> selectedMessages() const;
    CanDbMessage *currentMessage() const;
signals:
    void selectionChanged();
    void messageActivated();
private:
    QLineEdit *_search;
    QTreeWidget *_tree;
    void applyFilter();
};
