#ifndef BLURBLOCK_H
#define BLURBLOCK_H

#include "demoblock.h"

#include <QSpinBox>
#include <QLabel>

/** 插件内部的具体块（宿主源码里没有这个类） */
class BlurBlock : public DemoBlock
{
    Q_OBJECT
public:
    explicit BlurBlock(QWidget *parent = nullptr);

    QString blockId() const override { return QStringLiteral("blur"); }
    QString blockTitle() const override { return QStringLiteral("模糊处理"); }
    QString process(const QString &input) override;

private:
    QSpinBox *m_radiusSpin = nullptr;
};

#endif
