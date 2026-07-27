ROOT = $$PWD/../..
PLUGIN_TARGET = block_graytransform
PLUGIN_HEADERS = \
    graytransformplugin.h \
    graytransformblock.h \
    graytransform.h
PLUGIN_SOURCES = \
    graytransformplugin.cpp \
    graytransformblock.cpp \
    graytransform.cpp
PLUGIN_QM_SRC = $$PWD/i18n/block_graytransform_en.qm
include($$PWD/../plugin_common.pri)
