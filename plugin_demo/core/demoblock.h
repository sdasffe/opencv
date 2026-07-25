#ifndef DEMOBLOCK_H
#define DEMOBLOCK_H

#include "democore_global.h"

#include <QWidget>
#include <QString>
#include <QVBoxLayout>
#include <QLabel>

/**
 * @brief 精简版「处理块」基类（类比你工程里的 BaseBlock）
 *
 * 插件里的具体算法块继承它；宿主拿到的永远是 DemoBlock*。
 */
class DEMOCORE_EXPORT DemoBlock : public QWidget
{
    Q_OBJECT
public:
    explicit DemoBlock(QWidget *parent = nullptr);
    ~DemoBlock() override = default;

    virtual QString blockId() const = 0;
    virtual QString blockTitle() const = 0;

    /** 假装处理：返回一句说明文字即可（演示用，不做真图像） */
    virtual QString process(const QString &input) = 0;

protected:
    QVBoxLayout *contentLayout() { return m_layout; }
    void setBlockTitle(const QString &title);

private:
    QVBoxLayout *m_layout = nullptr;
    QLabel *m_titleLabel = nullptr;
};

#endif
