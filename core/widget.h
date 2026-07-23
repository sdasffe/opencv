#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsItem>
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
#include <QComboBox>
#include <QMenu>
#include <QTranslator>
#include <QEvent>
#include <QHash>

#include "imageprocessor.h"
#include "imagesession.h"
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
 * 重写 mimeData()：拖出去时把算法稳定 id（UserRole）放进剪贴板文本。
 * 右侧面板 Drop 时读这个名字，再 createBlockByName() 创建对应块。
 */
class MyListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit MyListWidget(QWidget *parent = nullptr) {
        setDragEnabled(true);                              // 允许鼠标按住列表项向外拖出
        setDragDropMode(QAbstractItemView::DragOnly);      // 只允许拖出，不接受外部 drop 进来
    }
protected:
    /** 生成拖拽 MIME：优先 UserRole 稳定 id，保证中英切换后仍能匹配块工厂 */
    QMimeData *mimeData(const QList<QListWidgetItem *> &items) const override {
        if (items.isEmpty()) return nullptr;               // 无选中项则不启动拖拽
        auto *mimeData = new QMimeData;
        QString id = items.first()->data(Qt::UserRole).toString(); // 稳定块 id（如「二值化处理」）
        if (id.isEmpty())
            id = items.first()->text();                    // 兼容：无 UserRole 时退回显示文字
        mimeData->setText(id);                             // Drop 侧用 mimeData->text() 取名建块
        return mimeData;
    }
};

/**
 * @brief 主窗口：图像显示 + ROI + 处理链 UI
 *
 * 【界面分区】（见 widget.ui）
 *   - 菜单栏：文件 / 编辑 / 设置 / 帮助 / 关于；工具条：应用、对比、保存、耗时
 *   - 左侧：算法列表 listWidget，可拖出算法名
 *   - 中间：graphicsView 画布；下方 folderBrowserPanel 显示文件夹缩略图
 *   - 右侧：图像处理工具箱 widget_3，放置 BaseBlock 子控件
 *
 * 【典型用户路径】
 *   打开图片 / 打开文件夹（点缩略图切换）→（可选）ROI → 拖入算法 → 调参 → 对比/保存
 *
 * 【与 ImageProcessor 的关系】
 *   Widget 管 UI 与 ROI；ImageProcessor 管“按块顺序算图”。
 *   重算统一走 onApplyProcessing()：先 getAllRoiInfo() → setRois → reprocess。
 */
class Widget : public QWidget
{
    Q_OBJECT

public:
    /** 构造：加载 .ui、搭场景/菜单/拖放/会话，并接好重算相关信号槽 */
    explicit Widget(QWidget *parent = nullptr);             // parent 一般为 nullptr；子对象随 this 销毁
    /** 析构：只 delete ui；scene/processor/timer 有 parent 由 Qt 自动释放 */
    ~Widget() override;                                    // 勿在此手动 delete m_scene / m_processor

protected:
    /** 滚轮缩放：以鼠标下场景点为锚，限制缩放范围，刷新信息栏 */
    void wheelEvent(QWheelEvent *event) override;          // 有图时才缩放；无图交给基类默认处理
    /** 键盘：Delete 删 ROI；Ctrl+0 适应窗口；其余交给 QWidget */
    void keyPressEvent(QKeyEvent *event) override;         // 需 StrongFocus 才能收到按键
    /** 语言切换事件：retranslateUi + 动态 ROI/算法名 + 各块 retranslateUi */
    void changeEvent(QEvent *event) override;              // 仅处理 LanguageChange，其它走基类
    /**
     * 事件过滤：
     *   - graphicsView viewport：空白处拖动画布（点在 ROI 上不抢）
     *   - widget_3 / blockScrollArea：接收算法名 Drop 建块
     */
    bool eventFilter(QObject *obj, QEvent *event) override;// 返回 true 表示已消费该事件

private slots:
    /** 文件→打开图片：弹出文件框，成功则 loadImageFromPath，失败提示 */
    void actOpenImage();                          // 单张打开会 clearFolderBrowser
    /** 文件→打开文件夹：扫常见图片后缀，填缩略图条并加载第一张 */
    void actOpenFolder();                        // 空文件夹会警告并清空缩略图
    /** 编辑→ROI→添加：按下拉形状在图像中心附近创建 ROI（压撤销栈） */
    void AddRoi();                        // 无图时直接 return；形状下标 0/1/2
    /** 编辑→ROI→删除 或 Delete 键：有选中则删选中，否则清空全部并重算 */
    void DelteteRoi();                             // 对象名历史拼写 deltete，槽名保持兼容
    /** 文件→退出：关闭主窗口（aboutToQuit 会落盘会话） */
    void onExitApp();                                      // 等价于 close()
    /** 关于：模态框显示应用名、版本与简要说明 */
    void onAboutApp();                                     // 文案走 tr，支持中英
    /** 帮助：列出快捷键与主要操作步骤 */
    void onHelpShortcuts();                                // 文案走 tr，支持中英
    /** 设置→语言→中文：卸英文翻译器，写 QSettings，刷界面 */
    void onLanguageChinese();                              // m_englishUi=false
    /** 设置→语言→English：装 :/i18n/opencv_en.qm，写配置，刷界面 */
    void onLanguageEnglish();                              // m_englishUi=true
    /** 设置→外观→浅色：StyleLoader 加载 light QSS 并 polish */
    void onThemeLight();                                   // m_themeId="light"
    /** 设置→外观→深色：StyleLoader 加载 dark QSS 并 polish */
    void onThemeDark();                                    // m_themeId="dark"

    /** 引擎 processingFinished：刷画布、写耗时 label、updateInfoLabel */
    void onProcessingFinished(qint64 elapsedMs);           // elapsedMs 为本次 reprocess 毫秒
    /** 【核心】统一重算：同步多 ROI → setRois → reprocess（无图/空链则只刷新） */
    void onApplyProcessing();                              // 参数变化、应用键、ROI 防抖、撤销后都进这里
    /** ROI 几何变更（预留/兼容；主路径是 scene.changed + 60ms 定时器） */
    void onRoiGeometryChanged();                           // 内部仍转到防抖重算逻辑
    /** 工具条「应用」：用户手动请求按当前链与 ROI 重算 */
    void on_btnApply_clicked();                            // 直接调用 onApplyProcessing()
    /** 「对比」按下：m_showOriginal=true，只改显示不重算 */
    void on_btnCompare_pressed();                          // 显示 processor->originalImage()
    /** 「对比」松开：恢复显示结果图 */
    void on_btnCompare_released();                         // m_showOriginal=false 后 refreshDisplay
    /** 「保存」：文件对话框保存当前结果图（png/jpg/bmp 等） */
    void on_btnSave_clicked();                             // 无结果时提示；写失败打日志并弹框
    /** 工具箱「清空」：压撤销栈后 clearAllBlocks(true) 并复位结果为原图 */
    void on_btnClearChain_clicked();                       // 会触发重算/刷新
    /** 「导出」：把各块 saveParams 写成 {"blocks":[...]} JSON */
    void on_btnExportChain_clicked();                      // 空链也可导出空数组
    /** 「导入」：读 JSON，静默清链后按名建块 loadParams，再重算 */
    void on_btnImportChain_clicked();                      // 非法 JSON / 无块会警告
    /** 点文件夹缩略图：按 UserRole 路径 loadImageFromPath（带会话切换） */
    void on_folderImageList_itemClicked(QListWidgetItem *item); // m_loadingFromFolderList 防递归
    /** Ctrl+Z / 编辑→撤销：弹出 m_undoStack 顶，restoreSession 后再 onApplyProcessing */
    void onUndo();                                         // 空栈则忽略；恢复中设 m_undoRestoring

private:
    // ----- ROI 管理（可多个，算法侧做蒙版并集） -----
    /** 删除场景中全部 ROI 图元，清空三个列表，并同步防抖缓存 */
    void clearAllRoi();                                    // 不压撤销栈（由调用方决定是否 push）
    /** 在场景添加轴对齐矩形 ROI，加入 m_rectItems，压「添加矩形ROI」快照 */
    void addRectItem(qreal x, qreal y, qreal w, qreal h);  // 坐标为场景=图像像素
    /** 添加椭圆 ROI，压「添加椭圆ROI」快照 */
    void addEllipseItem(qreal x, qreal y, qreal w, qreal h); // 包围盒左上+宽高
    /** 添加可旋转矩形 ROI，压「添加旋转矩形ROI」快照 */
    void addRotatedRectItem(qreal x, qreal y, qreal w, qreal h); // 初始 angle=0
    /** 按 RoiInfo.shape 重建对应 Resizable*Item（会话恢复/撤销，不压栈） */
    void applyRoiFromInfo(const RoiInfo &info);            // 恢复期间应先 disconnect 防抖
    /** 判断 item 是否在 m_rect/ellipse/rotated 三个列表中 */
    bool isRoiItem(QGraphicsItem *item) const;             // 平移手势用来区分点空白还是点 ROI
    /** 遍历三类 ROI，全部 setSelected(false) */
    void deselectAllRoiItems();                            // 添加新 ROI 前可先取消旧选中

    // ----- 处理块管理 -----
    /** 工厂：按 AppConfig 块名 / UserRole id 创建具体 BaseBlock 子类 */
    BaseBlock *createBlockByName(const QString &name);     // 未知名返回 nullptr 并打 warn 日志
    /** 把块加入右侧布局与 m_blockList，processor->addBlock，接线删除/复制/粘贴 */
    void addBlockToPanel(BaseBlock *block);                // 非撤销恢复时压「添加处理块」并可 requestReprocess
    /** 拖拽换序：insertBefore 为目标下标，同步 layout / m_blockList / processor */
    void reorderBlock(BaseBlock *block, int insertBefore); // 会 pushUndoSnapshot 并重算
    /** 根据容器内 Y 坐标，返回应插入的下标（插到该块之前） */
    int blockInsertIndexAtY(int yInContainer) const;       // 落在空白底部则返回 count()
    /** 清空全部处理块；reprocessAfter=false 用于导入前静默清理 */
    void clearAllBlocks(bool reprocessAfter);              // true 时 disconnect/reconnect 防抖并重算
    /** 二值化块「Otsu」按钮：用原图（或 ROI 并集外接框）算阈值写回 spin */
    void wireBinarizationOtsu(class BinarizationBlock *block); // 无图时提示；算完触发 paramsChanged
    /** 将 block->saveParams() 包成 JSON 放入系统剪贴板 */
    void copyBlockToClipboard(BaseBlock *block);           // 供右键「复制」
    /** 从剪贴板解析块 JSON，createBlockByName + loadParams，插到 after 后或末尾 */
    void pasteBlockFromClipboard(BaseBlock *afterBlock);   // afterBlock 为空则追加；失败弹提示
    /** 探测剪贴板是否含本工具可识别的单块 JSON */
    bool clipboardHasBlock() const;                        // 空白处右键「粘贴」使能判断

    // ----- 显示 -----
    /** 替换场景中的底图 pixmap（临时挂起 scene.changed，避免误触发重算） */
    void updatePixmapItem(const QPixmap &pixmap);          // 保持 ROI 在上层（Z 值）
    /** 按 m_showOriginal 选择显示原图或 resultImage */
    void refreshDisplay();                                 // 无图时清空/保持安全
    /** 刷新 labelInfo / 标题等：分辨率、缩放百分比、文件名 */
    void updateInfoLabel();                                // 缩放取自 view transform
    /** Ctrl+0：resetTransform + fitInView，m_viewScale 重读为矩阵 m11 */
    void fitViewToImage();                                 // 无 sceneRect 则直接 return
    /** 把当前所有 ROI 图元导出为 RoiInfo 列表；空列表表示全图 */
    QList<RoiInfo> getAllRoiInfo() const;                  // onApplyProcessing / 会话快照都用它
    /** 返回当前 pixmap 在场景坐标下的中心，供默认放置新 ROI */
    QPointF imageCenterInScene() const;                    // 无图时退回 (0,0) 或视口中心策略
    /**
     * 从路径加载图片：存旧会话 → setOriginalImage → 换底图 fitInView
     * → 有会话则 restore，否则清空链与 ROI → 有块则重算
     */
    bool loadImageFromPath(const QString &filePath);       // 失败返回 false（坏图/读失败）

    /** 若当前有路径：captureSessionSnapshot 写入 m_sessions[path] */
    void saveCurrentSession();                             // 换图/退出前调用
    /** 按快照重建块列表与 ROI（挂起防抖）；不自动 reprocess */
    void restoreSession(const ImageSession &session);      // 调用方负责随后 onApplyProcessing
    /** 采集当前链 JSON 数组 + getAllRoiInfo() 成 ImageSession */
    ImageSession captureSessionSnapshot() const;           // 撤销与会话共用格式
    /** 结构变更前压栈；超过 MAX_UNDO 丢掉最旧；撤销恢复中跳过 */
    void pushUndoSnapshot(const QString &reason);          // reason 仅用于日志
    /** 将 m_sessions 序列化到 {exe}/sessions/app_sessions.json */
    void saveSessionsToDisk() const;                       // aboutToQuit 时调用
    /** 启动时读 app_sessions.json 填满 m_sessions（version 校验） */
    void loadSessionsFromDisk();                           // 文件不存在则静默跳过
    /** 返回会话 JSON 的绝对路径 */
    QString sessionsFilePath() const;                      // 目录不存在时由保存侧创建

    // ----- 文件夹浏览 -----
    /** 配置 folderImageList：图标模式、尺寸、点击信号等 */
    void setupFolderBrowser();                             // 默认面板隐藏，打开文件夹后显示
    /** 清空缩略图项并隐藏 folderBrowserPanel */
    void clearFolderBrowser();                             // 打开单张图时调用
    /** 按文件列表生成缩略图 QListWidgetItem（UserRole=绝对路径） */
    void fillFolderBrowser(const QString &dirPath, const QFileInfoList &files); // 并 show 面板
    /** 选中并滚动到与 filePath 匹配的缩略图项 */
    void selectFolderThumbnail(const QString &filePath);   // 加载图成功后同步高亮

    // ----- 初始化（构造函数中按序调用） -----
    /** 绑定 scene、缩放锚点、拖拽模式、scene.changed→防抖定时器 */
    void setupGraphicsView();                              // 坐标系：场景坐标=图像像素
    /** 左侧 DragOnly；右侧 AcceptDrops + installEventFilter */
    void setupDragDrop();                                  // Drop 数据来自 MyListWidget::mimeData
    /** 取得 blockListLayout，加空白处粘贴菜单，配置滚动条策略 */
    void setupBlockPanel();                                // 块实际由 addBlockToPanel 动态插入
    /** 注册 Ctrl+0、Delete 快捷键（Ctrl+Z 在菜单 QAction 上） */
    void setupShortcuts();                                 // WindowShortcut，窗口激活即有效
    /** 连接 .ui 菜单动作；为 ROI 子菜单挂 ComboBox+添加/删除面板 */
    void setupMenus();                                     // 语言/外观用 QActionGroup 互斥
    /** 给 listWidget 每项 setData(UserRole, 稳定中文块 id) */
    void setupAlgoListIds();                               // 显示名仍可由 tr/retranslate 改变
    /** 按 m_englishUi 装/卸翻译器，retranslateUi，同步菜单勾选 */
    void applyLanguage();                                  // 偏好写入 QSettings ui/english
    /** 按 m_themeId 加载 QSS，unpolish/polish，写 ui/theme */
    void applyTheme();                                     // light / dark
    /** 刷新代码创建的 ROI 下拉/按钮文案与左侧算法显示名 */
    void retranslateDynamicUi();                           // 菜单等 .ui 文案由 retranslateUi 负责
    /** 按当前屏幕 availableGeometry 限制最小尺寸并缩小窗口 */
    void adaptWindowToScreen();                            // 避免笔记本小屏启动被裁切
    /** 拖拽进入/离开右侧时切换属性，驱动 QSS 高亮 */
    void setDropPanelHighlight(bool on);                   // on=true 表示可放置
    /** 处理链为空时显示「拖入算法」，有块则隐藏提示标签 */
    void refreshChainHint();                               // 增删块后调用
    /** 三类 ROI 列表是否至少有一个图元 */
    bool hasAnyRoi() const;                                // scene.changed 里无 ROI 可早退
    /** 处理 viewport 上的平移手势；命中 ROI 则返回 false 不拦截 */
    bool viewportPanEvent(QEvent *event);                  // 中键或左键空白拖动画布

private:
    Ui::Widget *ui;                                        // Designer 生成界面；析构时 delete

    QGraphicsScene *m_scene = nullptr;                     // 放底图 + ROI；parent=this
    QGraphicsPixmapItem *m_pixmapItem = nullptr;           // 当前底图项；换图时 delete 再建
    double m_viewScale = 1.0;                              // 相对 fitInView 后的额外缩放倍数

    QList<ResizableRectItem *> m_rectItems;                // 轴对齐矩形 ROI
    QList<ResizableEllipseItem *> m_ellipseItems;          // 椭圆 ROI
    QList<ResizableRotatedRectItem *> m_rotatedRectItems;  // 旋转矩形 ROI

    ImageProcessor *m_processor = nullptr;                 // 原图/结果/块序列/reprocess

    QVBoxLayout *m_blockLayout = nullptr;                  // 指向 ui->blockListLayout
    QList<BaseBlock *> m_blockList;                        // 与引擎块顺序严格一致

    QHash<QString, ImageSession> m_sessions;               // key=图片绝对路径 → 链+ROI
    QList<ImageSession> m_undoStack;                       // 当前图结构撤销栈
    bool m_undoRestoring = false;                          // true 时禁止 pushUndoSnapshot
    static constexpr int MAX_UNDO = 40;                    // 单图最多保留 40 层快照

    QComboBox *m_roiTypeCombo = nullptr;                   // ROI 菜单：矩形/圆形/旋转矩形
    QPushButton *m_roiAddBtn = nullptr;                    // ROI 菜单「添加」
    QPushButton *m_roiDeleteBtn = nullptr;                 // ROI 菜单「删除」
    QMenu *m_menuRoi = nullptr;                            // 操作后 close() 收起菜单
    QTranslator m_translatorEn;                            // 英文 qm 翻译器实例
    bool m_englishUi = false;                              // false=中文界面
    bool m_translatorLoaded = false;                       // 是否已 installTranslator
    QString m_themeId = QStringLiteral("light");           // "light" 或 "dark"
    QTimer *m_roiUpdateTimer = nullptr;                    // 单次 60ms，超时→onApplyProcessing

    bool m_isDragInside = false;                           // 拖算法经过右侧面板
    bool m_showOriginal = false;                           // 按住对比时为 true
    bool m_suspendSceneReprocess = false;                  // 换底图时禁止 changed→重算
    bool m_panning = false;                                // 正在平移画布
    bool m_loadingFromFolderList = false;                  // 缩略图点击加载中
    QList<RoiInfo> m_lastRoisForDebounce;                  // 上次已处理的 ROI，防重复重算
    QPoint m_panLastPos;                                   // 平移上一帧光标位置
    QString m_currentImagePath;                            // 当前图路径；空表示未打开
};

#endif // WIDGET_H
