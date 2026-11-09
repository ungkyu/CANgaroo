// Fork changes (2026-11-08): Windows build / CAN timing / Tx Generator development.
// See docs/windows-development_eng.md for the change record.
#include "CandleApiTiming.h"

CandleApiTiming::CandleApiTiming(uint32_t baseClk, uint32_t bitrate, uint32_t samplePoint, uint32_t brp, uint32_t phase_seg1, uint32_t phase_seg2)
  : _baseClk(baseClk),
    _bitrate(bitrate),
    _samplePoint(samplePoint)
{
    _timing.brp = brp;
    _timing.phase_seg1 = phase_seg1;
    _timing.phase_seg2 = phase_seg2;
    _timing.sjw = 1;
    _timing.prop_seg = 1;
}

uint32_t CandleApiTiming::getBaseClk() const
{
    return _baseClk;
}

uint32_t CandleApiTiming::getBitrate() const
{
    return _bitrate;
}

uint32_t CandleApiTiming::getSamplePoint() const
{
    return _samplePoint;
}

candle_bittiming_t CandleApiTiming::getTiming() const
{
    return _timing;
}

bool CandleApiTiming::with80Percent(const candle_capability_t &caps,
                                  const candle_bittiming_t &original,
                                  candle_bittiming_t &result)
{
    return withSamplePoint(caps, original, 800, result);
}

// Fork changes (2026-11-09): add exact 75% CAN FD data-phase timings.
bool CandleApiTiming::with75PercentData(const candle_bittiming_t &original,
                                      candle_bittiming_t &result)
{
    // Conservative data-phase limits used by the existing FD timing table.
    candle_capability_t limits{};
    limits.brp_min = 1; limits.brp_max = 32; limits.brp_inc = 1;
    limits.tseg1_min = 2; limits.tseg1_max = 31;
    limits.tseg2_min = 1; limits.tseg2_max = 16; limits.sjw_max = 1;
    return withSamplePoint(limits, original, 750, result);
}

bool CandleApiTiming::withSamplePoint(const candle_capability_t &caps,
                                    const candle_bittiming_t &original,
                                    uint32_t samplePoint, candle_bittiming_t &result)
{
    const uint64_t cycles = uint64_t(original.brp)
        * (1ULL + original.prop_seg + original.phase_seg1 + original.phase_seg2);
    if (!cycles || !caps.brp_min || !caps.brp_inc || !caps.sjw_max
        || samplePoint == 0 || samplePoint >= 1000)
        return false;
    for (uint64_t brp = caps.brp_min; brp <= caps.brp_max; brp += caps.brp_inc) {
        if (cycles % brp)
            continue;
        const uint64_t tq = cycles / brp;
        if (tq < 3 || (tq * samplePoint) % 1000)
            continue;
        const uint64_t tseg1 = tq * samplePoint / 1000 - 1;
        const uint64_t tseg2 = tq - tseg1 - 1;
        if (tseg1 < 2 || tseg1 < caps.tseg1_min || tseg1 > caps.tseg1_max
            || tseg2 < caps.tseg2_min || tseg2 > caps.tseg2_max)
            continue;
        result = {1, uint32_t(tseg1 - 1), uint32_t(tseg2), 1, uint32_t(brp)};
        return true;
    }
    return false;
}
