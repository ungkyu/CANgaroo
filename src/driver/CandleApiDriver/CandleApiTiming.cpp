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
    const uint64_t cycles = uint64_t(original.brp)
        * (1ULL + original.prop_seg + original.phase_seg1 + original.phase_seg2);
    if (!cycles || !caps.brp_min || !caps.brp_inc || !caps.sjw_max)
        return false;
    for (uint64_t brp = caps.brp_min; brp <= caps.brp_max; brp += caps.brp_inc) {
        if (cycles % brp)
            continue;
        const uint64_t tq = cycles / brp;
        if (tq % 5 || tq < 5)
            continue;
        const uint64_t tseg1 = tq * 4 / 5 - 1;
        const uint64_t tseg2 = tq / 5;
        if (tseg1 < 2 || tseg1 < caps.tseg1_min || tseg1 > caps.tseg1_max
            || tseg2 < caps.tseg2_min || tseg2 > caps.tseg2_max)
            continue;
        result = {1, uint32_t(tseg1 - 1), uint32_t(tseg2), 1, uint32_t(brp)};
        return true;
    }
    return false;
}
