#ifndef BINARIZATIONBLOCK_H
#define BINARIZATIONBLOCK_H

#include "baseblock.h"
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include "../config/appconfig.h"

/**
 * @brief 二值化处理块
 *
 * 提供上下阈值调节、Otsu自动阈值功能。
 * 支持 ROI 区域二值化（仅对指定区域做黑白处理，其余保持原样）。
 */
class BinarizationBlock : public BaseBlock
{
    Q_OBJECT
public:
    explicit BinarizationBlock(QWidget *parent = nullptr);

    // ========== BaseBlock 接口 ==========
    QPixmap process(const QPixmap &input, const QList<RoiInfo> &rois = {}) override;
    QString blockName() const override { return QString::fromUtf8(AppConfig::BLOCK_NAME_BINARIZATION); }

    int lowerThreshold() const { return m_lowerSpin->value(); }
    int upperThreshold() const { return m_upperSpin->value(); }
    /** @brief 外部设置阈值（Otsu 自动阈值完成后由 Widget 调用） */
    void setThresholds(int lower, int upper);

    QJsonObject saveParams() const override;
    void loadParams(const QJsonObject &obj) override;
    void retranslateUi() override;

signals:
    /** 请求用当前原图计算 Otsu（由主窗口响应） */
    void otsuRequested();

private slots:
    void onAutoThresholdClicked();
    void onLowerChanged(int value);
    void onUpperChanged(int value);

private:
    void setupUI();

    QLabel *m_lowerLabel = nullptr;
    QLabel *m_upperLabel = nullptr;
    QSpinBox *m_lowerSpin;
    QSpinBox *m_upperSpin;
    QPushButton *m_autoBtn;
};

#endif // BINARIZATIONBLOCK_H
