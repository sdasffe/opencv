#include "blurplugin.h"
#include "blurblock.h"

DemoBlock *BlurPlugin::create(QWidget *parent)
{
    return new BlurBlock(parent);
}
