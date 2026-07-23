#ifndef BASEBLOCK_H
#define BASEBLOCK_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QFrame>
#include <QPixmap>
#include <QPoint>
#include <QJsonObject>
#include <QList>
#include <QSet>
#include <QContextMenuEvent>
#include "../roi/roiinfo.h"

/**
 * @brief 处理块基类
 *
 * 所有图像处理块的父类，提供统一的：
 * - 标题栏（图标、名称、使能复选框、删除按钮）
 * - 内容容器（子类填充具体参数控件）
 * - 信号：参数变化、删除请求、使能变化
 * - 纯虚接口：process() 子类实现具体算法
 *
 * 【在整条链路中的位置】
 *   左侧拖入名字 → Widget::createBlockByName 创建子类
 *   → ImageProcessor::addBlock 监听 paramsChanged
 *   → reprocess() 时按顺序调用 process(current, rois)
 *
 * 【写一个新算法块的步骤】
 *   1. 子类构造中调用 setupTitle() 设置标题
 *   2. 子类将自定义参数控件加入 contentLayout()
 *   3. 子类重写 process() 实现图像处理逻辑（可参考 BinarizationBlock）
 *   4. 参数变化时 emit paramsChanged() 触发重算
 *   5. 在 Widget::createBlockByName 和 AppConfig 里登记名字
 */
class BaseBlock : public QWidget
{
    Q_OBJECT
public:
    explicit BaseBlock(QWidget *parent = nullptr);
    ~BaseBlock() override = default;

    /**
     * @brief 处理图像（纯虚函数，子类必须实现）
     * @param input 输入图像
     * @param rois 感兴趣区域列表（空 = 全图；多个为并集）
     * @return 处理后的图像
     */
    virtual QPixmap process(const QPixmap &input, const QList<RoiInfo> &rois = {}) = 0;

    /** @brief 处理块名称（用于日志、拖拽匹配等） */
    virtual QString blockName() const = 0;

    /**
     * @brief 导出本块参数（含 name / enabled，子类追加自己的字段）
     * 用于处理链 JSON 导出
     */
    virtual QJsonObject saveParams() const;

    /**
     * @brief 从 JSON 恢复参数（子类先读自己的字段，再调基类或自行处理 enabled）
     */
    virtual void loadParams(const QJsonObject &obj);

    /** @brief 是否启用 */
    bool isEnabled() const { return m_enableCheckBox->isChecked(); }
    void setEnabledBlock(bool enabled) { m_enableCheckBox->setChecked(enabled); }

    /** @brief 参数变化时 emit paramsChanged() 触发重算；语言切换时刷文案 */
    virtual void retranslateUi();

signals:
    /** @brief 用户即将改参（值仍旧）；Widget 据此 pushUndoSnapshot */
    void paramsAboutToChange();
    /** @brief 参数发生变化，需要重新处理图像 */
    void paramsChanged();

    /** @brief 用户点击删除按钮 */
    void removeRequested();

    /** @brief 右键菜单：复制本块参数到剪贴板（JSON） */
    void copyRequested();
    /** @brief 右键菜单：在本块之后粘贴剪贴板中的块 */
    void pasteRequested();

    /** @brief 使能状态变化 */
    void enabledChanged(bool enabled);

protected:
    /** 图标/标题拖拽 + 已 track 的参数控件 */
    bool eventFilter(QObject *watched, QEvent *event) override;
    /** 右键：复制 / 粘贴 / 删除 */
    void contextMenuEvent(QContextMenuEvent *event) override;

    /**
     * @brief 初始化标题栏（子类构造中调用）
     * @param icon 图标文字（emoji或字符）
     * @param title 处理块名称
     */
    void setupTitle(const QString &icon, const QString &title);

    /** @brief 获取内容布局，子类往里面加参数控件 */
    QVBoxLayout *contentLayout() { return m_contentLayout; }

    /** @brief 添加分隔线 */
    void addSeparator();

    /** 子类对每个会改参的控件调用；FocusIn/按下/滚轮时发 paramsAboutToChange */
    void trackParamWidget(QWidget *w);
    /** 程序改参前也可手动调用（如 Otsu）；同一次编辑只发一次 */
    void notifyParamsAboutToChange();

private:
    void initStyle();
    /** 从标题栏拖出：mime 带本块指针，Widget 负责插入新位置 */
    void startBlockDrag();

    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_titleLayout;
    QVBoxLayout *m_contentLayout;
    QCheckBox *m_enableCheckBox;
    QPushButton *m_deleteBtn;
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QPoint m_dragStartPos;  ///< 标题栏拖拽起点，超过 startDragDistance 才发起换序
    QSet<QWidget *> m_trackedParamWidgets;
    bool m_paramEditArmed = false;
};

#endif // BASEBLOCK_H
