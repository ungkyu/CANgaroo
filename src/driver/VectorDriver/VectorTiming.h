// SPDX-License-Identifier: GPL-2.0-or-later
// Fork changes (2026-11-08): Windows build / CAN timing / Tx Generator development.
// See docs/windows-development_eng.md for the change record.
#pragma once
#include <cstdint>

// Split one bit into exact integer TQ segments. Values are per-mille.
inline bool vectorTimingSegments(uint32_t tq, uint32_t samplePoint,
                                 uint32_t &tseg1, uint32_t &tseg2)
{
    if (samplePoint == 0 || samplePoint >= 1000 || tq < 3
        || (uint64_t(tq) * samplePoint) % 1000 != 0)
        return false;
    const uint32_t sampleTq = uint64_t(tq) * samplePoint / 1000;
    if (sampleTq < 2 || sampleTq >= tq)
        return false;
    tseg1 = sampleTq - 1;
    tseg2 = tq - sampleTq;
    return true;
}
