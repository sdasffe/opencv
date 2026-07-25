QT += widgets
TEMPLATE = app
CONFIG += c++17
TARGET = plugin_host

win32-msvc*|win32-clang-msvc*|msvc {
    QMAKE_CXXFLAGS += /utf-8
    QMAKE_CFLAGS += /utf-8
}

INCLUDEPATH += $$PWD/../core
LIBS += -L$$PWD/../bin -ldemocore

HEADERS += mainwindow.h
SOURCES += main.cpp mainwindow.cpp

DESTDIR = $$PWD/../bin
