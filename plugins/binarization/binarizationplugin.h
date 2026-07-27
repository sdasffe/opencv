#ifndef BINARIZATIONPLUGIN_H
#define BINARIZATIONPLUGIN_H

#include "iblockplugin.h"
#include <QObject>

class BinarizationPlugin : public QObject, public IBlockPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IBlockPlugin_iid)
    Q_INTERFACES(IBlockPlugin)
public:
    QString id() const override;
    int order() const override { return 10; }
    BaseBlock *createBlock(QWidget *parent) override;
};

#endif
