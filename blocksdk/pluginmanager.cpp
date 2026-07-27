#include "pluginmanager.h"
#include "baseblock.h"
#include "applogger.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QPluginLoader>
#include <QTranslator>
#include <QtGlobal>
#include <algorithm>

PluginManager &PluginManager::instance()
{
    static PluginManager s;
    return s;
}

void PluginManager::setHost(IBlockHost *host)
{
    m_host = host;
}

QString PluginManager::defaultPluginDirectory()
{
    const QString dir = QCoreApplication::applicationDirPath()
                        + QStringLiteral("/plugins");
    QDir().mkpath(dir);
    return dir;
}

void PluginManager::sortEntries()
{
    std::sort(m_entries.begin(), m_entries.end(),
              [](const Entry &a, const Entry &b) {
        if (a.order != b.order)
            return a.order < b.order;
        return a.id < b.id;
    });
}

PluginManager::Entry *PluginManager::findEntry(const QString &id)
{
    for (Entry &e : m_entries) {
        if (e.id == id)
            return &e;
    }
    return nullptr;
}

const PluginManager::Entry *PluginManager::findEntry(const QString &id) const
{
    for (const Entry &e : m_entries) {
        if (e.id == id)
            return &e;
    }
    return nullptr;
}

void PluginManager::registerBuiltin(const QString &id, int order, Factory factory)
{
    if (id.isEmpty() || !factory)
        return;
    if (findEntry(id)) {
        AppLogger::warn(QStringLiteral("内置块 id 重复"), id);
        return;
    }
    Entry e;
    e.id = id;
    e.order = order;
    e.factory = std::move(factory);
    m_entries.append(e);
    sortEntries();
}

void PluginManager::attachTranslation(Entry &e)
{
    if (e.dllPath.isEmpty() || e.translator)
        return;

    const QFileInfo fi(e.dllPath);
    // 约定：block_xxx.dll → block_xxx_en.qm（与 DLL 同目录）
    const QString qmPath = fi.absolutePath() + QLatin1Char('/')
                           + fi.completeBaseName() + QStringLiteral("_en.qm");
    if (!QFileInfo::exists(qmPath)) {
        AppLogger::info(QStringLiteral("插件无英文翻译"),
                        QStringLiteral("%1（可选）").arg(qmPath));
        return;
    }

    auto *tr = new QTranslator(qApp);
    if (!tr->load(qmPath)) {
        AppLogger::warn(QStringLiteral("插件翻译加载失败"), qmPath);
        delete tr;
        return;
    }
    e.translator = tr;
    if (m_englishUi) {
        qApp->installTranslator(tr);
        e.translatorInstalled = true;
    }
    AppLogger::info(QStringLiteral("已加载插件翻译"), qmPath);
}

bool PluginManager::loadOnePlugin(const QString &absoluteDllPath, QString *outId)
{
    m_lastError.clear();
    const QFileInfo fi(absoluteDllPath);
    if (!fi.exists() || !fi.isFile()) {
        m_lastError = QStringLiteral("文件不存在：%1").arg(absoluteDllPath);
        return false;
    }

    auto *loader = new QPluginLoader(fi.absoluteFilePath());
    QObject *obj = loader->instance();
    if (!obj) {
        m_lastError = QStringLiteral("%1 | %2").arg(fi.fileName(), loader->errorString());
        AppLogger::warn(QStringLiteral("插件加载失败"), m_lastError);
        delete loader;
        return false;
    }

    IBlockPlugin *plugin = qobject_cast<IBlockPlugin *>(obj);
    if (!plugin) {
        m_lastError = QStringLiteral("未实现 IBlockPlugin：%1").arg(fi.fileName());
        AppLogger::warn(QStringLiteral("非算法插件"), fi.fileName());
        loader->unload();
        delete loader;
        return false;
    }

    if (findEntry(plugin->id())) {
        m_lastError = QStringLiteral("插件 id 已存在：%1").arg(plugin->id());
        AppLogger::warn(QStringLiteral("插件 id 冲突"), plugin->id());
        loader->unload();
        delete loader;
        return false;
    }

    Entry e;
    e.id = plugin->id();
    e.order = plugin->order();
    e.plugin = plugin;
    e.dllPath = fi.absoluteFilePath();
    e.factory = [plugin](QWidget *parent) { return plugin->createBlock(parent); };
    attachTranslation(e);
    m_entries.append(e);
    m_loaders.append(loader);
    sortEntries();

    if (outId)
        *outId = e.id;
    AppLogger::info(QStringLiteral("已加载外部插件"),
                    QStringLiteral("%1 → %2").arg(fi.fileName(), e.id));
    return true;
}

int PluginManager::loadFromDirectory(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists())
        return 0;

    int ok = 0;
    for (const QString &fileName : dir.entryList(QDir::Files)) {
#if defined(Q_OS_WIN)
        if (!fileName.endsWith(QLatin1String(".dll"), Qt::CaseInsensitive))
            continue;
#elif defined(Q_OS_MACOS)
        if (!fileName.endsWith(QLatin1String(".dylib")))
            continue;
#else
        if (!fileName.endsWith(QLatin1String(".so")))
            continue;
#endif
        if (fileName.startsWith(QLatin1String("blocksdk"), Qt::CaseInsensitive)
            || fileName.startsWith(QLatin1String("opencv"), Qt::CaseInsensitive)
            || fileName.startsWith(QLatin1String("Qt"), Qt::CaseInsensitive))
            continue;

        if (loadOnePlugin(dir.absoluteFilePath(fileName)))
            ++ok;
    }
    return ok;
}

QString PluginManager::loadPluginFile(const QString &absoluteDllPath)
{
    QString id;
    if (!loadOnePlugin(absoluteDllPath, &id))
        return QString();
    return id;
}

void PluginManager::setEnglishUi(bool english)
{
    m_englishUi = english;
    for (Entry &e : m_entries) {
        if (!e.translator)
            continue;
        if (english) {
            if (!e.translatorInstalled) {
                qApp->installTranslator(e.translator);
                e.translatorInstalled = true;
            }
        } else if (e.translatorInstalled) {
            qApp->removeTranslator(e.translator);
            e.translatorInstalled = false;
        }
    }
}

QVector<BlockPluginInfo> PluginManager::infos() const
{
    QVector<BlockPluginInfo> list;
    list.reserve(m_entries.size());
    for (const Entry &e : m_entries)
        list.append({e.id, e.order});
    return list;
}

QStringList PluginManager::ids() const
{
    QStringList list;
    for (const Entry &e : m_entries)
        list.append(e.id);
    return list;
}

BaseBlock *PluginManager::createBlock(const QString &id, QWidget *parent)
{
    Entry *e = findEntry(id);
    if (!e || !e->factory)
        return nullptr;
    BaseBlock *block = e->factory(parent);
    if (block)
        block->setHost(m_host);
    return block;
}
