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

#include <QJsonObject>

PseudoColorBlock::PseudoColorBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupUI();
    retranslateUi();
}

/**
 * @brief 创建伪彩色映射表选择控件
 *
 * 每个 addItem 的 userData 存 PseudoColorAlgorithm::Map 枚举，
 * 算法层再映射到 OpenCV 的 COLORMAP_* 常量
 */
void PseudoColorBlock::setupUI()
{
    addSeparator();                                                  // 标题栏与参数区细线

    m_mapCombo = new QComboBox(this);
    for (int m : {int(PseudoColorAlgorithm::Map::Jet), int(PseudoColorAlgorithm::Map::Hot),
                  int(PseudoColorAlgorithm::Map::Cool), int(PseudoColorAlgorithm::Map::Rainbow),
                  int(PseudoColorAlgorithm::Map::Ocean), int(PseudoColorAlgorithm::Map::Summer),
                  int(PseudoColorAlgorithm::Map::Winter), int(PseudoColorAlgorithm::Map::Autumn),
                  int(PseudoColorAlgorithm::Map::Bone), int(PseudoColorAlgorithm::Map::Pink)}) {
        m_mapCombo->addItem(QString(), m);                           // userData 存 Map，算法层映射 COLORMAP_*
    }
    contentLayout()->addWidget(m_mapCombo);

    connect(m_mapCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { emit paramsChanged(); });            // 换色图 → 整链重算
    trackParamWidget(m_mapCombo);                                    // 换色图前压撤销
}

void PseudoColorBlock::retranslateUi()
{
    setupTitle(QStringLiteral("🌈"), tr("伪彩色处理"));
    BaseBlock::retranslateUi();
    if (!m_mapCombo || m_mapCombo->count() < 10)
        return;
    m_mapCombo->setItemText(0, tr("喷射色"));
    m_mapCombo->setItemText(1, tr("热力色"));
    m_mapCombo->setItemText(2, tr("冷色系"));
    m_mapCombo->setItemText(3, tr("彩虹色"));
    m_mapCombo->setItemText(4, tr("海洋色"));
    m_mapCombo->setItemText(5, tr("夏日色"));
    m_mapCombo->setItemText(6, tr("冬日色"));
    m_mapCombo->setItemText(7, tr("秋日色"));
    m_mapCombo->setItemText(8, tr("骨白色"));
    m_mapCombo->setItemText(9, tr("粉红色"));
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
QPixmap PseudoColorBlock::process(const QPixmap &input, const QList<RoiInfo> &rois)
{
    if (input.isNull()) return input;
    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);
    cv::Mat out = RoiProcess::apply(src, rois, [&](const cv::Mat &m) {
        return PseudoColorAlgorithm::apply(m, currentMap());
    });
    return ImageConverter::matToPixmap(out);
}

/** @brief 导出 colormap 枚举到 JSON */
QJsonObject PseudoColorBlock::saveParams() const
{
    QJsonObject obj = BaseBlock::saveParams();
    obj.insert(QStringLiteral("map"), m_mapCombo->currentData().toInt());
    return obj;
}

/** @brief 从 JSON 恢复 colormap；blockSignals 避免恢复过程中触发重算 */
void PseudoColorBlock::loadParams(const QJsonObject &obj)
{
    BaseBlock::loadParams(obj);
    const int map = obj.value(QStringLiteral("map")).toInt(m_mapCombo->currentData().toInt());
    const int idx = m_mapCombo->findData(map);
    m_mapCombo->blockSignals(true);
    if (idx >= 0)
        m_mapCombo->setCurrentIndex(idx);
    m_mapCombo->blockSignals(false);
}
