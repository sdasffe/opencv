#ifndef PSEUDOCOLORBLOCK_H
#define PSEUDOCOLORBLOCK_H

#include "baseblock.h"
#include <QComboBox>
#include "../config/appconfig.h"
#include "../algorithms/pseudocolor.h"

/**
 * @brief 伪彩色处理块：灰度强度映射为 Jet/Hot 等色图
 *
 * 算法内先转灰度再上色；彩色输入会丢失原有色相。
 */
class PseudoColorBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit PseudoColorBlock(QWidget *parent = nullptr);               // 搭 UI：色图下拉；接线 paramsChanged
    QPixmap process(const QPixmap &input, const QList<RoiInfo> &rois = {}) override; // 调 PseudoColorAlgorithm；空 rois=全图
    QString blockName() const override { return QString::fromUtf8(AppConfig::BLOCK_NAME_PSEUDOCOLOR); } // 稳定 id，导入导出用

    QJsonObject saveParams() const override;                           // 导出 map 枚举下标 + 基类字段
    void loadParams(const QJsonObject &obj) override;                  // 恢复色图选择与 enabled
    void retranslateUi() override;                                     // 刷新标题与下拉项中英文

private:
    void setupUI();                                                    // 创建色图 ComboBox 并入 contentLayout
    PseudoColorAlgorithm::Map currentMap() const;                      // 当前选中的 colormap 枚举

    QComboBox *m_mapCombo = nullptr;                                   // 色图类型：Jet / Hot / Cool 等
};

#endif
