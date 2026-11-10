// SPDX-License-Identifier: GPL-2.0-or-later
// Fork changes (2026-11-10): multi-message signal panel, without auto-transmit.
#include "TxSignalPanel.h"
#include "TxGeneratorListUtils.h"
#include <QApplication>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <cmath>
#include <limits>

bool applyTxPanelSignal(BusMessage &frame, const CanDbSignal &signal, double value)
{
    if (!std::isfinite(value) || !std::isfinite(signal.getFactor())
        || signal.getFactor() == 0 || !signal.isPresentInMessage(frame)
        || frame.isRTR() || signal.length() == 0 || signal.length() > 64)
        return false;
    if (signal.getMinimumValue() < signal.getMaximumValue()
        && (value < signal.getMinimumValue() || value > signal.getMaximumValue()))
        return false;
    const double raw = std::round((value - signal.getOffset()) / signal.getFactor());
    if (!std::isfinite(raw)) return false;
    const long double lower = signal.isUnsigned() ? 0 : -std::ldexp(1.0L, signal.length() - 1);
    const long double upperExclusive = std::ldexp(1.0L,
        signal.isUnsigned() ? signal.length() : signal.length() - 1);
    if (raw < lower || raw >= upperExclusive) return false;
    signal.injectPhysicalIntoMessage(frame, value);
    return true;
}

TxSignalPanel::TxSignalPanel(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Live Signal Panel"));
    setModal(false); // Closing hides the panel and retains the session's selection.
    resize(1100, 550);
    auto *layout = new QVBoxLayout(this);
    auto *note = new QLabel(tr("Choose signals from registered TX messages, then edit Value with Enter or focus change. "
        "Edits never start transmission; running messages use changes on their next scheduled send."), this);
    note->setWordWrap(true);
    layout->addWidget(note);
    auto *splitter = new QSplitter(this);
    auto *source = new QWidget(splitter);
    auto *sourceLayout = new QVBoxLayout(source);
    _search = new QLineEdit(source);
    _search->setObjectName("panelSearch");
    _search->setPlaceholderText(tr("Search signal, message, CAN ID or interface..."));
    _search->setClearButtonEnabled(true);
    sourceLayout->addWidget(_search);
    _available = new QTreeWidget(source);
    _available->setObjectName("panelAvailable");
    _available->setHeaderLabels({tr("Message"), tr("Signal"), tr("CAN ID"), tr("Interface")});
    _available->setRootIsDecorated(false);
    _available->setSelectionMode(QAbstractItemView::ExtendedSelection);
    sourceLayout->addWidget(_available);
    auto *add = new QPushButton(tr("Add Selected Signals"), source);
    add->setObjectName("panelAdd");
    sourceLayout->addWidget(add);
    auto *target = new QWidget(splitter);
    auto *targetLayout = new QVBoxLayout(target);
    _selected = new QTableWidget(0, 8, target);
    _selected->setObjectName("panelSelected");
    _selected->setHorizontalHeaderLabels({tr("Message"), tr("CAN ID"), tr("Interface"), tr("Signal"),
        tr("Value"), tr("Unit"), tr("Interval (ms)"), tr("State")});
    _selected->setSelectionBehavior(QAbstractItemView::SelectRows);
    _selected->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _selected->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    targetLayout->addWidget(_selected);
    auto *remove = new QPushButton(tr("Remove from Panel"), target);
    remove->setObjectName("panelRemove");
    targetLayout->addWidget(remove);
    splitter->addWidget(source); splitter->addWidget(target);
    splitter->setStretchFactor(1, 2);
    layout->addWidget(splitter, 1);
    _feedback = new QLabel(this);
    _feedback->setWordWrap(true);
    layout->addWidget(_feedback);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);
    connect(_search, &QLineEdit::textChanged, this, &TxSignalPanel::filter);
    connect(add, &QPushButton::clicked, this, &TxSignalPanel::addSelected);
    connect(remove, &QPushButton::clicked, this, &TxSignalPanel::removeSelected);
    connect(_available, &QTreeWidget::itemDoubleClicked, this, [this] { addSelected(); });
    connect(_selected, &QTableWidget::cellChanged, this, &TxSignalPanel::editValue);
}

void TxSignalPanel::synchronize(const QList<TxPanelMessage> &messages)
{
    QByteArray key;
    QDataStream stream(&key, QIODevice::WriteOnly);
    _messages.clear();
    for (const auto &message : messages) {
        _messages.insert(message.id, message);
        stream << message.id << message.name << message.interfaceName << message.frame.getId();
        if (message.databaseMessage)
            for (auto *signal : message.databaseMessage->getSignals()) stream << signal->name();
    }
    if (key != _candidateKey) {
        const QSignalBlocker blocker(_available);
        _available->clear();
        for (const auto &message : messages) {
            if (!message.databaseMessage) continue;
            for (auto *signal : message.databaseMessage->getSignals()) {
                auto *item = new QTreeWidgetItem(_available, {message.name, signal->name(),
                    QString("0x%1").arg(message.frame.getId(), 3, 16, QChar('0')), message.interfaceName});
                item->setData(0, Qt::UserRole, message.id);
            }
        }
        _candidateKey = key;
        filter();
    }
    const auto count = _pins.size();
    for (int index = _pins.size() - 1; index >= 0; --index) {
        const auto message = _messages.value(_pins[index].first);
        if (!message.databaseMessage || !message.databaseMessage->getSignalByName(_pins[index].second))
            _pins.removeAt(index);
    }
    if (count != _pins.size()) rebuildPins();
    refreshValues();
}

void TxSignalPanel::filter()
{
    for (int index = 0; index < _available->topLevelItemCount(); ++index) {
        auto *item = _available->topLevelItem(index);
        const bool visible = txGeneratorMatchesSearch(_search->text(),
            {item->text(0), item->text(1), item->text(2), item->text(3)});
        item->setHidden(!visible);
        if (!visible) item->setSelected(false);
    }
}

void TxSignalPanel::addSelected()
{
    for (auto *item : _available->selectedItems()) {
        const auto pin = qMakePair(item->data(0, Qt::UserRole).toUuid(), item->text(1));
        if (!item->isHidden() && !_pins.contains(pin)) _pins.append(pin);
    }
    rebuildPins();
}

void TxSignalPanel::removeSelected()
{
    for (int row = _pins.size() - 1; row >= 0; --row)
        if (_selected->selectionModel()->isRowSelected(row, QModelIndex())) _pins.removeAt(row);
    rebuildPins();
}

void TxSignalPanel::rebuildPins()
{
    const QSignalBlocker blocker(_selected);
    _selected->setRowCount(0);
    _selected->setRowCount(_pins.size());
    for (int row = 0; row < _pins.size(); ++row)
        for (int column = 0; column < 8; ++column) {
            auto *item = new QTableWidgetItem;
            if (column != 4) item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            _selected->setItem(row, column, item);
        }
    refreshValues();
}

void TxSignalPanel::refreshValues()
{
    const QSignalBlocker blocker(_selected);
    for (int row = 0; row < _pins.size(); ++row) {
        const auto message = _messages.value(_pins[row].first);
        auto *signal = message.databaseMessage->getSignalByName(_pins[row].second);
        const bool present = !message.frame.isRTR() && signal->isPresentInMessage(message.frame);
        const QStringList fields{message.name, QString("0x%1").arg(message.frame.getId(), 3, 16, QChar('0')),
            message.interfaceName, signal->name(), QString::number(signal->extractPhysicalFromMessage(message.frame), 'g', 15),
            signal->getUnit(), QString::number(message.interval),
            !present ? tr("Inactive signal") : (message.transmitting ? tr("Transmitting") : tr("Stopped"))};
        for (int column = 0; column < fields.size(); ++column) {
            if (column == 4 && _selected->currentRow() == row && _selected->currentColumn() == 4
                && QApplication::focusWidget() != _selected && _selected->isAncestorOf(QApplication::focusWidget()))
                continue;
            _selected->item(row, column)->setText(fields[column]);
        }
        auto flags = _selected->item(row, 4)->flags();
        _selected->item(row, 4)->setFlags(present ? flags | Qt::ItemIsEditable : flags & ~Qt::ItemIsEditable);
    }
}

void TxSignalPanel::editValue(int row, int column)
{
    if (column != 4 || row < 0 || row >= _pins.size()) return;
    const auto pin = _pins[row];
    auto message = _messages.value(pin.first);
    auto *signal = message.databaseMessage ? message.databaseMessage->getSignalByName(pin.second) : nullptr;
    bool ok = false;
    const double value = _selected->item(row, 4)->text().toDouble(&ok);
    if (!ok || !signal || !applyTxPanelSignal(message.frame, *signal, value)) {
        _feedback->setText(tr("Value rejected: check the signal range, numeric value, and multiplexer state."));
        const QSignalBlocker blocker(_selected);
        _selected->item(row, 4)->setText(signal ? QString::number(signal->extractPhysicalFromMessage(_messages.value(pin.first).frame), 'g', 15) : QString());
        return;
    }
    _feedback->clear();
    emit signalEdited(pin.first, pin.second, value);
    QTimer::singleShot(0, this, &TxSignalPanel::refreshValues);
}
