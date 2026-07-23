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
 * @brief 灰度变换处理块
 *
 * 转灰度、亮度/对比度、伽马、直方图均衡等点运算。
 * 不同模式所需参数不同，由 updateParamEnabled() 动态启用控件。
 */
class GrayTransformBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit GrayTransformBlock(QWidget *parent = nullptr);
    QPixmap process(const QPixmap &input, const QList<RoiInfo> &rois = {}) override;
    QString blockName() const override { return QString::fromUtf8(AppConfig::BLOCK_NAME_GRAYTRANSFORM); }

    QJsonObject saveParams() const override;
    void loadParams(const QJsonObject &obj) override;
    void retranslateUi() override;

private:
    void setupUI();
    void updateParamEnabled();  ///< 按变换类型启用/禁用亮度/对比度/伽马控件
    GrayTransformAlgorithm::Type currentType() const;

    QComboBox *m_typeCombo = nullptr;
    QLabel *m_brightLabel = nullptr;
    QLabel *m_contrastLabel = nullptr;
    QLabel *m_gammaLabel = nullptr;
    QSpinBox *m_brightSpin = nullptr;
    QSpinBox *m_contrastSpin = nullptr;
    QDoubleSpinBox *m_gammaSpin = nullptr;
};

#endif
