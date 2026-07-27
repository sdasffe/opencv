#include "filterplugin.h"
#include "filterblock.h"
#include "appconfig.h"

QString FilterPlugin::id() const
{
    return QString::fromUtf8(AppConfig::BLOCK_NAME_FILTER);
}

BaseBlock *FilterPlugin::createBlock(QWidget *parent)
{
    return new FilterBlock(parent);
}
