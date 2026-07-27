# 算法插件公共片段。调用前设置：ROOT、PLUGIN_TARGET、PLUGIN_HEADERS、PLUGIN_SOURCES

isEmpty(ROOT): error(ROOT must be set)
isEmpty(PLUGIN_TARGET): error(PLUGIN_TARGET must be set)
isEmpty(PLUGIN_SOURCES): error(PLUGIN_SOURCES must be set)

QT += widgets
TEMPLATE = lib
CONFIG += plugin c++17
TARGET = $$PLUGIN_TARGET

win32-msvc*|win32-clang-msvc*|msvc {
    QMAKE_CXXFLAGS += /utf-8
    QMAKE_CFLAGS += /utf-8
}

# 每个算法的源码就在本插件目录；公共依赖走相对工程根路径
INCLUDEPATH += \
    $$PWD \
    $$ROOT/blocksdk \
    $$ROOT/blocks \
    $$ROOT/roi \
    $$ROOT/utils \
    $$ROOT/config \
    D:/opencv/opencv/build/include

LIBS += -LD:/opencv/opencv/build/x64/vc16/lib
CONFIG(debug, debug|release) {
    LIBS += -lopencv_world4120d
    DESTDIR = $$ROOT/bin/debug/plugins
    LIBS += -L$$ROOT/bin/debug -lblocksdk
} else {
    LIBS += -lopencv_world4120
    DESTDIR = $$ROOT/bin/release/plugins
    LIBS += -L$$ROOT/bin/release -lblocksdk
}

HEADERS += $$PLUGIN_HEADERS
SOURCES += $$PLUGIN_SOURCES

# 构建后把英文 .qm 拷到 DLL 同目录（约定：PLUGIN_QM_SRC = 插件目录/i18n/block_xxx_en.qm）
!isEmpty(PLUGIN_QM_SRC) {
    exists($$PLUGIN_QM_SRC) {
        win32 {
            QMAKE_POST_LINK += $$quote(cmd /c copy /Y $$shell_path($$PLUGIN_QM_SRC) $$shell_path($$DESTDIR)\\$$basename($$PLUGIN_QM_SRC))
        } else {
            QMAKE_POST_LINK += $$quote(cp -f $$PLUGIN_QM_SRC $$DESTDIR/)
        }
    }
}
