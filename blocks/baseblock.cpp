/**
 * @file baseblock.cpp
 * @brief 处理块基类实现
 *
 * 每个算法块（二值化/滤波/...）都是一个小面板：
 *   ┌─────────────────────────────┐
 *   │ 图标  标题        [开] [✕]  │  ← 标题栏（本类创建）
 *   ├─────────────────────────────┤
 *   │  子类自己的参数控件...       │  ← contentLayout()
 *   └─────────────────────────────┘
 *
 * 子类只要：
 *   1. 构造里 setupTitle + 往 contentLayout 加控件
 *   2. 实现 process() / blockName()
 *   3. 参数变了 emit paramsChanged()
 */

#include "baseblock.h"

#include <QCursor>
#include <QSizePolicy>

BaseBlock::BaseBlock(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("BaseBlock"));  // 供 app.qss 匹配样式

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 6, 8, 6);
    m_mainLayout->setSpacing(4);

    // ----- 标题栏：图标 | 名称 | 弹性空白 | 使能 | 删除 -----
    m_titleLayout = new QHBoxLayout();
    m_titleLayout->setSpacing(4);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName(QStringLiteral("blockIconLabel"));

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("blockTitleLabel"));

    m_enableCheckBox = new QCheckBox(this);
    m_enableCheckBox->setChecked(true);  // 默认启用，参与处理链
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

    // 子类通过 contentLayout() 往这里塞 SpinBox、ComboBox 等
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setSpacing(3);
    m_mainLayout->addLayout(m_contentLayout);

    // ✕ → Widget 从面板和 processor 里移除本块
    connect(m_deleteBtn, &QPushButton::clicked, this, &BaseBlock::removeRequested);
    // 开关 → ImageProcessor 决定是否跳过本块
    connect(m_enableCheckBox, &QCheckBox::toggled, this, &BaseBlock::enabledChanged);
    // 开关也算一种“参数变化”，直接触发重算
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
    // Minimum：高度不低于内容 sizeHint，多块时由外层 QScrollArea 滚动，不被挤扁
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
}
