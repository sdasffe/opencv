QT += widgets

CONFIG += c++17

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
    algorithms/binarization.cpp \
    algorithms/otsu.cpp \
    algorithms/morphology.cpp \
    algorithms/filter.cpp \
    algorithms/graytransform.cpp \
    algorithms/pseudocolor.cpp \
    utils/imageconverter.cpp \
    utils/timemeasurer.cpp \
    utils/roiprocess.cpp \
    styles/styleloader.cpp

# ========== 头文件 ==========
HEADERS += \
    core/widget.h \
    core/imageprocessor.h \
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
    algorithms/binarization.h \
    algorithms/otsu.h \
    algorithms/morphology.h \
    algorithms/filter.h \
    algorithms/graytransform.h \
    algorithms/pseudocolor.h \
    utils/imageconverter.h \
    utils/timemeasurer.h \
    utils/roiprocess.h \
    config/appconfig.h \
    styles/styleloader.h

FORMS += widget.ui
RESOURCES += resources.qrc

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
