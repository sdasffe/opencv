#include "widget.h"
#include "ui_widget.h"
#include "../config/appconfig.h"
#include "../blocks/binarizationblock.h"
#include "../blocks/morphologyblock.h"
#include "../blocks/filterblock.h"
#include "../blocks/graytransformblock.h"
#include "../blocks/pseudocolorblock.h"
#include "../algorithms/otsu.h"
#include "../utils/imageconverter.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QStyle>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QDir>
#include <QFileInfo>
#include <QScrollBar>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_scene(new QGraphicsScene(this))
    , m_processor(new ImageProcessor(this))
    , m_roiUpdateTimer(new QTimer(this))
{
    ui->setupUi(this);
    setObjectName(QStringLiteral("Widget"));
    setAttribute(Qt::WA_StyledBackground, true);
    setWindowTitle(QStringLiteral("图像处理工具"));
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    // 侧栏最小宽度收窄，小窗口多留给画布
    ui->widget_2->setMinimumWidth(120);
    ui->widget_3->setMinimumWidth(180);
    if (auto *root = ui->horizontalLayout) {
        root->setSpacing(6);
        root->setContentsMargins(6, 6, 6, 6);
    }
    if (ui->horizontalLayout_2)
        ui->horizontalLayout_2->setSpacing(4);

    setupGraphicsView();
    setupDragDrop();
    setupBlockPanel();
    setupToolbar();

    // ROI 几何变化后短延迟重算（拖动过程防抖，松手后会立刻跟上）
    m_roiUpdateTimer->setSingleShot(true);
    m_roiUpdateTimer->setInterval(60);
    connect(m_roiUpdateTimer, &QTimer::timeout, this, &Widget::onApplyProcessing);

    connect(m_processor, &ImageProcessor::processingFinished,
            this, &Widget::onProcessingFinished);
    connect(m_processor, &ImageProcessor::requestReprocess,
            this, &Widget::onApplyProcessing);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::setupGraphicsView()
{
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing, false);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    // 不用 ScrollHandDrag：会抢走左键，导致 ROI 拖不动
    ui->graphicsView->setDragMode(QGraphicsView::NoDrag);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    ui->graphicsView->setCacheMode(QGraphicsView::CacheNone);
    ui->graphicsView->viewport()->installEventFilter(this);

    // ROI 移动/缩放时触发更新（刷新 pixmap 时会挂起，避免死循环）
    connect(m_scene, &QGraphicsScene::changed, this, [this](const QList<QRectF> &) {
        if (m_suspendSceneReprocess) return;
        if (!m_processor->hasImage()) return;
        if (m_blockList.isEmpty()) return;
        if (!hasAnyRoi()) return;
        m_roiUpdateTimer->start();
    });
}

void Widget::setupToolbar()
{
    // 按钮已在 widget.ui 中，这里只接线
    connect(ui->btnApply, &QPushButton::clicked, this, &Widget::onApplyProcessing);
    connect(ui->btnCompare, &QPushButton::clicked, this, &Widget::onToggleOriginal);
    connect(ui->btnSave, &QPushButton::clicked, this, &Widget::onSaveResult);
    connect(ui->btnClearChain, &QPushButton::clicked, this, &Widget::onClearBlocks);
}

void Widget::setupDragDrop()
{
    ui->listWidget->setDragEnabled(true);
    ui->listWidget->setDragDropMode(QAbstractItemView::DragOnly);
    ui->listWidget->setSpacing(2);

    ui->widget_2->setAttribute(Qt::WA_StyledBackground, true);
    ui->widget_3->setAttribute(Qt::WA_StyledBackground, true);
    ui->widget_3->setAcceptDrops(true);
    ui->widget_3->installEventFilter(this);
    setDropPanelHighlight(false);
}

void Widget::setupBlockPanel()
{
    auto *outer = qobject_cast<QVBoxLayout *>(ui->widget_3->layout());
    if (!outer) {
        outer = new QVBoxLayout(ui->widget_3);
        outer->setContentsMargins(10, 10, 10, 10);
        outer->setSpacing(6);
    }

    auto *container = new QWidget(ui->widget_3);
    container->setObjectName(QStringLiteral("blockListContainer"));
    container->setAttribute(Qt::WA_StyledBackground, true);

    m_blockLayout = new QVBoxLayout(container);
    m_blockLayout->setContentsMargins(0, 0, 0, 0);
    m_blockLayout->setSpacing(AppConfig::BLOCK_LAYOUT_SPACING);
    m_blockLayout->addStretch();

    int insertAt = 1;
    if (ui->chainHintLabel)
        insertAt = outer->indexOf(ui->chainHintLabel);
    if (insertAt < 0)
        insertAt = outer->count();
    outer->insertWidget(insertAt, container, 1);

    refreshChainHint();
}

void Widget::setDropPanelHighlight(bool on)
{
    ui->widget_3->setProperty("dragOver", on);
    ui->widget_3->style()->unpolish(ui->widget_3);
    ui->widget_3->style()->polish(ui->widget_3);
    ui->widget_3->update();
}

void Widget::refreshChainHint()
{
    if (!ui->chainHintLabel)
        return;
    ui->chainHintLabel->setVisible(m_blockList.isEmpty());
    ui->chainHintLabel->setText(
        m_blockList.isEmpty()
            ? QStringLiteral("拖入算法")
            : QString());
}

bool Widget::loadImageFromPath(const QString &filePath)
{
    QPixmap pixmap(filePath);
    if (pixmap.isNull())
        return false;

    clearAllRoi();
    m_showOriginal = false;
    if (ui->btnCompare)
        ui->btnCompare->setChecked(false);

    m_processor->setOriginalImage(pixmap);
    m_viewScale = 1.0;

    if (m_pixmapItem) {
        m_scene->removeItem(m_pixmapItem);
        delete m_pixmapItem;
    }
    m_pixmapItem = m_scene->addPixmap(pixmap);
    m_pixmapItem->setZValue(-1);
    m_scene->setSceneRect(pixmap.rect());
    ui->graphicsView->resetTransform();
    ui->graphicsView->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    m_viewScale = ui->graphicsView->transform().m11();

    if (!m_blockList.isEmpty())
        onApplyProcessing();
    else
        refreshDisplay();

    updateInfoLabel();
    setWindowTitle(QStringLiteral("图像处理工具 — %1  (%2 × %3)")
                       .arg(QFileInfo(filePath).fileName())
                       .arg(pixmap.width())
                       .arg(pixmap.height()));
    return true;
}

void Widget::on_pushButton_clicked()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择图片"), QString(), AppConfig::IMAGE_FILE_FILTER);
    if (filePath.isEmpty()) return;

    if (!loadImageFromPath(filePath))
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("无法加载图片，文件可能已损坏"));
}

void Widget::on_pushButton_2_clicked()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this, QStringLiteral("选择图片文件夹"));
    if (dir.isEmpty()) return;

    const QStringList filters = {
        QStringLiteral("*.png"), QStringLiteral("*.jpg"), QStringLiteral("*.jpeg"),
        QStringLiteral("*.bmp"), QStringLiteral("*.gif"), QStringLiteral("*.tif"),
        QStringLiteral("*.tiff")
    };
    QDir d(dir);
    const QFileInfoList files = d.entryInfoList(filters, QDir::Files, QDir::Name);
    if (files.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("该文件夹下没有找到图片文件"));
        return;
    }

    if (!loadImageFromPath(files.first().absoluteFilePath())) {
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("无法加载图片"));
        return;
    }

    QMessageBox::information(
        this, QStringLiteral("打开文件夹"),
        QStringLiteral("共找到 %1 张图片，已加载：%2")
            .arg(files.size())
            .arg(files.first().fileName()));
}

void Widget::wheelEvent(QWheelEvent *event)
{
    if (!m_pixmapItem) {
        event->ignore();
        return;
    }

    const double factor = AppConfig::SCROLL_SCALE_STEP;
    double next = m_viewScale;
    if (event->angleDelta().y() > 0)
        next *= factor;
    else
        next /= factor;

    if (next < AppConfig::MIN_SCALE_FACTOR || next > AppConfig::MAX_SCALE_FACTOR) {
        event->accept();
        return;
    }

    if (event->angleDelta().y() > 0)
        ui->graphicsView->scale(factor, factor);
    else
        ui->graphicsView->scale(1.0 / factor, 1.0 / factor);

    m_viewScale = next;
    updateInfoLabel();
    event->accept();
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        on_deltete_clicked();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

QPointF Widget::imageCenterInScene() const
{
    if (m_pixmapItem)
        return m_pixmapItem->boundingRect().center();
    return m_scene->sceneRect().center();
}

void Widget::clearAllRoi()
{
    auto wipe = [&](auto *&ptr) {
        if (!ptr) return;
        m_scene->removeItem(ptr);
        delete ptr;
        ptr = nullptr;
    };
    wipe(m_rectItem);
    wipe(m_ellipseItem);
    wipe(m_rotatedRectItem);
}

void Widget::addRectItem(qreal x, qreal y, qreal width, qreal height)
{
    clearAllRoi();
    m_rectItem = new ResizableRectItem(x - width / 2, y - height / 2, width, height);
    m_scene->addItem(m_rectItem);
    m_rectItem->setSelected(true);
}

void Widget::addEllipseItem(qreal x, qreal y, qreal w, qreal h)
{
    clearAllRoi();
    m_ellipseItem = new ResizableEllipseItem(x - w / 2, y - h / 2, w, h);
    m_scene->addItem(m_ellipseItem);
    m_ellipseItem->setSelected(true);
}

void Widget::addRotatedRectItem(qreal x, qreal y, qreal w, qreal h)
{
    clearAllRoi();
    m_rotatedRectItem = new ResizableRotatedRectItem(w, h);
    m_rotatedRectItem->setPos(x, y);
    m_rotatedRectItem->setRotation(0);
    m_scene->addItem(m_rotatedRectItem);
    m_rotatedRectItem->setSelected(true);
}

void Widget::on_comboBox_currentIndexChanged(int)
{
}

void Widget::on_pushButton_3_clicked()
{
    if (!m_pixmapItem) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先打开一张图片"));
        return;
    }

    const QPointF c = imageCenterInScene();
    const qreal size = AppConfig::DEFAULT_ROI_SIZE;
    const QString type = ui->comboBox->currentText();

    if (type == QStringLiteral("矩形"))
        addRectItem(c.x(), c.y(), size, size);
    else if (type == QStringLiteral("圆形"))
        addEllipseItem(c.x(), c.y(), size, size);
    else if (type == QStringLiteral("旋转矩形"))
        addRotatedRectItem(c.x(), c.y(), size * 1.4, size);
}

void Widget::on_deltete_clicked()
{
    const auto selected = m_scene->selectedItems();
    if (selected.isEmpty()) {
        clearAllRoi();
        return;
    }
    for (QGraphicsItem *item : selected) {
        if (item == m_rectItem) m_rectItem = nullptr;
        if (item == m_ellipseItem) m_ellipseItem = nullptr;
        if (item == m_rotatedRectItem) m_rotatedRectItem = nullptr;
        m_scene->removeItem(item);
        delete item;
    }
}

RoiInfo Widget::getCurrentRoiInfo() const
{
    RoiInfo info;
    if (!m_processor->hasImage())
        return info;

    if (m_rectItem) {
        info.shape = RoiInfo::Shape::Rect;
        info.rect = m_rectItem->mapToScene(m_rectItem->rect()).boundingRect();
        return info;
    }
    if (m_ellipseItem) {
        info.shape = RoiInfo::Shape::Ellipse;
        info.rect = m_ellipseItem->mapToScene(m_ellipseItem->rect()).boundingRect();
        return info;
    }
    if (m_rotatedRectItem) {
        info.shape = RoiInfo::Shape::RotatedRect;
        info.center = m_rotatedRectItem->mapToScene(m_rotatedRectItem->localRect().center());
        info.size = m_rotatedRectItem->localRect().size();
        info.angleDeg = m_rotatedRectItem->rotation();
        return info;
    }
    return info;
}

void Widget::onRoiGeometryChanged()
{
    onApplyProcessing();
}

void Widget::onApplyProcessing()
{
    if (!m_processor->hasImage())
        return;
    if (m_blockList.isEmpty()) {
        updatePixmapItem(m_processor->originalImage());
        return;
    }
    m_showOriginal = false;
    if (ui->btnCompare)
        ui->btnCompare->setChecked(false);
    m_processor->setRoi(getCurrentRoiInfo());
    m_processor->reprocess();
}

void Widget::onSaveResult()
{
    if (!m_processor->hasImage() || m_processor->resultImage().isNull()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("没有可保存的结果图"));
        return;
    }
    const QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("保存结果"), QStringLiteral("result.png"),
        QStringLiteral("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));
    if (path.isEmpty()) return;

    if (!m_processor->resultImage().save(path))
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("保存失败"));
}

void Widget::onClearBlocks()
{
    // 批量删除时先断开，避免每个块都触发一次重算
    disconnect(m_processor, &ImageProcessor::requestReprocess,
               this, &Widget::onApplyProcessing);

    while (!m_blockList.isEmpty()) {
        BaseBlock *block = m_blockList.takeFirst();
        m_blockLayout->removeWidget(block);
        m_processor->removeBlock(block);
        block->deleteLater();
    }

    connect(m_processor, &ImageProcessor::requestReprocess,
            this, &Widget::onApplyProcessing);

    refreshChainHint();
    if (m_processor->hasImage()) {
        m_showOriginal = false;
        if (ui->btnCompare)
            ui->btnCompare->setChecked(false);
        m_processor->setOriginalImage(m_processor->originalImage());
        updatePixmapItem(m_processor->originalImage());
        ui->label_3->setText(QStringLiteral("0 ms"));
    }
}

void Widget::onToggleOriginal()
{
    if (!m_processor->hasImage()) {
        if (ui->btnCompare) ui->btnCompare->setChecked(false);
        return;
    }
    m_showOriginal = ui->btnCompare && ui->btnCompare->isChecked();
    refreshDisplay();
}

void Widget::refreshDisplay()
{
    if (!m_processor->hasImage()) return;
    updatePixmapItem(m_showOriginal ? m_processor->originalImage()
                                    : m_processor->resultImage());
}

void Widget::updateInfoLabel()
{
    if (!ui->labelInfo) return;
    if (!m_processor->hasImage()) {
        ui->labelInfo->setText(QStringLiteral("-"));
        return;
    }
    const QPixmap &img = m_processor->originalImage();
    ui->labelInfo->setText(
        QStringLiteral("%1×%2 %3%")
            .arg(img.width())
            .arg(img.height())
            .arg(qRound(m_viewScale * 100)));
    ui->labelInfo->setToolTip(
        QStringLiteral("分辨率 %1×%2，缩放 %3%")
            .arg(img.width())
            .arg(img.height())
            .arg(qRound(m_viewScale * 100)));
}

void Widget::onProcessingFinished(qint64 elapsedMs)
{
    refreshDisplay();
    ui->label_3->setText(QStringLiteral("%1 ms").arg(elapsedMs));
    updateInfoLabel();
}

void Widget::updatePixmapItem(const QPixmap &pixmap)
{
    if (!m_pixmapItem) return;
    m_suspendSceneReprocess = true;
    m_pixmapItem->setPixmap(pixmap);
    m_suspendSceneReprocess = false;
}

bool Widget::hasAnyRoi() const
{
    return m_rectItem || m_ellipseItem || m_rotatedRectItem;
}

bool Widget::viewportPanEvent(QEvent *event)
{
    auto *view = ui->graphicsView;
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto *e = static_cast<QMouseEvent *>(event);
        QGraphicsItem *hit = view->itemAt(e->pos());
        // 点在 ROI 上时绝不抢事件，保证 ROI 可随时拖动/缩放
        const bool onRoi = hit
            && (hit == m_rectItem || hit == m_ellipseItem || hit == m_rotatedRectItem);
        if (onRoi)
            break;

        // 中键，或左键点在空白/图片上：拖动画布
        if (e->button() == Qt::MiddleButton
            || (e->button() == Qt::LeftButton && (hit == nullptr || hit == m_pixmapItem))) {
            m_panning = true;
            m_panLastPos = e->pos();
            view->viewport()->setCursor(Qt::ClosedHandCursor);
            return true;
        }
        break;
    }
    case QEvent::MouseMove: {
        if (!m_panning) break;
        auto *e = static_cast<QMouseEvent *>(event);
        const QPoint delta = e->pos() - m_panLastPos;
        m_panLastPos = e->pos();
        view->horizontalScrollBar()->setValue(
            view->horizontalScrollBar()->value() - delta.x());
        view->verticalScrollBar()->setValue(
            view->verticalScrollBar()->value() - delta.y());
        return true;
    }
    case QEvent::MouseButtonRelease: {
        auto *e = static_cast<QMouseEvent *>(event);
        if (m_panning && (e->button() == Qt::MiddleButton || e->button() == Qt::LeftButton)) {
            m_panning = false;
            view->viewport()->setCursor(Qt::ArrowCursor);
            return true;
        }
        break;
    }
    default:
        break;
    }
    return false;
}

bool Widget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->graphicsView->viewport()) {
        if (viewportPanEvent(event))
            return true;
        return QWidget::eventFilter(obj, event);
    }

    if (obj != ui->widget_3)
        return QWidget::eventFilter(obj, event);

    switch (event->type()) {
    case QEvent::DragEnter: {
        auto *e = static_cast<QDragEnterEvent *>(event);
        if (e->mimeData()->hasText()) {
            m_isDragInside = true;
            setDropPanelHighlight(true);
            e->acceptProposedAction();
            return true;
        }
        break;
    }
    case QEvent::DragLeave:
        if (m_isDragInside) {
            m_isDragInside = false;
            setDropPanelHighlight(false);
        }
        return true;
    case QEvent::Drop: {
        m_isDragInside = false;
        setDropPanelHighlight(false);
        auto *e = static_cast<QDropEvent *>(event);
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

void Widget::wireBinarizationOtsu(BinarizationBlock *block)
{
    if (!block) return;
    connect(block, &BinarizationBlock::otsuRequested, this, [this, block]() {
        if (!m_processor->hasImage()) {
            QMessageBox::information(this, QStringLiteral("提示"),
                                     QStringLiteral("请先打开图片"));
            return;
        }
        cv::Mat src = ImageConverter::pixmapToMatRGB(m_processor->originalImage());
        cv::cvtColor(src, src, cv::COLOR_RGB2BGR);

        // 若有 ROI，尽量只在 ROI 外包络内算 Otsu
        const RoiInfo roi = getCurrentRoiInfo();
        if (!roi.isEmpty()) {
            const QRectF br = roi.boundingRect();
            cv::Rect rr(qRound(br.x()), qRound(br.y()),
                        qRound(br.width()), qRound(br.height()));
            rr &= cv::Rect(0, 0, src.cols, src.rows);
            if (!rr.empty())
                src = src(rr).clone();
        }

        const int t = OtsuAlgorithm::calculateThresholdFromBGR(src);
        block->setThresholds(t, 255);
    });
}

void Widget::createBlockByName(const QString &name)
{
    BaseBlock *block = nullptr;

    if (name == AppConfig::BLOCK_NAME_BINARIZATION) {
        auto *bin = new BinarizationBlock(ui->widget_3);
        wireBinarizationOtsu(bin);
        block = bin;
    } else if (name == AppConfig::BLOCK_NAME_MORPHOLOGY) {
        block = new MorphologyBlock(ui->widget_3);
    } else if (name == AppConfig::BLOCK_NAME_FILTER) {
        block = new FilterBlock(ui->widget_3);
    } else if (name == AppConfig::BLOCK_NAME_GRAYTRANSFORM) {
        block = new GrayTransformBlock(ui->widget_3);
    } else if (name == AppConfig::BLOCK_NAME_PSEUDOCOLOR) {
        block = new PseudoColorBlock(ui->widget_3);
    }

    if (block)
        addBlockToPanel(block);
    else
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("未识别的算法：%1").arg(name));
}

void Widget::addBlockToPanel(BaseBlock *block)
{
    if (!block) return;

    m_blockLayout->insertWidget(m_blockLayout->count() - 1, block);
    m_blockList.append(block);

    connect(block, &BaseBlock::removeRequested, this, [this, block]() {
        m_blockLayout->removeWidget(block);
        m_blockList.removeOne(block);
        m_processor->removeBlock(block);
        block->deleteLater();
        refreshChainHint();
    });

    m_processor->addBlock(block);
    block->show();
    refreshChainHint();
}
