#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QVBoxLayout>
#include <QList>
#include <QString>
#include <QListWidget>
#include <QMimeData>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QPoint>
#include <QFileInfo>

#include "imageprocessor.h"
#include "../roi/resizablerectitem.h"
#include "../roi/resizableellipseitem.h"
#include "../roi/resizablerotatedrectitem.h"
#include "../roi/roiinfo.h"
#include "../blocks/baseblock.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

/**
 * @brief 可拖拽的算法列表（左侧工具箱）
 *
 * 重写 mimeData()：拖出去时把算法名称（如“二值化处理”）放进剪贴板文本。
 * 右侧面板 Drop 时读这个名字，再 createBlockByName() 创建对应块。
 */
class MyListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit MyListWidget(QWidget *parent = nullptr) {
        setDragEnabled(true);
        setDragDropMode(QAbstractItemView::DragOnly);
    }
protected:
    QMimeData *mimeData(const QList<QListWidgetItem *> &items) const override {
        if (items.isEmpty()) return nullptr;
        auto *mimeData = new QMimeData;
        mimeData->setText(items.first()->text());
        return mimeData;
    }
};

/**
 * @brief 主窗口：图像显示 + ROI + 处理链 UI
 *
 * 【界面分区】（见 widget.ui）
 *   - 左侧：算法列表 listWidget，可拖出算法名
 *   - 中间：graphicsView 画布；下方 folderBrowserPanel 显示文件夹缩略图
 *   - 右侧：处理链面板 widget_3，放置 BaseBlock 子控件
 *
 * 【典型用户路径】
 *   打开图片 / 打开文件夹（点缩略图切换）→（可选）ROI → 拖入算法 → 调参 → 对比/保存
 *
 * 【与 ImageProcessor 的关系】
 *   Widget 管 UI 与 ROI；ImageProcessor 管“按块顺序算图”。
 *   重算统一走 onApplyProcessing()：先 getCurrentRoiInfo() → setRoi → reprocess。
 */
class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

protected:
    /** 滚轮：以鼠标为锚点缩放画布 */
    void wheelEvent(QWheelEvent *event) override;
    /** Delete 键：删除选中的 ROI */
    void keyPressEvent(QKeyEvent *event) override;
    /**
     * 事件过滤：
     *   - graphicsView viewport：中键/左键空白处平移画布
     *   - widget_3：接收算法块拖放
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_pushButton_clicked();    // 打开单张图片
    void on_pushButton_2_clicked();  // 打开文件夹（缩略图条 + 可切换）
    void on_pushButton_3_clicked();  // 按 comboBox 类型添加 ROI
    void on_deltete_clicked();       // 删除 ROI（按钮）
    void on_comboBox_currentIndexChanged(int index);

    /** 处理完成：刷新画布 + 显示耗时 */
    void onProcessingFinished(qint64 elapsedMs);

    /** 统一入口：同步 ROI 后调用 ImageProcessor::reprocess() */
    void onApplyProcessing();

    void onRoiGeometryChanged();

    void on_btnApply_clicked();

    /** 按住对比：显示原图 */
    void on_btnCompare_pressed();
    /** 松开对比：恢复结果图 */
    void on_btnCompare_released();

    void on_btnSave_clicked();

    void on_btnClearChain_clicked();

    /** 点击文件夹缩略图 → 切换当前图片 */
    void on_folderImageList_itemClicked(QListWidgetItem *item);

private:
    // ----- ROI 管理 -----
    void clearAllRoi();
    void addRectItem(qreal x, qreal y, qreal w, qreal h);
    void addEllipseItem(qreal x, qreal y, qreal w, qreal h);
    void addRotatedRectItem(qreal x, qreal y, qreal w, qreal h);

    // ----- 处理块管理 -----
    /** 根据拖入的名字（AppConfig::BLOCK_NAME_*）创建具体块 */
    void createBlockByName(const QString &name);
    /** 把块挂到右侧面板 + 注册到 ImageProcessor */
    void addBlockToPanel(BaseBlock *block);
    /** 二值化块的 Otsu 按钮：用原图（或 ROI）算自动阈值 */
    void wireBinarizationOtsu(class BinarizationBlock *block);

    // ----- 显示 -----
    void updatePixmapItem(const QPixmap &pixmap);
    void refreshDisplay();
    void updateInfoLabel();
    /** 从当前场景里的 ROI 图元，打包成 RoiInfo（图像像素坐标） */
    RoiInfo getCurrentRoiInfo() const;
    QPointF imageCenterInScene() const;
    bool loadImageFromPath(const QString &filePath);

    // ----- 文件夹浏览 -----
    void setupFolderBrowser();
    void clearFolderBrowser();
    void fillFolderBrowser(const QString &dirPath, const QFileInfoList &files);
    void selectFolderThumbnail(const QString &filePath);

    // ----- 初始化 -----
    void setupGraphicsView();
    void setupDragDrop();
    void setupBlockPanel();
    void setDropPanelHighlight(bool on);
    void refreshChainHint();
    bool hasAnyRoi() const;
    /** 画布平移手势；点在 ROI 上时不拦截，保证 ROI 可拖 */
    bool viewportPanEvent(QEvent *event);

private:
    Ui::Widget *ui;

    QGraphicsScene *m_scene = nullptr;           // 场景：放 pixmap + ROI 图元
    QGraphicsPixmapItem *m_pixmapItem = nullptr; // 当前显示的图像
    double m_viewScale = 1.0;                    // 相对 fitInView 后的缩放倍数

    // 三种 ROI 同时最多一种有效（添加时会 clearAllRoi）
    ResizableRectItem *m_rectItem = nullptr;
    ResizableEllipseItem *m_ellipseItem = nullptr;
    ResizableRotatedRectItem *m_rotatedRectItem = nullptr;

    ImageProcessor *m_processor = nullptr; // 处理链引擎

    QVBoxLayout *m_blockLayout = nullptr;  // 右侧块列表布局
    QList<BaseBlock *> m_blockList;        // 与 processor 中块顺序一致

    /** ROI 拖动防抖：scene.changed → 启动 60ms 单次定时器 → onApplyProcessing */
    QTimer *m_roiUpdateTimer = nullptr;

    bool m_isDragInside = false;           // 拖拽是否在右侧面板上方
    bool m_showOriginal = false;           // true = 正在按住「对比」，显示原图
    bool m_suspendSceneReprocess = false;  // 更新 pixmap 时挂起，避免 changed 死循环
    bool m_panning = false;                // 是否正在拖动画布
    bool m_loadingFromFolderList = false;  // 避免缩略图选中与加载互相递归
    QPoint m_panLastPos;
    QString m_currentImagePath;            // 当前画布图片路径（用于高亮缩略图）
};

#endif // WIDGET_H
