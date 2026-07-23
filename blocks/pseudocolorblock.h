#ifndef PSEUDOCOLORBLOCK_H
#define PSEUDOCOLORBLOCK_H

#include "baseblock.h"
#include <QComboBox>
#include "../config/appconfig.h"
#include "../algorithms/pseudocolor.h"

/**
 * @brief 伪彩色处理块
 *
 * 将灰度强度映射为 Jet/Hot 等色图，便于可视化。
 * 算法内先转灰度再上色，彩色输入会丢失原有色相。
 */
class PseudoColorBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit PseudoColorBlock(QWidget *parent = nullptr);
    QPixmap process(const QPixmap &input, const QList<RoiInfo> &rois = {}) override;
    QString blockName() const override { return QString::fromUtf8(AppConfig::BLOCK_NAME_PSEUDOCOLOR); }

    QJsonObject saveParams() const override;
    void loadParams(const QJsonObject &obj) override;
    void retranslateUi() override;

private:
    void setupUI();
    PseudoColorAlgorithm::Map currentMap() const;  ///< 当前选中的 colormap 枚举

    QComboBox *m_mapCombo = nullptr;
};

#endif
