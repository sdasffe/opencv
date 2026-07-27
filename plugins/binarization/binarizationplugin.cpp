#include "binarizationplugin.h"
#include "binarizationblock.h"
#include "appconfig.h"

QString BinarizationPlugin::id() const
{
    return QString::fromUtf8(AppConfig::BLOCK_NAME_BINARIZATION);
}

BaseBlock *BinarizationPlugin::createBlock(QWidget *parent)
{
    return new BinarizationBlock(parent);
}
