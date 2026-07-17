#include "widget.h"
#include "ui_widget.h"
#include "../config/appconfig.h"
#include "../blocks/binarizationblock.h"
#include "../algorithms/otsu.h"
#include "../utils/imageconverter.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_scene(new QGraphicsScene(this))
    , m_pixmapItem(nullptr)
    , m_rectItem(nullptr)
    , m_ellipseItem(nullptr)
    , m_scaleFactor(1.0)
    , m_processor(new ImageProcessor(this))
    , m_blockLayout(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("图片浏览器");
    setMouseTracking(true);

    setupGraphicsView();
    setupDragDrop();
    setupBlockPanel();

    // 连接处理器信号
    connect(m_processor, &ImageProcessor::processingFinished,
            this, &Widget::onProcessingFinished);
}

Widget::~Widget()
{
    delete ui;
}

// ========== 初始化 ==========

void Widget::setupGraphicsView()
{
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing, false);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    ui->graphicsView->setCacheMode(QGraphicsView::CacheNone);

    // 场景变化时更新 ROI 并重处理
    connect(m_scene, &QGraphicsScene::changed,
            this, &Widget::onSceneChanged);
}

void Widget::setupDragDrop()
{
    // 列表可拖拽
    ui->listWidget->setDragEnabled(true);
    ui->listWidget->setDragDropMode(QAbstractItemView::DragOnly);

    // 右侧面板接收拖放
    ui->widget_3->setAcceptDrops(true);
    ui->widget_3->installEventFilter(this);
    ui->widget_3->setStyleSheet(
        "QWidget#widget_3 {"
        "    background-color: #f5f5f5;"
        "    border: 2px dashed #bbb;"
        "    border-radius: 8px;"
        "}"
        );
}

void Widget::setupBlockPanel()
{
    m_blockLayout = new QVBoxLayout(ui->widget_3);
    m_blockLayout->setContentsMargins(
        AppConfig::BLOCK_LAYOUT_MARGIN,
        AppConfig::BLOCK_LAYOUT_MARGIN,
        AppConfig::BLOCK_LAYOUT_MARGIN,
        AppConfig::BLOCK_LAYOUT_MARGIN
        );
    m_blockLayout->setSpacing(AppConfig::BLOCK_LAYOUT_SPACING);
    m_blockLayout->addStretch();
}

// ========== 打开图片 ==========

void Widget::on_pushButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择图片", "", AppConfig::IMAGE_FILE_FILTER);

    if (filePath.isEmpty()) return;

    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "错误", "无法加载图片，文件可能已损坏");
        return;
    }

    // 交给处理器
    m_processor->setOriginalImage(pixmap);

    // 显示
    if (m_pixmapItem) {
        m_scene->removeItem(m_pixmapItem);
        delete m_pixmapItem;
    }
    m_pixmapItem = m_scene->addPixmap(pixmap);
    m_scene->setSceneRect(pixmap.rect());

    // 自适应
    ui->graphicsView->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);

    setWindowTitle(QString("图片浏览器 — %1  (%2 x %3)")
                       .arg(filePath)
                       .arg(pixmap.width())
                       .arg(pixmap.height()));
}

// ========== 滚轮缩放 ==========

void Widget::wheelEvent(QWheelEvent *event)
{
    if (!m_pixmapItem) {
        event->ignore();
        return;
    }

    double factor = AppConfig::SCROLL_SCALE_STEP;
    if (event->angleDelta().y() > 0) {
        ui->graphicsView->scale(factor, factor);
    } else {
        ui->graphicsView->scale(1.0 / factor, 1.0 / factor);
    }
    event->accept();
}

// ========== ROI 管理 ==========

void Widget::addRectItem(qreal x, qreal y, qreal width, qreal height)
{
    if (m_rectItem) {
        m_scene->removeItem(m_rectItem);
        delete m_rectItem;
    }
    m_rectItem = new ResizableRectItem(x, y, width, height);
    m_scene->addItem(m_rectItem);
    m_rectItem->setSelected(true);
}

void Widget::addEllipseItem(qreal x, qreal y, qreal w, qreal h)
{
    if (m_ellipseItem) {
        m_scene->removeItem(m_ellipseItem);
        delete m_ellipseItem;
    }
    m_ellipseItem = new ResizableEllipseItem(x, y, w, h);
    m_scene->addItem(m_ellipseItem);
    m_ellipseItem->setSelected(true);
}

void Widget::on_comboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    if (!m_pixmapItem) return;
}

void Widget::on_pushButton_3_clicked()
{
    if (ui->comboBox->currentText() == "矩形") {
        addRectItem(ui->graphicsView->size().width() / 2.0,
                    ui->graphicsView->size().height() / 2.0,
                    AppConfig::DEFAULT_ROI_SIZE,
                    AppConfig::DEFAULT_ROI_SIZE);
    }
    if (ui->comboBox->currentText() == "圆形") {
        addEllipseItem(ui->graphicsView->size().width() / 2.0,
                       ui->graphicsView->size().height() / 2.0,
                       AppConfig::DEFAULT_ROI_SIZE,
                       AppConfig::DEFAULT_ROI_SIZE);
    }
}

void Widget::on_deltete_clicked()
{
    QList<QGraphicsItem*> selectedItems = m_scene->selectedItems();
    for (QGraphicsItem* item : selectedItems) {
        if (item == m_rectItem)    m_rectItem = nullptr;
        if (item == m_ellipseItem) m_ellipseItem = nullptr;
        m_scene->removeItem(item);
        delete item;
    }
}

// ========== ROI 坐标映射（场景坐标 → 图像像素坐标） ==========

QRectF Widget::getCurrentRoiImageRect() const
{
    // 优先矩形 ROI
    if (m_rectItem) {
        QRectF sceneRect = m_rectItem->mapToScene(m_rectItem->rect()).boundingRect();
        QRectF imageSceneRect = m_scene->sceneRect();

        qreal scaleX = m_processor->originalImage().width() / imageSceneRect.width();
        qreal scaleY = m_processor->originalImage().height() / imageSceneRect.height();

        return QRectF(
            (sceneRect.x() - imageSceneRect.x()) * scaleX,
            (sceneRect.y() - imageSceneRect.y()) * scaleY,
            sceneRect.width() * scaleX,
            sceneRect.height() * scaleY
            );
    }
    return QRectF(); // 空矩形 = 全图处理
}

// ========== 场景变化（ROI 拖拽）触发重处理 ==========

void Widget::onSceneChanged(const QList<QRectF> &region)
{
    Q_UNUSED(region)
    if (!m_processor->hasImage()) return;
    if (m_processingSuspended) return;

    m_processor->setRoiRect(getCurrentRoiImageRect());
    m_processor->reprocess();
}

// ========== 处理完成回调 ==========

void Widget::onProcessingFinished(qint64 elapsedMs)
{
    updatePixmapItem(m_processor->resultImage());
    ui->label_3->setText(QString("%1ms").arg(elapsedMs));
}

void Widget::updatePixmapItem(const QPixmap &pixmap)
{
    if (m_pixmapItem) {
        m_pixmapItem->setPixmap(pixmap);
    }
}

// ========== 拖放事件处理 ==========

bool Widget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != ui->widget_3)
        return QWidget::eventFilter(obj, event);

    switch (event->type()) {
    case QEvent::DragEnter: {
        auto *e = static_cast<QDragEnterEvent*>(event);
        if (e->mimeData()->hasText()) {
            m_isDragInside = true;
            ui->widget_3->setStyleSheet(
                "QWidget#widget_3 {"
                "    background-color: #e8f5e9;"
                "    border: 2px solid #4CAF50;"
                "    border-radius: 8px;"
                "}"
                );
            e->acceptProposedAction();
            return true;
        }
        break;
    }
    case QEvent::DragLeave:
        if (m_isDragInside) {
            m_isDragInside = false;
            ui->widget_3->setStyleSheet(
                "QWidget#widget_3 {"
                "    background-color: #f5f5f5;"
                "    border: 2px dashed #bbb;"
                "    border-radius: 8px;"
                "}"
                );
        }
        return true;

    case QEvent::Drop: {
        m_isDragInside = false;
        ui->widget_3->setStyleSheet(
            "QWidget#widget_3 {"
            "    background-color: #f5f5f5;"
            "    border: 2px dashed #bbb;"
            "    border-radius: 8px;"
            "}"
            );
        auto *e = static_cast<QDropEvent*>(event);
        if (e->mimeData()->hasText()) {
            createBlockByName(e->mimeData()->text());
            e->acceptProposedAction();
            return true;
        }
        break;
    }
    default:
        break;
    }
    return QWidget::eventFilter(obj, event);
}

// ========== 根据名称创建处理块 ==========

void Widget::createBlockByName(const QString &name)
{
    BaseBlock *block = nullptr;

    if (name == AppConfig::BLOCK_NAME_BINARIZATION) {
        block = new BinarizationBlock(ui->widget_3);

        // 自动设置按钮的 Otsu 计算在这里处理（需要访问原图）
        auto *binBlock = qobject_cast<BinarizationBlock*>(block);
        if (binBlock) {
            // 找到自动按钮并连接（这里通过 block 内部信号处理，简化：直接连 paramsChanged）
            // Otsu 自动阈值的逻辑：点击自动按钮时计算并设置
            // 由于 BinarizationBlock 内部的 autoBtn 是私有的，我们这里用另一种方式
            // 实际项目中建议给 BinarizationBlock 加一个 autoThresholdRequested 信号
        }
    }
    // TODO: 其他处理块...

    if (block) {
        addBlockToPanel(block);
    }
}

void Widget::addBlockToPanel(BaseBlock *block)
{
    if (!block) return;

    // 插入到弹性空间之前
    m_blockLayout->insertWidget(m_blockLayout->count() - 1, block);
    m_blockList.append(block);

    // 删除按钮
    connect(block, &BaseBlock::removeRequested, this, [this, block]() {
        m_blockLayout->removeWidget(block);
        m_blockList.removeOne(block);
        m_processor->removeBlock(block);
        block->deleteLater();
    });

    // 交给处理器
    m_processor->addBlock(block);

    block->show();
}
