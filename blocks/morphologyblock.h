#ifndef MORPHOLOGYBLOCK_H
#define MORPHOLOGYBLOCK_H

#include "baseblock.h"
#include <QComboBox>
#include <QSpinBox>
#include "../config/appconfig.h"
#include "../algorithms/morphology.h"

class MorphologyBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit MorphologyBlock(QWidget *parent = nullptr);
    QPixmap process(const QPixmap &input, const RoiInfo &roi = RoiInfo()) override;
    QString blockName() const override { return AppConfig::BLOCK_NAME_MORPHOLOGY; }

private:
    void setupUI();
    MorphologyAlgorithm::Op currentOp() const;

    QComboBox *m_opCombo = nullptr;
    QSpinBox *m_kxSpin = nullptr;
    QSpinBox *m_kySpin = nullptr;
    QSpinBox *m_iterSpin = nullptr;
};

#endif
