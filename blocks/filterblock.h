#ifndef FILTERBLOCK_H
#define FILTERBLOCK_H

#include "baseblock.h"
#include <QComboBox>
#include <QSpinBox>
#include "../config/appconfig.h"
#include "../algorithms/filter.h"

class FilterBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit FilterBlock(QWidget *parent = nullptr);
    QPixmap process(const QPixmap &input, const RoiInfo &roi = RoiInfo()) override;
    QString blockName() const override { return AppConfig::BLOCK_NAME_FILTER; }

private:
    void setupUI();
    FilterAlgorithm::Type currentType() const;

    QComboBox *m_typeCombo = nullptr;
    QSpinBox *m_kxSpin = nullptr;
    QSpinBox *m_kySpin = nullptr;
    QSpinBox *m_iterSpin = nullptr;
};

#endif
