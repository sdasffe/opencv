#ifndef GLCMPLUGIN_H
#define GLCMPLUGIN_H

#include "iblockplugin.h"
#include <QObject>

class GlcmPlugin : public QObject, public IBlockPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IBlockPlugin_iid)
    Q_INTERFACES(IBlockPlugin)
public:
    QString id() const override;
    int order() const override { return 60; }
    BaseBlock *createBlock(QWidget *parent) override;
};

#endif
