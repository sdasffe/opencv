#ifndef FILTERBLOCK_H
#define FILTERBLOCK_H

#include "baseblock.h"
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include "../config/appconfig.h"
#include "../algorithms/filter.h"

/**
 * @brief 滤波/边缘检测块：平滑（均值/高斯/中值）与边缘（Sobel 等）
 *
 * 支持 ROI；平滑类可设迭代次数。
 */
class FilterBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit FilterBlock(QWidget *parent = nullptr);                   // 搭类型/核/迭代控件并接线
    QPixmap process(const QPixmap &input, const QList<RoiInfo> &rois = {}) override; // 调 FilterAlgorithm；空 rois=全图
    QString blockName() const override { return QString::fromUtf8(AppConfig::BLOCK_NAME_FILTER); } // 稳定块 id

    QJsonObject saveParams() const override;                           // 导出 type/kx/ky/iter
    void loadParams(const QJsonObject &obj) override;                  // 从 JSON 恢复参数
    void retranslateUi() override;                                     // 刷新标题与标签文案

private:
    void setupUI();                                                    // 创建类型下拉与 kx/ky/iter SpinBox
    FilterAlgorithm::Type currentType() const;                         // 当前滤波/边缘类型枚举

    QComboBox *m_typeCombo = nullptr;                                  // 滤波/边缘类型选择
    QLabel *m_kxLabel = nullptr;                                       // 「核宽」标签
    QLabel *m_kyLabel = nullptr;                                       // 「核高」标签
    QLabel *m_iterLabel = nullptr;                                     // 「迭代」标签（平滑类）
    QSpinBox *m_kxSpin = nullptr;                                      // 核宽度（奇数）
    QSpinBox *m_kySpin = nullptr;                                      // 核高度（奇数）
    QSpinBox *m_iterSpin = nullptr;                                    // 平滑迭代次数
};

#endif
