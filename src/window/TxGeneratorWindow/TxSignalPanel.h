// SPDX-License-Identifier: GPL-2.0-or-later
// Fork changes (2026-11-10): selected signals from multiple live TX messages.
#pragma once
#include <QDialog>
#include <QHash>
#include <QUuid>
#include "core/BusMessage.h"
#include "db/model/CanDbMessage.h"

struct TxPanelMessage {
    QUuid id;
    QString name, interfaceName;
    BusMessage frame;
    CanDbMessage *databaseMessage = nullptr;
    int interval = 0;
    bool transmitting = false;
};

class QTreeWidget;
class QTableWidget;
class QLineEdit;
class QLabel;

// Applies only the requested signal bits to the current authoritative frame.
bool applyTxPanelSignal(BusMessage &frame, const CanDbSignal &signal, double value);

class TxSignalPanel : public QDialog {
    Q_OBJECT
public:
    explicit TxSignalPanel(QWidget *parent = nullptr);
    void synchronize(const QList<TxPanelMessage> &messages);
signals:
    void signalEdited(const QUuid &messageId, const QString &signal, double physicalValue);
private:
    QTreeWidget *_available;
    QTableWidget *_selected;
    QLineEdit *_search;
    QLabel *_feedback;
    QHash<QUuid, TxPanelMessage> _messages;
    QList<QPair<QUuid, QString>> _pins;
    QByteArray _candidateKey;
    void filter();
    void addSelected();
    void removeSelected();
    void rebuildPins();
    void refreshValues();
    void editValue(int row, int column);
};
