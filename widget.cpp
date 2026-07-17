#include "widget.h"
#include "ui_widget.h"
#include "opencv2/opencv.hpp"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , scene(new QGraphicsScene(this))
    , pixmapItem(nullptr)
    , rectItem(nullptr)
    , ellipseItem(nullptr)
    , scaleFactor(1.0)
{
    ui->setupUi(this);

    setWindowTitle("图片浏览器");
    setMouseTracking(true);
    // ========== 设置 GraphicsView ==========
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing, false);  // 关闭抗锯齿
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // 设置视图更新模式
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);  // 全视图更新
    ui->graphicsView->setCacheMode(QGraphicsView::CacheNone);  // 禁用视图缓存
    // ========== 设置 GraphicsView ==========
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // ========== 初始化 listWidget ==========
    // UI 里的 listWidget 已经被提升为 MyListWidget
    ui->listWidget->setDragEnabled(true);
    ui->listWidget->setDragDropMode(QAbstractItemView::DragOnly);

    // ========== 让 widget_3 接收拖放 ==========
    ui->widget_3->setAcceptDrops(true);
    ui->widget_3->installEventFilter(this);

    ui->widget_3->setStyleSheet(
        "QWidget#widget_3 {"
        "    background-color: #f5f5f5;"
        "    border: 2px dashed #bbb;"
        "    border-radius: 8px;"
        "}"
        );
    // ========== 给 widget_3 设置布局（用于排列处理块） ==========
    blockLayout = new QVBoxLayout(ui->widget_3);
    blockLayout->setContentsMargins(8, 8, 8, 8);   // 边距
    blockLayout->setSpacing(8);                     // 块之间的间距
    blockLayout->addStretch();                      // 底部弹性空间，让块从顶部排列
}

Widget::~Widget()
{
    delete ui;
}

// ========== 添加矩形框 ==========
void Widget::addRectItem(qreal x, qreal y, qreal width, qreal height)
{
    if (rectItem) {
        scene->removeItem(rectItem);
        delete rectItem;
    }

    rectItem = new ResizableRectItem(x, y, width, height);
    scene->addItem(rectItem);
    rectItem->setSelected(true);  // 自动显示调整手柄

    // ✅ 连接矩形改变信号
    connect(rectItem, &ResizableRectItem::rectChanged, [this]() {
        createBinarizationBlock();
        updatePixmapItem;
    });
}
void Widget::addEllipseItem(qreal x,qreal y,qreal w,qreal h)
{
    if(ellipseItem){
        scene->removeItem(ellipseItem);
        delete ellipseItem;
    }
    ellipseItem = new ResizableEllipseItem(x,y,w,h);
    scene->addItem(ellipseItem);
    ellipseItem->setSelected(true);
}
// ========== 事件过滤器：widget_3 接收拖放 ==========
bool Widget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->widget_3) {
        if (event->type() == QEvent::DragEnter) {
            QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent*>(event);
            if (dragEvent->mimeData()->hasText()) {
                isDragInside = true;
                qDebug() << "✅ 拖入 widget_3";
                ui->widget_3->setStyleSheet(
                    "QWidget#widget_3 {"
                    "    background-color: #e8f5e9;"
                    "    border: 2px solid #4CAF50;"
                    "    border-radius: 8px;"
                    "}"
                    );
                dragEvent->acceptProposedAction();
                return true;
            }
        }
        else if (event->type() == QEvent::DragLeave) {
            if (isDragInside) {
                isDragInside = false;
                qDebug() << "拖出 widget_3";
                ui->widget_3->setStyleSheet(
                    "QWidget#widget_3 {"
                    "    background-color: #f5f5f5;"
                    "    border: 2px dashed #bbb;"
                    "    border-radius: 8px;"
                    "}"
                    );
            }
            return true;
        }
        else if (event->type() == QEvent::Drop) {
            isDragInside = false;
            QDropEvent *dropEvent = static_cast<QDropEvent*>(event);

            ui->widget_3->setStyleSheet(
                "QWidget#widget_3 {"
                "    background-color: #f5f5f5;"
                "    border: 2px dashed #bbb;"
                "    border-radius: 8px;"
                "}"
                );

            if (dropEvent->mimeData()->hasText()) {
                QString text = dropEvent->mimeData()->text();
                qDebug() << "✅ widget_3 接收到：" << text;

                // ========== 根据拖入的内容创建不同的处理块 ==========
                if (text == "二值化处理") {
                    createBinarizationBlock();  // 创建二值化处理块
                }
                dropEvent->acceptProposedAction();
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

// ========== 打开图片 ==========
void Widget::on_pushButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择图片",
        "",
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.tiff);;所有文件 (*.*)"
        );

    if (filePath.isEmpty()) return;

    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "错误", "无法加载图片，文件可能已损坏");
        return;
    }

    currentPixmap = pixmap;
    originalPixmap = pixmap;  // 保存原始图片

    // ========== 在 GraphicsView 中显示图片 ==========
    if (pixmapItem) {
        scene->removeItem(pixmapItem);
        delete pixmapItem;
    }

    pixmapItem = scene->addPixmap(currentPixmap);
    scene->setSceneRect(currentPixmap.rect());

    // 自适应窗口
    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    setWindowTitle(QString("图片浏览器 — %1  (%2 x %3)")
                       .arg(filePath)
                       .arg(pixmap.width())
                       .arg(pixmap.height()));
}


// ========== 滚轮缩放 ==========
void Widget::wheelEvent(QWheelEvent *event)
{
    if (!pixmapItem) {
        event->ignore();
        return;
    }

    // 滚轮缩放
    double factor = 1.25;
    if (event->angleDelta().y() > 0) {
        ui->graphicsView->scale(factor, factor);
    } else {
        ui->graphicsView->scale(1.0 / factor, 1.0 / factor);
    }

    event->accept();
}


// ========== ComboBox 切换 ==========
void Widget::on_comboBox_currentIndexChanged(int index)
{
    qDebug() << "选择了：" << ui->comboBox->currentText();

    if (!pixmapItem) return;
}
void Widget::on_pushButton_3_clicked()
{
    if(ui->comboBox->currentText()=="矩形")
    {
        addRectItem(ui->graphicsView->size().width()/2.0,ui->graphicsView->size().height()/2.0,100,100);
    }
    if(ui->comboBox->currentText()=="圆形")
    {
        addEllipseItem(ui->graphicsView->size().width()/2.0,ui->graphicsView->size().height()/2.0,100,100);
    }
}

//删除ROI区域
void Widget::on_deltete_clicked()
{
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();

    for (QGraphicsItem* item : selectedItems) {
        // 先检查是否是追踪的图形，清空对应指针
        if (item == rectItem) {
            rectItem = nullptr;       //  先清空指针
        }
        if (item == ellipseItem) {
            ellipseItem = nullptr;    //  先清空指针
        }

        scene->removeItem(item);  // 从场景移除
        delete item;              // 析构对象
    }
}
// ========== 创建二值化处理块（追加到列表末尾） ==========
void Widget::createBinarizationBlock()
{
    // ========== 创建主容器 ==========
    QWidget *block = new QWidget(ui->widget_3);
    block->setStyleSheet(
        "QWidget {"
        "    background-color: white;"
        "    border: 1px solid #ddd;"
        "    border-radius: 8px;"
        "}"
        );
    block->setMinimumHeight(160);  // 最小高度

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(block);
    mainLayout->setContentsMargins(12, 8, 12, 8);
    mainLayout->setSpacing(6);

    // ========== 标题行 ==========
    QHBoxLayout *titleLayout = new QHBoxLayout();

    QLabel *iconLabel = new QLabel("🔲", block);
    iconLabel->setStyleSheet("font-size: 18px; border: none;");

    QLabel *titleLabel = new QLabel("二值化处理", block);
    titleLabel->setStyleSheet(
        "font-size: 14px;"
        "font-weight: bold;"
        "color: #333;"
        "border: none;"
        );

    // 删除按钮
    QPushButton *deleteBtn = new QPushButton("✕", block);
    deleteBtn->setFixedSize(24, 24);
    deleteBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #ff5252;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 12px;"
        "    font-size: 12px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #ff1744;"
        "}"
        );

    // 使能复选框
    QCheckBox *enableCheckBox = new QCheckBox(block);
    enableCheckBox->setToolTip("启用二值化处理");

    titleLayout->addWidget(iconLabel);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(enableCheckBox);
    titleLayout->addWidget(deleteBtn);

    mainLayout->addLayout(titleLayout);

    // ========== 分隔线 ==========
    QFrame *line = new QFrame(block);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #eee; border: none;");
    mainLayout->addWidget(line);


    // ========== 下限值 ==========
    QHBoxLayout *lowerLayout = new QHBoxLayout();
    QLabel *lowerLabel = new QLabel("下限值：", block);
    lowerLabel->setStyleSheet("font-size: 12px; color: #666; border: none;");
    lowerLabel->setFixedWidth(55);  // ✅ 固定标签宽度，对齐

    QSpinBox *lowerSpinBox = new QSpinBox(block);
    lowerSpinBox->setRange(0, 255);
    lowerSpinBox->setValue(0);
    lowerSpinBox->setFixedWidth(80);  // ✅ 固定输入框宽度
    lowerLayout->addWidget(lowerLabel);
    lowerLayout->addWidget(lowerSpinBox);
    lowerLayout->addStretch();  // 把剩余空间推到右边

    mainLayout->addLayout(lowerLayout);
    lowerSpinBox->setStyleSheet(
        "QSpinBox {"
        "    padding: 3px 6px;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    font-size: 12px;"
        "}"
        // ========== 上按钮样式 ==========
        "QSpinBox::up-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: top right;"  // 右上角
        "    width: 20px;"
        "    height: 12px;"                    // 一半高度
        "    border-left: 1px solid #ddd;"
        "    border-bottom: 1px solid #ddd;"   // 下边框分隔
        "    border-top-right-radius: 4px;"
        "}"
        // ========== 下按钮样式 ==========
        "QSpinBox::down-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: bottom right;" // 右下角
        "    width: 20px;"
        "    height: 12px;"                      // 一半高度
        "    border-left: 1px solid #ddd;"
        "    border-bottom-right-radius: 4px;"
        "}"
        // ========== 上按钮悬停 ==========
        "QSpinBox::up-button:hover {"
        "    background-color: #e0e0e0;"
        "}"
        // ========== 下按钮悬停 ==========
        "QSpinBox::down-button:hover {"
        "    background-color: #e0e0e0;"
        "}"
        // ========== 上按钮按下 ==========
        "QSpinBox::up-button:pressed {"
        "    background-color: #ccc;"
        "}"
        // ========== 下按钮按下 ==========
        "QSpinBox::down-button:pressed {"
        "    background-color: #ccc;"
        "}"
        // ========== 箭头图标 ==========
        "QSpinBox::up-arrow {"
        "    width: 6px;"
        "    height: 6px;"
        "}"
        "QSpinBox::down-arrow {"
        "    width: 6px;"
        "    height: 6px;"
        "}"
        );
    // ========== 上限值 ==========
    QHBoxLayout *upperLayout = new QHBoxLayout();
    QLabel *upperLabel = new QLabel("上限值：", block);
    upperLabel->setStyleSheet("font-size: 12px; color: #666; border: none;");
    upperLabel->setFixedWidth(55);  // ✅ 固定标签宽度

    QSpinBox *upperSpinBox = new QSpinBox(block);
    upperSpinBox->setRange(0, 255);
    upperSpinBox->setValue(255);
    upperSpinBox->setFixedWidth(80);  // ✅ 固定输入框宽度
    upperLayout->addWidget(upperLabel);
    upperLayout->addWidget(upperSpinBox);
    upperLayout->addStretch();  // 把剩余空间推到右边

    mainLayout->addLayout(upperLayout);
    upperSpinBox->setStyleSheet(
        "QSpinBox {"
        "    padding: 3px 6px;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    font-size: 12px;"
        "}"
        // ========== 上按钮样式 ==========
        "QSpinBox::up-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: top right;"  // 右上角
        "    width: 20px;"
        "    height: 12px;"                    // 一半高度
        "    border-left: 1px solid #ddd;"
        "    border-bottom: 1px solid #ddd;"   // 下边框分隔
        "    border-top-right-radius: 4px;"
        "}"
        // ========== 下按钮样式 ==========
        "QSpinBox::down-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: bottom right;" // 右下角
        "    width: 20px;"
        "    height: 12px;"                      // 一半高度
        "    border-left: 1px solid #ddd;"
        "    border-bottom-right-radius: 4px;"
        "}"
        // ========== 上按钮悬停 ==========
        "QSpinBox::up-button:hover {"
        "    background-color: #e0e0e0;"
        "}"
        // ========== 下按钮悬停 ==========
        "QSpinBox::down-button:hover {"
        "    background-color: #e0e0e0;"
        "}"
        // ========== 上按钮按下 ==========
        "QSpinBox::up-button:pressed {"
        "    background-color: #ccc;"
        "}"
        // ========== 下按钮按下 ==========
        "QSpinBox::down-button:pressed {"
        "    background-color: #ccc;"
        "}"
        // ========== 箭头图标 ==========
        "QSpinBox::up-arrow {"
        "    width: 6px;"
        "    height: 6px;"
        "}"
        "QSpinBox::down-arrow {"
        "    width: 6px;"
        "    height: 6px;"
        "}"
        );

    // ========== 自动设置按钮 ==========
    QPushButton *autoBtn = new QPushButton("自动设置", block);
    autoBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 6px 12px;"
        "    font-size: 12px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "}"
        );

    mainLayout->addWidget(autoBtn);

    // ========== 初始状态：启用 ==========
    enableCheckBox->setChecked(true);
    lowerSpinBox->setEnabled(true);
    upperSpinBox->setEnabled(true);
    autoBtn->setEnabled(true);

    // ========== 信号槽连接（使用局部变量捕获） ==========

    // ========== 使能复选框 ==========
    connect(enableCheckBox, &QCheckBox::toggled, [this, lowerSpinBox, upperSpinBox,autoBtn](bool checked) {
        lowerSpinBox->setEnabled(checked);
        upperSpinBox->setEnabled(checked);
        autoBtn->setEnabled(checked);

        if (checked) {
            // ✅ 启用：对矩形区域做二值化
            QPixmap result = applyBinaryThreshold(lowerSpinBox->value(),
                                                  upperSpinBox->value());
            updatePixmapItem(result);
        } else {
            // ⛔ 禁用：恢复原图
            updatePixmapItem(originalPixmap);
        }
    });

    // ========== 下限值改变 ==========
    connect(lowerSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, enableCheckBox, lowerSpinBox, upperSpinBox](int value) {
                if (value > upperSpinBox->value()) {
                    upperSpinBox->setValue(value);
                }
                if (enableCheckBox->isChecked()) {
                    QPixmap result = applyBinaryThreshold(value, upperSpinBox->value());
                    updatePixmapItem(result);
                }
            });

    // ========== 上限值改变 ==========
    connect(upperSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this, enableCheckBox, lowerSpinBox, upperSpinBox](int value) {
                if (value < lowerSpinBox->value()) {
                    lowerSpinBox->setValue(value);
                }
                if (enableCheckBox->isChecked()) {
                    QPixmap result = applyBinaryThreshold(lowerSpinBox->value(), value);
                    updatePixmapItem(result);
                }
            });

    // ========== 自动设置 ==========
    connect(autoBtn, &QPushButton::clicked, [this, lowerSpinBox, upperSpinBox, enableCheckBox]() {
        int autoThreshold = calculateOtsuThreshold();
        lowerSpinBox->setValue(autoThreshold);
        upperSpinBox->setValue(255);
        enableCheckBox->setChecked(true);
    });
    // 删除按钮
    connect(deleteBtn, &QPushButton::clicked, [this, block]() {
        blockLayout->removeWidget(block);  // 从布局移除
        blockList.removeOne(block);        // 从列表移除
        block->deleteLater();              // 延迟删除
    });
    //矩形改变连接
    connect(rectItem, &ResizableRectItem::rectChanged, [this, enableCheckBox, lowerSpinBox, upperSpinBox]() {
        applyBinaryThreshold(lower,upper)
    });

    // ========== 添加到布局和列表 ==========
    // 在底部弹性空间之前插入
    blockLayout->insertWidget(blockLayout->count() - 1, block);
    blockList.append(block);

    block->show();


}

// ========== Otsu 自动阈值计算 ==========
int Widget::calculateOtsuThreshold()
{
    if (currentPixmap.isNull()) {
        QMessageBox::warning(this, "提示", "请先打开一张图片！");
        return 128;
    }

    // ========== QPixmap 转 cv::Mat ==========
    QImage image = currentPixmap.toImage().convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(image.height(), image.width(), CV_8UC3,
                const_cast<uchar*>(image.constBits()),
                image.bytesPerLine());
    cv::Mat matRGB = mat.clone();  // 复制一份（避免 const 问题）

    // ========== 转为灰度图 ==========
    cv::Mat gray;
    cv::cvtColor(matRGB, gray, cv::COLOR_RGB2GRAY);

    // ========== Otsu 自动计算阈值 ==========
    // threshold 函数使用 Otsu 会自动计算最优阈值
    // 返回值就是 Otsu 算法算出的最优阈值
    cv::Mat binary;
    double otsuThreshold = cv::threshold(gray, binary, 0, 255,
                                         cv::THRESH_BINARY | cv::THRESH_OTSU);

    qDebug() << "OpenCV Otsu 自动阈值：" << otsuThreshold;

    return static_cast<int>(otsuThreshold);
}

// ========== 对矩形区域做二值化 ==========
QPixmap Widget::applyBinaryThreshold(int lower, int upper)
{
    // 没有图片或没有矩形，返回原图
    if (originalPixmap.isNull()) return QPixmap();
    if (!rectItem) return originalPixmap;

    // 获取原始图片
    QImage image = originalPixmap.toImage().convertToFormat(QImage::Format_Grayscale8);

    // 获取矩形在场景中的位置和大小
    QRectF rectScene = rectItem->mapToScene(rectItem->rect()).boundingRect();

    // 场景坐标转图片坐标（需要考虑缩放）
    // pixmapItem 的位置就是场景原点，所以场景坐标 = 图片坐标
    QRect rect(
        qMax(0, (int)rectScene.x()),
        qMax(0, (int)rectScene.y()),
        qMin(image.width() - (int)rectScene.x(), (int)rectScene.width()),
        qMin(image.height() - (int)rectScene.y(), (int)rectScene.height())
        );

    // 确保矩形在图片范围内
    rect = rect.intersected(image.rect());

    if (rect.isEmpty()) return originalPixmap;

    // ========== 只对矩形区域内的像素做二值化 ==========
    for (int y = rect.top(); y <= rect.bottom(); y++) {
        uchar *line = image.scanLine(y);
        for (int x = rect.left(); x <= rect.right(); x++) {
            if (line[x] >= lower && line[x] <= upper) {
                line[x] = 255;  // 白色
            } else {
                line[x] = 0;    // 黑色
            }
        }
    }

    return QPixmap::fromImage(image);
}

void Widget::updatePixmapItem(const QPixmap &pixmap)
{
    if (pixmapItem) {
        pixmapItem->setPixmap(pixmap);
    }
}