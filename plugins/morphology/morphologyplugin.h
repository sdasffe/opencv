#ifndef MORPHOLOGYPLUGIN_H
#define MORPHOLOGYPLUGIN_H

#include "iblockplugin.h"
#include <QObject>

class MorphologyPlugin : public QObject, public IBlockPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IBlockPlugin_iid)
    Q_INTERFACES(IBlockPlugin)
public:
    QString id() const override;
    int order() const override { return 20; }
    BaseBlock *createBlock(QWidget *parent) override;
};

#endif
