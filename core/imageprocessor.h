#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QPixmap>
#include <QList>
#include "../blocks/baseblock.h"
#include "../roi/roiinfo.h"
#include "../utils/timemeasurer.h"

/**
 * @brief 图像处理调度引擎（处理链的“大脑”，不含任何界面控件）
 *
 * Widget 管 UI 与 ROI 图元；本类持有原图/结果/块链/ROI 数据，按序调用 block->process()。
 * 块参数变化只 emit requestReprocess，由 Widget 在 onApplyProcessing 内先 setRois 再 reprocess。
 */
class ImageProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ImageProcessor(QObject *parent = nullptr);              // parent 一般为 Widget；子对象随 parent 销毁

    void setOriginalImage(const QPixmap &pixmap);                    // 写入原图并重置 m_result；不触发重算；loadImageFromPath 成功后调用
    QPixmap originalImage() const { return m_original; }             // 用户加载的原始图（处理链只读，永不修改）
    QPixmap resultImage() const { return m_result; }                 // 最近一次 reprocess 输出；未跑链时等于原图
    void resetResultToOriginal();                                    // 仅 m_result=原图；清空链后对比/保存用；不修改 m_original、不跑链
    bool hasImage() const { return !m_original.isNull(); }            // 是否已加载非空图片；无图时 reprocess 直接返回 0

    void addBlock(BaseBlock *block);                                 // 链尾追加块并接线 params/enabled；有图则 emit requestReprocess
    void removeBlock(BaseBlock *block);                              // 从链移除（不 delete）；disconnect 后若有图则 requestReprocess
    void moveBlock(BaseBlock *block, int toIndex);                  // 调整执行顺序；toIndex 夹紧到合法范围；有图则 requestReprocess
    const QList<BaseBlock*>& blocks() const { return m_blocks; }   // 只读处理链；顺序=面板从上到下=执行顺序

    void setRois(const QList<RoiInfo> &rois);                        // 重算前由 Widget 写入；空列表=全图；只缓存不触发重算
    QList<RoiInfo> rois() const { return m_rois; }                   // 当前 ROI 快照（纯数据，无 QGraphicsItem）

    qint64 reprocess();                                              // 核心：按序跑所有启用块；无图返回 0；结束 emit processingFinished
    qint64 lastElapsedMs() const { return m_lastElapsed; }           // 上次 reprocess 墙钟耗时（毫秒）；供 label_3 显示

signals:
    void processingFinished(qint64 elapsedMs);                       // 一次重算完成；Widget 刷新画布并更新耗时标签
    void requestReprocess();                                         // 请求 Widget 先 getAllRoiInfo→setRois 再 reprocess；块增删/换序/参数/开关变时

private slots:
    void onBlockParamsChanged();                                     // 块 emit paramsChanged → 有图时 requestReprocess
    void onBlockEnabledChanged(bool enabled);                        // 块 emit enabledChanged → 有图时 requestReprocess（开/关都需整链重算）

private:
    QPixmap m_original;                                              // 原始图像（只读数据源；块 process 不修改它）
    QPixmap m_result;                                                // 流水线输出；对比/保存/画布默认显示此图
    QList<BaseBlock*> m_blocks;                                      // 处理链；顺序敏感；块所有权在 Widget
    QList<RoiInfo> m_rois;                                           // 当前 ROI 列表；Widget 在 onApplyProcessing 内 setRois 写入
    qint64 m_lastElapsed = 0;                                        // 最近一次 reprocess 耗时缓存（毫秒）
};

#endif // IMAGEPROCESSOR_H
