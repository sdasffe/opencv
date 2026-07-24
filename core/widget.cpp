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
 *                ├─ getAllRoiInfo()          // 图元 → 纯数据列表
 *                ├─ ImageProcessor::setRois()
 *                ├─ ImageProcessor::reprocess()  // 按块顺序 process()
 *                └─ processingFinished → onProcessingFinished() → 刷新画布
 *
 * =============================================================================
 * 二、建议阅读顺序（带行号大致位置）
 * =============================================================================
 *   1. 构造函数 Widget::Widget          —— 初始化 + 信号槽总接线
 *   2. loadImageFromPath                —— 图片如何进系统（含 per-image 会话）
 *   3. createBlockByName / addBlockToPanel —— 算法块如何进处理链
 *   4. onApplyProcessing                —— 【最重要】何时、如何重算
 *   5. getAllRoiInfo                    —— 多 ROI 图元如何变成算法参数
 *   6. eventFilter / viewportPanEvent   —— 拖放建块 + 画布平移
 *   7. onProcessingFinished             —— 结果如何画回屏幕
 *
 * =============================================================================
 * 三、界面控件对应关系（来自 widget.ui）
 * =============================================================================
 *   listWidget     左侧算法工具箱（可拖出名字）
 *   graphicsView   中间画布（显示图 + ROI）
 *   widget_3       右侧图像处理工具箱（放置 BaseBlock）
 *   btnMenuFile/Roi/Settings  顶栏：文件 / ROI / 设置
 *   pushButton 系列槽：由菜单触发（打开图、文件夹、添加 ROI、删 ROI）
 *   btnApply       手动应用处理
 *   btnCompare     按住看原图，松开看结果
 *   btnSave        保存结果
 *   btnClearChain  清空工具箱（右侧）
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
#include "../blocks/glcmblock.h"
#include "../algorithms/otsu.h"
#include "../utils/imageconverter.h"
#include "../utils/applogger.h"
#include "../styles/styleloader.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDataStream>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QFile>
#include <QShortcut>
#include <QKeySequence>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QWidgetAction>
#include <QVBoxLayout>
#include <QActionGroup>
#include <QSettings>
#include <QApplication>
#include <QCoreApplication>
#include <QClipboard>
#include <QTranslator>
#include <QEvent>
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
#include <QScreen>
#include <QGuiApplication>
#include <QtGlobal>

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

    // ---------- 2. 布局：留白 + 圆角面板（避免贴边生硬） ----------
    ui->widget_2->setMinimumWidth(140);
    ui->widget_3->setMinimumWidth(210);
    if (auto *root = ui->horizontalLayout) {
        root->setSpacing(8);
        root->setContentsMargins(10, 10, 10, 10);
    }
    if (ui->horizontalLayout_2) {
        ui->horizontalLayout_2->setSpacing(6);
        ui->horizontalLayout_2->setContentsMargins(8, 6, 8, 4);
    }
    if (ui->chainIoLayout) {
        ui->chainIoLayout->setContentsMargins(8, 4, 8, 4);
        ui->chainIoLayout->setSpacing(6);
    }
    if (ui->blockListLayout) {
        ui->blockListLayout->setContentsMargins(6, 4, 6, 8);
        ui->blockListLayout->setSpacing(8);
    }
    if (ui->verticalLayout) {
        ui->verticalLayout->setSpacing(8);
        ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
    }
    if (ui->verticalLayout_3) {
        ui->verticalLayout_3->setSpacing(4);
        ui->verticalLayout_3->setContentsMargins(8, 8, 8, 8);
    }
    if (ui->blockPanelOuterLayout) {
        ui->blockPanelOuterLayout->setSpacing(4);
        ui->blockPanelOuterLayout->setContentsMargins(8, 8, 8, 8);
    }
    // 关键面板开 StyledBackground，Fusion 下 QSS 背景才能画出来
    for (QWidget *panel : {static_cast<QWidget *>(ui->widget),
                           static_cast<QWidget *>(ui->widget_2),
                           static_cast<QWidget *>(ui->widget_3),
                           static_cast<QWidget *>(ui->folderBrowserPanel),
                           static_cast<QWidget *>(ui->blockListContainer)}) {
        if (!panel)
            continue;
        panel->setAttribute(Qt::WA_StyledBackground, true);
        panel->setStyleSheet(QString());
    }
    if (ui->listWidget)
        ui->listWidget->setStyleSheet(QString());
    if (ui->graphicsView)
        ui->graphicsView->setFrameShape(QFrame::NoFrame);

    // 低分辨率机器上 .ui 默认尺寸会超出屏幕，启动时按可用区域收缩
    adaptWindowToScreen();

    // ---------- 3. 分模块初始化 ----------
    setupGraphicsView();  // 中间：场景、缩放锚点、ROI 变化监听
    setupDragDrop();      // 左拖右放：算法名字的拖放通道
    setupBlockPanel();    // 右侧：用来垂直排列各个 BaseBlock 的布局
    setupFolderBrowser(); // 画布下方文件夹缩略图（默认隐藏）
    setupMenus();         // 文件 / ROI / 设置
    setupAlgoListIds();   // 算法列表稳定 id（与语言无关）
    setupShortcuts();     // Delete / Ctrl+0 / Ctrl+Z

    loadSessionsFromDisk();
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
        saveCurrentSession();
        saveSessionsToDisk();
    });

    {
        QSettings settings(QStringLiteral("OpenCVLab"), QStringLiteral("ImageTool"));
        m_englishUi = settings.value(QStringLiteral("ui/english"), false).toBool();
        m_themeId = settings.value(QStringLiteral("ui/theme"),
                                   QLatin1String(StyleLoader::ThemeLight)).toString();
        applyLanguage();
        applyTheme();
    }

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
/**
 * @brief 按当前屏幕可用区域调整窗口，避免低分辨率下超出屏幕
 *
 * 规则：
 *   - 屏幕够大时用舒适默认尺寸（约 1200×720），不主动缩得很小
 *   - 放不下时尽量铺满可用区域，保证字和控件可读
 *   - 最小尺寸不超过屏幕，且尽量保持可用下限
 */
void Widget::adaptWindowToScreen()
{
    QScreen *scr = screen();
    if (!scr)
        scr = QGuiApplication::primaryScreen();
    if (!scr)
        return;

    const QRect avail = scr->availableGeometry();
    const int screenW = avail.width();
    const int screenH = avail.height();
    if (screenW <= 0 || screenH <= 0)
        return;

    // 舒适下限；仅当屏幕更小时才再降低
    constexpr int kPreferredMinW = 900;
    constexpr int kPreferredMinH = 600;
    const int minW = qMin(kPreferredMinW, screenW);
    const int minH = qMin(kPreferredMinH, screenH);
    setMinimumSize(minW, minH);

    constexpr int kPreferredW = 1200;
    constexpr int kPreferredH = 720;
    constexpr int kMargin = 12;

    int w = kPreferredW;
    int h = kPreferredH;
    // 默认尺寸放不下：铺满可用区域，避免缩成难读的小窗
    if (w + kMargin > screenW || h + kMargin > screenH) {
        w = qMax(minW, screenW - kMargin);
        h = qMax(minH, screenH - kMargin);
    }
    w = qBound(minW, w, screenW);
    h = qBound(minH, h, screenH);
    resize(w, h);
    move(avail.x() + (screenW - w) / 2,
         avail.y() + (screenH - h) / 2);
}

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
    // scene.changed 在任何图元变化时都会发（含 setPixmap）。
    // Qt 常把 changed 排队到下一拍才发，仅靠 m_suspendSceneReprocess 挡不住，
    // 会变成：重算 → 改图 → changed → 定时器 → 再重算 → 耗时一直跳。
    // 因此：只有 ROI 几何相对上次真的变了，才重启防抖定时器。
    connect(m_scene, &QGraphicsScene::changed, this, [this](const QList<QRectF> &) {
        if (m_suspendSceneReprocess) return;
        if (!m_processor->hasImage()) return;
        if (m_blockList.isEmpty()) return;
        if (!hasAnyRoi()) return;

        const QList<RoiInfo> now = getAllRoiInfo();
        if (now == m_lastRoisForDebounce)
            return;  // 只是换底图等，ROI 没动

        m_lastRoisForDebounce = now;
        m_roiUpdateTimer->start();
    });
}

/**
 * @brief 顶栏操作说明（菜单在 setupMenus 中组装）
 *
 * 文件 → 打开图片 / 打开文件夹 / 退出
 * ROI  → 形状下拉 + 添加 / 删除
 * 设置 → 语言（中/英） / 关于
 * 应用 / 对比 / 保存 / 耗时 仍在顶栏右侧
 * 清空 / 导入 / 导出 在右侧「图像处理工具箱」
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
        QMessageBox::warning(this, tr("界面错误"),
                             tr("缺少 blockListContainer，请检查 widget.ui"));
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
    ui->blockListContainer->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->blockListContainer, &QWidget::customContextMenuRequested,
            this, [this](const QPoint &pos) {
        // 点在某个块上时由 BaseBlock 自己弹菜单，这里只处理空白处
        if (ui->blockListContainer->childAt(pos))
            return;
        QMenu menu(this);
        QAction *pasteAct = menu.addAction(tr("粘贴"));
        pasteAct->setEnabled(clipboardHasBlock());
        if (menu.exec(ui->blockListContainer->mapToGlobal(pos)) == pasteAct)
            pasteBlockFromClipboard(nullptr);
    });

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
        m_blockList.isEmpty() ? tr("拖入算法") : QString());
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
    if (pixmap.isNull()) {
        AppLogger::error(QStringLiteral("加载图片失败"), filePath);
        return false;
    }

    const QString prevPath = m_currentImagePath;

    // 离开旧图前先写入会话（含「仍是同一路径」时：先存再恢复，避免误清空）
    saveCurrentSession();

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

    // ----- 按路径恢复或清空（新图 = 空链 + 无 ROI） -----
    const bool restored = m_sessions.contains(filePath);
    if (restored)
        restoreSession(m_sessions.value(filePath));
    else {
        clearAllBlocks(false);
        clearAllRoi();
    }

    m_currentImagePath = filePath;
    m_undoStack.clear(); // 换图后撤销针对当前图

    if (!m_blockList.isEmpty())
        onApplyProcessing();
    else
        refreshDisplay();  // 没有块：显示原图即可

    updateInfoLabel();
    setWindowTitle(QStringLiteral("图像处理工具 — %1  (%2 × %3)")
                       .arg(QFileInfo(filePath).fileName())
                       .arg(pixmap.width())
                       .arg(pixmap.height()));

    AppLogger::info(
        QStringLiteral("加载图片"),
        QStringLiteral("path=%1 size=%2x%3 prev=%4 session=%5 blocks=%6 rois=%7")
            .arg(filePath)
            .arg(pixmap.width())
            .arg(pixmap.height())
            .arg(prevPath.isEmpty() ? QStringLiteral("(无)") : prevPath)
            .arg(restored ? QStringLiteral("恢复") : QStringLiteral("新建空会话"))
            .arg(m_blockList.size())
            .arg(getAllRoiInfo().size()));
    return true;
}

// ============================================================================
// 打开图片 / 缩放 / 键盘
// ============================================================================

/**
 * @brief 文件→打开图片：选单张图，收起文件夹条后 loadImageFromPath
 */
void Widget::actOpenImage()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this, tr("选择图片"), QString(), tr(AppConfig::IMAGE_FILE_FILTER));
    if (filePath.isEmpty())
        return;                                                      // 用户取消

    AppLogger::info(QStringLiteral("打开单张图片"), filePath);
    clearFolderBrowser();                                            // 单张模式不显示缩略图条

    if (!loadImageFromPath(filePath))
        QMessageBox::warning(this, tr("错误"),
                             tr("无法加载图片，文件可能已损坏"));
}

/**
 * @brief 文件→打开文件夹：扫图片 → 填缩略图条 → 加载第一张
 */
void Widget::actOpenFolder()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this, tr("选择图片文件夹"));
    if (dir.isEmpty())
        return;                                                      // 用户取消

    const QStringList filters = {                                    // 支持的图片后缀
        QStringLiteral("*.png"), QStringLiteral("*.jpg"), QStringLiteral("*.jpeg"),
        QStringLiteral("*.bmp"), QStringLiteral("*.gif"), QStringLiteral("*.tif"),
        QStringLiteral("*.tiff")
    };
    QDir d(dir);
    const QFileInfoList files = d.entryInfoList(filters, QDir::Files, QDir::Name); // 按文件名排序
    if (files.isEmpty()) {
        AppLogger::warn(QStringLiteral("打开文件夹无图片"), dir);
        QMessageBox::information(this, tr("提示"),
                                 tr("该文件夹下没有找到图片文件"));
        return;
    }

    AppLogger::info(QStringLiteral("打开文件夹"),
                    QStringLiteral("dir=%1 count=%2").arg(dir).arg(files.size()));

    fillFolderBrowser(dir, files);                                   // 底部显示缩略图条

    const QString firstPath = files.first().absoluteFilePath();
    if (!loadImageFromPath(firstPath)) {                             // 默认加载第一张
        QMessageBox::warning(this, tr("错误"),
                             tr("无法加载图片"));
        return;
    }
    selectFolderThumbnail(firstPath);                                // 高亮当前缩略图
}

/**
 * @brief 点击文件夹缩略图：切换到对应图片
 *
 * 路径存在 item 的 Qt::UserRole 里；当前已是该图则忽略。
 * 处理链与 ROI 按路径各自保存：换图后恢复该图会话（见 loadImageFromPath）。
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
        QMessageBox::warning(this, tr("错误"),
                             tr("无法加载图片：%1")
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
 * @brief 键盘：Delete 删 ROI；Ctrl+0 画布适应窗口
 *
 * 另有 setupShortcuts 的 QShortcut 兜底（子控件抢焦点时本函数可能收不到）。
 */
void Widget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        DelteteRoi();                                                // 删选中 ROI，无选中则清空全部
        event->accept();                                             // 标记已处理，不再往下传
        return;
    }
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_0) {
        fitViewToImage();                                            // 重置缩放铺满视口
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);                                   // 其余键交给基类
}

/**
 * @brief 注册窗口级快捷键：Ctrl+0 适应窗口、Delete 删 ROI
 *
 * Ctrl+Z 只挂在 actUndo 上，此处不再 new QShortcut，避免 Ambiguous shortcut。
 */
void Widget::setupShortcuts()
{
    auto *fitSc = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_0), this);
    fitSc->setContext(Qt::WindowShortcut);                           // 窗口激活即可用，不要求焦点在主窗
    fitSc->setObjectName(QStringLiteral("shortcutFitView"));
    connect(fitSc, &QShortcut::activated, this, &Widget::fitViewToImage);

    auto *delSc = new QShortcut(QKeySequence::Delete, this);
    delSc->setContext(Qt::WindowShortcut);                           // 同上：子控件有焦点也能删 ROI
    delSc->setObjectName(QStringLiteral("shortcutDeleteRoi"));
    connect(delSc, &QShortcut::activated, this, &Widget::DelteteRoi);
}

/**
 * @brief 连接 .ui 菜单栏动作，并为「编辑→ROI」挂入形状下拉 + 添加/删除面板
 *
 * 菜单结构在 widget.ui；此处只接线与动态 ROI 子面板。
 */
void Widget::setupMenus()
{
    if (!ui->menuBar)
        return;                                                      // .ui 未定义菜单栏则跳过

    connect(ui->actOpenImage, &QAction::triggered, this, &Widget::actOpenImage);   // 文件→打开图片
    connect(ui->actOpenFolder, &QAction::triggered, this, &Widget::actOpenFolder); // 文件→打开文件夹
    connect(ui->actExit, &QAction::triggered, this, &Widget::onExitApp);           // 文件→退出
    connect(ui->actUndo, &QAction::triggered, this, &Widget::onUndo);              // 编辑→撤销
    connect(ui->actHelp, &QAction::triggered, this, &Widget::onHelpShortcuts);     // 帮助
    connect(ui->actAbout, &QAction::triggered, this, &Widget::onAboutApp);         // 关于

    ui->actUndo->setShortcut(QKeySequence::Undo);                    // Ctrl+Z
    ui->actUndo->setShortcutContext(Qt::WindowShortcut);             // 子控件有焦点也能撤销
    addAction(ui->actUndo);                                          // 挂到主窗口，避免被焦点抢走

    m_menuRoi = ui->menuRoi;                                         // 编辑→ROI 子菜单
    auto *roiPanel = new QWidget(m_menuRoi);                         // 内嵌面板：形状 + 按钮
    auto *roiLay = new QVBoxLayout(roiPanel);
    roiLay->setContentsMargins(10, 8, 10, 8);
    roiLay->setSpacing(6);

    m_roiTypeCombo = new QComboBox(roiPanel);                        // 矩形 / 圆形 / 旋转矩形
    m_roiTypeCombo->setObjectName(QStringLiteral("comboBox"));
    m_roiTypeCombo->addItem(tr("矩形"));
    m_roiTypeCombo->addItem(tr("圆形"));
    m_roiTypeCombo->addItem(tr("旋转矩形"));
    m_roiTypeCombo->setMinimumWidth(120);

    m_roiAddBtn = new QPushButton(tr("添加"), roiPanel);
    m_roiAddBtn->setObjectName(QStringLiteral("pushButton_3"));
    m_roiAddBtn->setCursor(Qt::PointingHandCursor);
    m_roiDeleteBtn = new QPushButton(tr("删除"), roiPanel);
    m_roiDeleteBtn->setObjectName(QStringLiteral("deltete"));        // 历史 objectName，供 QSS 匹配
    m_roiDeleteBtn->setCursor(Qt::PointingHandCursor);

    roiLay->addWidget(m_roiTypeCombo);
    roiLay->addWidget(m_roiAddBtn);
    roiLay->addWidget(m_roiDeleteBtn);

    auto *roiAction = new QWidgetAction(m_menuRoi);                  // 把 QWidget 塞进 QMenu
    roiAction->setDefaultWidget(roiPanel);
    m_menuRoi->clear();                                              // 清掉 .ui 占位项
    m_menuRoi->addAction(roiAction);                                 // 只保留内嵌面板

    connect(m_roiAddBtn, &QPushButton::clicked, this, [this]() {
        AddRoi();                                                    // 按当前形状在图中心加 ROI
        if (m_menuRoi)
            m_menuRoi->close();                                      // 操作完收起菜单
    });
    connect(m_roiDeleteBtn, &QPushButton::clicked, this, [this]() {
        DelteteRoi();                                                // 删选中或清空全部 ROI
        if (m_menuRoi)
            m_menuRoi->close();
    });

    auto *langGroup = new QActionGroup(this);                        // 语言互斥：中/英只能选一个
    langGroup->setExclusive(true);
    langGroup->addAction(ui->actLangZh);
    langGroup->addAction(ui->actLangEn);
    connect(ui->actLangZh, &QAction::triggered, this, &Widget::onLanguageChinese);
    connect(ui->actLangEn, &QAction::triggered, this, &Widget::onLanguageEnglish);

    auto *themeGroup = new QActionGroup(this);                       // 外观互斥：浅色/深色
    themeGroup->setExclusive(true);
    themeGroup->addAction(ui->actThemeLight);
    themeGroup->addAction(ui->actThemeDark);
    connect(ui->actThemeLight, &QAction::triggered, this, &Widget::onThemeLight);
    connect(ui->actThemeDark, &QAction::triggered, this, &Widget::onThemeDark);
}

void Widget::setupAlgoListIds()
{
    if (!ui->listWidget)
        return;
    // 显示名走 .ui / tr；UserRole 固定中文 id，拖放与 JSON 不随语言变
    const QStringList ids = {
        QString::fromUtf8(AppConfig::BLOCK_NAME_BINARIZATION),
        QString::fromUtf8(AppConfig::BLOCK_NAME_MORPHOLOGY),
        QString::fromUtf8(AppConfig::BLOCK_NAME_FILTER),
        QString::fromUtf8(AppConfig::BLOCK_NAME_GRAYTRANSFORM),
        QString::fromUtf8(AppConfig::BLOCK_NAME_PSEUDOCOLOR),
        QString::fromUtf8(AppConfig::BLOCK_NAME_GLCM)
    };
    for (int i = 0; i < ui->listWidget->count() && i < ids.size(); ++i) {
        if (auto *item = ui->listWidget->item(i))
            item->setData(Qt::UserRole, ids.at(i));
    }
}

void Widget::onExitApp()
{
    QApplication::quit();
}

void Widget::onAboutApp()
{
    QMessageBox::about(
        this,
        tr("关于"),
        tr("%1\n版本 %2\n\n基于 Qt Widgets + OpenCV 的图像处理演示程序。\n\n日志文件：\n%3\n\n会话文件：\n%4")
            .arg(QString::fromUtf8(AppConfig::APP_NAME_ZH),
                 QString::fromUtf8(AppConfig::APP_VERSION),
                 AppLogger::currentLogPath(),
                 sessionsFilePath()));
}

void Widget::onHelpShortcuts()
{
    QMessageBox::information(
        this,
        tr("帮助"),
        tr("快捷键\n"
           "• Delete — 删除选中的 ROI（无选中则清空全部）\n"
           "• Ctrl+Z — 撤销（处理链 / ROI 结构变更）\n"
           "• Ctrl+0 — 画布适应窗口\n"
           "• 滚轮 — 缩放画布（以鼠标为中心）\n"
           "• 中键 / 左键空白处拖动 — 平移画布\n"
           "• 按住「对比」— 查看原图，松开恢复结果\n"
           "\n"
           "ROI\n"
           "• 可添加多个选区，算法在并集区域内生效\n"
           "• 打开文件夹后，每张图各自记住处理链与 ROI（可落盘）\n"
           "\n"
           "处理链\n"
           "• 从左侧拖入算法到右侧工具箱\n"
           "• 拖动块标题 — 调整处理顺序\n"
           "• 右键处理块 — 复制 / 粘贴 / 删除\n"
           "• 右键空白处 — 粘贴到链尾\n"
           "• 导入 / 导出 — 当前图的处理链 JSON 文件\n"
           "\n"
           "日志\n"
           "• 操作记录写入程序目录下 logs/app_日期.log\n"
           "• 路径见「设置 → 关于」\n"
           "\n"
           "外观\n"
           "• 设置 → 外观 → 浅色 / 深色（IDEA 风格）"));
}

void Widget::onLanguageChinese()
{
    m_englishUi = false;
    QSettings settings(QStringLiteral("OpenCVLab"), QStringLiteral("ImageTool"));
    settings.setValue(QStringLiteral("ui/english"), false);
    applyLanguage();
    AppLogger::info(QStringLiteral("切换语言"), QStringLiteral("zh"));
}

void Widget::onLanguageEnglish()
{
    m_englishUi = true;
    QSettings settings(QStringLiteral("OpenCVLab"), QStringLiteral("ImageTool"));
    settings.setValue(QStringLiteral("ui/english"), true);
    applyLanguage();
    AppLogger::info(QStringLiteral("切换语言"), QStringLiteral("en"));
}

void Widget::onThemeLight()
{
    m_themeId = QLatin1String(StyleLoader::ThemeLight);
    applyTheme();
}

void Widget::onThemeDark()
{
    m_themeId = QLatin1String(StyleLoader::ThemeDark);
    applyTheme();
}

void Widget::applyTheme()
{
    if (m_themeId.compare(QLatin1String(StyleLoader::ThemeDark), Qt::CaseInsensitive) != 0)
        m_themeId = QLatin1String(StyleLoader::ThemeLight);

    // 清掉易冲突的内联样式，避免盖住全局主题
    const QStringList clearNames = {
        QStringLiteral("widget"), QStringLiteral("widget_2"), QStringLiteral("widget_3"),
        QStringLiteral("listWidget"), QStringLiteral("folderBrowserPanel"),
        QStringLiteral("folderImageList"), QStringLiteral("deltete")
    };
    for (QWidget *w : findChildren<QWidget *>()) {
        const QString n = w->objectName();
        if (clearNames.contains(n) || n.startsWith(QLatin1String("btn"))
            || n.startsWith(QLatin1String("pushButton"))) {
            if (!w->styleSheet().isEmpty())
                w->setStyleSheet(QString());
        }
    }
    for (QWidget *panel : {static_cast<QWidget *>(ui->widget),
                           static_cast<QWidget *>(ui->widget_2),
                           static_cast<QWidget *>(ui->widget_3),
                           static_cast<QWidget *>(ui->folderBrowserPanel)}) {
        if (panel)
            panel->setAttribute(Qt::WA_StyledBackground, true);
    }

    qApp->setStyleSheet(StyleLoader::loadTheme(m_themeId));
    qApp->style()->unpolish(this);
    qApp->style()->polish(this);
    update();

    QSettings settings(QStringLiteral("OpenCVLab"), QStringLiteral("ImageTool"));
    settings.setValue(QStringLiteral("ui/theme"), m_themeId);

    if (ui->actThemeLight)
        ui->actThemeLight->setChecked(m_themeId == QLatin1String(StyleLoader::ThemeLight));
    if (ui->actThemeDark)
        ui->actThemeDark->setChecked(m_themeId == QLatin1String(StyleLoader::ThemeDark));

    AppLogger::info(QStringLiteral("切换主题"), m_themeId);
}

void Widget::applyLanguage()
{
    // 源码字符串为中文：装上 opencv_en.qm = 英文
    if (m_translatorLoaded) {
        qApp->removeTranslator(&m_translatorEn);
        m_translatorLoaded = false;
    }
    if (m_englishUi) {
        if (m_translatorEn.load(QStringLiteral(":/i18n/opencv_en.qm"))) {
            qApp->installTranslator(&m_translatorEn);
            m_translatorLoaded = true;
        }
    }
    // install/removeTranslator 会给各窗口发 LanguageChange；
    // 这里再显式刷一次，保证启动时与动态菜单也更新
    ui->retranslateUi(this);
    retranslateDynamicUi();
    for (BaseBlock *block : m_blockList) {
        if (block)
            block->retranslateUi();
    }
    setupAlgoListIds();
    if (ui->actLangZh)
        ui->actLangZh->setChecked(!m_englishUi);
    if (ui->actLangEn)
        ui->actLangEn->setChecked(m_englishUi);
    refreshChainHint();
}

void Widget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateDynamicUi();
        for (BaseBlock *block : m_blockList) {
            if (block)
                block->retranslateUi();
        }
        setupAlgoListIds();
        refreshChainHint();
    }
    QWidget::changeEvent(event);
}

void Widget::retranslateDynamicUi()
{
    // 菜单栏文案由 ui->retranslateUi 负责；这里只刷代码创建的 ROI 面板与算法列表
    if (m_roiTypeCombo && m_roiTypeCombo->count() >= 3) {
        const int idx = m_roiTypeCombo->currentIndex();
        m_roiTypeCombo->setItemText(0, tr("矩形"));
        m_roiTypeCombo->setItemText(1, tr("圆形"));
        m_roiTypeCombo->setItemText(2, tr("旋转矩形"));
        m_roiTypeCombo->setCurrentIndex(idx);
    }
    if (m_roiAddBtn)
        m_roiAddBtn->setText(tr("添加"));
    if (m_roiDeleteBtn)
        m_roiDeleteBtn->setText(tr("删除"));

    // 左侧算法显示名（UserRole 仍是中文 id，由 setupAlgoListIds 维护）
    if (ui->listWidget && ui->listWidget->count() >= 6) {
        ui->listWidget->item(0)->setText(tr("二值化处理"));
        ui->listWidget->item(1)->setText(tr("形态学处理"));
        ui->listWidget->item(2)->setText(tr("滤波处理"));
        ui->listWidget->item(3)->setText(tr("灰度变换"));
        ui->listWidget->item(4)->setText(tr("伪彩色处理"));
        ui->listWidget->item(5)->setText(tr("灰度共生矩阵"));
    }
}

/**
 * @brief 重置缩放到适应窗口（与刚打开图片时一致）
 */
void Widget::fitViewToImage()
{
    if (!m_scene || m_scene->sceneRect().isEmpty())
        return;

    ui->graphicsView->resetTransform();
    ui->graphicsView->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    m_viewScale = ui->graphicsView->transform().m11();
    updateInfoLabel();
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
 * @brief 清除全部 ROI 图元（多 ROI 列表一并清空）
 *
 * 切换图片、恢复会话、删除全部 ROI 时调用；不触发重算（由调用方决定）。
 */
void Widget::clearAllRoi()
{
    auto wipeList = [&](auto &list) {
        for (auto *ptr : list) {
            if (!ptr) continue;
            m_scene->removeItem(ptr);
            delete ptr;
        }
        list.clear();
    };
    wipeList(m_rectItems);
    wipeList(m_ellipseItems);
    wipeList(m_rotatedRectItems);
    m_lastRoisForDebounce.clear();
}

void Widget::deselectAllRoiItems()
{
    for (auto *item : m_rectItems)
        item->setSelected(false);
    for (auto *item : m_ellipseItems)
        item->setSelected(false);
    for (auto *item : m_rotatedRectItems)
        item->setSelected(false);
}

bool Widget::isRoiItem(QGraphicsItem *item) const
{
    if (!item)
        return false;
    for (auto *r : m_rectItems) {
        if (r == item) return true;
    }
    for (auto *e : m_ellipseItems) {
        if (e == item) return true;
    }
    for (auto *rr : m_rotatedRectItems) {
        if (rr == item) return true;
    }
    return false;
}

/**
 * @brief 在场景中添加轴对齐矩形 ROI（不清除已有 ROI）
 * @param x,y 中心点场景坐标；w,h 宽高
 */
void Widget::addRectItem(qreal x, qreal y, qreal width, qreal height)
{
    pushUndoSnapshot(QStringLiteral("添加矩形ROI"));
    deselectAllRoiItems();
    auto *item = new ResizableRectItem(x - width / 2, y - height / 2, width, height);
    m_scene->addItem(item);
    m_rectItems.append(item);
    item->setSelected(true);
    AppLogger::info(QStringLiteral("添加ROI"),
                    QStringLiteral("type=rect center=(%1,%2) size=%3x%4 total=%5")
                        .arg(x).arg(y).arg(width).arg(height)
                        .arg(m_rectItems.size() + m_ellipseItems.size() + m_rotatedRectItems.size()));
}

/** @brief 添加椭圆 ROI（外接矩形中心 + 宽高；追加到列表，不清除已有 ROI） */
void Widget::addEllipseItem(qreal x, qreal y, qreal w, qreal h)
{
    pushUndoSnapshot(QStringLiteral("添加椭圆ROI"));
    deselectAllRoiItems();
    auto *item = new ResizableEllipseItem(x - w / 2, y - h / 2, w, h);
    m_scene->addItem(item);
    m_ellipseItems.append(item);
    item->setSelected(true);
    AppLogger::info(QStringLiteral("添加ROI"),
                    QStringLiteral("type=ellipse center=(%1,%2) size=%3x%4 total=%5")
                        .arg(x).arg(y).arg(w).arg(h)
                        .arg(m_rectItems.size() + m_ellipseItems.size() + m_rotatedRectItems.size()));
}

/**
 * @brief 添加可旋转矩形 ROI（追加到列表，不清除已有 ROI）
 */
void Widget::addRotatedRectItem(qreal x, qreal y, qreal w, qreal h)
{
    pushUndoSnapshot(QStringLiteral("添加旋转矩形ROI"));
    deselectAllRoiItems();
    auto *item = new ResizableRotatedRectItem(w, h);
    item->setPos(x, y);
    item->setRotation(0);
    m_scene->addItem(item);
    m_rotatedRectItems.append(item);
    item->setSelected(true);
    AppLogger::info(QStringLiteral("添加ROI"),
                    QStringLiteral("type=rotatedRect center=(%1,%2) size=%3x%4 total=%5")
                        .arg(x).arg(y).arg(w).arg(h)
                        .arg(m_rectItems.size() + m_ellipseItems.size() + m_rotatedRectItems.size()));
}

/**
 * @brief 从 RoiInfo 在场景重建单个 ROI 图元（不 push 撤销）
 *
 * restoreSession / 会话恢复专用：按 shape 创建对应 Resizable*Item 并加入列表。
 * 与 add*Item 不同，不在图像中心新建、也不写撤销栈。
 */
void Widget::applyRoiFromInfo(const RoiInfo &info)
{
    if (info.isEmpty())
        return;
    switch (info.shape) {
    case RoiInfo::Shape::Rect: {
        const QRectF r = info.rect.normalized();
        auto *item = new ResizableRectItem(r.x(), r.y(), r.width(), r.height());
        m_scene->addItem(item);
        m_rectItems.append(item);
        break;
    }
    case RoiInfo::Shape::Ellipse: {
        const QRectF r = info.rect.normalized();
        auto *item = new ResizableEllipseItem(r.x(), r.y(), r.width(), r.height());
        m_scene->addItem(item);
        m_ellipseItems.append(item);
        break;
    }
    case RoiInfo::Shape::RotatedRect: {
        auto *item = new ResizableRotatedRectItem(info.size.width(), info.size.height());
        item->setPos(info.center);
        item->setRotation(info.angleDeg);
        m_scene->addItem(item);
        m_rotatedRectItems.append(item);
        break;
    }
    default:
        break;
    }
}

/**
 * @brief ROI 菜单「添加」：按形状下拉，在图像中心放 DEFAULT_ROI_SIZE 选区
 */
void Widget::AddRoi()
{
    if (!m_pixmapItem) {                                             // 还没打开图，场景无 pixmap
        QMessageBox::information(this, tr("提示"), tr("请先打开一张图片"));
        return;
    }

    const QPointF c = imageCenterInScene();                          // 图像中心（场景坐标）
    const qreal size = AppConfig::DEFAULT_ROI_SIZE;                  // 默认边长/直径
    const int type = m_roiTypeCombo ? m_roiTypeCombo->currentIndex() : 0; // 0矩 1圆 2旋转矩

    if (type == 0)
        addRectItem(c.x(), c.y(), size, size);                       // 矩形（内部会 push 撤销）
    else if (type == 1)
        addEllipseItem(c.x(), c.y(), size, size);                    // 椭圆
    else
        addRotatedRectItem(c.x(), c.y(), size * 1.4, size);          // 旋转矩形略扁长
}

/**
 * @brief 删 ROI（菜单删除 / Delete）：有选中只删选中，无选中则清空全部
 *
 * 删完后若已有图且处理链非空须立刻 onApplyProcessing：
 * scene.changed 在 ROI 为空时会直接 return，不能再靠防抖定时器刷新。
 */
void Widget::DelteteRoi()
{
    pushUndoSnapshot(QStringLiteral("删除ROI"));                     // 先记整份会话（块+ROI）
    const auto selected = m_scene->selectedItems();                  // 当前选中的场景图元
    const int before = m_rectItems.size() + m_ellipseItems.size() + m_rotatedRectItems.size();
    if (selected.isEmpty()) {
        clearAllRoi();                                               // 无选中 → 清空三类 ROI 列表
        AppLogger::info(QStringLiteral("删除ROI"),
                        QStringLiteral("mode=全部 before=%1").arg(before));
    } else {
        int removed = 0;
        for (QGraphicsItem *item : selected) {                       // 只处理我们自己的 ROI 类型
            if (auto *r = dynamic_cast<ResizableRectItem *>(item)) {
                m_rectItems.removeOne(r);                            // 从跟踪列表摘掉
                m_scene->removeItem(r);                              // 从场景移除
                delete r;                                            // 释放图元
                ++removed;
            } else if (auto *e = dynamic_cast<ResizableEllipseItem *>(item)) {
                m_ellipseItems.removeOne(e);
                m_scene->removeItem(e);
                delete e;
                ++removed;
            } else if (auto *rr = dynamic_cast<ResizableRotatedRectItem *>(item)) {
                m_rotatedRectItems.removeOne(rr);
                m_scene->removeItem(rr);
                delete rr;
                ++removed;
            }
        }
        m_lastRoisForDebounce.clear();                               // 强制下次防抖视为「ROI 变了」
        AppLogger::info(QStringLiteral("删除ROI"),
                        QStringLiteral("mode=选中 removed=%1 remain=%2")
                            .arg(removed)
                            .arg(m_rectItems.size() + m_ellipseItems.size()
                                 + m_rotatedRectItems.size()));
    }

    if (m_processor->hasImage() && !m_blockList.isEmpty())
        onApplyProcessing();                                         // 立刻按剩余 ROI 重跑处理链
}

/**
 * @brief 从场景图元打包全部 RoiInfo（算法层只认这个纯数据结构）
 *
 * 空列表 → 全图处理；多个 ROI → RoiProcess 做并集 mask。
 */
QList<RoiInfo> Widget::getAllRoiInfo() const
{
    QList<RoiInfo> list;
    if (!m_processor->hasImage())
        return list;

    for (ResizableRectItem *item : m_rectItems) {
        if (!item) continue;
        RoiInfo info;
        info.shape = RoiInfo::Shape::Rect;
        info.rect = item->mapToScene(item->rect()).boundingRect();
        list.append(info);
    }
    for (ResizableEllipseItem *item : m_ellipseItems) {
        if (!item) continue;
        RoiInfo info;
        info.shape = RoiInfo::Shape::Ellipse;
        info.rect = item->mapToScene(item->rect()).boundingRect();
        list.append(info);
    }
    for (ResizableRotatedRectItem *item : m_rotatedRectItems) {
        if (!item) continue;
        RoiInfo info;
        info.shape = RoiInfo::Shape::RotatedRect;
        info.center = item->mapToScene(item->localRect().center());
        info.size = item->localRect().size();
        info.angleDeg = item->rotation();
        list.append(info);
    }
    return list;
}

/**
 * @brief 把当前图片的处理链 + 多 ROI 写入内存会话表并落盘
 *
 * 键为 m_currentImagePath；切图前、关闭前由调用方触发。
 */
void Widget::saveCurrentSession()
{
    if (m_currentImagePath.isEmpty())
        return;

    const ImageSession session = captureSessionSnapshot();
    m_sessions.insert(m_currentImagePath, session);
    AppLogger::info(QStringLiteral("保存会话"),
                    QStringLiteral("path=%1 blocks=%2 rois=%3")
                        .arg(m_currentImagePath)
                        .arg(session.chain.size())
                        .arg(session.rois.size()));
    saveSessionsToDisk();
}

/**
 * @brief 采集当前 UI 快照：处理块 saveParams 链 + 全部 ROI
 *
 * 不写磁盘；供撤销栈与会话内存缓存共用同一数据结构 ImageSession。
 */
ImageSession Widget::captureSessionSnapshot() const
{
    ImageSession session;
    for (BaseBlock *block : m_blockList) {
        if (block)
            session.chain.append(block->saveParams());               // 每块类型+参数打成 JSON
    }
    session.rois = getAllRoiInfo();                                  // 场景图元 → 纯数据 RoiInfo
    return session;
}

/**
 * @brief 可撤销操作前压栈一份「改之前」的会话快照
 *
 * m_undoRestoring=true 时跳过（撤销/导入恢复链时内部 clear/add 不再递归入栈）。
 * 超过 MAX_UNDO 丢最旧。reason 仅打日志，不进快照内容。
 */
void Widget::pushUndoSnapshot(const QString &reason)
{
    if (m_undoRestoring)
        return;                                                      // 恢复过程中禁止再压栈
    m_undoStack.append(captureSessionSnapshot());                    // 记下当前块链+ROI
    while (m_undoStack.size() > MAX_UNDO)
        m_undoStack.removeFirst();                                   // 超限丢队首（最旧）
    AppLogger::info(QStringLiteral("撤销快照"),
                    QStringLiteral("reason=%1 depth=%2").arg(reason).arg(m_undoStack.size()));
}

/**
 * @brief 编辑→撤销 / Ctrl+Z：弹出栈顶 → restoreSession → 重算画面
 *
 * 一次撤销整份会话（所有块参数 + 所有 ROI），不是单控件逐步回退。
 */
void Widget::onUndo()
{
    if (m_undoStack.isEmpty()) {
        AppLogger::info(QStringLiteral("撤销"), QStringLiteral("栈空"));
        return;                                                      // 无可撤
    }
    const ImageSession snap = m_undoStack.takeLast();                 // 取出最近一次操作前状态
    m_undoRestoring = true;                                          // 恢复期：add/clear 不 push
    restoreSession(snap);                                            // 清块重建 + 重建 ROI
    m_undoRestoring = false;
    onApplyProcessing();                                             // 按恢复后的链/ROI 刷新结果图
    AppLogger::info(QStringLiteral("撤销"),
                    QStringLiteral("remain=%1 blocks=%2 rois=%3")
                        .arg(m_undoStack.size())
                        .arg(snap.chain.size())
                        .arg(snap.rois.size()));
}

/** @brief 会话 JSON 路径：<应用目录>/sessions/app_sessions.json */
QString Widget::sessionsFilePath() const
{
    const QString dir = QCoreApplication::applicationDirPath() + QStringLiteral("/sessions");
    QDir().mkpath(dir);
    return dir + QStringLiteral("/app_sessions.json");
}

/** @brief 将全部 m_sessions 序列化写入磁盘（启动时由 loadSessionsFromDisk 读回） */
void Widget::saveSessionsToDisk() const
{
    const QString path = sessionsFilePath();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        AppLogger::warn(QStringLiteral("会话落盘失败"), path);
        return;
    }
    const QJsonDocument doc(sessionsToJson(m_sessions));
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    AppLogger::info(QStringLiteral("会话已落盘"),
                    QStringLiteral("path=%1 count=%2").arg(path).arg(m_sessions.size()));
}

/** @brief 启动时加载历史会话；文件不存在或 JSON 无效则保持空表 */
void Widget::loadSessionsFromDisk()
{
    const QString path = sessionsFilePath();
    QFile file(path);
    if (!file.exists())
        return;
    if (!file.open(QIODevice::ReadOnly)) {
        AppLogger::warn(QStringLiteral("会话读取失败"), path);
        return;
    }
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    file.close();
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        AppLogger::warn(QStringLiteral("会话 JSON 无效"), err.errorString());
        return;
    }
    m_sessions = sessionsFromJson(doc.object());
    AppLogger::info(QStringLiteral("会话已加载"),
                    QStringLiteral("path=%1 count=%2").arg(path).arg(m_sessions.size()));
}

/**
 * @brief 用 ImageSession 还原处理链与多 ROI（切图恢复 / 撤销 / 导入链）
 *
 * m_suspendSceneReprocess：恢复过程中 scene 几何变化不触发 ROI 防抖重算
 *   （否则 clearAllRoi/applyRoiFromInfo 会排队 onApplyProcessing）。
 * m_undoRestoring：恢复内 clearAllBlocks/createBlock 不 pushUndoSnapshot。
 * disconnect requestReprocess：批量 loadParams 期间块参数变化不逐块重算，
 *   全部建完后再 connect，由调用方统一 onApplyProcessing。
 * m_lastRoisForDebounce 同步为当前 ROI，避免恢复后首帧误判「几何已变」。
 * singleShot(0) 解除 m_suspendSceneReprocess，让后续用户拖拽 ROI 仍可重算。
 */
void Widget::restoreSession(const ImageSession &session)
{
    m_suspendSceneReprocess = true;
    const bool wasUndo = m_undoRestoring;
    m_undoRestoring = true; // 恢复过程中建块/清块不进撤销栈

    clearAllBlocks(false);

    disconnect(m_processor, &ImageProcessor::requestReprocess,
               this, &Widget::onApplyProcessing);

    for (const QJsonValue &v : session.chain) {
        const QJsonObject obj = v.toObject();
        const QString name = obj.value(QStringLiteral("name")).toString();
        BaseBlock *block = createBlockByName(name);
        if (block)
            block->loadParams(obj);
    }

    clearAllRoi();
    for (const RoiInfo &roi : session.rois)
        applyRoiFromInfo(roi);
    m_lastRoisForDebounce = getAllRoiInfo();

    connect(m_processor, &ImageProcessor::requestReprocess,
            this, &Widget::onApplyProcessing);
    refreshChainHint();

    m_undoRestoring = wasUndo;

    AppLogger::info(QStringLiteral("恢复会话"),
                    QStringLiteral("blocks=%1 rois=%2")
                        .arg(session.chain.size())
                        .arg(session.rois.size()));

    QTimer::singleShot(0, this, [this]() {
        m_suspendSceneReprocess = false;
    });
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
 *   所以约定：Widget 负责“读 ROI → setRois → reprocess”。
 *
 * -------------------------------------------------------------------------
 * 执行步骤
 * -------------------------------------------------------------------------
 *   无图 → 直接返回
 *   无块 → 显示原图（相当于处理链为空）
 *   有块 → 退出对比模式 → setRois → reprocess
 *          reprocess 结束后发 processingFinished → onProcessingFinished
 */
void Widget::onApplyProcessing()
{
    if (!m_processor->hasImage())
        return;

    // 开始处理时强制显示结果图（若正在按住对比，也会被打断回结果）
    m_showOriginal = false;

    // 关键两步：先同步 ROI 列表，再跑流水线（空链时 reprocess 会把 m_result 置回原图）
    const QList<RoiInfo> rois = getAllRoiInfo();
    m_lastRoisForDebounce = rois;
    m_processor->setRois(rois);

    if (m_blockList.isEmpty()) {
        AppLogger::info(QStringLiteral("应用处理"),
                        QStringLiteral("空链，结果重置为原图 path=%1").arg(m_currentImagePath));
    } else {
        QStringList names;
        int enabled = 0;
        for (BaseBlock *b : m_blockList) {
            if (!b) continue;
            names << b->blockName();
            if (b->isEnabled())
                ++enabled;
        }
        AppLogger::info(
            QStringLiteral("开始处理"),
            QStringLiteral("path=%1 blocks=%2 enabled=%3 rois=%4 chain=[%5]")
                .arg(m_currentImagePath)
                .arg(m_blockList.size())
                .arg(enabled)
                .arg(rois.size())
                .arg(names.join(QStringLiteral(" -> "))));
    }

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
        QMessageBox::information(this, tr("提示"),
                                 tr("没有可保存的结果图"));
        return;
    }
    const QString path = QFileDialog::getSaveFileName(
        this, tr("保存结果"), QStringLiteral("result.png"),
        QStringLiteral("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));
    if (path.isEmpty()) return;  // 取消

    if (!m_processor->resultImage().save(path)) {
        AppLogger::error(QStringLiteral("保存结果失败"), path);
        QMessageBox::warning(this, tr("错误"),
                             tr("保存失败"));
    } else {
        AppLogger::info(QStringLiteral("保存结果"),
                        QStringLiteral("path=%1 size=%2x%3")
                            .arg(path)
                            .arg(m_processor->resultImage().width())
                            .arg(m_processor->resultImage().height()));
    }
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
    int ret = QMessageBox::question(this,
                                    tr("是否删除"),
                                    tr("你确定要执行这个操作吗？"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        // 用户点了 "是"，执行对应操作
        clearAllBlocks(true);
    } else {
        return;
    }

}

/**
 * @brief 移除全部处理块
 * @param reprocessAfter true 时清空后把预览打回原图并重算入口
 *
 * reprocessAfter 为 true 且仍有图像时：resetResultToOriginal 清空 m_result，
 * 避免链已空但对比/显示仍用旧处理结果；updatePixmapItem 直接显示原图。
 */
void Widget::clearAllBlocks(bool reprocessAfter)
{
    if (!m_undoRestoring && !m_blockList.isEmpty())
        pushUndoSnapshot(QStringLiteral("清空处理链"));
    const int n = m_blockList.size();
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
    AppLogger::info(QStringLiteral("清空处理链"),
                    QStringLiteral("removed=%1 reprocessAfter=%2")
                        .arg(n)
                        .arg(reprocessAfter ? QStringLiteral("是") : QStringLiteral("否")));
    if (!reprocessAfter)
        return;
    if (m_processor->hasImage()) {
        m_showOriginal = false;
        // 清空链后必须把 m_result 也打回原图，否则「对比」松开仍显示旧处理结果
        m_processor->resetResultToOriginal();
        updatePixmapItem(m_processor->originalImage());
        ui->label_3->setText(QStringLiteral("0 ms"));
    }
}

/**
 * @brief 导出处理链为 JSON（块顺序 + 各块参数）
 */
void Widget::on_btnExportChain_clicked()
{
    if (m_blockList.isEmpty()) {
        QMessageBox::information(this, tr("提示"),
                                 tr("当前没有处理块可导出"));
        return;
    }

    const QString path = QFileDialog::getSaveFileName(
        this, tr("导出处理链"), QStringLiteral("chain.json"),
        tr(AppConfig::CHAIN_FILE_FILTER));
    if (path.isEmpty())
        return;

    QJsonArray blocks;
    for (BaseBlock *block : m_blockList) {
        if (block)
            blocks.append(block->saveParams());
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("blocks"), blocks);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        AppLogger::error(QStringLiteral("导出处理链失败"), path);
        QMessageBox::warning(this, tr("错误"),
                             tr("无法写入文件：%1").arg(path));
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();

    AppLogger::info(QStringLiteral("导出处理链"),
                    QStringLiteral("path=%1 blocks=%2").arg(path).arg(blocks.size()));
    QMessageBox::information(this, tr("完成"),
                             tr("已导出 %1 个处理块").arg(blocks.size()));
}

/**
 * @brief 从 JSON 导入处理链（替换现有块，参数一并恢复）
 */
void Widget::on_btnImportChain_clicked()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("导入处理链"), QString(),
        tr(AppConfig::CHAIN_FILE_FILTER));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        AppLogger::error(QStringLiteral("导入处理链失败"), path);
        QMessageBox::warning(this, tr("错误"),
                             tr("无法读取文件：%1").arg(path));
        return;
    }

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    file.close();
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        QMessageBox::warning(this, tr("错误"),
                             tr("JSON 无效：%1").arg(err.errorString()));
        return;
    }

    const QJsonArray blocks = doc.object().value(QStringLiteral("blocks")).toArray();
    if (blocks.isEmpty()) {
        QMessageBox::information(this, tr("提示"),
                                 tr("文件中没有处理块"));
        return;
    }

    if (!m_blockList.isEmpty()) {
        const auto ret = QMessageBox::question(
            this, tr("导入处理链"),
            tr("导入将替换当前处理链，是否继续？"));
        if (ret != QMessageBox::Yes)
            return;
    }

    pushUndoSnapshot(QStringLiteral("导入处理链"));

    // 批量重建：断开重算，建完再算一次
    m_undoRestoring = true;
    disconnect(m_processor, &ImageProcessor::requestReprocess,
               this, &Widget::onApplyProcessing);

    while (!m_blockList.isEmpty()) {
        BaseBlock *block = m_blockList.takeFirst();
        m_blockLayout->removeWidget(block);
        m_processor->removeBlock(block);
        block->deleteLater();
    }

    int ok = 0;
    for (const QJsonValue &v : blocks) {
        const QJsonObject obj = v.toObject();
        const QString name = obj.value(QStringLiteral("name")).toString();
        BaseBlock *block = createBlockByName(name);
        if (!block)
            continue;
        block->loadParams(obj);
        ++ok;
    }

    connect(m_processor, &ImageProcessor::requestReprocess,
            this, &Widget::onApplyProcessing);

    m_undoRestoring = false;
    refreshChainHint();
    onApplyProcessing();

    AppLogger::info(QStringLiteral("导入处理链"),
                    QStringLiteral("path=%1 ok=%2 total=%3").arg(path).arg(ok).arg(blocks.size()));

    if (ok == 0) {
        QMessageBox::warning(this, tr("错误"),
                             tr("未能识别文件中的任何处理块"));
    } else if (ok < blocks.size()) {
        QMessageBox::information(this, tr("完成"),
                                 tr("已导入 %1 / %2 个处理块（部分名称无法识别）")
                                     .arg(ok).arg(blocks.size()));
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
 *
 * 处理链为空时始终显示 originalImage：m_result 可能仍保留上次链的输出，
 * 此时对比模式也应等价于原图，不能误用 resultImage。
 */
void Widget::refreshDisplay()
{
    if (!m_processor->hasImage()) return;
    // 无处理块时对比也应等于原图，避免 m_result 残留旧处理结果
    if (m_blockList.isEmpty()) {
        updatePixmapItem(m_processor->originalImage());
        return;
    }
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
    AppLogger::info(QStringLiteral("处理完成"),
                    QStringLiteral("elapsed=%1 ms path=%2 result=%3x%4")
                        .arg(elapsedMs)
                        .arg(m_currentImagePath)
                        .arg(m_processor->resultImage().width())
                        .arg(m_processor->resultImage().height()));
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
    // changed 多为排队发出：同步清标志会漏拦；放到下一事件循环再关
    QTimer::singleShot(0, this, [this]() {
        m_suspendSceneReprocess = false;
    });
}

/** @brief 场景里是否存在任意 ROI */
bool Widget::hasAnyRoi() const
{
    return !m_rectItems.isEmpty() || !m_ellipseItems.isEmpty()
        || !m_rotatedRectItems.isEmpty();
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
        if (isRoiItem(hit))
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
 *   → DragEnter/Move：左侧算法名(text) 或 已有块换序(MIME_BLOCK_REORDER)
 *   → DragLeave：取消高亮
 *   → Drop：换序 → reorderBlock；新建 → createBlockByName
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

    auto acceptIfKnownMime = [](QMimeData const *mime) {
        return mime
            && (mime->hasFormat(QLatin1String(AppConfig::MIME_BLOCK_REORDER))
                || mime->hasText());
    };

    switch (event->type()) {
    case QEvent::DragEnter: {
        auto *e = static_cast<QDragEnterEvent *>(event);
        if (acceptIfKnownMime(e->mimeData())) {
            m_isDragInside = true;
            setDropPanelHighlight(true);
            e->acceptProposedAction();
            return true;
        }
        break;
    }
    case QEvent::DragMove: {
        auto *e = static_cast<QDragMoveEvent *>(event);
        if (acceptIfKnownMime(e->mimeData())) {
            e->acceptProposedAction();
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
        const QMimeData *mime = e->mimeData();
        if (!mime)
            break;

        // 1) 已有块换序（优先于 text，避免误当成新建）
        if (mime->hasFormat(QLatin1String(AppConfig::MIME_BLOCK_REORDER))) {
            QByteArray bytes = mime->data(QLatin1String(AppConfig::MIME_BLOCK_REORDER));
            QDataStream stream(&bytes, QIODevice::ReadOnly);
            quintptr ptr = 0;
            stream >> ptr;
            auto *block = reinterpret_cast<BaseBlock *>(ptr);
            if (block && m_blockList.contains(block) && ui->blockListContainer) {
                auto *receiver = qobject_cast<QWidget *>(obj);
                const QPoint dropPos = e->position().toPoint();
                const QPoint inContainer = receiver
                    ? ui->blockListContainer->mapFrom(receiver, dropPos)
                    : dropPos;
                reorderBlock(block, blockInsertIndexAtY(inContainer.y()));
            }
            e->acceptProposedAction();
            return true;
        }

        // 2) 左侧工具箱拖入算法名 → 新建块
        if (mime->hasText()) {
            if (!createBlockByName(mime->text())) {
                QMessageBox::information(this, tr("提示"),
                                         tr("未识别的算法：%1")
                                             .arg(mime->text()));
            }
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
            QMessageBox::information(this, tr("提示"),
                                     tr("请先打开图片"));
            return;
        }

        // 与各 Block::process 相同的颜色约定：先 RGB Mat，再转 BGR 给 OpenCV
        cv::Mat src = ImageConverter::pixmapToMatRGB(m_processor->originalImage());
        cv::cvtColor(src, src, cv::COLOR_RGB2BGR);

        // 有 ROI 时只在并集外包络矩形内统计直方图，减少背景干扰
        const QList<RoiInfo> rois = getAllRoiInfo();
        if (!rois.isEmpty()) {
            QRectF br;
            for (const RoiInfo &roi : rois) {
                if (roi.isEmpty())
                    continue;
                const QRectF b = roi.boundingRect();
                br = br.isNull() ? b : br.united(b);
            }
            if (!br.isNull()) {
                cv::Rect rr(qRound(br.x()), qRound(br.y()),
                            qRound(br.width()), qRound(br.height()));
                rr &= cv::Rect(0, 0, src.cols, src.rows);
                if (!rr.empty())
                    src = src(rr).clone();
            }
        }

        const int t = OtsuAlgorithm::calculateThresholdFromBGR(src);
        // 下限=Otsu 阈值，上限=255：常见“大于阈值变白”的设定
        block->setThresholds(t, 255);
        AppLogger::info(QStringLiteral("Otsu自动阈值"),
                        QStringLiteral("t=%1 rois=%2").arg(t).arg(rois.size()));
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
 * @return 新建的块；名字无法识别时返回 nullptr
 */
BaseBlock *Widget::createBlockByName(const QString &name)
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
    } else if (name == QString::fromUtf8(AppConfig::BLOCK_NAME_PSEUDOCOLOR)
               || name == AppConfig::BLOCK_NAME_PSEUDOCOLOR) {
        block = new PseudoColorBlock(parent);
    } else if (name == QString::fromUtf8(AppConfig::BLOCK_NAME_GLCM)
               || name == AppConfig::BLOCK_NAME_GLCM) {
        block = new GlcmBlock(parent);
    }

    if (block)
        addBlockToPanel(block);
    return block;
}

/**
 * @brief 把已创建的块挂到右侧面板，并注册到处理引擎
 *
 * 顺序：撤销快照 → 布局/列表 → 信号（删/复制粘贴/参数将改）→ processor.addBlock。
 * m_blockList 与 processor.blocks() 须同序（reprocess 跟引擎序，面板跟 m_blockList）。
 */
void Widget::addBlockToPanel(BaseBlock *block)
{
    if (!block || !m_blockLayout) return;                             // 空块或布局未就绪

    if (!m_undoRestoring)
        pushUndoSnapshot(QStringLiteral("添加处理块"));              // 添加前记快照（恢复建块跳过）

    block->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum); // 纵向不被外层挤扁

    m_blockLayout->addWidget(block);                                 // 挂到右侧滚动区布局
    m_blockList.append(block);                                       // 同步 UI 侧顺序列表

    connect(block, &BaseBlock::removeRequested, this, [this, block]() {
        pushUndoSnapshot(QStringLiteral("删除处理块"));              // 删前记快照
        const QString name = block->blockName();
        m_blockLayout->removeWidget(block);                          // 从布局摘掉
        m_blockList.removeOne(block);                                // 从跟踪列表摘掉
        m_processor->removeBlock(block);                             // 引擎移除并 requestReprocess
        block->deleteLater();                                        // 延迟销毁控件
        refreshChainHint();                                          // 更新链提示文案
        AppLogger::info(QStringLiteral("删除处理块"),
                        QStringLiteral("name=%1 remain=%2").arg(name).arg(m_blockList.size()));
    });
    connect(block, &BaseBlock::copyRequested, this, [this, block]() {
        copyBlockToClipboard(block);                                 // 类型+参数进剪贴板
        AppLogger::info(QStringLiteral("复制处理块"), block->blockName());
    });
    connect(block, &BaseBlock::pasteRequested, this, [this, block]() {
        pasteBlockFromClipboard(block);                              // 在该块后插入剪贴板块
        AppLogger::info(QStringLiteral("粘贴处理块"),
                        QStringLiteral("after=%1").arg(block->blockName()));
    });
    connect(block, &BaseBlock::paramsAboutToChange, this, [this]() {
        if (!m_undoRestoring)                                        // 撤销恢复期不再压栈
            pushUndoSnapshot(QStringLiteral("修改处理块参数"));      // 旋钮动手前记整份会话
    });

    m_processor->addBlock(block);                                    // 入引擎；有图则触发重算
    block->show();                                                   // 确保可见
    refreshChainHint();                                              // 刷新「当前链」提示
    AppLogger::info(QStringLiteral("添加处理块"),
                    QStringLiteral("name=%1 index=%2")
                        .arg(block->blockName())
                        .arg(m_blockList.size() - 1));

    if (ui->blockScrollArea)
        ui->blockScrollArea->ensureWidgetVisible(block);             // 新块滚进可视区
}

int Widget::blockInsertIndexAtY(int yInContainer) const
{
    for (int i = 0; i < m_blockList.size(); ++i) {
        BaseBlock *b = m_blockList.at(i);
        if (!b) continue;
        if (yInContainer < b->geometry().center().y())
            return i;
    }
    return m_blockList.size();
}

/**
 * @brief 拖放改序：把块挪到 insertBefore，并同步 processor 顺序后重算
 */
void Widget::reorderBlock(BaseBlock *block, int insertBefore)
{
    if (!block || !m_blockLayout) return;

    const int from = m_blockList.indexOf(block);                     // 拖放前下标
    if (from < 0) return;

    int to = insertBefore;                                           // 「插到该下标之前」
    if (to > from)
        --to;                                                        // 先移除后再插，后方下标左移
    to = qBound(0, to, m_blockList.size() - 1);
    if (to == from)
        return;                                                      // 位置没变

    pushUndoSnapshot(QStringLiteral("调整处理块顺序"));              // 改序前记快照

    m_blockList.move(from, to);                                      // UI 列表重排
    m_blockLayout->removeWidget(block);
    m_blockLayout->insertWidget(to, block);                          // 布局同步
    block->show();

    m_processor->moveBlock(block, to);                               // 引擎同序并 requestReprocess
    AppLogger::info(QStringLiteral("调整处理块顺序"),
                    QStringLiteral("name=%1 from=%2 to=%3")
                        .arg(block->blockName()).arg(from).arg(to));
}

bool Widget::clipboardHasBlock() const
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    if (!mime)
        return false;
    if (mime->hasFormat(QLatin1String(AppConfig::MIME_BLOCK_CLIPBOARD)))
        return true;
    if (!mime->hasText())
        return false;
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(mime->text().toUtf8(), &err);
    return err.error == QJsonParseError::NoError
        && doc.isObject()
        && doc.object().contains(QStringLiteral("name"));
}

void Widget::copyBlockToClipboard(BaseBlock *block)
{
    if (!block)
        return;
    const QByteArray bytes = QJsonDocument(block->saveParams()).toJson(QJsonDocument::Compact);
    auto *mime = new QMimeData;
    mime->setData(QLatin1String(AppConfig::MIME_BLOCK_CLIPBOARD), bytes);
    mime->setText(QString::fromUtf8(bytes));
    QApplication::clipboard()->setMimeData(mime);
}

void Widget::pasteBlockFromClipboard(BaseBlock *afterBlock)
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    if (!mime)
        return;

    QByteArray bytes;
    if (mime->hasFormat(QLatin1String(AppConfig::MIME_BLOCK_CLIPBOARD)))
        bytes = mime->data(QLatin1String(AppConfig::MIME_BLOCK_CLIPBOARD));
    else if (mime->hasText())
        bytes = mime->text().toUtf8();
    else
        return;

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        QMessageBox::information(this, tr("提示"),
                                 tr("剪贴板中没有有效的处理块"));
        return;
    }

    const QJsonObject obj = doc.object();
    const QString name = obj.value(QStringLiteral("name")).toString();
    if (name.isEmpty()) {
        QMessageBox::information(this, tr("提示"),
                                 tr("剪贴板中没有有效的处理块"));
        return;
    }

    // 先建到末尾，再挪到 afterBlock 之后；批量时断开避免双重重算
    disconnect(m_processor, &ImageProcessor::requestReprocess,
               this, &Widget::onApplyProcessing);

    BaseBlock *created = createBlockByName(name);
    if (!created) {
        connect(m_processor, &ImageProcessor::requestReprocess,
                this, &Widget::onApplyProcessing);
        QMessageBox::information(this, tr("提示"),
                                 tr("未识别的算法：%1").arg(name));
        return;
    }
    created->loadParams(obj);

    if (afterBlock) {
        const int idx = m_blockList.indexOf(afterBlock);
        if (idx >= 0)
            reorderBlock(created, idx + 1);
    }

    connect(m_processor, &ImageProcessor::requestReprocess,
            this, &Widget::onApplyProcessing);
    onApplyProcessing();
}




