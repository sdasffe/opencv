#ifndef BINARIZATIONBLOCK_H
#define BINARIZATIONBLOCK_H

#include "baseblock.h"
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include "../config/appconfig.h"

/** 二值化处理块：上下阈值调节、Otsu 自动阈值，支持 ROI 区域二值化 */
class BinarizationBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit BinarizationBlock(QWidget *parent = nullptr);  // 构造参数 UI 并初始化文案

    QPixmap process(const QPixmap &input, const QList<RoiInfo> &rois = {}) override;  // 对输入图做范围二值化，ROI 外保持原样
    QString blockName() const override { return QString::fromUtf8(AppConfig::BLOCK_NAME_BINARIZATION); }  // 返回块注册名

    int lowerThreshold() const { return m_lowerSpin->value(); }  // 当前下限阈值（0~255）
    int upperThreshold() const { return m_upperSpin->value(); }  // 当前上限阈值（0~255）
    void setThresholds(int lower, int upper);  // 外部设置阈值（Otsu 完成后由 Widget 调用）

    QJsonObject saveParams() const override;  // 导出 lower/upper 及基类通用字段
    void loadParams(const QJsonObject &obj) override;  // 从 JSON 恢复阈值与 enabled
    void retranslateUi() override;  // 刷新标题与控件文案

signals:
    void otsuRequested();  // 请求主窗口用当前原图计算 Otsu 阈值

private slots:
    void onAutoThresholdClicked();  // Otsu 按钮：发出 otsuRequested
    void onLowerChanged(int value);  // 下限变化：维持 lower≤upper 并触发重算
    void onUpperChanged(int value);  // 上限变化：维持 lower≤upper 并触发重算

private:
    void setupUI();  // 创建下限/上限 SpinBox 与 Otsu 按钮

    QLabel *m_lowerLabel = nullptr;  // 「下限值」标签
    QLabel *m_upperLabel = nullptr;  // 「上限值」标签
    QSpinBox *m_lowerSpin;           // 下限阈值输入框
    QSpinBox *m_upperSpin;           // 上限阈值输入框
    QPushButton *m_autoBtn;            // Otsu 自动阈值按钮
};

#endif // BINARIZATIONBLOCK_H
