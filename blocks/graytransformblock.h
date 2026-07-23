#ifndef GRAYTRANSFORMBLOCK_H
#define GRAYTRANSFORMBLOCK_H

#include "baseblock.h"
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include "../config/appconfig.h"
#include "../algorithms/graytransform.h"

/**
 * @brief 灰度变换块：转灰度、亮度/对比度、伽马、直方图均衡等
 *
 * 不同模式所需参数不同，由 updateParamEnabled() 动态启用控件。
 */
class GrayTransformBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit GrayTransformBlock(QWidget *parent = nullptr);            // 搭类型与参数控件并接线
    QPixmap process(const QPixmap &input, const QList<RoiInfo> &rois = {}) override; // 调 GrayTransformAlgorithm
    QString blockName() const override { return QString::fromUtf8(AppConfig::BLOCK_NAME_GRAYTRANSFORM); } // 稳定块 id

    QJsonObject saveParams() const override;                           // 导出 type/亮度/对比度/伽马
    void loadParams(const QJsonObject &obj) override;                  // 从 JSON 恢复并刷新控件使能
    void retranslateUi() override;                                     // 刷新标题与标签

private:
    void setupUI();                                                    // 创建类型下拉与亮度/对比度/伽马
    void updateParamEnabled();                                         // 按变换类型启用/禁用相关 SpinBox
    GrayTransformAlgorithm::Type currentType() const;                  // 当前变换类型枚举

    QComboBox *m_typeCombo = nullptr;                                  // 变换类型下拉
    QLabel *m_brightLabel = nullptr;                                   // 「亮度」标签
    QLabel *m_contrastLabel = nullptr;                                 // 「对比度」标签
    QLabel *m_gammaLabel = nullptr;                                    // 「伽马」标签
    QSpinBox *m_brightSpin = nullptr;                                  // 亮度偏移
    QSpinBox *m_contrastSpin = nullptr;                                // 对比度系数相关
    QDoubleSpinBox *m_gammaSpin = nullptr;                             // 伽马值
};

#endif
