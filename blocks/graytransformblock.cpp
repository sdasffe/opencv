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

#include <QJsonObject>

GrayTransformBlock::GrayTransformBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupUI();
    retranslateUi();
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
    for (int t : {int(GrayTransformAlgorithm::Type::ToGray),
                  int(GrayTransformAlgorithm::Type::BrightContrast),
                  int(GrayTransformAlgorithm::Type::Invert),
                  int(GrayTransformAlgorithm::Type::Log),
                  int(GrayTransformAlgorithm::Type::Gamma),
                  int(GrayTransformAlgorithm::Type::Equalize),
                  int(GrayTransformAlgorithm::Type::Normalize)}) {
        m_typeCombo->addItem(QString(), t);
    }
    contentLayout()->addWidget(m_typeCombo);

    auto *bRow = new QHBoxLayout();
    m_brightLabel = new QLabel(this);
    m_brightLabel->setObjectName(QStringLiteral("blockFieldLabel"));
    m_brightLabel->setFixedWidth(AppConfig::BLOCK_FIELD_LABEL_WIDTH);
    m_brightSpin = new QSpinBox(this);
    m_brightSpin->setRange(-100, 100);
    m_brightSpin->setValue(0);
    m_brightSpin->setFixedWidth(AppConfig::BLOCK_SPIN_WIDTH);
    bRow->addWidget(m_brightLabel);
    bRow->addWidget(m_brightSpin);
    bRow->addStretch();
    contentLayout()->addLayout(bRow);

    auto *cRow = new QHBoxLayout();
    m_contrastLabel = new QLabel(this);
    m_contrastLabel->setObjectName(QStringLiteral("blockFieldLabel"));
    m_contrastLabel->setFixedWidth(AppConfig::BLOCK_FIELD_LABEL_WIDTH);
    m_contrastSpin = new QSpinBox(this);
    m_contrastSpin->setRange(0, 300);
    m_contrastSpin->setValue(100);
    m_contrastSpin->setFixedWidth(AppConfig::BLOCK_SPIN_WIDTH);
    cRow->addWidget(m_contrastLabel);
    cRow->addWidget(m_contrastSpin);
    cRow->addStretch();
    contentLayout()->addLayout(cRow);

    auto *gRow = new QHBoxLayout();
    m_gammaLabel = new QLabel(this);
    m_gammaLabel->setObjectName(QStringLiteral("blockFieldLabel"));
    m_gammaLabel->setFixedWidth(AppConfig::BLOCK_FIELD_LABEL_WIDTH);
    m_gammaSpin = new QDoubleSpinBox(this);
    m_gammaSpin->setRange(0.1, 5.0);
    m_gammaSpin->setSingleStep(0.1);
    m_gammaSpin->setValue(1.0);
    m_gammaSpin->setFixedWidth(AppConfig::BLOCK_SPIN_WIDTH);
    gRow->addWidget(m_gammaLabel);
    gRow->addWidget(m_gammaSpin);
    gRow->addStretch();
    contentLayout()->addLayout(gRow);

    auto emitChangeInt = [this](int) { emit paramsChanged(); };
    auto emitChangeD = [this](double) { emit paramsChanged(); };
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        updateParamEnabled();
        emit paramsChanged();
    });
    connect(m_brightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChangeInt);
    connect(m_contrastSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChangeInt);
    connect(m_gammaSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, emitChangeD);

    updateParamEnabled();
}

void GrayTransformBlock::retranslateUi()
{
    setupTitle(QStringLiteral("🎚"), tr("灰度变换"));
    BaseBlock::retranslateUi();
    if (!m_typeCombo || m_typeCombo->count() < 7)
        return;
    m_typeCombo->setItemText(0, tr("转灰度"));
    m_typeCombo->setItemText(1, tr("亮度/对比度"));
    m_typeCombo->setItemText(2, tr("反转"));
    m_typeCombo->setItemText(3, tr("对数变换"));
    m_typeCombo->setItemText(4, tr("伽马变换"));
    m_typeCombo->setItemText(5, tr("直方图均衡"));
    m_typeCombo->setItemText(6, tr("归一化"));
    if (m_brightLabel)
        m_brightLabel->setText(tr("亮度"));
    if (m_contrastLabel)
        m_contrastLabel->setText(tr("对比度"));
    if (m_gammaLabel)
        m_gammaLabel->setText(tr("伽马"));
}

/** @brief 按变换类型启用/禁用亮度、对比度、伽马控件 */
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
QPixmap GrayTransformBlock::process(const QPixmap &input, const QList<RoiInfo> &rois)
{
    if (input.isNull()) return input;
    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);
    cv::Mat out = RoiProcess::apply(src, rois, [&](const cv::Mat &m) {
        return GrayTransformAlgorithm::apply(
            m, currentType(),
            m_brightSpin->value(), m_contrastSpin->value(),
            m_gammaSpin->value());
    });
    return ImageConverter::matToPixmap(out);
}

/** @brief 导出变换类型与附加参数到 JSON */
QJsonObject GrayTransformBlock::saveParams() const
{
    QJsonObject obj = BaseBlock::saveParams();
    obj.insert(QStringLiteral("type"), m_typeCombo->currentData().toInt());
    obj.insert(QStringLiteral("brightness"), m_brightSpin->value());
    obj.insert(QStringLiteral("contrast"), m_contrastSpin->value());
    obj.insert(QStringLiteral("gamma"), m_gammaSpin->value());
    return obj;
}

/** @brief 从 JSON 恢复参数；恢复后 updateParamEnabled 同步控件可用状态 */
void GrayTransformBlock::loadParams(const QJsonObject &obj)
{
    BaseBlock::loadParams(obj);
    const int type = obj.value(QStringLiteral("type")).toInt(m_typeCombo->currentData().toInt());
    const int idx = m_typeCombo->findData(type);
    m_typeCombo->blockSignals(true);
    m_brightSpin->blockSignals(true);
    m_contrastSpin->blockSignals(true);
    m_gammaSpin->blockSignals(true);
    if (idx >= 0)
        m_typeCombo->setCurrentIndex(idx);
    m_brightSpin->setValue(obj.value(QStringLiteral("brightness")).toInt(m_brightSpin->value()));
    m_contrastSpin->setValue(obj.value(QStringLiteral("contrast")).toInt(m_contrastSpin->value()));
    m_gammaSpin->setValue(obj.value(QStringLiteral("gamma")).toDouble(m_gammaSpin->value()));
    m_typeCombo->blockSignals(false);
    m_brightSpin->blockSignals(false);
    m_contrastSpin->blockSignals(false);
    m_gammaSpin->blockSignals(false);
    updateParamEnabled();
}
