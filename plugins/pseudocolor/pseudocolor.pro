ROOT = $$PWD/../..
PLUGIN_TARGET = block_pseudocolor
PLUGIN_HEADERS = \
    pseudocolorplugin.h \
    pseudocolorblock.h \
    pseudocolor.h
PLUGIN_SOURCES = \
    pseudocolorplugin.cpp \
    pseudocolorblock.cpp \
    pseudocolor.cpp
PLUGIN_QM_SRC = $$PWD/i18n/block_pseudocolor_en.qm
include($$PWD/../plugin_common.pri)
