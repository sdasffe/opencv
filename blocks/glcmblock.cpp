/**
 * @file glcmblock.cpp
 * @brief 灰度共生矩阵（GLCM）特征块 —— 纹理统计，图像原样通过
 *
 * 【在整条链路中的位置】
 *   拖入「灰度共生矩阵」→ ImageProcessor 流水线节点
 *   与其它块不同：process() 不改变像素，只在面板显示纹理特征值
 *
 * 【数据流】
 *   QPixmap → cv::Mat(BGR) →（可选 ROI 裁剪）→ GlcmAlgorithm::compute → 更新标签
 *   返回值始终为 input，供后续块继续处理原图
 *
 * 【UI 特点】
 *   下拉切换「查看哪类特征」，只显示对应一行数值（与灰度变换块类似）
 */

#include "glcmblock.h"
#include "../utils/imageconverter.h"

#include <QHBoxLayout>
#include <QJsonObject>

namespace {

/** 特征值格式化为 4 位小数 */
QString fmt(double v)
{
    return QString::number(v, 'f', 4);
}

} // namespace

GlcmBlock::GlcmBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupUI();
    retranslateUi();
}

/**
 * @brief 添加一行「名称 + 数值」指标控件
 * @param nameOut  输出：左侧标签指针
 * @param valueOut 输出：右侧数值标签指针（初始为「—」）
 */
QWidget *GlcmBlock::addMetricRow(const char * /*objectTag*/, QLabel **nameOut, QLabel **valueOut)
{
    auto *row = new QWidget(this);
    auto *lay = new QHBoxLayout(row);
    lay->setContentsMargins(0, 2, 0, 2);
    lay->setSpacing(6);
    *nameOut = new QLabel(row);
    (*nameOut)->setObjectName(QStringLiteral("blockFieldLabel"));
    (*nameOut)->setFixedWidth(AppConfig::BLOCK_FIELD_LABEL_WIDTH);
    *valueOut = new QLabel(QStringLiteral("—"), row);
    (*valueOut)->setObjectName(QStringLiteral("glcmValueLabel"));
    (*valueOut)->setTextInteractionFlags(Qt::TextSelectableByMouse);
    lay->addWidget(*nameOut);
    lay->addWidget(*valueOut, 1);
    contentLayout()->addWidget(row);
    return row;
}

/** @brief 构建指标下拉、量化级/距离 SpinBox 及六行特征数值 */
void GlcmBlock::setupUI()
{
    addSeparator();

    m_metricCombo = new QComboBox(this);
    for (int i = 0; i < 6; ++i)
        m_metricCombo->addItem(QString(), i);
    contentLayout()->addWidget(m_metricCombo);

    auto *lvRow = new QHBoxLayout();
    m_levelsLabel = new QLabel(this);
    m_levelsLabel->setObjectName(QStringLiteral("blockFieldLabel"));
    m_levelsLabel->setFixedWidth(AppConfig::BLOCK_FIELD_LABEL_WIDTH);
    m_levelsSpin = new QSpinBox(this);
    m_levelsSpin->setRange(8, 64);
    m_levelsSpin->setSingleStep(8);
    m_levelsSpin->setValue(32);
    m_levelsSpin->setFixedWidth(AppConfig::BLOCK_SPIN_WIDTH);
    lvRow->addWidget(m_levelsLabel);
    lvRow->addWidget(m_levelsSpin);
    lvRow->addStretch();
    contentLayout()->addLayout(lvRow);

    auto *dRow = new QHBoxLayout();
    m_distLabel = new QLabel(this);
    m_distLabel->setObjectName(QStringLiteral("blockFieldLabel"));
    m_distLabel->setFixedWidth(AppConfig::BLOCK_FIELD_LABEL_WIDTH);
    m_distSpin = new QSpinBox(this);
    m_distSpin->setRange(1, 8);
    m_distSpin->setValue(1);
    m_distSpin->setFixedWidth(AppConfig::BLOCK_SPIN_WIDTH);
    dRow->addWidget(m_distLabel);
    dRow->addWidget(m_distSpin);
    dRow->addStretch();
    contentLayout()->addLayout(dRow);

    addSeparator();

    m_rowContrast = addMetricRow("contrast", &m_nameContrast, &m_valContrast);
    m_rowCorrelation = addMetricRow("corr", &m_nameCorrelation, &m_valCorrelation);
    m_rowEnergy = addMetricRow("energy", &m_nameEnergy, &m_valEnergy);
    m_rowHomogeneity = addMetricRow("homo", &m_nameHomogeneity, &m_valHomogeneity);
    m_rowEntropy = addMetricRow("entropy", &m_nameEntropy, &m_valEntropy);
    m_rowDissimilarity = addMetricRow("diss", &m_nameDissimilarity, &m_valDissimilarity);

    connect(m_metricCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        updateMetricVisible();
    });
    connect(m_levelsSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) {
        emit paramsChanged();
    });
    connect(m_distSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) {
        emit paramsChanged();
    });

    trackParamWidget(m_metricCombo);
    trackParamWidget(m_levelsSpin);
    trackParamWidget(m_distSpin);

    updateMetricVisible();
}

void GlcmBlock::retranslateUi()
{
    setupTitle(QStringLiteral("▦"), tr("灰度共生矩阵"));
    BaseBlock::retranslateUi();
    if (!m_metricCombo || m_metricCombo->count() < 6)
        return;
    m_metricCombo->setItemText(0, tr("查看：对比度"));
    m_metricCombo->setItemText(1, tr("查看：相关性"));
    m_metricCombo->setItemText(2, tr("查看：能量"));
    m_metricCombo->setItemText(3, tr("查看：均匀性"));
    m_metricCombo->setItemText(4, tr("查看：熵"));
    m_metricCombo->setItemText(5, tr("查看：相异性"));
    if (m_levelsLabel)
        m_levelsLabel->setText(tr("量化级"));
    if (m_distLabel)
        m_distLabel->setText(tr("距离"));
    if (m_nameContrast)
        m_nameContrast->setText(tr("对比度"));
    if (m_nameCorrelation)
        m_nameCorrelation->setText(tr("相关性"));
    if (m_nameEnergy)
        m_nameEnergy->setText(tr("能量"));
    if (m_nameHomogeneity)
        m_nameHomogeneity->setText(tr("均匀性"));
    if (m_nameEntropy)
        m_nameEntropy->setText(tr("熵"));
    if (m_nameDissimilarity)
        m_nameDissimilarity->setText(tr("相异性"));
}

GlcmBlock::ViewMetric GlcmBlock::currentMetric() const
{
    return static_cast<ViewMetric>(m_metricCombo->currentData().toInt());
}

/** @brief 按当前下拉选项，只显示对应特征行，其余隐藏 */
void GlcmBlock::updateMetricVisible()
{
    const auto m = currentMetric();
    m_rowContrast->setVisible(m == ViewMetric::Contrast);
    m_rowCorrelation->setVisible(m == ViewMetric::Correlation);
    m_rowEnergy->setVisible(m == ViewMetric::Energy);
    m_rowHomogeneity->setVisible(m == ViewMetric::Homogeneity);
    m_rowEntropy->setVisible(m == ViewMetric::Entropy);
    m_rowDissimilarity->setVisible(m == ViewMetric::Dissimilarity);
}

/** @brief 将 m_lastFeatures 刷新到各数值标签；未计算过则显示「—」 */
void GlcmBlock::refreshMetricLabels()
{
    if (!m_hasFeatures) {
        m_valContrast->setText(QStringLiteral("—"));
        m_valCorrelation->setText(QStringLiteral("—"));
        m_valEnergy->setText(QStringLiteral("—"));
        m_valHomogeneity->setText(QStringLiteral("—"));
        m_valEntropy->setText(QStringLiteral("—"));
        m_valDissimilarity->setText(QStringLiteral("—"));
        return;
    }
    m_valContrast->setText(fmt(m_lastFeatures.contrast));
    m_valCorrelation->setText(fmt(m_lastFeatures.correlation));
    m_valEnergy->setText(fmt(m_lastFeatures.energy));
    m_valHomogeneity->setText(fmt(m_lastFeatures.homogeneity));
    m_valEntropy->setText(fmt(m_lastFeatures.entropy));
    m_valDissimilarity->setText(fmt(m_lastFeatures.dissimilarity));
}

/**
 * @brief 计算 GLCM 特征并更新面板；图像原样返回
 *
 * ROI 非空时取所有 ROI 外接矩形的并集作为统计区域；
 * 空 ROI 或未指定时在全图上计算。
 */
QPixmap GlcmBlock::process(const QPixmap &input, const QList<RoiInfo> &rois)
{
    if (input.isNull())
        return input;

    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);

    // GLCM 在 probe 上统计；probe 可为 ROI 裁剪区或全图
    cv::Mat probe = src;
    if (!rois.isEmpty()) {
        QRectF br;
        for (const RoiInfo &r : rois) {
            if (r.isEmpty())
                continue;
            const QRectF b = r.boundingRect();
            br = br.isNull() ? b : br.united(b);
        }
        if (!br.isNull()) {
            cv::Rect rr(qRound(br.x()), qRound(br.y()),
                        qRound(br.width()), qRound(br.height()));
            rr &= cv::Rect(0, 0, src.cols, src.rows);
            if (!rr.empty())
                probe = src(rr);
        }
    }

    m_lastFeatures = GlcmAlgorithm::compute(probe, m_levelsSpin->value(), m_distSpin->value());
    m_hasFeatures = true;
    refreshMetricLabels();
    return input;
}

/** @brief 导出查看指标、量化级、距离到 JSON */
QJsonObject GlcmBlock::saveParams() const
{
    QJsonObject obj = BaseBlock::saveParams();
    obj.insert(QStringLiteral("metric"), m_metricCombo->currentData().toInt());
    obj.insert(QStringLiteral("levels"), m_levelsSpin->value());
    obj.insert(QStringLiteral("distance"), m_distSpin->value());
    return obj;
}

/** @brief 从 JSON 恢复参数；blockSignals 避免恢复过程中触发重算 */
void GlcmBlock::loadParams(const QJsonObject &obj)
{
    BaseBlock::loadParams(obj);
    m_metricCombo->blockSignals(true);
    m_levelsSpin->blockSignals(true);
    m_distSpin->blockSignals(true);
    const int metric = obj.value(QStringLiteral("metric")).toInt(0);
    const int idx = m_metricCombo->findData(metric);
    if (idx >= 0)
        m_metricCombo->setCurrentIndex(idx);
    m_levelsSpin->setValue(obj.value(QStringLiteral("levels")).toInt(32));
    m_distSpin->setValue(obj.value(QStringLiteral("distance")).toInt(1));
    m_metricCombo->blockSignals(false);
    m_levelsSpin->blockSignals(false);
    m_distSpin->blockSignals(false);
    updateMetricVisible();
}
