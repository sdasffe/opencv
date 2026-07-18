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

#include "imageprocessor.h"
#include "../roi/resizablerectitem.h"
#include "../roi/resizableellipseitem.h"
#include "../roi/resizablerotatedrectitem.h"
#include "../roi/roiinfo.h"
#include "../blocks/baseblock.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

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

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_deltete_clicked();
    void on_comboBox_currentIndexChanged(int index);
    void onProcessingFinished(qint64 elapsedMs);
    void onApplyProcessing();
    void onSaveResult();
    void onClearBlocks();
    void onToggleOriginal();
    void onRoiGeometryChanged();

private:
    void clearAllRoi();
    void addRectItem(qreal x, qreal y, qreal w, qreal h);
    void addEllipseItem(qreal x, qreal y, qreal w, qreal h);
    void addRotatedRectItem(qreal x, qreal y, qreal w, qreal h);

    void createBlockByName(const QString &name);
    void addBlockToPanel(BaseBlock *block);
    void wireBinarizationOtsu(class BinarizationBlock *block);

    void updatePixmapItem(const QPixmap &pixmap);
    void refreshDisplay();
    void updateInfoLabel();
    RoiInfo getCurrentRoiInfo() const;
    QPointF imageCenterInScene() const;
    bool loadImageFromPath(const QString &filePath);

    void setupGraphicsView();
    void setupDragDrop();
    void setupBlockPanel();
    void setupToolbar();
    void setDropPanelHighlight(bool on);
    void refreshChainHint();
    bool hasAnyRoi() const;
    bool viewportPanEvent(QEvent *event);

private:
    Ui::Widget *ui;

    QGraphicsScene *m_scene = nullptr;
    QGraphicsPixmapItem *m_pixmapItem = nullptr;
    double m_viewScale = 1.0;

    ResizableRectItem *m_rectItem = nullptr;
    ResizableEllipseItem *m_ellipseItem = nullptr;
    ResizableRotatedRectItem *m_rotatedRectItem = nullptr;

    ImageProcessor *m_processor = nullptr;

    QVBoxLayout *m_blockLayout = nullptr;
    QList<BaseBlock *> m_blockList;

    QTimer *m_roiUpdateTimer = nullptr;

    bool m_isDragInside = false;
    bool m_showOriginal = false;
    bool m_suspendSceneReprocess = false;
    bool m_panning = false;
    QPoint m_panLastPos;
};

#endif // WIDGET_H
