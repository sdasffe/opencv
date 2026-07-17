QT += widgets

CONFIG += c++17

# ========== OpenCV 配置 ==========
# 注意：当前使用 vc16 (VS 编译) 版本的库
INCLUDEPATH += D:/opencv/opencv/build/include
LIBS += -LD:/opencv/opencv/build/x64/vc16/lib
LIBS += -lopencv_world4120d

# ========== 头文件搜索路径（分层目录） ==========
INCLUDEPATH += $$PWD/core
INCLUDEPATH += $$PWD/roi
INCLUDEPATH += $$PWD/blocks
INCLUDEPATH += $$PWD/algorithms
INCLUDEPATH += $$PWD/utils
INCLUDEPATH += $$PWD/config

# ========== 源文件 ==========
SOURCES += \
    main.cpp \
    core/widget.cpp \
    core/imageprocessor.cpp \
    roi/resizablerectitem.cpp \
    roi/resizableellipseitem.cpp \
    blocks/baseblock.cpp \
    blocks/binarizationblock.cpp \
    algorithms/binarization.cpp \
    algorithms/otsu.cpp \
    utils/imageconverter.cpp \
    utils/timemeasurer.cpp

# ========== 头文件 ==========
HEADERS += \
    core/widget.h \
    core/imageprocessor.h \
    roi/resizablerectitem.h \
    roi/resizableellipseitem.h \
    blocks/baseblock.h \
    blocks/binarizationblock.h \
    algorithms/binarization.h \
    algorithms/otsu.h \
    utils/imageconverter.h \
    utils/timemeasurer.h \
    config/appconfig.h

# ========== UI 文件 ==========
FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
