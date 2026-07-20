/**
 * @file pseudocolorblock.cpp
 * @brief 伪彩色处理块 —— 将灰度强度映射为彩色，便于可视化
 *
 * 【在整条链路中的位置】
 *   通常放在流水线末尾或中间：把灰度/强度图转成 Jet/Hot 等色图
 *
 * 【数据流】
 *   QPixmap → cv::Mat(BGR) → 算法内转灰度 → applyColorMap → QPixmap
 *
 * 【参数】
 *   仅一个 colormap 下拉框，无核大小/迭代等额外参数
 */

#include "pseudocolorblock.h"
#include "../utils/imageconverter.h"
#include "../utils/roiprocess.h"

PseudoColorBlock::PseudoColorBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupTitle(QStringLiteral("🌈"), AppConfig::BLOCK_NAME_PSEUDOCOLOR);
    setupUI();
}

/**
 * @brief 创建伪彩色映射表选择控件
 *
 * 每个 addItem 的 userData 存 PseudoColorAlgorithm::Map 枚举，
 * 算法层再映射到 OpenCV 的 COLORMAP_* 常量
 */
void PseudoColorBlock::setupUI()
{
    addSeparator();

    m_mapCombo = new QComboBox(this);
    m_mapCombo->addItem(QStringLiteral("Jet"), int(PseudoColorAlgorithm::Map::Jet));
    m_mapCombo->addItem(QStringLiteral("Hot"), int(PseudoColorAlgorithm::Map::Hot));
    m_mapCombo->addItem(QStringLiteral("Cool"), int(PseudoColorAlgorithm::Map::Cool));
    m_mapCombo->addItem(QStringLiteral("Rainbow"), int(PseudoColorAlgorithm::Map::Rainbow));
    m_mapCombo->addItem(QStringLiteral("Ocean"), int(PseudoColorAlgorithm::Map::Ocean));
    m_mapCombo->addItem(QStringLiteral("Summer"), int(PseudoColorAlgorithm::Map::Summer));
    m_mapCombo->addItem(QStringLiteral("Winter"), int(PseudoColorAlgorithm::Map::Winter));
    m_mapCombo->addItem(QStringLiteral("Autumn"), int(PseudoColorAlgorithm::Map::Autumn));
    m_mapCombo->addItem(QStringLiteral("Bone"), int(PseudoColorAlgorithm::Map::Bone));
    m_mapCombo->addItem(QStringLiteral("Pink"), int(PseudoColorAlgorithm::Map::Pink));
    contentLayout()->addWidget(m_mapCombo);

    connect(m_mapCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { emit paramsChanged(); });
}

PseudoColorAlgorithm::Map PseudoColorBlock::currentMap() const
{
    return static_cast<PseudoColorAlgorithm::Map>(m_mapCombo->currentData().toInt());
}

/**
 * @brief 执行伪彩色映射
 *
 * 谁调用：ImageProcessor::reprocess()
 * 注意：算法先转灰度再上色，彩色输入会先失去色相信息
 */
QPixmap PseudoColorBlock::process(const QPixmap &input, const RoiInfo &roi)
{
    if (input.isNull()) return input;
    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);
    cv::Mat out = RoiProcess::apply(src, roi, [&](const cv::Mat &m) {
        return PseudoColorAlgorithm::apply(m, currentMap());
    });
    return ImageConverter::matToPixmap(out);
}
