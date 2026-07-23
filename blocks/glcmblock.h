#ifndef GLCMBLOCK_H
#define GLCMBLOCK_H

#include "baseblock.h"
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QWidget>
#include "../config/appconfig.h"
#include "../algorithms/glcm.h"

/**
 * @brief 灰度共生矩阵（GLCM）特征块
 *
 * 图像原样通过；在块内显示纹理统计量。
 * 与灰度变换类似：用下拉切换「查看哪类特征」，只显示相关那一行数据。
 */
class GlcmBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit GlcmBlock(QWidget *parent = nullptr);

    QPixmap process(const QPixmap &input, const QList<RoiInfo> &rois = {}) override;
    QString blockName() const override { return QString::fromUtf8(AppConfig::BLOCK_NAME_GLCM); }

    QJsonObject saveParams() const override;
    void loadParams(const QJsonObject &obj) override;
    void retranslateUi() override;

private:
    /** @brief 面板可切换查看的纹理特征类型 */
    enum class ViewMetric {
        Contrast = 0,
        Correlation,
        Energy,
        Homogeneity,
        Entropy,
        Dissimilarity
    };

    void setupUI();
    void updateMetricVisible();
    void refreshMetricLabels();
    ViewMetric currentMetric() const;
    QWidget *addMetricRow(const char *objectTag, QLabel **nameOut, QLabel **valueOut);

    QComboBox *m_metricCombo = nullptr;   ///< 查看哪类特征（下拉切换显示行）
    QLabel *m_levelsLabel = nullptr;
    QSpinBox *m_levelsSpin = nullptr;
    QLabel *m_distLabel = nullptr;
    QSpinBox *m_distSpin = nullptr;

    QWidget *m_rowContrast = nullptr;
    QWidget *m_rowCorrelation = nullptr;
    QWidget *m_rowEnergy = nullptr;
    QWidget *m_rowHomogeneity = nullptr;
    QWidget *m_rowEntropy = nullptr;
    QWidget *m_rowDissimilarity = nullptr;

    QLabel *m_nameContrast = nullptr;
    QLabel *m_nameCorrelation = nullptr;
    QLabel *m_nameEnergy = nullptr;
    QLabel *m_nameHomogeneity = nullptr;
    QLabel *m_nameEntropy = nullptr;
    QLabel *m_nameDissimilarity = nullptr;

    QLabel *m_valContrast = nullptr;
    QLabel *m_valCorrelation = nullptr;
    QLabel *m_valEnergy = nullptr;
    QLabel *m_valHomogeneity = nullptr;
    QLabel *m_valEntropy = nullptr;
    QLabel *m_valDissimilarity = nullptr;

    GlcmAlgorithm::Features m_lastFeatures; ///< 最近一次 process 的统计结果
    bool m_hasFeatures = false;             ///< 是否已成功计算过（否则显示「—」）
};

#endif // GLCMBLOCK_H
