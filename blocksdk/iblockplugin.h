#ifndef IBLOCKPLUGIN_H
#define IBLOCKPLUGIN_H

#include <QtPlugin>
#include <QString>

class BaseBlock;
class QWidget;

/**
 * @brief 外部算法插件接口（可选）
 *
 * 新算法做成 DLL：实现本接口，链接 blocksdk，放入 exe 旁 plugins/ 即可。
 * 现有内置块不必做成 DLL，用 PluginManager::registerBuiltin 注册。
 */
class IBlockPlugin
{
public:
    virtual ~IBlockPlugin() = default;
    virtual QString id() const = 0;
    virtual int order() const { return 100; }
    virtual BaseBlock *createBlock(QWidget *parent) = 0;
};

#define IBlockPlugin_iid "com.opencvlab.IBlockPlugin/1.0"
Q_DECLARE_INTERFACE(IBlockPlugin, IBlockPlugin_iid)

#endif
