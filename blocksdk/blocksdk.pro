# 块基类 + 工具 + 插件管理（主程序与外部算法 DLL 共用）
QT += widgets
TEMPLATE = lib
CONFIG += shared c++17
DEFINES += BLOCKSDK_LIBRARY
TARGET = blocksdk

ROOT = $$PWD/..

win32-msvc*|win32-clang-msvc*|msvc {
    QMAKE_CXXFLAGS += /utf-8
    QMAKE_CFLAGS += /utf-8
}

INCLUDEPATH += \
    $$PWD \
    $$ROOT/core \
    $$ROOT/roi \
    $$ROOT/blocks \
    $$ROOT/utils \
    $$ROOT/config \
    D:/opencv/opencv/build/include

LIBS += -LD:/opencv/opencv/build/x64/vc16/lib
CONFIG(debug, debug|release) {
    LIBS += -lopencv_world4120d
} else {
    LIBS += -lopencv_world4120
}

# 与主程序输出到同一目录，便于运行时找到 blocksdk.dll
CONFIG(debug, debug|release) {
    DESTDIR = $$ROOT/bin/debug
} else {
    DESTDIR = $$ROOT/bin/release
}

HEADERS += \
    blocksdk_global.h \
    iblockhost.h \
    iblockplugin.h \
    pluginmanager.h \
    $$ROOT/blocks/baseblock.h \
    $$ROOT/utils/imageconverter.h \
    $$ROOT/utils/roiprocess.h \
    $$ROOT/utils/applogger.h \
    $$ROOT/utils/timemeasurer.h \
    $$ROOT/roi/roiinfo.h \
    $$ROOT/config/appconfig.h

SOURCES += \
    pluginmanager.cpp \
    $$ROOT/blocks/baseblock.cpp \
    $$ROOT/utils/imageconverter.cpp \
    $$ROOT/utils/roiprocess.cpp \
    $$ROOT/utils/applogger.cpp \
    $$ROOT/utils/timemeasurer.cpp
