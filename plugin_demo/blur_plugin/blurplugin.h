#ifndef BLURPLUGIN_H
#define BLURPLUGIN_H

#include "iblockplugin.h"

#include <QObject>

/**
 * @brief 插件入口：宿主通过 IBlockPlugin 调用，从不 #include "blurblock.h"
 */
class BlurPlugin : public QObject, public IBlockPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IBlockPlugin_iid)
    Q_INTERFACES(IBlockPlugin)

public:
    QString id() const override { return QStringLiteral("blur"); }
    QString displayName() const override { return QStringLiteral("模糊处理"); }
    DemoBlock *create(QWidget *parent) override;
};

#endif
