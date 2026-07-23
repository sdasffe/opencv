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
#include <QAction>
#include <QTranslator>
#include <QEvent>
#include <QHash>
#include <QJsonArray>
#include <QVector>

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
        // 优先用 UserRole 稳定 id（中英切换后显示名会变）
        QString id = items.first()->data(Qt::UserRole).toString();
        if (id.isEmpty())
            id = items.first()->text();
        mimeData->setText(id);
        return mimeData;
    }
};

/**
 * @brief 主窗口：图像显示 + ROI + 处理链 UI
 *
 * 【界面分区】（见 widget.ui）
 *   - 顶栏：文件 / ROI / 设置菜单；应用、对比、保存、耗时
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
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

protected:
    /** 滚轮：以鼠标为锚点缩放画布 */
    void wheelEvent(QWheelEvent *event) override;
    /** Delete 键：删除选中的 ROI；Ctrl+0 适应窗口 */
    void keyPressEvent(QKeyEvent *event) override;
    /** 语言切换：QTranslator 安装后收到 LanguageChange */
    void changeEvent(QEvent *event) override;
    /**
     * 事件过滤：
     *   - graphicsView viewport：中键/左键空白处平移画布
     *   - widget_3：接收算法块拖放
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_pushButton_clicked();    // 打开单张图片（文件菜单）
    void on_pushButton_2_clicked();  // 打开文件夹（文件菜单）
    void on_pushButton_3_clicked();  // 按 ROI 类型添加选区
    void on_deltete_clicked();       // 删除 ROI
    void onExitApp();
    void onAboutApp();
    void onHelpShortcuts();
    void onLanguageChinese();
    void onLanguageEnglish();
    void onThemeLight();
    void onThemeDark();

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

    /** 导出当前处理链为 JSON */
    void on_btnExportChain_clicked();
    /** 从 JSON 导入处理链（替换现有块） */
    void on_btnImportChain_clicked();

    /** 点击文件夹缩略图 → 切换当前图片 */
    void on_folderImageList_itemClicked(QListWidgetItem *item);

    /** Ctrl+Z：撤销上一次结构/ROI 变更（弹出 m_undoStack 恢复） */
    void onUndo();

private:
    // ----- ROI 管理（可多个，并集参与算法） -----
    void clearAllRoi();
    void addRectItem(qreal x, qreal y, qreal w, qreal h);
    void addEllipseItem(qreal x, qreal y, qreal w, qreal h);
    void addRotatedRectItem(qreal x, qreal y, qreal w, qreal h);
    /** 按 RoiInfo 快照重建一个图元（会话恢复用） */
    void applyRoiFromInfo(const RoiInfo &info);
    bool isRoiItem(QGraphicsItem *item) const;
    void deselectAllRoiItems();

    // ----- 处理块管理 -----
    /** 根据拖入的名字（AppConfig::BLOCK_NAME_*）创建具体块；失败返回 nullptr */
    BaseBlock *createBlockByName(const QString &name);
    /** 把块挂到右侧面板 + 注册到 ImageProcessor */
    void addBlockToPanel(BaseBlock *block);
    /**
     * 按插入位置换序：同步 m_blockList / 布局 / processor，并触发重算
     * @param insertBefore 落点「插到该下标之前」（基于拖放瞬间的列表）
     */
    void reorderBlock(BaseBlock *block, int insertBefore);
    /** 根据落点 Y 坐标，算应插到第几个块之前 */
    int blockInsertIndexAtY(int yInContainer) const;
    /** 清空处理链（可选择是否随后重算；导入前用 silent） */
    void clearAllBlocks(bool reprocessAfter);
    /** 二值化块的 Otsu 按钮：用原图（或 ROI）算自动阈值 */
    void wireBinarizationOtsu(class BinarizationBlock *block);
    /** 复制块参数到系统剪贴板 */
    void copyBlockToClipboard(BaseBlock *block);
    /** 从剪贴板粘贴块；afterBlock 非空则插到其后，否则追加到末尾 */
    void pasteBlockFromClipboard(BaseBlock *afterBlock);
    /** 剪贴板是否含可粘贴的处理块 JSON */
    bool clipboardHasBlock() const;

    // ----- 显示 -----
    void updatePixmapItem(const QPixmap &pixmap);
    void refreshDisplay();
    void updateInfoLabel();
    /** Ctrl+0：重置缩放到适应窗口 */
    void fitViewToImage();
    /** 从当前场景里的全部 ROI 图元，打包成列表（图像像素坐标；空 = 全图） */
    QList<RoiInfo> getAllRoiInfo() const;
    QPointF imageCenterInScene() const;
    bool loadImageFromPath(const QString &filePath);

    /** 离开当前图前：把处理链 + ROI 写入 m_sessions */
    void saveCurrentSession();
    /** 恢复某张图的处理链与 ROI 图元（不自动 reprocess） */
    void restoreSession(const ImageSession &session);
    /** 把当前 UI 打成 ImageSession 快照（不写 hash） */
    ImageSession captureSessionSnapshot() const;
    /** 撤销栈：变更前压入 captureSessionSnapshot；Ctrl+Z 弹出并 restoreSession */
    void pushUndoSnapshot(const QString &reason);
    /** 会话落盘（app_sessions.json）/ 启动时 loadSessionsFromDisk */
    void saveSessionsToDisk() const;
    void loadSessionsFromDisk();
    QString sessionsFilePath() const;

    // ----- 文件夹浏览 -----
    void setupFolderBrowser();
    void clearFolderBrowser();
    void fillFolderBrowser(const QString &dirPath, const QFileInfoList &files);
    void selectFolderThumbnail(const QString &filePath);

    // ----- 初始化 -----
    void setupGraphicsView();
    void setupDragDrop();
    void setupBlockPanel();
    void setupShortcuts();
    /** 文件 / ROI / 设置 下拉菜单 */
    void setupMenus();
    /** 左侧算法列表：UserRole=稳定 id（不翻译），显示名由 tr/retranslateUi 负责 */
    void setupAlgoListIds();
    /**
     * 切换语言：安装/卸载英文 QTranslator（源文案为中文 tr("...")）
     * 真正刷界面在 changeEvent(LanguageChange) → retranslateDynamicUi
     */
    void applyLanguage();
    /** 应用浅色/深色 QSS，并写入 QSettings */
    void applyTheme();
    void retranslateDynamicUi();
    /** 按当前屏幕可用区域限制最小尺寸并缩放到可完整显示 */
    void adaptWindowToScreen();
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

    // 多 ROI：可同时存在多个矩形 / 椭圆 / 旋转矩形
    QList<ResizableRectItem *> m_rectItems;
    QList<ResizableEllipseItem *> m_ellipseItems;
    QList<ResizableRotatedRectItem *> m_rotatedRectItems;

    ImageProcessor *m_processor = nullptr; // 处理链引擎

    QVBoxLayout *m_blockLayout = nullptr;  // 右侧块列表布局
    QList<BaseBlock *> m_blockList;        // 与 processor 中块顺序一致

    /** 按绝对路径缓存每张图的处理链与 ROI（键 = m_currentImagePath） */
    QHash<QString, ImageSession> m_sessions;
    /** 当前图的撤销栈：每项为 captureSessionSnapshot 快照 */
    QList<ImageSession> m_undoStack;
    bool m_undoRestoring = false;  ///< 撤销恢复中，避免递归压栈
    static constexpr int MAX_UNDO = 40;  ///< 单图撤销深度上限

    QComboBox *m_roiTypeCombo = nullptr;   // ROI 菜单内的形状下拉
    QPushButton *m_roiAddBtn = nullptr;
    QPushButton *m_roiDeleteBtn = nullptr;
    QAction *m_actOpenImage = nullptr;
    QAction *m_actOpenFolder = nullptr;
    QAction *m_actUndo = nullptr;
    QAction *m_actExit = nullptr;
    QAction *m_actLangMenu = nullptr;      // “语言”子菜单标题 action
    QAction *m_actLangZh = nullptr;
    QAction *m_actLangEn = nullptr;
    QAction *m_actThemeMenu = nullptr;     // “外观”子菜单
    QAction *m_actThemeLight = nullptr;
    QAction *m_actThemeDark = nullptr;
    QAction *m_actAbout = nullptr;
    QAction *m_actHelp = nullptr;
    QTranslator m_translatorEn;            // 英文翻译（:/i18n/opencv_en.qm）
    bool m_englishUi = false;              // false=中文（无翻译器），true=装英文 qm
    bool m_translatorLoaded = false;
    QString m_themeId = QStringLiteral("light"); // light | dark
    /** ROI 拖动防抖：scene.changed → 启动 60ms 单次定时器 → onApplyProcessing */
    QTimer *m_roiUpdateTimer = nullptr;

    bool m_isDragInside = false;           // 拖拽是否在右侧面板上方
    bool m_showOriginal = false;           // true = 正在按住「对比」，显示原图
    bool m_suspendSceneReprocess = false;  // 更新 pixmap 时挂起，避免 changed 死循环
    bool m_panning = false;                // 是否正在拖动画布
    bool m_loadingFromFolderList = false;  // 避免缩略图选中与加载互相递归
    QList<RoiInfo> m_lastRoisForDebounce;  // 上次已响应的 ROI 列表；几何未变则忽略 scene.changed
    QPoint m_panLastPos;
    QString m_currentImagePath;            // 当前画布图片路径（用于高亮缩略图 / 会话键）
};

#endif // WIDGET_H
