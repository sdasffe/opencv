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
    void paramsAboutToChange();                                    // 即将改参（值仍旧）；Widget 据此压撤销栈
    void paramsChanged();                                          // 参数已变，请求引擎重算图像
    void removeRequested();                                        // 用户点 ✕，请求 Widget 移除本块
    void copyRequested();                                          // 右键：复制本块参数 JSON
    void pasteRequested();                                         // 右键：在本块之后粘贴
    void enabledChanged(bool enabled);                             // 使能勾选变化

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;    // 标题栏拖拽 + 已 track 参数控件
    void contextMenuEvent(QContextMenuEvent *event) override;      // 右键复制/粘贴/删除

    void setupTitle(const QString &icon, const QString &title);     // 子类构造中设标题栏图标与名称
    QVBoxLayout *contentLayout() { return m_contentLayout; }       // 子类往此布局加参数控件
    void addSeparator();                                           // 标题下加水平分隔线

    void trackParamWidget(QWidget *w);                             // 登记改参控件；开始编辑发 aboutToChange
    void notifyParamsAboutToChange();                              // 即将改参；同一次编辑只发一次防刷栈

private:
    void initStyle();                                              // WA_StyledBackground + sizePolicy
    void startBlockDrag();                                         // 标题栏 DnD 换序

    QVBoxLayout *m_mainLayout;                                     // 块面板主布局
    QHBoxLayout *m_titleLayout;                                    // 标题栏：图标|名称|开|✕
    QVBoxLayout *m_contentLayout;                                  // 参数区容器
    QCheckBox *m_enableCheckBox;                                   // 是否参与处理链
    QPushButton *m_deleteBtn;                                      // 删除本块
    QLabel *m_iconLabel;                                           // 可拖拽换序
    QLabel *m_titleLabel;                                          // 可拖拽换序
    QPoint m_dragStartPos;                                         // 标题栏拖拽起点
    QSet<QWidget *> m_trackedParamWidgets;                         // trackParamWidget 登记的控件
    bool m_paramEditArmed = false;                                 // 本次编辑已发过 aboutToChange
};

#endif // BASEBLOCK_H
