/**
 * @file filterblock.cpp
 * @brief 滤波/边缘检测处理块 —— UI 层封装 FilterAlgorithm
 *
 * 【在整条链路中的位置】
 *   拖入「滤波」块 → ImageProcessor 流水线节点
 *   支持平滑滤波（均值/高斯/中值）与边缘检测（Sobel/Laplacian/Prewitt/Roberts）
 *
 * 【数据流】
 *   QPixmap → cv::Mat(BGR) → RoiProcess::apply → FilterAlgorithm::apply → QPixmap
 *
 * 【设计说明】
 *   平滑类可多次迭代（iterations）；边缘类算法内部只执行一遍（见 filter.cpp）
 */

#include "filterblock.h"
#include "../utils/imageconverter.h"
#include "../utils/roiprocess.h"

/**
 * @brief 构造函数
 * 谁调用：Widget::createBlockByName
 */
FilterBlock::FilterBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupTitle(QStringLiteral("🌀"), AppConfig::BLOCK_NAME_FILTER);
    setupUI();
}

/**
 * @brief 创建滤波类型与核参数控件
 *
 * 参数：
 *   - 类型：7 种滤波/边缘算法
 *   - 核 X/Y：卷积核尺寸（边缘类实际用固定小核，此处主要影响平滑类）
 *   - 次数：平滑滤波重复应用次数
 */
void FilterBlock::setupUI()
{
    addSeparator();

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(QStringLiteral("均值滤波"), int(FilterAlgorithm::Type::Mean));
    m_typeCombo->addItem(QStringLiteral("高斯滤波"), int(FilterAlgorithm::Type::Gaussian));
    m_typeCombo->addItem(QStringLiteral("中值滤波"), int(FilterAlgorithm::Type::Median));
    m_typeCombo->addItem(QStringLiteral("Sobel"), int(FilterAlgorithm::Type::Sobel));
    m_typeCombo->addItem(QStringLiteral("Laplacian"), int(FilterAlgorithm::Type::Laplacian));
    m_typeCombo->addItem(QStringLiteral("Prewitt"), int(FilterAlgorithm::Type::Prewitt));
    m_typeCombo->addItem(QStringLiteral("Roberts"), int(FilterAlgorithm::Type::Roberts));
    contentLayout()->addWidget(m_typeCombo);

    // 简化版 Spin 行工厂（范围固定 1~31）
    auto addSpin = [&](const QString &label, int def) {
        auto *row = new QHBoxLayout();
        auto *lb = new QLabel(label, this);
        lb->setObjectName(QStringLiteral("blockFieldLabel"));
        lb->setFixedWidth(40);
        auto *sp = new QSpinBox(this);
        sp->setRange(1, 31);
        sp->setValue(def);
        sp->setFixedWidth(64);
        row->addWidget(lb);
        row->addWidget(sp);
        row->addStretch();
        contentLayout()->addLayout(row);
        return sp;
    };

    m_kxSpin = addSpin(QStringLiteral("核 X"), 3);
    m_kySpin = addSpin(QStringLiteral("核 Y"), 3);
    m_iterSpin = addSpin(QStringLiteral("次数"), 1);
    m_iterSpin->setRange(1, 20); // 迭代上限单独设置

    auto emitChange = [this](int) { emit paramsChanged(); };
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChange);
    connect(m_kxSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    connect(m_kySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    connect(m_iterSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
}

/**
 * @brief 读取当前滤波类型枚举
 */
FilterAlgorithm::Type FilterBlock::currentType() const
{
    return static_cast<FilterAlgorithm::Type>(m_typeCombo->currentData().toInt());
}

/**
 * @brief 执行滤波/边缘检测
 *
 * 谁调用：ImageProcessor::reprocess()
 * 输入：上一块 QPixmap + 当前 ROI
 * 输出：处理后的 QPixmap
 *
 * 与形态学块相同的转换套路：QPixmap↔Mat、RGB↔BGR、RoiProcess 包装
 */
QPixmap FilterBlock::process(const QPixmap &input, const RoiInfo &roi)
{
    if (input.isNull()) return input;
    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);
    cv::Mat out = RoiProcess::apply(src, roi, [&](const cv::Mat &m) {
        return FilterAlgorithm::apply(m, currentType(),
                                      m_kxSpin->value(), m_kySpin->value(),
                                      m_iterSpin->value());
    });
    return ImageConverter::matToPixmap(out);
}
