# 公共库：宿主和插件都链接它（同一份 DemoBlock / 接口）
QT += widgets
TEMPLATE = lib
CONFIG += c++17 shared
TARGET = democore

DEFINES += DEMOCORE_LIBRARY

win32-msvc*|win32-clang-msvc*|msvc {
    QMAKE_CXXFLAGS += /utf-8
    QMAKE_CFLAGS += /utf-8
}

HEADERS += \
    democore_global.h \
    demoblock.h \
    iblockplugin.h

SOURCES += \
    demoblock.cpp

DESTDIR = $$PWD/../bin
