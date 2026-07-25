#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>

class QListWidget;
class QVBoxLayout;
class QLabel;
class IBlockPlugin;
class DemoBlock;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onCreateClicked();
    void onRunClicked();

private:
    void loadPlugins();

    QListWidget *m_list = nullptr;
    QWidget *m_blockHost = nullptr;
    QVBoxLayout *m_blockLayout = nullptr;
    QLabel *m_result = nullptr;
    DemoBlock *m_current = nullptr;

    QHash<QString, IBlockPlugin *> m_plugins; // id → 插件
};

#endif
