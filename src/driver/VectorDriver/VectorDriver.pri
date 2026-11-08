CONFIG += c++20

# Qt Serial Bus API with an application-local Vector backend on Windows.
# The Vector XL Driver Library must be installed on the target machine.
QT += serialbus
DEFINES += VECTOR_DRIVER

SOURCES += \
    $$PWD/VectorInterface.cpp \
    $$PWD/VectorDriver.cpp

win32 {
SOURCES += $$PWD/qtvectorcan/vectorcanbackend.cpp
HEADERS += $$PWD/qtvectorcan/vectorcanbackend.h \
    $$PWD/qtvectorcan/vectorcanbackend_p.h \
    $$PWD/qtvectorcan/vectorcan_symbols_p.h \
    $$PWD/VectorTiming.h
}

HEADERS += \
    $$PWD/VectorInterface.h \
    $$PWD/VectorDriver.h

FORMS +=
