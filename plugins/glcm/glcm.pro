ROOT = $$PWD/../..
PLUGIN_TARGET = block_glcm
PLUGIN_HEADERS = \
    glcmplugin.h \
    glcmblock.h \
    glcm.h
PLUGIN_SOURCES = \
    glcmplugin.cpp \
    glcmblock.cpp \
    glcm.cpp
PLUGIN_QM_SRC = $$PWD/i18n/block_glcm_en.qm
include($$PWD/../plugin_common.pri)
