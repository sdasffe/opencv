#ifndef GRAYTRANSFORMBLOCK_H
#define GRAYTRANSFORMBLOCK_H

#include "baseblock.h"
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "../config/appconfig.h"
#include "../algorithms/graytransform.h"

class GrayTransformBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit GrayTransformBlock(QWidget *parent = nullptr);
    QPixmap process(const QPixmap &input, const RoiInfo &roi = RoiInfo()) override;
    QString blockName() const override { return AppConfig::BLOCK_NAME_GRAYTRANSFORM; }

private:
    void setupUI();
    void updateParamEnabled();
    GrayTransformAlgorithm::Type currentType() const;

    QComboBox *m_typeCombo = nullptr;
    QSpinBox *m_brightSpin = nullptr;
    QSpinBox *m_contrastSpin = nullptr;
    QDoubleSpinBox *m_gammaSpin = nullptr;
};

#endif
