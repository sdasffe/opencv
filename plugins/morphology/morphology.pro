ROOT = $$PWD/../..
PLUGIN_TARGET = block_morphology
PLUGIN_HEADERS = \
    morphologyplugin.h \
    morphologyblock.h \
    morphology.h
PLUGIN_SOURCES = \
    morphologyplugin.cpp \
    morphologyblock.cpp \
    morphology.cpp
PLUGIN_QM_SRC = $$PWD/i18n/block_morphology_en.qm
include($$PWD/../plugin_common.pri)
