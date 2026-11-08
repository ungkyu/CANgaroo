#include "TxLiveEditDialog.h"
#include "TxGeneratorListUtils.h"
#include "window/RawTxWindow/RawTxWindow.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QScopedValueRollback>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <limits>

TxLiveEditDialog::TxLiveEditDialog(QWidget *parent, Backend &backend) : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(false);
    resize(850, 520);
    auto *layout = new QVBoxLayout(this);
    auto *note = new QLabel(tr("Changes apply immediately. Editing does not start transmission. "
                              "While running, the next scheduled frame uses the new values."), this);
    note->setWordWrap(true);
    layout->addWidget(note);
    auto *controls = new QHBoxLayout;
    controls->addWidget(new QLabel(tr("Interval:"), this));
    _interval = new QSpinBox(this);
    _interval->setObjectName("liveInterval");
    _interval->setRange(0, std::numeric_limits<int>::max());
    _interval->setSuffix(tr(" ms"));
    _interval->setKeyboardTracking(false);
    _interval->setToolTip(tr("0 stops cyclic transmission. A positive value does not start it."));
    controls->addWidget(_interval);
    _status = new QLabel(this);
    controls->addWidget(_status, 1);
    layout->addLayout(controls);
    _editor = new RawTxWindow(this, backend);
    _editor->setLivePayloadEditing(true);
    layout->addWidget(_editor, 1);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);
    layout->addWidget(buttons);
    connect(_interval, &QSpinBox::valueChanged, this, [this](int value) {
        if (!_synchronizing)
            emit intervalEdited(value);
    });
    connect(_editor, &RawTxWindow::messageUpdated, this, [this](const BusMessage &message) {
        if (_synchronizing)
            return;
        QScopedValueRollback<bool> publishing(_publishing, true);
        emit payloadEdited(message);
    });
}

void TxLiveEditDialog::synchronize(const BusMessage &message, const QString &name,
    BusInterfaceId interfaceId, CanDbMessage *dbMessage, int interval, bool transmitting)
{
    QScopedValueRollback<bool> syncing(_synchronizing, true);
    setWindowTitle(tr("Live Edit: %1 — 0x%2").arg(name).arg(message.getId(), 3, 16, QChar('0')));
    const QSignalBlocker blocker(_interval);
    _interval->setValue(interval);
    _status->setText(transmitting ? tr("Transmitting — live values enabled")
                                 : tr("Stopped — edits update values only"));
    const QByteArray payload(reinterpret_cast<const char *>(message.getData()), message.getLength());
    const auto key = txGeneratorDuplicateKey(QString::number(interfaceId), message.getId(),
        message.isExtended(), message.isFD(), message.isBRS(), message.isRTR(), 0, payload);
    // Do not destroy an active table editor when only run/stop or interval changes.
    if (!_publishing && (key != _frameKey || dbMessage != _dbMessage))
        _editor->setMessage(message, name, interfaceId, dbMessage);
    _frameKey = key;
    _dbMessage = dbMessage;
}
