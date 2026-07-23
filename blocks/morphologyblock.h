#ifndef MORPHOLOGYBLOCK_H
#define MORPHOLOGYBLOCK_H

#include "baseblock.h"
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include "../config/appconfig.h"
#include "../algorithms/morphology.h"

/**
 * @brief 形态学处理块
 *
 * 提供膨胀/腐蚀/开闭运算及顶帽、底帽、形态学梯度。
 * 支持 ROI 区域处理（RoiProcess 并集 mask，区域外保持原像素）。
 */
class MorphologyBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit MorphologyBlock(QWidget *parent = nullptr);
    QPixmap process(const QPixmap &input, const QList<RoiInfo> &rois = {}) override;
    QString blockName() const override { return QString::fromUtf8(AppConfig::BLOCK_NAME_MORPHOLOGY); }

    QJsonObject saveParams() const override;
    void loadParams(const QJsonObject &obj) override;
    void retranslateUi() override;

private:
    void setupUI();
    MorphologyAlgorithm::Op currentOp() const;  ///< 当前选中的形态学运算枚举

    QComboBox *m_opCombo = nullptr;
    QLabel *m_kxLabel = nullptr;
    QLabel *m_kyLabel = nullptr;
    QLabel *m_iterLabel = nullptr;
    QSpinBox *m_kxSpin = nullptr;
    QSpinBox *m_kySpin = nullptr;
    QSpinBox *m_iterSpin = nullptr;
};

#endif
