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
#include <QRectF>

/**
 * @brief 处理块基类
 *
 * 所有图像处理块的父类，提供统一的：
 * - 标题栏（图标、名称、使能复选框、删除按钮）
 * - 内容容器（子类填充具体参数控件）
 * - 信号：参数变化、删除请求、使能变化
 * - 纯虚接口：process() 子类实现具体算法
 *
 * 使用方式：
 * 1. 子类构造中调用 setupTitle() 设置标题
 * 2. 子类将自定义参数控件加入 contentLayout()
 * 3. 子类重写 process() 实现图像处理逻辑
 * 4. 参数变化时 emit paramsChanged() 触发重算
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
     * @param roi 可选的感兴趣区域（空矩形表示全图处理）
     * @return 处理后的图像
     */
    virtual QPixmap process(const QPixmap &input, const QRectF &roi = QRectF()) = 0;

    /** @brief 处理块名称（用于日志、拖拽匹配等） */
    virtual QString blockName() const = 0;

    /** @brief 是否启用 */
    bool isEnabled() const { return m_enableCheckBox->isChecked(); }
    void setEnabledBlock(bool enabled) { m_enableCheckBox->setChecked(enabled); }

signals:
    /** @brief 参数发生变化，需要重新处理图像 */
    void paramsChanged();

    /** @brief 用户点击删除按钮 */
    void removeRequested();

    /** @brief 使能状态变化 */
    void enabledChanged(bool enabled);

protected:
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

private:
    void initStyle();

    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_titleLayout;
    QVBoxLayout *m_contentLayout;
    QCheckBox *m_enableCheckBox;
    QPushButton *m_deleteBtn;
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
};

#endif // BASEBLOCK_H
