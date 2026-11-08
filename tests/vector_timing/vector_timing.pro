# SPDX-License-Identifier: GPL-2.0-or-later
# Fork changes (2026-11-08): Windows build or CAN/Tx Generator test integration.
# See docs/windows-development_eng.md for the change record.
TARGET = vector_timing_test
include(../common.pri)
SOURCES += test_vector_timing.cpp
win32 {
    QT += serialbus
    HEADERS += ../../src/driver/VectorDriver/qtvectorcan/vectorcanbackend.h
}
