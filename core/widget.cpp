/**
 * @file widget.cpp
 * @brief 主窗口实现（本项目最核心的 UI 文件，建议精读）
 *
 * =============================================================================
 * 一、本文件在整条链路中的位置
 * =============================================================================
 *
 *   main.cpp
 *     └─ 创建 Widget 并 show()
 *          │
 *          ├─ 用户打开图片 ──► loadImageFromPath()
 *          │                      └─ ImageProcessor::setOriginalImage()
 *          │                      └─ 场景里显示 QGraphicsPixmapItem
 *          │
 *          ├─ 用户拖入算法 ──► eventFilter(Drop)
 *          │                      └─ createBlockByName()
 *          │                      └─ addBlockToPanel()
 *          │                      └─ ImageProcessor::addBlock()
 *          │
 *          ├─ 用户调参 / 开关块 ──► BaseBlock::paramsChanged
 *          │                          └─ ImageProcessor::requestReprocess
 *          │                          └─ onApplyProcessing()   ◄── 统一重算入口
 *          │
 *          ├─ 用户拖 ROI ──► QGraphicsScene::changed
 *          │                    └─ m_roiUpdateTimer (60ms 防抖)
 *          │                    └─ onApplyProcessing()
 *          │
 *          └─ onApplyProcessing()
 *                ├─ getCurrentRoiInfo()      // 图元 → 纯数据
 *                ├─ ImageProcessor::setRoi()
 *                ├─ ImageProcessor::reprocess()  // 按块顺序 process()
 *                └─ processingFinished → onProcessingFinished() → 刷新画布
 *
 * =============================================================================
 * 二、建议阅读顺序（带行号大致位置）
 * =============================================================================
 *   1. 构造函数 Widget::Widget          —— 初始化 + 信号槽总接线
 *   2. loadImageFromPath                —— 图片如何进系统
 *   3. createBlockByName / addBlockToPanel —— 算法块如何进处理链
 *   4. onApplyProcessing                —— 【最重要】何时、如何重算
 *   5. getCurrentRoiInfo                —— ROI 图元如何变成算法参数
 *   6. eventFilter / viewportPanEvent   —— 拖放建块 + 画布平移
 *   7. onProcessingFinished             —— 结果如何画回屏幕
 *
 * =============================================================================
 * 三、界面控件对应关系（来自 widget.ui）
 * =============================================================================
 *   listWidget     左侧算法工具箱（可拖出名字）
 *   graphicsView   中间画布（显示图 + ROI）
 *   widget_3       右侧处理链面板（放置 BaseBlock）
 *   comboBox       ROI 类型：矩形 / 圆形 / 旋转矩形
 *   pushButton     打开单张图
 *   pushButton_2   打开文件夹
 *   pushButton_3   添加 ROI
 *   deltete        删除 ROI（拼写是历史遗留）
 *   btnApply       手动应用处理
 *   btnCompare     按住看原图，松开看结果
 *   btnSave        保存结果
 *   btnClearChain  清空处理链
 *   label_3        显示处理耗时（如 "12 ms"）
 *   labelInfo      显示分辨率与缩放百分比
 */

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
#include <QListWidgetItem>
#include <QIcon>
#include <QAbstractItemView>
#include <QScrollArea>
#include <QFrame>
#include <QLayout>
#include <QCursor>
#include <QSizePolicy>

// ============================================================================
// 构造 / 析构 / 初始化
// ============================================================================

/**
 * @brief 主窗口构造：把 UI、画布、拖放、工具栏、信号槽全部接好
 *
 * 初始化成员（初始化列表）：
 *   - ui          ：由 Qt Designer 生成的界面对象
 *   - m_scene     ：图形场景，里面放“底图 pixmap + ROI 图元”
 *   - m_processor ：处理链引擎（本窗口的子 QObject，随窗口销毁）
 *   - m_roiUpdateTimer ：ROI 拖动防抖定时器
 *
 * 注意：这里只做“搭架子 + 接线”，真正处理图片发生在用户操作之后。
 */
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_scene(new QGraphicsScene(this))
    , m_processor(new ImageProcessor(this))
    , m_roiUpdateTimer(new QTimer(this))
{
    // ---------- 1. 加载 .ui 文件定义的布局与控件 ----------
    // setupUi 会创建 listWidget、graphicsView、各种按钮等，并挂到 this 上
    ui->setupUi(this);

    // objectName 供 QSS（app.qss）按名字匹配样式，例如 #Widget { ... }
    setObjectName(QStringLiteral("Widget"));

    // 允许用样式表画背景（否则有些平台背景色不生效）
    setAttribute(Qt::WA_StyledBackground, true);

    setWindowTitle(QStringLiteral("图像处理工具"));

    // 即使没按下鼠标也追踪移动（方便以后扩展悬停提示等）
    setMouseTracking(true);

    // StrongFocus：窗口能收到键盘事件（Delete 删 ROI 需要这个）
    setFocusPolicy(Qt::StrongFocus);

    // ---------- 2. 微调布局：侧栏别占太宽，画布多留点 ----------
    // widget_2 = 左侧工具箱区域；widget_3 = 右侧处理链区域
    ui->widget_2->setMinimumWidth(120);
    ui->widget_3->setMinimumWidth(180);
    if (auto *root = ui->horizontalLayout) {//用if防止为空指针导致崩溃
        root->setSpacing(6);
        root->setContentsMargins(6, 6, 6, 6);
    }
    if (ui->horizontalLayout_2)
        ui->horizontalLayout_2->setSpacing(4);

    // ---------- 3. 分模块初始化 ----------
    setupGraphicsView();  // 中间：场景、缩放锚点、ROI 变化监听
    setupDragDrop();      // 左拖右放：算法名字的拖放通道
    setupBlockPanel();    // 右侧：用来垂直排列各个 BaseBlock 的布局
    setupFolderBrowser(); // 画布下方文件夹缩略图（默认隐藏）

    // ---------- 4. ROI 防抖定时器 ----------
    // 问题：拖动 ROI 时 scene.changed 会疯狂触发，若每次都跑 OpenCV 会卡死
    // 做法：每次 changed 只 restart 一个 60ms 的单次定时器
    //       用户停手约 60ms 后才真正 onApplyProcessing()
    // singleShot=true：超时只触发一次，不会周期性重复
    m_roiUpdateTimer->setSingleShot(true);
    m_roiUpdateTimer->setInterval(60);
    connect(m_roiUpdateTimer, &QTimer::timeout, this, &Widget::onApplyProcessing);

    // ---------- 5. 与 ImageProcessor 的两条核心信号 ----------
    // (A) 算完了 → 刷新画布 + 显示耗时
    connect(m_processor, &ImageProcessor::processingFinished,
            this, &Widget::onProcessingFinished);

    // (B) 块参数/使能/加删块变化 → 不要直接 reprocess，
    //     先走 onApplyProcessing，好把最新 ROI 同步进去
    connect(m_processor, &ImageProcessor::requestReprocess,
            this, &Widget::onApplyProcessing);
}

/**
 * @brief 析构：释放 UI 对象
 *
 * m_scene / m_processor / m_roiUpdateTimer 都以 this 为 parent，
 * Qt 父子机制会自动 delete，不必手动写。
 * ui 是 new 出来的、没有 parent，必须手动 delete。
 */
Widget::~Widget()
{
    delete ui;
}

/**
 * @brief 配置中间画布 QGraphicsView
 *
 * 坐标系约定（很重要）：
 *   - 场景坐标 = 图像像素坐标（pixmap 放在 (0,0)，宽高=图像宽高）
 *   - 因此 ROI 的 scene 坐标可以直接当像素坐标交给 OpenCV
 *
 * 为什么不用 ScrollHandDrag？
 *   Qt 自带的“抓手拖拽”会占用左键，和 ROI 的拖动/缩放冲突。
 *   所以 DragMode=NoDrag，平移自己在 viewportPanEvent 里实现。
 */
void Widget::setupGraphicsView()
{
    // 把场景挂到视图上：之后 addItem 的内容都会显示在 graphicsView 里
    ui->graphicsView->setScene(m_scene);

    // 抗锯齿关掉：大图缩放时更快；平滑变换打开：缩放图片不那么糊
    ui->graphicsView->setRenderHint(QPainter::Antialiasing, false);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);

    // 不用内置拖拽模式，避免抢 ROI 的鼠标事件
    ui->graphicsView->setDragMode(QGraphicsView::NoDrag);

    // 滚轮缩放时，以鼠标下方为锚点（缩放中心跟着鼠标）
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // 全视口刷新：ROI 边框变化时少花屏（略损性能，但交互更稳）
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    ui->graphicsView->setCacheMode(QGraphicsView::CacheNone);

    // 在 viewport 上装事件过滤器 → eventFilter / viewportPanEvent 能收到鼠标
    ui->graphicsView->viewport()->installEventFilter(this);

    // ---------- ROI 几何变化 → 防抖重算 ----------
    // scene.changed 在任何图元位置/形状变化时都会发（包括我们改 pixmap）
    // 所以必须用 m_suspendSceneReprocess 在“只是换显示图”时挂起，否则死循环：
    //   重算 → 改 pixmap → changed → 再重算 → ...
    connect(m_scene, &QGraphicsScene::changed, this, [this](const QList<QRectF> &) {
        if (m_suspendSceneReprocess) return;   // 正在 updatePixmapItem，忽略
        if (!m_processor->hasImage()) return;   // 还没图，谈不上处理
        if (m_blockList.isEmpty()) return;      // 没有算法块，ROI 动了也不用算
        if (!hasAnyRoi()) return;               // 没有 ROI：算法本来就是全图，ROI 事件无意义
        m_roiUpdateTimer->start();              // 重新开始 60ms 倒计时（防抖）
    });
}

/**
 * @brief 工具栏四个按钮接线（按钮控件本身在 .ui 里已画好）
 *
 * btnApply      → 手动触发 onApplyProcessing（一般调参已自动重算，此钮作兜底）
 * btnCompare    → 按住显示原图，松开恢复结果（不重新计算）
 * btnSave       → 把 resultImage 存盘
 * btnClearChain → 删掉右侧所有处理块，恢复显示原图
 */

/**
 * @brief 配置“左拖右放”通道
 *
 * 数据流：
 *   用户按住 listWidget 某项拖动
 *     → MyListWidget::mimeData 把项文字（如“二值化处理”）放进 QMimeData
 *     → 拖到 widget_3 上方
 *     → eventFilter 收到 DragEnter / Drop
 *     → createBlockByName(文字)
 */
void Widget::setupDragDrop()
{
    // 左侧：只允许拖出（DragOnly），不允许把东西拖回来
    ui->listWidget->setDragEnabled(true);
    ui->listWidget->setDragDropMode(QAbstractItemView::DragOnly);
    ui->listWidget->setSpacing(2);

    // WA_StyledBackground：让 QSS 的 background 能画在这两个面板上
    ui->widget_2->setAttribute(Qt::WA_StyledBackground, true);
    ui->widget_3->setAttribute(Qt::WA_StyledBackground, true);

    // 右侧必须显式接受 drop，并且用 eventFilter 处理（而不是子类化 widget_3）
    ui->widget_3->setAcceptDrops(true);
    ui->widget_3->installEventFilter(this);
    setDropPanelHighlight(false);
}

/**
 * @brief 接上设计器里的处理链滚动区（blockScrollArea + blockListContainer）
 *
 * 布局已在 widget.ui 中画好：
 *   标题 → 提示 → QScrollArea（可上下滚）→ blockListContainer
 * 这里只做：取布局指针、加底部 stretch、保证块不被压扁。
 */
void Widget::setupBlockPanel()
{
    // 使用 .ui 里的滚动区域与容器，不再代码里 new 布局树
    if (!ui->blockListContainer || !ui->blockListLayout) {
        QMessageBox::warning(this, QStringLiteral("界面错误"),
                             QStringLiteral("缺少 blockListContainer，请检查 widget.ui"));
        return;
    }

    ui->blockListContainer->setAttribute(Qt::WA_StyledBackground, true);
    m_blockLayout = ui->blockListLayout;
    m_blockLayout->setSpacing(AppConfig::BLOCK_LAYOUT_SPACING);
    m_blockLayout->setAlignment(Qt::AlignTop);
    // 容器高度至少等于所有块内容之和 → 超出视口时出现滚动条，块不被挤扁
    m_blockLayout->setSizeConstraint(QLayout::SetMinimumSize);

    if (ui->blockScrollArea) {
        ui->blockScrollArea->setWidgetResizable(true);
        ui->blockScrollArea->setFrameShape(QFrame::NoFrame);
        ui->blockScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->blockScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        // 允许拖到滚动区上松手也能建块
        ui->blockScrollArea->setAcceptDrops(true);
        ui->blockScrollArea->viewport()->setAcceptDrops(true);
        ui->blockScrollArea->installEventFilter(this);
        ui->blockScrollArea->viewport()->installEventFilter(this);
    }
    ui->blockListContainer->setAcceptDrops(true);
    ui->blockListContainer->installEventFilter(this);

    refreshChainHint();
}

/**
 * @brief 配置画布下方的文件夹缩略图条（控件在 widget.ui 里）
 *
 * 横向 IconMode 列表：点击项切换当前图片。
 * 默认隐藏，打开文件夹后才显示。
 */
void Widget::setupFolderBrowser()
{
    if (!ui->folderImageList)
        return;

    ui->folderImageList->setViewMode(QListView::IconMode);
    ui->folderImageList->setFlow(QListView::LeftToRight);
    ui->folderImageList->setWrapping(false);
    ui->folderImageList->setResizeMode(QListView::Adjust);
    ui->folderImageList->setMovement(QListView::Static);
    ui->folderImageList->setIconSize(QSize(64, 64));
    ui->folderImageList->setGridSize(QSize(86, 88));
    ui->folderImageList->setSpacing(4);
    ui->folderImageList->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->folderImageList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->folderImageList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->folderImageList->setSelectionMode(QAbstractItemView::SingleSelection);
}

void Widget::clearFolderBrowser()
{
    if (ui->folderImageList)
        ui->folderImageList->clear();
    if (ui->folderBrowserPanel)
        ui->folderBrowserPanel->setVisible(false);
    if (ui->folderBrowserTitle)
        ui->folderBrowserTitle->setText(QStringLiteral("文件夹图片（点击切换）"));
}

/**
 * @brief 填充文件夹缩略图，并显示面板
 */
void Widget::fillFolderBrowser(const QString &dirPath, const QFileInfoList &files)
{
    if (!ui->folderImageList || !ui->folderBrowserPanel)
        return;

    ui->folderImageList->clear();

    for (const QFileInfo &fi : files) {
        const QString path = fi.absoluteFilePath();
        QPixmap thumb(path);
        if (thumb.isNull())
            continue;
        thumb = thumb.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        auto *item = new QListWidgetItem(QIcon(thumb), fi.fileName(), ui->folderImageList);
        item->setData(Qt::UserRole, path);
        item->setToolTip(path);
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
        item->setSizeHint(QSize(84, 84));
    }

    if (ui->folderBrowserTitle) {
        ui->folderBrowserTitle->setText(
            QStringLiteral("文件夹：%1（%2 张，点击切换）")
                .arg(QDir(dirPath).dirName())
                .arg(ui->folderImageList->count()));
    }

    ui->folderBrowserPanel->setVisible(ui->folderImageList->count() > 0);
}

void Widget::selectFolderThumbnail(const QString &filePath)
{
    if (!ui->folderImageList || filePath.isEmpty())
        return;

    ui->folderImageList->blockSignals(true);
    for (int i = 0; i < ui->folderImageList->count(); ++i) {
        QListWidgetItem *item = ui->folderImageList->item(i);
        if (item && item->data(Qt::UserRole).toString() == filePath) {
            ui->folderImageList->setCurrentItem(item);
            ui->folderImageList->scrollToItem(item, QAbstractItemView::PositionAtCenter);
            break;
        }
    }
    ui->folderImageList->blockSignals(false);
}

/**
 * @brief 拖拽悬停时高亮右侧面板
 *
 * 原理：给 widget_3 设动态属性 dragOver=true/false，
 * QSS 里可以写 #widget_3[dragOver="true"] { border: ... }
 * 改属性后必须 unpolish/polish 才能让样式立刻重算。
 */
void Widget::setDropPanelHighlight(bool on)
{
    ui->widget_3->setProperty("dragOver", on);
    ui->widget_3->style()->unpolish(ui->widget_3);
    ui->widget_3->style()->polish(ui->widget_3);
    ui->widget_3->update();
}

/**
 * @brief 处理链为空时显示“拖入算法”提示；有块则隐藏
 */
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

/**
 * @brief 从磁盘路径加载一张图，送入 processor + 画布
 *
 * 调用时机：
 *   - on_pushButton_clicked（打开文件）
 *   - on_pushButton_2_clicked（打开文件夹后取第一张）
 *
 * 步骤拆解：
 *   1. QPixmap 读文件；失败返回 false
 *   2. 清掉旧 ROI、退出对比模式（新图不应沿用旧选区）
 *   3. processor 记住原图（结果先等于原图）
 *   4. 场景里换一张底图，fitInView 适配窗口
 *   5. 若已有处理块 → 立刻按当前参数重跑；否则只刷新显示
 *   6. 更新信息栏和窗口标题
 *
 * @return true=加载成功；false=文件坏了或不是图
 */
bool Widget::loadImageFromPath(const QString &filePath)
{
    // QPixmap 构造函数会按后缀解码；失败时 isNull()==true
    QPixmap pixmap(filePath);
    if (pixmap.isNull())
        return false;

    // ----- 换图时的状态重置 -----
    clearAllRoi();           // 旧 ROI 坐标对新图无意义
    m_showOriginal = false;  // 退出按住对比

    // 引擎保存原图；同时 m_result 被设成原图副本
    m_processor->setOriginalImage(pixmap);
    m_viewScale = 1.0;  // 稍后 fitInView 后再读真实缩放

    // ----- 更新场景中的底图 -----
    // 先拆掉旧的 pixmap item，再 add 新的
    if (m_pixmapItem) {
        m_scene->removeItem(m_pixmapItem);
        delete m_pixmapItem;
    }
    m_pixmapItem = m_scene->addPixmap(pixmap);
    // Z=-1：保证以后加的 ROI（默认 Z=0）画在图片上方，才能点到 ROI
    m_pixmapItem->setZValue(-1);

    // 场景矩形 = 图像像素矩形，这样场景坐标直接对应像素
    m_scene->setSceneRect(pixmap.rect());

    // 重置之前的缩放/平移，再按窗口大小等比铺满
    ui->graphicsView->resetTransform();
    ui->graphicsView->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);

    // m11() 是变换矩阵的 X 缩放分量；fitInView 后通常 < 1（缩略显示）
    m_viewScale = ui->graphicsView->transform().m11();

    // 处理链里如果已经有块（用户先拖算法再开图，或换图），立刻重算
    if (!m_blockList.isEmpty())
        onApplyProcessing();
    else
        refreshDisplay();  // 没有块：显示原图即可

    updateInfoLabel();
    setWindowTitle(QStringLiteral("图像处理工具 — %1  (%2 × %3)")
                       .arg(QFileInfo(filePath).fileName())
                       .arg(pixmap.width())
                       .arg(pixmap.height()));
    m_currentImagePath = filePath;
    return true;
}

// ============================================================================
// 打开图片 / 缩放 / 键盘
// ============================================================================

/**
 * @brief 槽：点击“打开图片”按钮（pushButton）
 *
 * Qt 的自动连接约定：on_<objectName>_<signal>
 * 所以函数名必须叫 on_pushButton_clicked，才会在 setupUi 时自动接上。
 */
void Widget::on_pushButton_clicked()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择图片"), QString(), AppConfig::IMAGE_FILE_FILTER);
    if (filePath.isEmpty()) return;  // 用户点了取消

    // 单张打开时收起文件夹缩略图条
    clearFolderBrowser();

    if (!loadImageFromPath(filePath))
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("无法加载图片，文件可能已损坏"));
}

/**
 * @brief 槽：点击“打开文件夹”（pushButton_2）
 *
 * 扫描文件夹图片 → 画布下方显示缩略图条 → 加载第一张。
 * 之后点击缩略图即可切换（on_folderImageList_itemClicked）。
 */
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

    fillFolderBrowser(dir, files);

    const QString firstPath = files.first().absoluteFilePath();
    if (!loadImageFromPath(firstPath)) {
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("无法加载图片"));
        return;
    }
    selectFolderThumbnail(firstPath);
}

/**
 * @brief 点击文件夹缩略图：切换到对应图片
 *
 * 路径存在 item 的 Qt::UserRole 里；当前已是该图则忽略。
 * 处理链保留：换图后若有块会自动重算（loadImageFromPath 内）。
 */
void Widget::on_folderImageList_itemClicked(QListWidgetItem *item)
{
    if (!item || m_loadingFromFolderList)
        return;

    const QString path = item->data(Qt::UserRole).toString();
    if (path.isEmpty())
        return;
    if (path == m_currentImagePath)
        return;

    m_loadingFromFolderList = true;
    if (!loadImageFromPath(path)) {
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("无法加载图片：%1")
                                 .arg(QFileInfo(path).fileName()));
    }
    m_loadingFromFolderList = false;
}

/**
 * @brief 滚轮缩放画布
 *
 * 缩放逻辑：
 *   - 向上滚：放大 SCROLL_SCALE_STEP 倍（默认 1.25）
 *   - 向下滚：缩小同倍数
 *   - 限制在 [MIN_SCALE_FACTOR, MAX_SCALE_FACTOR]，超出则吞掉事件但不缩放
 *
 * 注意：这里改的是 QGraphicsView 的变换矩阵，不是图像本身像素。
 * 处理算法始终基于原始分辨率的 pixmap，与屏幕缩放无关。
 */
void Widget::wheelEvent(QWheelEvent *event)
{
    // 鼠标在文件夹缩略图条上时，把滚轮留给横向滚动，不要缩放画布
    if (ui->folderBrowserPanel && ui->folderBrowserPanel->isVisible()
        && ui->folderImageList && ui->folderImageList->underMouse()) {
        event->ignore();
        return;
    }

    if (!m_pixmapItem) {
        event->ignore();  // 没图时把事件交给父类/别的控件
        return;
    }

    const double factor = AppConfig::SCROLL_SCALE_STEP;
    double next = m_viewScale;
    if (event->angleDelta().y() > 0)
        next *= factor;   // 放大
    else
        next /= factor;   // 缩小

    // 到边界就停止（accept 防止继续冒泡造成别的滚动）
    if (next < AppConfig::MIN_SCALE_FACTOR || next > AppConfig::MAX_SCALE_FACTOR) {
        event->accept();
        return;
    }

    // AnchorUnderMouse：缩放时鼠标指向的场景点尽量钉住不动
    if (event->angleDelta().y() > 0)
        ui->graphicsView->scale(factor, factor);
    else
        ui->graphicsView->scale(1.0 / factor, 1.0 / factor);

    m_viewScale = next;
    updateInfoLabel();  // 刷新 “宽×高 缩放%”
    event->accept();
}

/**
 * @brief 键盘：Delete 删除当前选中的 ROI
 */
void Widget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        on_deltete_clicked();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

// ============================================================================
// ROI：添加 / 删除 / 读取
// ============================================================================

/**
 * @brief 当前图像在场景中的中心点（用于默认放置 ROI）
 */
QPointF Widget::imageCenterInScene() const
{
    if (m_pixmapItem)
        return m_pixmapItem->boundingRect().center();
    return m_scene->sceneRect().center();
}

/**
 * @brief 清除三种 ROI 图元（同时最多只存在一种）
 *
 * wipe 是个泛型 lambda：对每种指针做
 *   removeItem → delete → 置空指针
 * 避免场景里留野指针。
 */
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

/**
 * @brief 在场景中添加轴对齐矩形 ROI
 * @param x,y 中心点场景坐标；w,h 宽高
 *
 * 构造参数是左上角，所以要先 x-w/2, y-h/2。
 * 添加前 clearAllRoi：本项目约定同时只有一个 ROI。
 */
void Widget::addRectItem(qreal x, qreal y, qreal width, qreal height)
{
    clearAllRoi();
    m_rectItem = new ResizableRectItem(x - width / 2, y - height / 2, width, height);
    m_scene->addItem(m_rectItem);
    m_rectItem->setSelected(true);  // 选中后才显示缩放把手（见图元实现）
}

/** @brief 添加椭圆 ROI（外接矩形中心 + 宽高） */
void Widget::addEllipseItem(qreal x, qreal y, qreal w, qreal h)
{
    clearAllRoi();
    m_ellipseItem = new ResizableEllipseItem(x - w / 2, y - h / 2, w, h);
    m_scene->addItem(m_ellipseItem);
    m_ellipseItem->setSelected(true);
}

/**
 * @brief 添加可旋转矩形 ROI
 *
 * 与前两种不同：旋转矩形用 setPos 设中心，本地矩形以原点为中心。
 * 详见 ResizableRotatedRectItem。
 */
void Widget::addRotatedRectItem(qreal x, qreal y, qreal w, qreal h)
{
    clearAllRoi();
    m_rotatedRectItem = new ResizableRotatedRectItem(w, h);
    m_rotatedRectItem->setPos(x, y);
    m_rotatedRectItem->setRotation(0);
    m_scene->addItem(m_rotatedRectItem);
    m_rotatedRectItem->setSelected(true);
}

/** comboBox 切换时目前无额外逻辑（类型在点“添加 ROI”时才读取） */
void Widget::on_comboBox_currentIndexChanged(int)
{
}

/**
 * @brief 槽：点击“添加 ROI”（pushButton_3）
 *
 * 读取 comboBox 当前文字，在图像中心放一个 DEFAULT_ROI_SIZE 大小的选区。
 * 之后用户可拖动/缩放；scene.changed 会触发防抖重算。
 */
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
        addRotatedRectItem(c.x(), c.y(), size * 1.4, size);  // 略扁长一点更好认
}

/**
 * @brief 槽：删除 ROI（按钮 deltete 或键盘 Delete）
 *
 * 若有选中项：只删选中的；
 * 若无选中：清掉全部 ROI（方便一键清空）。
 *
 * 删 ROI 后如果还有处理块，下一次 scene.changed / 手动应用会按“全图”重算
 * （因为 getCurrentRoiInfo 会返回 shape=None）。
 */
void Widget::on_deltete_clicked()
{
    const auto selected = m_scene->selectedItems();
    if (selected.isEmpty()) {
        clearAllRoi();
        return;
    }
    for (QGraphicsItem *item : selected) {
        // 先把成员指针置空，再删对象，避免悬空指针
        if (item == m_rectItem) m_rectItem = nullptr;
        if (item == m_ellipseItem) m_ellipseItem = nullptr;
        if (item == m_rotatedRectItem) m_rotatedRectItem = nullptr;
        m_scene->removeItem(item);
        delete item;
    }
}

/**
 * @brief 从场景图元打包 RoiInfo（算法层只认这个纯数据结构）
 *
 * 为什么要转换？
 *   - 场景里是 QGraphicsItem（带变换、可交互）
 *   - 算法层（OpenCV）只要数字：矩形范围 / 椭圆外接框 / 中心+尺寸+角度
 *
 * 坐标：因为底图放在场景原点且未对 pixmap 再做变换，
 * mapToScene 得到的就是图像像素坐标。
 *
 * @return shape=None 表示没有 ROI → 全图处理
 */
RoiInfo Widget::getCurrentRoiInfo() const
{
    RoiInfo info;
    if (!m_processor->hasImage())
        return info;  // 默认 shape=None

    if (m_rectItem) {
        info.shape = RoiInfo::Shape::Rect;
        // rect() 是图元本地坐标；mapToScene 后取包围盒得到轴对齐像素矩形
        info.rect = m_rectItem->mapToScene(m_rectItem->rect()).boundingRect();
        return info;
    }
    if (m_ellipseItem) {
        info.shape = RoiInfo::Shape::Ellipse;
        // 椭圆用外接矩形描述，RoiProcess::makeMask 里再画椭圆
        info.rect = m_ellipseItem->mapToScene(m_ellipseItem->rect()).boundingRect();
        return info;
    }
    if (m_rotatedRectItem) {
        info.shape = RoiInfo::Shape::RotatedRect;
        info.center = m_rotatedRectItem->mapToScene(m_rotatedRectItem->localRect().center());
        info.size = m_rotatedRectItem->localRect().size();
        // rotation() 单位是度，逆时针，与 Qt 一致；RoiProcess 里再转弧度
        info.angleDeg = m_rotatedRectItem->rotation();
        return info;
    }
    return info;  // None → 全图
}

/** 兼容接口：直接转到统一重算入口 */
void Widget::onRoiGeometryChanged()
{
    onApplyProcessing();
}

// ============================================================================
// 处理链：应用 / 保存 / 清空 / 对比 / 显示
// ============================================================================

/**
 * @brief 【最重要】统一重算入口
 *
 * -------------------------------------------------------------------------
 * 谁会调用到这里？
 * -------------------------------------------------------------------------
 *   1. 用户点击 btnApply
 *   2. ImageProcessor::requestReprocess
 *        （来源：加块、删块、块 paramsChanged、块 enabledChanged）
 *   3. m_roiUpdateTimer 超时（ROI 拖完防抖）
 *   4. loadImageFromPath 发现已有处理块
 *
 * -------------------------------------------------------------------------
 * 为什么不让 ImageProcessor 自己读 ROI？
 * -------------------------------------------------------------------------
 *   ROI 活在 Widget 的场景图元里，Processor 不该依赖 QGraphicsItem。
 *   所以约定：Widget 负责“读 ROI → setRoi → reprocess”。
 *
 * -------------------------------------------------------------------------
 * 执行步骤
 * -------------------------------------------------------------------------
 *   无图 → 直接返回
 *   无块 → 显示原图（相当于处理链为空）
 *   有块 → 退出对比模式 → setRoi → reprocess
 *          reprocess 结束后发 processingFinished → onProcessingFinished
 */
void Widget::onApplyProcessing()
{
    if (!m_processor->hasImage())
        return;

    if (m_blockList.isEmpty()) {
        // 处理链空了：结果就是原图
        updatePixmapItem(m_processor->originalImage());
        return;
    }

    // 开始处理时强制显示结果图（若正在按住对比，也会被打断回结果）
    m_showOriginal = false;

    // 关键两步：先同步 ROI，再跑流水线
    m_processor->setRoi(getCurrentRoiInfo());
    m_processor->reprocess();
}
void Widget::on_btnApply_clicked()
{
    onApplyProcessing();
}

/**
 * @brief 把当前结果图保存到磁盘
 *
 * 保存的是 ImageProcessor::resultImage()（处理链输出），
 * 不是画布上可能正在对比显示的原图。
 */
void Widget::on_btnSave_clicked()
{
    if (!m_processor->hasImage() || m_processor->resultImage().isNull()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("没有可保存的结果图"));
        return;
    }
    const QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("保存结果"), QStringLiteral("result.png"),
        QStringLiteral("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));
    if (path.isEmpty()) return;  // 取消

    if (!m_processor->resultImage().save(path))
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("保存失败"));
}

/**
 * @brief 清空右侧整条处理链
 *
 * 细节：批量删块时，每个 removeBlock 都会 emit requestReprocess。
 * 若不断开，会重算 N 次，又慢又闪。所以：
 *   1. 先 disconnect requestReprocess
 *   2. 循环删光
 *   3. 再 connect 回去
 *   4. 手动把显示恢复成原图，耗时归零
 */
void Widget::on_btnClearChain_clicked()
{
    disconnect(m_processor, &ImageProcessor::requestReprocess,
               this, &Widget::onApplyProcessing);

    while (!m_blockList.isEmpty()) {
        BaseBlock *block = m_blockList.takeFirst();
        m_blockLayout->removeWidget(block);
        m_processor->removeBlock(block);
        block->deleteLater();  // 下一轮事件循环再真正析构，更安全
    }

    connect(m_processor, &ImageProcessor::requestReprocess,
            this, &Widget::onApplyProcessing);

    refreshChainHint();
    if (m_processor->hasImage()) {
        m_showOriginal = false;
        // setOriginalImage 会把 m_result 重置为原图
        m_processor->setOriginalImage(m_processor->originalImage());
        updatePixmapItem(m_processor->originalImage());
        ui->label_3->setText(QStringLiteral("0 ms"));
    }
}

/**
 * @brief 对比按钮按下：立刻显示原图（按住不放可持续看）
 *
 * 不重新跑算法，只换画布上的 pixmap。
 * 松开见 on_btnCompare_released。
 */
void Widget::on_btnCompare_pressed()
{
    if (!m_processor->hasImage())
        return;
    m_showOriginal = true;
    refreshDisplay();
}

/**
 * @brief 对比按钮松开：恢复显示处理结果
 */
void Widget::on_btnCompare_released()
{
    if (!m_processor->hasImage())
        return;
    m_showOriginal = false;
    refreshDisplay();
}

/**
 * @brief 按 m_showOriginal 决定画原图还是结果图
 */
void Widget::refreshDisplay()
{
    if (!m_processor->hasImage()) return;
    updatePixmapItem(m_showOriginal ? m_processor->originalImage()
                                    : m_processor->resultImage());
}

/**
 * @brief 更新底部信息标签：分辨率 + 当前缩放百分比
 */
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

/**
 * @brief ImageProcessor 算完后的回调
 *
 * 此时 m_result 已更新。我们：
 *   1. refreshDisplay 把结果（或对比原图）画到场景
 *   2. label_3 显示耗时
 *   3. 刷新信息栏
 */
void Widget::onProcessingFinished(qint64 elapsedMs)
{
    refreshDisplay();
    ui->label_3->setText(QStringLiteral("%1 ms").arg(elapsedMs));
    updateInfoLabel();
}

/**
 * @brief 替换场景底图的像素内容（不重建 item）
 *
 * 关键：setPixmap 会触发 scene.changed。
 * 若此时不挂起，会再次启动 ROI 重算定时器，形成：
 *   reprocess → setPixmap → changed → timer → reprocess → ...
 * 所以用 m_suspendSceneReprocess 包住。
 */
void Widget::updatePixmapItem(const QPixmap &pixmap)
{
    if (!m_pixmapItem) return;
    m_suspendSceneReprocess = true;
    m_pixmapItem->setPixmap(pixmap);
    m_suspendSceneReprocess = false;
}

/** @brief 场景里是否存在任意一种 ROI */
bool Widget::hasAnyRoi() const
{
    return m_rectItem || m_ellipseItem || m_rotatedRectItem;
}

// ============================================================================
// 交互：画布平移 / 拖放建块
// ============================================================================

/**
 * @brief 处理画布平移手势（在 viewport 的 eventFilter 里调用）
 *
 * 规则：
 *   - 鼠标点在 ROI 上 → 绝不拦截，把事件留给 ROI 自己拖/缩
 *   - 中键，或左键点在空白/底图上 → 开始平移
 *   - 移动时通过改 ScrollBar 值实现平移（等价于拖动画布）
 *   - 松开结束，光标恢复箭头
 *
 * @return true 表示事件已处理，eventFilter 应拦截（不再往下传）
 */
bool Widget::viewportPanEvent(QEvent *event)
{
    auto *view = ui->graphicsView;
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto *e = static_cast<QMouseEvent *>(event);
        // itemAt：视口坐标 → 命中最上层图元
        QGraphicsItem *hit = view->itemAt(e->pos());

        // 点在 ROI 上：返回 false（不拦截），让 ROI 的 mousePress 正常工作
        const bool onRoi = hit
            && (hit == m_rectItem || hit == m_ellipseItem || hit == m_rotatedRectItem);
        if (onRoi)
            break;

        // 中键，或左键点在空白/图片上：进入平移模式
        if (e->button() == Qt::MiddleButton
            || (e->button() == Qt::LeftButton && (hit == nullptr || hit == m_pixmapItem))) {
            m_panning = true;
            m_panLastPos = e->pos();
            view->viewport()->setCursor(Qt::ClosedHandCursor);
            return true;  // 吃掉事件，避免同时选中别的东西
        }
        break;
    }
    case QEvent::MouseMove: {
        if (!m_panning) break;
        auto *e = static_cast<QMouseEvent *>(event);
        const QPoint delta = e->pos() - m_panLastPos;
        m_panLastPos = e->pos();
        // 鼠标向右拖 → 内容应向右移 → 滚动条值减小（与常见地图拖拽一致）
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
    return false;  // 未处理，继续默认分发
}

/**
 * @brief 事件过滤器：两路逻辑
 *
 * 路 A：graphicsView->viewport()
 *   → viewportPanEvent（平移）
 *
 * 路 B：widget_3（右侧处理链面板）
 *   → DragEnter：有文本就接受，并高亮面板
 *   → DragLeave：取消高亮
 *   → Drop：取出算法名，createBlockByName
 *
 * 安装位置：
 *   setupGraphicsView 里 viewport->installEventFilter(this)
 *   setupDragDrop 里 widget_3->installEventFilter(this)
 */
bool Widget::eventFilter(QObject *obj, QEvent *event)
{
    // ----- 路 A：画布平移 -----
    if (obj == ui->graphicsView->viewport()) {
        if (viewportPanEvent(event))
            return true;  // 已处理，拦截
        return QWidget::eventFilter(obj, event);
    }

    // ----- 路 B：右侧处理链拖放（面板 / 滚动区 / 内容容器）-----
    const bool onChainDropZone =
        obj == ui->widget_3
        || obj == ui->blockScrollArea
        || (ui->blockScrollArea && obj == ui->blockScrollArea->viewport())
        || obj == ui->blockListContainer;
    if (!onChainDropZone)
        return QWidget::eventFilter(obj, event);

    switch (event->type()) {
    case QEvent::DragEnter: {
        auto *e = static_cast<QDragEnterEvent *>(event);
        // 左侧拖出的 mime 带 text = 算法显示名
        if (e->mimeData()->hasText()) {
            m_isDragInside = true;
            setDropPanelHighlight(true);
            e->acceptProposedAction();  // 不 accept 的话后面收不到 Drop
            return true;
        }
        break;
    }
    case QEvent::DragLeave:
        // 从父移到子时也会 Leave，下一拍再看鼠标是否还在处理链面板内
        QTimer::singleShot(0, this, [this]() {
            if (!ui->widget_3)
                return;
            const QPoint g = QCursor::pos();
            if (!ui->widget_3->rect().contains(ui->widget_3->mapFromGlobal(g))) {
                m_isDragInside = false;
                setDropPanelHighlight(false);
            }
        });
        return true;
    case QEvent::Drop: {
        m_isDragInside = false;
        setDropPanelHighlight(false);
        auto *e = static_cast<QDropEvent *>(event);
        if (e->mimeData()->hasText()) {
            // 例如 text="二值化处理" → new BinarizationBlock
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

// ============================================================================
// 处理块：创建 / 注册 / Otsu 特殊接线
// ============================================================================

/**
 * @brief 给二值化块的「Otsu」按钮接线
 *
 * 为什么放在 Widget 而不在 BinarizationBlock 里算？
 *   - Otsu 需要“当前原图 + 当前 ROI”，这些数据在 Widget / Processor
 *   - Block 只应管自己的控件参数，不应直接摸主窗口状态
 *
 * 流程：
 *   用户点 Otsu
 *     → BinarizationBlock::otsuRequested
 *     → 本 lambda：
 *          原图转 Mat →（有 ROI 则裁外包络）→ 算阈值 t
 *          → block->setThresholds(t, 255)
 *          → 内部 emit paramsChanged → 自动重算整条链
 */
void Widget::wireBinarizationOtsu(BinarizationBlock *block)
{
    if (!block) return;
    connect(block, &BinarizationBlock::otsuRequested, this, [this, block]() {
        if (!m_processor->hasImage()) {
            QMessageBox::information(this, QStringLiteral("提示"),
                                     QStringLiteral("请先打开图片"));
            return;
        }

        // 与各 Block::process 相同的颜色约定：先 RGB Mat，再转 BGR 给 OpenCV
        cv::Mat src = ImageConverter::pixmapToMatRGB(m_processor->originalImage());
        cv::cvtColor(src, src, cv::COLOR_RGB2BGR);

        // 有 ROI 时只在外包络矩形内统计直方图，减少背景干扰
        const RoiInfo roi = getCurrentRoiInfo();
        if (!roi.isEmpty()) {
            const QRectF br = roi.boundingRect();
            cv::Rect rr(qRound(br.x()), qRound(br.y()),
                        qRound(br.width()), qRound(br.height()));
            // 与图像范围求交，防止 ROI 拖出界导致越界
            rr &= cv::Rect(0, 0, src.cols, src.rows);
            if (!rr.empty())
                src = src(rr).clone();  // clone：让这块内存独立，roi 引用安全
        }

        const int t = OtsuAlgorithm::calculateThresholdFromBGR(src);
        // 下限=Otsu 阈值，上限=255：常见“大于阈值变白”的设定
        block->setThresholds(t, 255);
    });
}

/**
 * @brief 根据拖入的算法名字，new 出对应的 Block 子类
 *
 * 名字必须与下列两者一致：
 *   - 左侧 listWidget 里显示的文字
 *   - AppConfig::BLOCK_NAME_* 常量
 *
 * 二值化要额外 wireBinarizationOtsu；其它块构造完即可。
 * 最后统一 addBlockToPanel 挂到 UI + Processor。
 */
void Widget::createBlockByName(const QString &name)
{
    // 块的父控件挂在滚动容器上，随列表一起滚
    QWidget *parent = ui->blockListContainer ? ui->blockListContainer
                                             : static_cast<QWidget *>(ui->widget_3);
    BaseBlock *block = nullptr;

    if (name == AppConfig::BLOCK_NAME_BINARIZATION) {
        auto *bin = new BinarizationBlock(parent);
        wireBinarizationOtsu(bin);
        block = bin;
    } else if (name == AppConfig::BLOCK_NAME_MORPHOLOGY) {
        block = new MorphologyBlock(parent);
    } else if (name == AppConfig::BLOCK_NAME_FILTER) {
        block = new FilterBlock(parent);
    } else if (name == AppConfig::BLOCK_NAME_GRAYTRANSFORM) {
        block = new GrayTransformBlock(parent);
    } else if (name == AppConfig::BLOCK_NAME_PSEUDOCOLOR) {
        block = new PseudoColorBlock(parent);
    }

    if (block)
        addBlockToPanel(block);
    else
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("未识别的算法：%1").arg(name));
}

/**
 * @brief 把已创建的块挂到右侧面板，并注册到处理引擎
 *
 * 做了三件事：
 *   1. UI：addWidget 到可滚动的 blockListLayout，加入 m_blockList
 *   2. 信号：块点 ✕ → 从布局/列表/processor 移除并 deleteLater
 *   3. 引擎：m_processor->addBlock
 *        - 内部会监听 paramsChanged / enabledChanged
 *        - 若已有图，立刻 requestReprocess → onApplyProcessing
 *
 * 注意 m_blockList 与 processor.blocks() 顺序应保持一致，
 * 因为 reprocess 按 processor 里的顺序执行，面板显示按 m_blockList 顺序。
 */
void Widget::addBlockToPanel(BaseBlock *block)
{
    if (!block || !m_blockLayout) return;

    // 纵向：不小于自身内容高度，避免被外层布局挤扁
    block->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    m_blockLayout->addWidget(block);
    m_blockList.append(block);

    // 用户点块标题栏的 ✕
    connect(block, &BaseBlock::removeRequested, this, [this, block]() {
        m_blockLayout->removeWidget(block);
        m_blockList.removeOne(block);
        m_processor->removeBlock(block);  // 会 requestReprocess（剩余块重算）
        block->deleteLater();
        refreshChainHint();
    });

    m_processor->addBlock(block);
    block->show();
    refreshChainHint();

    // 新块滚进可视区
    if (ui->blockScrollArea)
        ui->blockScrollArea->ensureWidgetVisible(block);
}




