ROOT = $$PWD/../..
PLUGIN_TARGET = block_filter
PLUGIN_HEADERS = \
    filterplugin.h \
    filterblock.h \
    filter.h
PLUGIN_SOURCES = \
    filterplugin.cpp \
    filterblock.cpp \
    filter.cpp
PLUGIN_QM_SRC = $$PWD/i18n/block_filter_en.qm
include($$PWD/../plugin_common.pri)
