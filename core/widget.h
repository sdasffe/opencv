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

#include "imageprocessor.h"
#include "../roi/resizablerectitem.h"
#include "../roi/resizableellipseitem.h"
#include "../blocks/baseblock.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

// ========== 自定义 ListWidget（支持拖出文本数据） ==========
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
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(items.first()->text());
        return mimeData;
    }
};

/**
 * @brief 主窗口（调度器角色）
 *
 * 职责：
 * 1. UI 布局与交互（GraphicsView、工具栏、拖放区域）
 * 2. ROI 图形项管理
 * 3. 处理块的创建与生命周期管理
 * 4. 协调 ImageProcessor 与显示更新
 *
 * 不包含具体算法逻辑，所有图像处理委托给 ImageProcessor。
 */
class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

protected:
    void wheelEvent(QWheelEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // ========== 按钮槽函数 ==========
    void on_pushButton_clicked();       // 打开图片
    void on_pushButton_3_clicked();     // 添加 ROI
    void on_deltete_clicked();          // 删除 ROI
    void on_comboBox_currentIndexChanged(int index);

    // ========== 处理相关 ==========
    void onProcessingFinished(qint64 elapsedMs);
    void onSceneChanged(const QList<QRectF> &region);

private:
    // ========== ROI 管理 ==========
    void addRectItem(qreal x, qreal y, qreal w, qreal h);
    void addEllipseItem(qreal x, qreal y, qreal w, qreal h);

    // ========== 处理块管理 ==========
    void createBlockByName(const QString &name);
    void addBlockToPanel(BaseBlock *block);

    // ========== 显示更新 ==========
    void updatePixmapItem(const QPixmap &pixmap);
    QRectF getCurrentRoiImageRect() const;

    // ========== 初始化 ==========
    void setupGraphicsView();
    void setupDragDrop();
    void setupBlockPanel();

private:
    Ui::Widget *ui;

    // 图像显示
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;
    double m_scaleFactor = 1.0;

    // ROI（当前只支持单个，后续可扩展为列表）
    ResizableRectItem *m_rectItem;
    ResizableEllipseItem *m_ellipseItem;

    // 图像处理调度引擎
    ImageProcessor *m_processor;

    // 处理块面板
    QVBoxLayout *m_blockLayout;
    QList<BaseBlock*> m_blockList;

    // 状态
    bool m_isDragInside = false;
    bool m_processingSuspended = false; // 暂停处理（避免频繁重入）
};

#endif // WIDGET_H
