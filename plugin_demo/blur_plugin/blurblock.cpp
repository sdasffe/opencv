#include "blurblock.h"

#include <QHBoxLayout>

BlurBlock::BlurBlock(QWidget *parent)
    : DemoBlock(parent)
{
    setBlockTitle(QStringLiteral("模糊处理（来自插件 DLL）"));

    auto *row = new QWidget(this);
    auto *lay = new QHBoxLayout(row);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(new QLabel(QStringLiteral("半径"), row));
    m_radiusSpin = new QSpinBox(row);
    m_radiusSpin->setRange(1, 50);
    m_radiusSpin->setValue(3);
    lay->addWidget(m_radiusSpin);
    contentLayout()->addWidget(row);
}

QString BlurBlock::process(const QString &input)
{
    return QStringLiteral("%1 → 模糊(半径=%2) 【代码在 blur_plugin.dll】")
        .arg(input)
        .arg(m_radiusSpin->value());
}
