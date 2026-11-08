// SPDX-License-Identifier: GPL-2.0-or-later
// Fork changes (2026-11-08): Windows build / CAN timing / Tx Generator development.
// See docs/windows-development_eng.md for the change record.
#pragma once
#include "core/BusMessage.h"

// Editing must not change routing, frame identity, or transmission state.
inline void applyTxLivePayload(BusMessage &target, const BusMessage &edited)
{
    for (int byte = 0; byte < target.getLength(); ++byte)
        target.setDataAt(byte, edited.getByte(byte));
}

inline void applyTxInterval(int &current, bool &enabled, int value)
{
    current = value > 0 ? value : 0;
    if (current == 0)
        enabled = false;
    // A positive interval never starts a stopped message.
}
