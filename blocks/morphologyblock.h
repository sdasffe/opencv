#ifndef MORPHOLOGYBLOCK_H
#define MORPHOLOGYBLOCK_H

#include "baseblock.h"
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include "../config/appconfig.h"
#include "../algorithms/morphology.h"

/**
 * @brief 形态学处理块：膨胀/腐蚀/开闭/顶帽/底帽/梯度
 *
 * ROI 经 RoiProcess 并集 mask，区域外保持原像素。
 */
class MorphologyBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit MorphologyBlock(QWidget *parent = nullptr);                // 搭运算/核/迭代控件并接线
    QPixmap process(const QPixmap &input, const QList<RoiInfo> &rois = {}) override; // 调 MorphologyAlgorithm
    QString blockName() const override { return QString::fromUtf8(AppConfig::BLOCK_NAME_MORPHOLOGY); } // 稳定块 id

    QJsonObject saveParams() const override;                           // 导出 op/kx/ky/iter
    void loadParams(const QJsonObject &obj) override;                  // 从 JSON 恢复参数
    void retranslateUi() override;                                     // 刷新标题与标签

private:
    void setupUI();                                                    // 创建形态学运算下拉与核参数
    MorphologyAlgorithm::Op currentOp() const;                          // 当前选中的形态学运算

    QComboBox *m_opCombo = nullptr;                                    // 运算类型下拉
    QLabel *m_kxLabel = nullptr;                                       // 「核宽」标签
    QLabel *m_kyLabel = nullptr;                                       // 「核高」标签
    QLabel *m_iterLabel = nullptr;                                     // 「迭代」标签
    QSpinBox *m_kxSpin = nullptr;                                      // 结构元素宽
    QSpinBox *m_kySpin = nullptr;                                      // 结构元素高
    QSpinBox *m_iterSpin = nullptr;                                    // 迭代次数
};

#endif
