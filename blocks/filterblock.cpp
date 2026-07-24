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

#include <QJsonObject>

/**
 * @brief 构造函数
 * 谁调用：Widget::createBlockByName
 */
FilterBlock::FilterBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupUI();
    retranslateUi();
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
    addSeparator();                                                  // 标题栏与参数区细线

    m_typeCombo = new QComboBox(this);
    for (int t : {int(FilterAlgorithm::Type::Mean), int(FilterAlgorithm::Type::Gaussian),
                  int(FilterAlgorithm::Type::Median), int(FilterAlgorithm::Type::Sobel),
                  int(FilterAlgorithm::Type::Laplacian), int(FilterAlgorithm::Type::Prewitt),
                  int(FilterAlgorithm::Type::Roberts)}) {
        m_typeCombo->addItem(QString(), t);                          // userData 存 Type 枚举
    }
    contentLayout()->addWidget(m_typeCombo);

    auto addSpin = [&](QLabel *&labelOut, int def) {
        auto *row = new QHBoxLayout();
        labelOut = new QLabel(this);
        labelOut->setObjectName(QStringLiteral("blockFieldLabel"));
        labelOut->setFixedWidth(AppConfig::BLOCK_FIELD_LABEL_WIDTH);
        auto *sp = new QSpinBox(this);
        sp->setRange(1, 31);
        sp->setValue(def);
        sp->setFixedWidth(AppConfig::BLOCK_SPIN_WIDTH);
        sp->setSingleStep(2);                                        // 核尺寸保持奇数步进
        row->addWidget(labelOut);
        row->addWidget(sp);
        row->addStretch();
        contentLayout()->addLayout(row);
        return sp;
    };

    m_kxSpin = addSpin(m_kxLabel, 3);                                // 核宽
    m_kySpin = addSpin(m_kyLabel, 3);                                // 核高
    m_iterSpin = addSpin(m_iterLabel, 1);                            // 平滑迭代次数
    m_iterSpin->setRange(1, 20);

    auto emitChange = [this](int) { emit paramsChanged(); };
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChange);
    connect(m_kxSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    connect(m_kySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    connect(m_iterSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    trackParamWidget(m_typeCombo);                                   // 换滤波类型前压撤销
    trackParamWidget(m_kxSpin);                                      // 改核宽前压撤销
    trackParamWidget(m_kySpin);                                      // 改核高前压撤销
    trackParamWidget(m_iterSpin);                                    // 改次数前压撤销
}

void FilterBlock::retranslateUi()
{
    setupTitle(QStringLiteral("🌀"), tr("滤波处理"));
    BaseBlock::retranslateUi();
    if (!m_typeCombo || m_typeCombo->count() < 7)
        return;
    m_typeCombo->setItemText(0, tr("均值滤波"));
    m_typeCombo->setItemText(1, tr("高斯滤波"));
    m_typeCombo->setItemText(2, tr("中值滤波"));
    m_typeCombo->setItemText(3, QStringLiteral("Sobel"));
    m_typeCombo->setItemText(4, QStringLiteral("Laplacian"));
    m_typeCombo->setItemText(5, QStringLiteral("Prewitt"));
    m_typeCombo->setItemText(6, QStringLiteral("Roberts"));
    if (m_kxLabel)
        m_kxLabel->setText(tr("核 X"));
    if (m_kyLabel)
        m_kyLabel->setText(tr("核 Y"));
    if (m_iterLabel)
        m_iterLabel->setText(tr("次数"));
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
QPixmap FilterBlock::process(const QPixmap &input, const QList<RoiInfo> &rois)
{
    if (input.isNull()) return input;
    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);
    cv::Mat out = RoiProcess::apply(src, rois, [&](const cv::Mat &m) {
        return FilterAlgorithm::apply(m, currentType(),
                                      m_kxSpin->value(), m_kySpin->value(),
                                      m_iterSpin->value());
    });
    return ImageConverter::matToPixmap(out);
}

/** @brief 导出滤波类型、核尺寸、迭代次数到 JSON */
QJsonObject FilterBlock::saveParams() const
{
    QJsonObject obj = BaseBlock::saveParams();
    obj.insert(QStringLiteral("type"), m_typeCombo->currentData().toInt());
    obj.insert(QStringLiteral("kx"), m_kxSpin->value());
    obj.insert(QStringLiteral("ky"), m_kySpin->value());
    obj.insert(QStringLiteral("iterations"), m_iterSpin->value());
    return obj;
}

/** @brief 从 JSON 恢复参数；blockSignals 避免恢复过程中触发重算 */
void FilterBlock::loadParams(const QJsonObject &obj)
{
    BaseBlock::loadParams(obj);
    const int type = obj.value(QStringLiteral("type")).toInt(m_typeCombo->currentData().toInt());
    const int idx = m_typeCombo->findData(type);
    m_typeCombo->blockSignals(true);
    m_kxSpin->blockSignals(true);
    m_kySpin->blockSignals(true);
    m_iterSpin->blockSignals(true);
    if (idx >= 0)
        m_typeCombo->setCurrentIndex(idx);
    m_kxSpin->setValue(obj.value(QStringLiteral("kx")).toInt(m_kxSpin->value()));
    m_kySpin->setValue(obj.value(QStringLiteral("ky")).toInt(m_kySpin->value()));
    m_iterSpin->setValue(obj.value(QStringLiteral("iterations")).toInt(m_iterSpin->value()));
    m_typeCombo->blockSignals(false);
    m_kxSpin->blockSignals(false);
    m_kySpin->blockSignals(false);
    m_iterSpin->blockSignals(false);
}
