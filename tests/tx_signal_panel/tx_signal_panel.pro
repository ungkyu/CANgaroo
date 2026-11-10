# SPDX-License-Identifier: GPL-2.0-or-later
# Fork changes (2026-11-10): multi-message live signal panel widget tests.
TARGET = tx_signal_panel_test
include(../common.pri)
QT += widgets xml
SOURCES += TxSignalPanelTest.cpp \
    $$SRC_DIR/window/TxGeneratorWindow/TxSignalPanel.cpp \
    $$SRC_DIR/core/BusMessage.cpp \
    $$SRC_DIR/db/model/CanDb.cpp \
    $$SRC_DIR/db/model/CanDbMessage.cpp \
    $$SRC_DIR/db/model/CanDbNode.cpp \
    $$SRC_DIR/db/model/CanDbSignal.cpp
HEADERS += $$SRC_DIR/window/TxGeneratorWindow/TxSignalPanel.h
