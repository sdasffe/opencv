#include "mainwindow.h"
#include "iblockplugin.h"
#include "demoblock.h"

#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPluginLoader>
#include <QDir>
#include <QCoreApplication>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("插件演示宿主（与主工程无关）"));
    resize(640, 420);

    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *root = new QHBoxLayout(central);

    // 左侧：插件扫出来的算法名
    auto *left = new QVBoxLayout;
    left->addWidget(new QLabel(QStringLiteral("已加载插件（exe 旁 *_plugin.dll）"), central));
    m_list = new QListWidget(central);
    left->addWidget(m_list);
    auto *btnCreate = new QPushButton(QStringLiteral("创建选中算法块"), central);
    auto *btnRun = new QPushButton(QStringLiteral("调用 process()"), central);
    left->addWidget(btnCreate);
    left->addWidget(btnRun);
    root->addLayout(left, 1);

    // 右侧：块面板 + 结果
    auto *right = new QVBoxLayout;
    right->addWidget(new QLabel(QStringLiteral("块实例（宿主不知道 BlurBlock 类名）"), central));
    m_blockHost = new QWidget(central);
    m_blockLayout = new QVBoxLayout(m_blockHost);
    m_blockLayout->addStretch();
    right->addWidget(m_blockHost, 1);
    m_result = new QLabel(QStringLiteral("结果：—"), central);
    m_result->setWordWrap(true);
    right->addWidget(m_result);
    root->addLayout(right, 2);

    connect(btnCreate, &QPushButton::clicked, this, &MainWindow::onCreateClicked);
    connect(btnRun, &QPushButton::clicked, this, &MainWindow::onRunClicked);

    loadPlugins();
}

void MainWindow::loadPlugins()
{
    m_list->clear();
    m_plugins.clear();

    // 与 exe、democore.dll 同目录下的 *_plugin.dll（演示简化，避免 plugins 子目录找依赖失败）
    const QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir);

    const QStringList files = dir.entryList(QStringList() << QStringLiteral("*_plugin.dll"),
                                            QDir::Files);
    for (const QString &file : files) {
        const QString path = dir.absoluteFilePath(file);
        auto *loader = new QPluginLoader(path, this);
        QObject *obj = loader->instance();
        if (!obj) {
            qWarning() << "加载失败" << path << loader->errorString();
            continue;
        }

        IBlockPlugin *plugin = qobject_cast<IBlockPlugin *>(obj);
        if (!plugin) {
            qWarning() << "不是 IBlockPlugin" << path;
            continue;
        }

        m_plugins.insert(plugin->id(), plugin);
        auto *item = new QListWidgetItem(plugin->displayName(), m_list);
        item->setData(Qt::UserRole, plugin->id());
        qInfo() << "已加载插件" << plugin->id() << path;
    }

    m_result->setText(QStringLiteral("已加载 %1 个插件（目录：%2）。选中后点「创建」。")
                          .arg(m_plugins.size())
                          .arg(appDir));
}

void MainWindow::onCreateClicked()
{
    QListWidgetItem *item = m_list->currentItem();
    if (!item) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选中一个插件"));
        return;
    }

    const QString id = item->data(Qt::UserRole).toString();
    IBlockPlugin *plugin = m_plugins.value(id);
    if (!plugin)
        return;

    // ★ 关键：宿主从不写 new BlurBlock，只调接口
    DemoBlock *block = plugin->create(m_blockHost);

    if (m_current) {
        m_blockLayout->removeWidget(m_current);
        m_current->deleteLater();
        m_current = nullptr;
    }
    m_current = block;
    m_blockLayout->insertWidget(0, m_current);
    m_result->setText(QStringLiteral("已创建块：%1（类型对宿主只是 DemoBlock*）")
                          .arg(m_current->blockTitle()));
}

void MainWindow::onRunClicked()
{
    if (!m_current) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先创建块"));
        return;
    }
    const QString out = m_current->process(QStringLiteral("原图"));
    m_result->setText(QStringLiteral("结果：%1").arg(out));
}
