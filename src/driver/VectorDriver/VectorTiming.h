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
