/**
 * @file graytransformblock.cpp
 * @brief 灰度变换处理块 —— 亮度/对比度/伽马/直方图等点运算与灰度转换
 *
 * 【在整条链路中的位置】
 *   常用于流水线早期：转灰度、增强对比度，为后续二值化/边缘检测做准备
 *
 * 【数据流】
 *   QPixmap → cv::Mat(BGR) → GrayTransformAlgorithm::apply → QPixmap
 *
 * 【UI 特点】
 *   不同变换类型需要的参数不同，updateParamEnabled() 动态启用/禁用控件
 */

#include "graytransformblock.h"
#include "../utils/imageconverter.h"
#include "../utils/roiprocess.h"

GrayTransformBlock::GrayTransformBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupTitle(QStringLiteral("🎚"), AppConfig::BLOCK_NAME_GRAYTRANSFORM);
    setupUI();
}

/**
 * @brief 构建变换类型与附加参数控件
 *
 * 附加参数：
 *   - 亮度/对比度：仅 BrightContrast 模式有效
 *   - 伽马：仅 Gamma 模式有效
 *   其他模式（转灰度、反转、对数、均衡、归一化）不需要这些 SpinBox
 */
void GrayTransformBlock::setupUI()
{
    addSeparator();

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(QStringLiteral("转灰度"), int(GrayTransformAlgorithm::Type::ToGray));
    m_typeCombo->addItem(QStringLiteral("亮度/对比度"), int(GrayTransformAlgorithm::Type::BrightContrast));
    m_typeCombo->addItem(QStringLiteral("反转"), int(GrayTransformAlgorithm::Type::Invert));
    m_typeCombo->addItem(QStringLiteral("对数变换"), int(GrayTransformAlgorithm::Type::Log));
    m_typeCombo->addItem(QStringLiteral("伽马变换"), int(GrayTransformAlgorithm::Type::Gamma));
    m_typeCombo->addItem(QStringLiteral("直方图均衡"), int(GrayTransformAlgorithm::Type::Equalize));
    m_typeCombo->addItem(QStringLiteral("归一化"), int(GrayTransformAlgorithm::Type::Normalize));
    contentLayout()->addWidget(m_typeCombo);

    // ----- 亮度 -----
    auto *bRow = new QHBoxLayout();
    auto *bLb = new QLabel(QStringLiteral("亮度"), this);
    bLb->setObjectName(QStringLiteral("blockFieldLabel"));
    bLb->setFixedWidth(40);
    m_brightSpin = new QSpinBox(this);
    m_brightSpin->setRange(-100, 100); // 对应算法层 convertTo 的 beta 偏移
    m_brightSpin->setValue(0);
    m_brightSpin->setFixedWidth(64);
    bRow->addWidget(bLb);
    bRow->addWidget(m_brightSpin);
    bRow->addStretch();
    contentLayout()->addLayout(bRow);

    // ----- 对比度（100 = 原图，0 = 全灰，>100 增强）-----
    auto *cRow = new QHBoxLayout();
    auto *cLb = new QLabel(QStringLiteral("对比度"), this);
    cLb->setObjectName(QStringLiteral("blockFieldLabel"));
    cLb->setFixedWidth(40);
    m_contrastSpin = new QSpinBox(this);
    m_contrastSpin->setRange(0, 300);
    m_contrastSpin->setValue(100);
    m_contrastSpin->setFixedWidth(64);
    cRow->addWidget(cLb);
    cRow->addWidget(m_contrastSpin);
    cRow->addStretch();
    contentLayout()->addLayout(cRow);

    // ----- 伽马（<1 提亮暗部，>1 压暗）-----
    auto *gRow = new QHBoxLayout();
    auto *gLb = new QLabel(QStringLiteral("伽马"), this);
    gLb->setObjectName(QStringLiteral("blockFieldLabel"));
    gLb->setFixedWidth(40);
    m_gammaSpin = new QDoubleSpinBox(this);
    m_gammaSpin->setRange(0.1, 5.0);
    m_gammaSpin->setSingleStep(0.1);
    m_gammaSpin->setValue(1.0);
    m_gammaSpin->setFixedWidth(64);
    gRow->addWidget(gLb);
    gRow->addWidget(m_gammaSpin);
    gRow->addStretch();
    contentLayout()->addLayout(gRow);

    auto emitChangeInt = [this](int) { emit paramsChanged(); };
    auto emitChangeD = [this](double) { emit paramsChanged(); };
    // 切换类型时先更新控件可用性，再触发重算
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        updateParamEnabled();
        emit paramsChanged();
    });
    connect(m_brightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChangeInt);
    connect(m_contrastSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChangeInt);
    connect(m_gammaSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, emitChangeD);

    updateParamEnabled(); // 初始状态：仅启用当前类型需要的控件
}

/**
 * @brief 根据当前变换类型，启用/禁用对应参数控件
 *
 * 为什么需要：避免用户在「转灰度」模式下误调伽马却看不到效果（算法层会忽略）
 * 何时调用：构造末尾、切换 typeCombo 时
 */
void GrayTransformBlock::updateParamEnabled()
{
    const auto t = currentType();
    const bool bc = (t == GrayTransformAlgorithm::Type::BrightContrast);
    const bool gm = (t == GrayTransformAlgorithm::Type::Gamma);
    m_brightSpin->setEnabled(bc);
    m_contrastSpin->setEnabled(bc);
    m_gammaSpin->setEnabled(gm);
}

GrayTransformAlgorithm::Type GrayTransformBlock::currentType() const
{
    return static_cast<GrayTransformAlgorithm::Type>(m_typeCombo->currentData().toInt());
}

/**
 * @brief 执行灰度变换
 *
 * 输入：QPixmap + RoiInfo
 * 输出：QPixmap（多数模式输出仍为 BGR 三通道，便于后续块统一处理）
 */
QPixmap GrayTransformBlock::process(const QPixmap &input, const RoiInfo &roi)
{
    if (input.isNull()) return input;
    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);
    cv::Mat out = RoiProcess::apply(src, roi, [&](const cv::Mat &m) {
        return GrayTransformAlgorithm::apply(
            m, currentType(),
            m_brightSpin->value(), m_contrastSpin->value(),
            m_gammaSpin->value());
    });
    return ImageConverter::matToPixmap(out);
}
