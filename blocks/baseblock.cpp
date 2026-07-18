#include "baseblock.h"

#include <QCursor>

BaseBlock::BaseBlock(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("BaseBlock"));

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 6, 8, 6);
    m_mainLayout->setSpacing(4);

    m_titleLayout = new QHBoxLayout();
    m_titleLayout->setSpacing(4);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName(QStringLiteral("blockIconLabel"));

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("blockTitleLabel"));

    m_enableCheckBox = new QCheckBox(this);
    m_enableCheckBox->setChecked(true);
    m_enableCheckBox->setText(QStringLiteral("开"));
    m_enableCheckBox->setToolTip(QStringLiteral("启用此处理块"));

    m_deleteBtn = new QPushButton(QStringLiteral("✕"), this);
    m_deleteBtn->setObjectName(QStringLiteral("blockDeleteBtn"));
    m_deleteBtn->setFixedSize(18, 18);
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    m_deleteBtn->setToolTip(QStringLiteral("删除此处理块"));

    m_titleLayout->addWidget(m_iconLabel);
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_enableCheckBox);
    m_titleLayout->addWidget(m_deleteBtn);

    m_mainLayout->addLayout(m_titleLayout);

    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setSpacing(3);
    m_mainLayout->addLayout(m_contentLayout);

    connect(m_deleteBtn, &QPushButton::clicked, this, &BaseBlock::removeRequested);
    connect(m_enableCheckBox, &QCheckBox::toggled, this, &BaseBlock::enabledChanged);
    connect(m_enableCheckBox, &QCheckBox::toggled, this, &BaseBlock::paramsChanged);

    initStyle();
}

void BaseBlock::setupTitle(const QString &icon, const QString &title)
{
    m_iconLabel->setText(icon);
    m_titleLabel->setText(title);
    m_enableCheckBox->setToolTip(QStringLiteral("启用%1").arg(title));
}

void BaseBlock::addSeparator()
{
    QFrame *line = new QFrame(this);
    line->setObjectName(QStringLiteral("blockSeparator"));
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    m_contentLayout->addWidget(line);
}

void BaseBlock::initStyle()
{
    // 样式由全局 app.qss 的 #BaseBlock 规则统一控制
    setAttribute(Qt::WA_StyledBackground, true);
    setMinimumHeight(0);
}
