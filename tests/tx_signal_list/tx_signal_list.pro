# SPDX-License-Identifier: GPL-2.0-or-later
# Fork changes (2026-11-08): Windows build or CAN/Tx Generator test integration.
# See docs/windows-development_eng.md for the change record.
TARGET = tx_signal_list_test
include(../common.pri)
QT += widgets xml
SOURCES += TxSignalListTest.cpp \
    $$SRC_DIR/window/TxGeneratorWindow/TxSignalListWidget.cpp \
    $$SRC_DIR/core/BusMessage.cpp \
    $$SRC_DIR/db/model/CanDb.cpp \
    $$SRC_DIR/db/model/CanDbMessage.cpp \
    $$SRC_DIR/db/model/CanDbNode.cpp \
    $$SRC_DIR/db/model/CanDbSignal.cpp
HEADERS += $$SRC_DIR/window/TxGeneratorWindow/TxSignalListWidget.h
