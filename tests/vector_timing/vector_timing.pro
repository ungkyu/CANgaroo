TARGET = vector_timing_test
include(../common.pri)
SOURCES += test_vector_timing.cpp
win32 {
    QT += serialbus
    HEADERS += ../../src/driver/VectorDriver/qtvectorcan/vectorcanbackend.h
}
