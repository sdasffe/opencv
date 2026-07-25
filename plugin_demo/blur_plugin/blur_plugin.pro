# 模糊插件 DLL（QPluginLoader 加载）
QT += widgets
TEMPLATE = lib
CONFIG += c++17 plugin
TARGET = blur_plugin

win32-msvc*|win32-clang-msvc*|msvc {
    QMAKE_CXXFLAGS += /utf-8
    QMAKE_CFLAGS += /utf-8
}

INCLUDEPATH += $$PWD/../core
LIBS += -L$$PWD/../bin -ldemocore

HEADERS += \
    blurblock.h \
    blurplugin.h

SOURCES += \
    blurblock.cpp \
    blurplugin.cpp

# 与 host、democore 同目录，避免 Windows 加载插件时找不到 democore.dll
DESTDIR = $$PWD/../bin
