# 主程序空壳：界面 / ROI / 处理链；算法全部来自 plugins/*.dll
QT += widgets
TEMPLATE = app
CONFIG += c++17
TARGET = opencv

ROOT = $$PWD

win32-msvc*|win32-clang-msvc*|msvc {
    QMAKE_CXXFLAGS += /utf-8
    QMAKE_CFLAGS += /utf-8
}

INCLUDEPATH += \
    $$ROOT/blocksdk \
    $$ROOT/core \
    $$ROOT/roi \
    $$ROOT/blocks \
    $$ROOT/utils \
    $$ROOT/config \
    $$ROOT/styles \
    D:/opencv/opencv/build/include

LIBS += -LD:/opencv/opencv/build/x64/vc16/lib
CONFIG(debug, debug|release) {
    LIBS += -lopencv_world4120d
    DESTDIR = $$ROOT/bin/debug
} else {
    LIBS += -lopencv_world4120
    DESTDIR = $$ROOT/bin/release
}
LIBS += -L$$DESTDIR -lblocksdk

SOURCES += \
    main.cpp \
    core/widget.cpp \
    core/imageprocessor.cpp \
    roi/resizablerectitem.cpp \
    roi/resizableellipseitem.cpp \
    roi/resizablerotatedrectitem.cpp \
    styles/styleloader.cpp

HEADERS += \
    core/widget.h \
    core/imageprocessor.h \
    core/imagesession.h \
    roi/resizablerectitem.h \
    roi/resizableellipseitem.h \
    roi/resizablerotatedrectitem.h \
    roi/roiinfo.h \
    config/appconfig.h \
    styles/styleloader.h \
    blocksdk/pluginmanager.h \
    blocksdk/iblockhost.h

FORMS += widget.ui
RESOURCES += resources.qrc
VERSION = 1.0.0
win32: RC_FILE = version.rc
TRANSLATIONS += i18n/opencv_en.ts
