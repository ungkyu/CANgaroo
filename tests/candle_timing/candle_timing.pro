# SPDX-License-Identifier: GPL-2.0-or-later
# Fork changes (2026-11-08): Windows build or CAN/Tx Generator test integration.
# See docs/windows-development_eng.md for the change record.
include(../common.pri)
TARGET = candle_timing_test
SOURCES += test_candle_timing.cpp ../../src/driver/CandleApiDriver/CandleApiTiming.cpp
