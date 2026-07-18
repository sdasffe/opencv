#ifndef PSEUDOCOLORBLOCK_H
#define PSEUDOCOLORBLOCK_H

#include "baseblock.h"
#include <QComboBox>
#include "../config/appconfig.h"
#include "../algorithms/pseudocolor.h"

class PseudoColorBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit PseudoColorBlock(QWidget *parent = nullptr);
    QPixmap process(const QPixmap &input, const RoiInfo &roi = RoiInfo()) override;
    QString blockName() const override { return AppConfig::BLOCK_NAME_PSEUDOCOLOR; }

private:
    void setupUI();
    PseudoColorAlgorithm::Map currentMap() const;

    QComboBox *m_mapCombo = nullptr;
};

#endif
