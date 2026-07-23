#ifndef FILTERBLOCK_H
#define FILTERBLOCK_H

#include "baseblock.h"
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include "../config/appconfig.h"
#include "../algorithms/filter.h"

/**
 * @brief 滤波/边缘检测处理块
 *
 * 平滑类（均值/高斯/中值）与边缘类（Sobel/Laplacian/Prewitt/Roberts）。
 * 支持 ROI 区域处理；平滑类可设置迭代次数。
 */
class FilterBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit FilterBlock(QWidget *parent = nullptr);
    QPixmap process(const QPixmap &input, const QList<RoiInfo> &rois = {}) override;
    QString blockName() const override { return QString::fromUtf8(AppConfig::BLOCK_NAME_FILTER); }

    QJsonObject saveParams() const override;
    void loadParams(const QJsonObject &obj) override;
    void retranslateUi() override;

private:
    void setupUI();
    FilterAlgorithm::Type currentType() const;  ///< 当前选中的滤波/边缘类型

    QComboBox *m_typeCombo = nullptr;
    QLabel *m_kxLabel = nullptr;
    QLabel *m_kyLabel = nullptr;
    QLabel *m_iterLabel = nullptr;
    QSpinBox *m_kxSpin = nullptr;
    QSpinBox *m_kySpin = nullptr;
    QSpinBox *m_iterSpin = nullptr;
};

#endif
