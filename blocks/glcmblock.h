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
 * @brief GLCM 纹理特征块：图像原样通过，面板显示统计量
 *
 * 下拉切换查看哪类特征；量化级与距离可调。
 */
class GlcmBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit GlcmBlock(QWidget *parent = nullptr);                     // 搭量化/距离/特征行 UI

    QPixmap process(const QPixmap &input, const QList<RoiInfo> &rois = {}) override; // 计算特征后仍返回原图
    QString blockName() const override { return QString::fromUtf8(AppConfig::BLOCK_NAME_GLCM); } // 稳定块 id

    QJsonObject saveParams() const override;                           // 导出 levels/distance/查看项
    void loadParams(const QJsonObject &obj) override;                  // 恢复参数并刷新标签
    void retranslateUi() override;                                     // 刷新特征名等文案

private:
    /** 面板可切换查看的纹理特征类型 */
    enum class ViewMetric {
        Contrast = 0,                                                  // 对比度
        Correlation,                                                   // 相关性
        Energy,                                                        // 能量
        Homogeneity,                                                   // 同质性
        Entropy,                                                       // 熵
        Dissimilarity                                                  // 相异性
    };

    void setupUI();                                                    // 创建参数控件与六行特征值
    void updateMetricVisible();                                        // 只显示当前选中特征那一行
    void refreshMetricLabels();                                        // 用 m_lastFeatures 刷新数值文案
    ViewMetric currentMetric() const;                                  // 当前下拉选中的特征
    QWidget *addMetricRow(const char *objectTag, QLabel **nameOut, QLabel **valueOut); // 建一行「名+值」

    QComboBox *m_metricCombo = nullptr;                                // 查看哪类特征
    QLabel *m_levelsLabel = nullptr;                                   // 「量化级」标签
    QSpinBox *m_levelsSpin = nullptr;                                  // 灰度量化级数
    QLabel *m_distLabel = nullptr;                                     // 「距离」标签
    QSpinBox *m_distSpin = nullptr;                                    // 像素对距离

    QWidget *m_rowContrast = nullptr;                                  // 对比度行容器
    QWidget *m_rowCorrelation = nullptr;                               // 相关性行
    QWidget *m_rowEnergy = nullptr;                                    // 能量行
    QWidget *m_rowHomogeneity = nullptr;                               // 同质性行
    QWidget *m_rowEntropy = nullptr;                                   // 熵行
    QWidget *m_rowDissimilarity = nullptr;                             // 相异性行

    QLabel *m_nameContrast = nullptr;                                  // 特征名标签
    QLabel *m_nameCorrelation = nullptr;
    QLabel *m_nameEnergy = nullptr;
    QLabel *m_nameHomogeneity = nullptr;
    QLabel *m_nameEntropy = nullptr;
    QLabel *m_nameDissimilarity = nullptr;

    QLabel *m_valContrast = nullptr;                                   // 特征数值标签
    QLabel *m_valCorrelation = nullptr;
    QLabel *m_valEnergy = nullptr;
    QLabel *m_valHomogeneity = nullptr;
    QLabel *m_valEntropy = nullptr;
    QLabel *m_valDissimilarity = nullptr;

    GlcmAlgorithm::Features m_lastFeatures;                            // 最近一次 process 的统计结果
    bool m_hasFeatures = false;                                        // 未算过时显示「—」
};

#endif // GLCMBLOCK_H
