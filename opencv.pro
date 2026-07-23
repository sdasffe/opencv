QT += widgets

CONFIG += c++17

# MSVC：源文件为 UTF-8，必须指定，否则中文 QStringLiteral 运行时乱码
# （算法块里中文最多，漏掉此选项时常见“只有块内乱码”）
win32-msvc*|win32-clang-msvc*|msvc {
    QMAKE_CXXFLAGS += /utf-8
    QMAKE_CFLAGS += /utf-8
}

# ========== OpenCV 配置 ==========
INCLUDEPATH += D:/opencv/opencv/build/include
LIBS += -LD:/opencv/opencv/build/x64/vc16/lib
LIBS += -lopencv_world4120d

# ========== 头文件搜索路径 ==========
INCLUDEPATH += $$PWD/core
INCLUDEPATH += $$PWD/roi
INCLUDEPATH += $$PWD/blocks
INCLUDEPATH += $$PWD/algorithms
INCLUDEPATH += $$PWD/utils
INCLUDEPATH += $$PWD/config
INCLUDEPATH += $$PWD/styles

# ========== 源文件 ==========
SOURCES += \
    main.cpp \
    core/widget.cpp \
    core/imageprocessor.cpp \
    roi/resizablerectitem.cpp \
    roi/resizableellipseitem.cpp \
    roi/resizablerotatedrectitem.cpp \
    blocks/baseblock.cpp \
    blocks/binarizationblock.cpp \
    blocks/morphologyblock.cpp \
    blocks/filterblock.cpp \
    blocks/graytransformblock.cpp \
    blocks/pseudocolorblock.cpp \
    blocks/glcmblock.cpp \
    algorithms/binarization.cpp \
    algorithms/otsu.cpp \
    algorithms/morphology.cpp \
    algorithms/filter.cpp \
    algorithms/graytransform.cpp \
    algorithms/pseudocolor.cpp \
    algorithms/glcm.cpp \
    utils/imageconverter.cpp \
    utils/timemeasurer.cpp \
    utils/roiprocess.cpp \
    utils/applogger.cpp \
    styles/styleloader.cpp

# ========== 头文件 ==========
HEADERS += \
    core/widget.h \
    core/imageprocessor.h \
    core/imagesession.h \
    roi/resizablerectitem.h \
    roi/resizableellipseitem.h \
    roi/resizablerotatedrectitem.h \
    roi/roiinfo.h \
    blocks/baseblock.h \
    blocks/binarizationblock.h \
    blocks/morphologyblock.h \
    blocks/filterblock.h \
    blocks/graytransformblock.h \
    blocks/pseudocolorblock.h \
    blocks/glcmblock.h \
    algorithms/binarization.h \
    algorithms/otsu.h \
    algorithms/morphology.h \
    algorithms/filter.h \
    algorithms/graytransform.h \
    algorithms/pseudocolor.h \
    algorithms/glcm.h \
    utils/imageconverter.h \
    utils/timemeasurer.h \
    utils/roiprocess.h \
    utils/applogger.h \
    config/appconfig.h \
    styles/styleloader.h

FORMS += widget.ui
RESOURCES += resources.qrc

# 版本信息（右键 exe → 属性 → 详细信息；与 AppConfig::APP_VERSION 一致）
VERSION = 1.0.0
win32: RC_FILE = version.rc

# 国际化：源语言中文，英文翻译见 i18n/opencv_en.ts
TRANSLATIONS += i18n/opencv_en.ts

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
