// SPDX-License-Identifier: GPL-2.0-or-later
// Fork changes (2026-11-08): Windows build / CAN timing / Tx Generator development.
// See docs/windows-development_eng.md for the change record.
#include "TxSignalListWidget.h"
#include "TxGeneratorListUtils.h"
#include "db/model/CanDbMessage.h"
#include "db/model/CanDbSignal.h"
#include <QHeaderView>
#include <QLineEdit>
#include <QSet>
#include <QSignalBlocker>
#include <QTreeWidget>
#include <QVBoxLayout>

TxSignalListWidget::TxSignalListWidget(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    _search = new QLineEdit(this);
    _search->setObjectName("signalSearch");
    _search->setPlaceholderText(tr("Signal name, message name or CAN ID..."));
    _search->setClearButtonEnabled(true);
    layout->addWidget(_search);
    _tree = new QTreeWidget(this);
    _tree->setObjectName("signalResults");
    _tree->setHeaderLabels({tr("Signal"), tr("Message"), tr("CAN ID"), tr("DBC")});
    _tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _tree->setMinimumHeight(0);
    _tree->setRootIsDecorated(false);
    _tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    layout->addWidget(_tree);
    connect(_search, &QLineEdit::textChanged, this, &TxSignalListWidget::applyFilter);
    connect(_tree, &QTreeWidget::itemSelectionChanged, this, &TxSignalListWidget::selectionChanged);
    connect(_tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *, int) {
        emit messageActivated();
    });
}

void TxSignalListWidget::setMessages(const QList<TxSignalMessageEntry> &entries)
{
    {
        const QSignalBlocker blocker(_tree);
        _tree->clear();
        for (const auto &entry : entries) {
            if (!entry.message)
                continue;
            const auto *message = entry.message;
            const QString id = QString("0x%1").arg(message->getRaw_id() & 0x1FFFFFFFu, 3, 16, QChar('0')).toUpper();
            for (const auto *signal : message->getSignals()) {
                auto *row = new QTreeWidgetItem(_tree,
                    {signal->name(), message->getName(), id, entry.database});
                row->setData(0, Qt::UserRole, QVariant::fromValue(static_cast<void *>(entry.message)));
                row->setData(2, Qt::UserRole, message->getRaw_id() & 0x1FFFFFFFu);
                row->setToolTip(0, signal->comment());
                row->setToolTip(3, entry.database);
            }
        }
        applyFilter();
    }
    emit selectionChanged();
}

void TxSignalListWidget::applyFilter()
{
    for (int index = 0; index < _tree->topLevelItemCount(); ++index) {
        auto *row = _tree->topLevelItem(index);
        const bool visible = txGeneratorMatchesSearch(_search->text(),
            {row->text(0), row->text(1), row->text(2), row->text(3),
             QString::number(row->data(2, Qt::UserRole).toUInt())});
        row->setHidden(!visible);
        if (!visible) {
            row->setSelected(false);
            if (_tree->currentItem() == row)
                _tree->setCurrentItem(nullptr);
        }
    }
}

QList<CanDbMessage *> TxSignalListWidget::selectedMessages() const
{
    QList<CanDbMessage *> result;
    QSet<CanDbMessage *> seen;
    for (auto *row : _tree->selectedItems()) {
        auto *message = static_cast<CanDbMessage *>(row->data(0, Qt::UserRole).value<void *>());
        if (!row->isHidden() && message && !seen.contains(message)) {
            result.append(message);
            seen.insert(message);
        }
    }
    return result;
}

CanDbMessage *TxSignalListWidget::currentMessage() const
{
    auto *row = _tree->currentItem();
    return row && !row->isHidden()
        ? static_cast<CanDbMessage *>(row->data(0, Qt::UserRole).value<void *>()) : nullptr;
}
