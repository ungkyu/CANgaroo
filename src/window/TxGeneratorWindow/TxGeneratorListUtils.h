#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QRegularExpression>
#include <QStringList>

inline bool txGeneratorMatchesSearch(const QString &query, const QStringList &fields)
{
    const auto words = query.trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    for (const auto &word : words) {
        bool found = false;
        for (const auto &field : fields)
            found |= field.contains(word, Qt::CaseInsensitive);
        if (!found)
            return false;
    }
    return true;
}

// Labels and run/stop state do not change the transmitted frame configuration.
// Distinct interfaces, frame formats, payloads and periods must be preserved.
inline QByteArray txGeneratorDuplicateKey(const QString &interfaceIdentity, quint32 id,
    bool extended, bool fd, bool brs, bool rtr, int interval, const QByteArray &payload)
{
    QByteArray key;
    QDataStream stream(&key, QIODevice::WriteOnly);
    stream << interfaceIdentity << id << extended << fd << brs << rtr << qint32(interval) << payload;
    return key;
}
