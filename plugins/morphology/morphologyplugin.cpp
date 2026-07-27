#include "morphologyplugin.h"
#include "morphologyblock.h"
#include "appconfig.h"

QString MorphologyPlugin::id() const
{
    return QString::fromUtf8(AppConfig::BLOCK_NAME_MORPHOLOGY);
}

BaseBlock *MorphologyPlugin::createBlock(QWidget *parent)
{
    return new MorphologyBlock(parent);
}
