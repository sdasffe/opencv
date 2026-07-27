#include "glcmplugin.h"
#include "glcmblock.h"
#include "appconfig.h"

QString GlcmPlugin::id() const
{
    return QString::fromUtf8(AppConfig::BLOCK_NAME_GLCM);
}

BaseBlock *GlcmPlugin::createBlock(QWidget *parent)
{
    return new GlcmBlock(parent);
}
