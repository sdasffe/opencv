#ifndef IBLOCKPLUGIN_H
#define IBLOCKPLUGIN_H

#include "democore_global.h"
#include "demoblock.h"

#include <QtPlugin>
#include <QString>

/**
 * @brief 插件合同：宿主只认这个接口，不认具体 BlurBlock 类名
 *
 * 每个算法 DLL 实现本接口，并在 create() 里 new 自己的 DemoBlock 子类。
 */
class IBlockPlugin
{
public:
    virtual ~IBlockPlugin() = default;

    virtual QString id() const = 0;            // 稳定 id，如 "blur"
    virtual QString displayName() const = 0;   // 列表显示名
    virtual DemoBlock *create(QWidget *parent = nullptr) = 0;
};

#define IBlockPlugin_iid "com.plugin_demo.IBlockPlugin/1.0"
Q_DECLARE_INTERFACE(IBlockPlugin, IBlockPlugin_iid)

#endif
