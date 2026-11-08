// SPDX-License-Identifier: GPL-2.0-or-later
// Fork changes (2026-11-08): Windows build / CAN timing / Tx Generator development.
// See docs/windows-development_eng.md for the change record.
#pragma once
#include <QDialog>
#include "core/BusMessage.h"
#include "driver/BusInterface.h"

class Backend;
class CanDbMessage;
class RawTxWindow;
class QSpinBox;
class QLabel;

class TxLiveEditDialog : public QDialog {
    Q_OBJECT
public:
    TxLiveEditDialog(QWidget *parent, Backend &backend);
    void synchronize(const BusMessage &message, const QString &name, BusInterfaceId interfaceId,
                     CanDbMessage *dbMessage, int interval, bool transmitting);
signals:
    void payloadEdited(const BusMessage &message);
    void intervalEdited(int interval);
private:
    RawTxWindow *_editor;
    QSpinBox *_interval;
    QLabel *_status;
    bool _synchronizing = false;
    bool _publishing = false;
    QByteArray _frameKey;
    CanDbMessage *_dbMessage = nullptr;
};
