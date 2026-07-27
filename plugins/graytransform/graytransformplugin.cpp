#include "graytransformplugin.h"
#include "graytransformblock.h"
#include "appconfig.h"

QString GrayTransformPlugin::id() const
{
    return QString::fromUtf8(AppConfig::BLOCK_NAME_GRAYTRANSFORM);
}

BaseBlock *GrayTransformPlugin::createBlock(QWidget *parent)
{
    return new GrayTransformBlock(parent);
}
