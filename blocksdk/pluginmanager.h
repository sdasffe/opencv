#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "blocksdk_global.h"
#include "iblockplugin.h"

#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>

class BaseBlock;
class IBlockHost;
class QTranslator;
class QWidget;

struct BLOCKSDK_EXPORT BlockPluginInfo
{
    QString id;
    int order = 100;
};

/**
 * @brief 算法块注册表：扫描 plugins/*.dll，并加载同名 *_en.qm 翻译
 */
class BLOCKSDK_EXPORT PluginManager
{
public:
    using Factory = std::function<BaseBlock *(QWidget *)>;

    static PluginManager &instance();

    void setHost(IBlockHost *host);
    IBlockHost *host() const { return m_host; }

    /** 主程序启动时注册内置块（当前工程已不用，保留接口） */
    void registerBuiltin(const QString &id, int order, Factory factory);

    /** 扫描目录加载插件 DLL；返回成功个数 */
    int loadFromDirectory(const QString &dirPath);

    /**
     * @brief 加载单个插件文件（设置→添加插件）
     * @return 成功返回 id；失败返回空并写入 lastError
     */
    QString loadPluginFile(const QString &absoluteDllPath);

    /** 默认插件目录：exe/plugins（不存在则创建） */
    static QString defaultPluginDirectory();

    /** 英文 UI 开/关：安装或卸载各插件自带的 *_en.qm */
    void setEnglishUi(bool english);
    bool englishUi() const { return m_englishUi; }

    QString lastError() const { return m_lastError; }

    QVector<BlockPluginInfo> infos() const;
    QStringList ids() const;

    BaseBlock *createBlock(const QString &id, QWidget *parent);

private:
    PluginManager() = default;
    Q_DISABLE_COPY(PluginManager)

    struct Entry {
        QString id;
        int order = 100;
        Factory factory;
        IBlockPlugin *plugin = nullptr;
        QString dllPath;
        QTranslator *translator = nullptr;
        bool translatorInstalled = false;
    };

    Entry *findEntry(const QString &id);
    const Entry *findEntry(const QString &id) const;
    void sortEntries();
    bool loadOnePlugin(const QString &absoluteDllPath, QString *outId = nullptr);
    void attachTranslation(Entry &e);

    IBlockHost *m_host = nullptr;
    bool m_englishUi = false;
    QString m_lastError;
    QVector<Entry> m_entries;
    QVector<class QPluginLoader *> m_loaders;
};

#endif
