ROOT = $$PWD/../..
PLUGIN_TARGET = block_binarization
PLUGIN_HEADERS = \
    binarizationplugin.h \
    binarizationblock.h \
    binarization.h \
    otsu.h
PLUGIN_SOURCES = \
    binarizationplugin.cpp \
    binarizationblock.cpp \
    binarization.cpp \
    otsu.cpp
PLUGIN_QM_SRC = $$PWD/i18n/block_binarization_en.qm
include($$PWD/../plugin_common.pri)
