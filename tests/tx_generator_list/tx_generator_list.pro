# SPDX-License-Identifier: GPL-2.0-or-later
# Fork changes (2026-11-08): Windows build or CAN/Tx Generator test integration.
# See docs/windows-development_eng.md for the change record.
TARGET = tx_generator_list_test
include(../common.pri)
SOURCES += TxGeneratorListTest.cpp
