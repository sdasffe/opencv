#include "baseblock.h"

BaseBlock::BaseBlock(QWidget *parent)
    : QWidget(parent)
{
    // 主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(12, 8, 12, 8);
    m_mainLayout->setSpacing(6);

    // 标题栏
    m_titleLayout = new QHBoxLayout();

    m_iconLabel = new QLabel(this);
    m_iconLabel->setStyleSheet("font-size: 18px; border: none;");

    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet(
        "font-size: 14px;"
        "font-weight: bold;"
        "color: #333;"
        "border: none;"
        );

    m_enableCheckBox = new QCheckBox(this);
    m_enableCheckBox->setChecked(true);

    m_deleteBtn = new QPushButton("✕", this);
    m_deleteBtn->setFixedSize(24, 24);
    m_deleteBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #ff5252;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 12px;"
        "    font-size: 12px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #ff1744; }"
        );

    m_titleLayout->addWidget(m_iconLabel);
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_enableCheckBox);
    m_titleLayout->addWidget(m_deleteBtn);

    m_mainLayout->addLayout(m_titleLayout);

    // 内容布局（子类填充）
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setSpacing(6);
    m_mainLayout->addLayout(m_contentLayout);

    // 信号连接
    connect(m_deleteBtn, &QPushButton::clicked, this, &BaseBlock::removeRequested);
    connect(m_enableCheckBox, &QCheckBox::toggled, this, &BaseBlock::enabledChanged);
    connect(m_enableCheckBox, &QCheckBox::toggled, this, &BaseBlock::paramsChanged);

    initStyle();
}

void BaseBlock::setupTitle(const QString &icon, const QString &title)
{
    m_iconLabel->setText(icon);
    m_titleLabel->setText(title);
    m_enableCheckBox->setToolTip(QString("启用%1").arg(title));
}

void BaseBlock::addSeparator()
{
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #eee; border: none;");
    m_contentLayout->addWidget(line);
}

void BaseBlock::initStyle()
{
    setStyleSheet(
        "QWidget {"
        "    background-color: white;"
        "    border: 1px solid #ddd;"
        "    border-radius: 8px;"
        "}"
        );
    setMinimumHeight(120);
}
