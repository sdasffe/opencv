#include "demoblock.h"

DemoBlock::DemoBlock(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet(QStringLiteral("font-weight:bold;"));
    m_layout->addWidget(m_titleLabel);
    setMinimumWidth(220);
    setStyleSheet(QStringLiteral(
        "DemoBlock { background:#f8fafc; border:1px solid #cbd5e1; border-radius:6px; }"
        "DemoBlock QLabel { color:#0f172a; }"));
}

void DemoBlock::setBlockTitle(const QString &title)
{
    m_titleLabel->setText(title);
}
