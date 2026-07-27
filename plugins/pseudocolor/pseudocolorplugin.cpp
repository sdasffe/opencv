#include "pseudocolorplugin.h"
#include "pseudocolorblock.h"
#include "appconfig.h"

QString PseudoColorPlugin::id() const
{
    return QString::fromUtf8(AppConfig::BLOCK_NAME_PSEUDOCOLOR);
}

BaseBlock *PseudoColorPlugin::createBlock(QWidget *parent)
{
    return new PseudoColorBlock(parent);
}
