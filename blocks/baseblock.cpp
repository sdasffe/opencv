/**
 * @file baseblock.cpp
 * @brief 处理块基类实现
 *
 * 每个算法块（二值化/滤波/...）都是一个小面板：
 *   ┌─────────────────────────────┐
 *   │ 图标  标题        [开] [✕]  │  ← 标题栏（本类创建）
 *   ├─────────────────────────────┤
 *   │  子类自己的参数控件...       │  ← contentLayout()
 *   └─────────────────────────────┘
 *
 * 子类只要：
 *   1. 构造里 setupTitle + 往 contentLayout 加控件
 *   2. 实现 process() / blockName()
 *   3. 参数变了 emit paramsChanged()
 *
 * 标题栏图标/名称可拖动：发出 MIME_BLOCK_REORDER，由 Widget 换序。
 */

#include "baseblock.h"
#include "../config/appconfig.h"

#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QDataStream>
#include <QDrag>
#include <QIODevice>
#include <QJsonObject>
#include <QLineEdit>
#include <QAbstractSpinBox>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QSizePolicy>

BaseBlock::BaseBlock(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("BaseBlock"));  // 供 app.qss 匹配样式

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 6, 8, 6);
    m_mainLayout->setSpacing(4);

    // ----- 标题栏：图标 | 名称 | 弹性空白 | 使能 | 删除 -----
    m_titleLayout = new QHBoxLayout();
    m_titleLayout->setSpacing(4);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName(QStringLiteral("blockIconLabel"));
    m_iconLabel->setCursor(Qt::OpenHandCursor);
    m_iconLabel->setToolTip(tr("拖动可调整处理顺序"));
    m_iconLabel->installEventFilter(this);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("blockTitleLabel"));
    m_titleLabel->setCursor(Qt::OpenHandCursor);
    m_titleLabel->setToolTip(tr("拖动可调整处理顺序"));
    m_titleLabel->installEventFilter(this);

    m_enableCheckBox = new QCheckBox(this);
    m_enableCheckBox->setChecked(true);  // 默认启用，参与处理链
    m_enableCheckBox->setText(tr("开"));
    m_enableCheckBox->setToolTip(tr("启用此处理块"));

    m_deleteBtn = new QPushButton(QStringLiteral("✕"), this);
    m_deleteBtn->setObjectName(QStringLiteral("blockDeleteBtn"));
    m_deleteBtn->setFixedSize(18, 18);
    m_deleteBtn->setCursor(Qt::PointingHandCursor);
    m_deleteBtn->setToolTip(tr("删除此处理块"));

    m_titleLayout->addWidget(m_iconLabel);
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_enableCheckBox);
    m_titleLayout->addWidget(m_deleteBtn);

    m_mainLayout->addLayout(m_titleLayout);

    // 子类通过 contentLayout() 往这里塞 SpinBox、ComboBox 等
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setSpacing(3);
    m_mainLayout->addLayout(m_contentLayout);

    // ✕ → Widget 从面板和 processor 里移除本块
    connect(m_deleteBtn, &QPushButton::clicked, this, &BaseBlock::removeRequested);
    // 开关 → ImageProcessor 决定是否跳过本块
    connect(m_enableCheckBox, &QCheckBox::toggled, this, &BaseBlock::enabledChanged);
    // 开关也算一种“参数变化”，直接触发重算
    connect(m_enableCheckBox, &QCheckBox::toggled, this, &BaseBlock::paramsChanged);
    trackParamWidget(m_enableCheckBox);                              // 勾选前压撤销（值仍旧）

    setContextMenuPolicy(Qt::DefaultContextMenu);
    setToolTip(tr("右键可复制、粘贴或删除"));
    initStyle();
}

void BaseBlock::setupTitle(const QString &icon, const QString &title)
{
    m_iconLabel->setText(icon);
    m_titleLabel->setText(title);
    m_enableCheckBox->setToolTip(tr("启用%1").arg(title));
}

void BaseBlock::retranslateUi()
{
    m_iconLabel->setToolTip(tr("拖动可调整处理顺序"));
    m_titleLabel->setToolTip(tr("拖动可调整处理顺序"));
    m_enableCheckBox->setText(tr("开"));
    m_enableCheckBox->setToolTip(tr("启用%1").arg(m_titleLabel->text()));
    m_deleteBtn->setToolTip(tr("删除此处理块"));
    setToolTip(tr("右键可复制、粘贴或删除"));
}

void BaseBlock::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QAction *copyAct = menu.addAction(tr("复制"));
    QAction *pasteAct = menu.addAction(tr("粘贴"));
    menu.addSeparator();
    QAction *delAct = menu.addAction(tr("删除"));

    // 剪贴板须含 MIME_BLOCK_CLIPBOARD 或纯文本 JSON 才允许粘贴
    const QMimeData *clip = QApplication::clipboard()->mimeData();
    const bool canPaste = clip
        && (clip->hasFormat(QLatin1String(AppConfig::MIME_BLOCK_CLIPBOARD))
            || clip->hasText());
    pasteAct->setEnabled(canPaste);

    QAction *chosen = menu.exec(event->globalPos());
    if (chosen == copyAct)
        emit copyRequested();
    else if (chosen == pasteAct)
        emit pasteRequested();
    else if (chosen == delAct)
        emit removeRequested();
}

/** @brief 导出 name、enabled 到 JSON（子类 append 自己的字段） */
QJsonObject BaseBlock::saveParams() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("name"), blockName());
    obj.insert(QStringLiteral("enabled"), isEnabled());
    return obj;
}

/** @brief 从 JSON 恢复 enabled；子类 loadParams 应先读自己的字段再调基类 */
void BaseBlock::loadParams(const QJsonObject &obj)
{
    if (obj.contains(QStringLiteral("enabled")))
        setEnabledBlock(obj.value(QStringLiteral("enabled")).toBool(true));
}

/**
 * @brief 登记会改参的控件，开始编辑时通过 eventFilter 发 paramsAboutToChange
 *
 * SpinBox 焦点常在内部 QLineEdit，故对 SpinBox 再 findChild 登记一行编辑框。
 */
void BaseBlock::trackParamWidget(QWidget *w)
{
    if (!w)
        return;                                                      // 空指针防护
    w->installEventFilter(this);                                     // 由本类 eventFilter 收开始编辑事件
    m_trackedParamWidgets.insert(w);                                 // 加入已登记集合
    if (qobject_cast<QAbstractSpinBox *>(w)) {                       // Spin/DoubleSpin
        if (QLineEdit *le = w->findChild<QLineEdit *>()) {           // lineEdit() 为 protected，用 findChild
            le->installEventFilter(this);                            // 内部编辑框获焦也能压栈
            m_trackedParamWidgets.insert(le);
        }
    }
}

/**
 * @brief 发出 paramsAboutToChange；同一次编辑只发一次，防止拖动刷栈
 */
void BaseBlock::notifyParamsAboutToChange()
{
    if (m_paramEditArmed)
        return;                                                      // 本次编辑已通知过
    m_paramEditArmed = true;                                         // 标记：FocusOut 前不再发
    emit paramsAboutToChange();                                      // Widget 据此 pushUndoSnapshot
}

/**
 * @brief 事件过滤：已 track 控件开始编辑 → 即将改参；标题栏拖拽换序
 */
bool BaseBlock::eventFilter(QObject *watched, QEvent *event)
{
    auto *w = qobject_cast<QWidget *>(watched);
    if (w && m_trackedParamWidgets.contains(w)) {                    // 参数控件路径
        switch (event->type()) {
        case QEvent::FocusIn:
        case QEvent::MouseButtonPress:
        case QEvent::Wheel:
            notifyParamsAboutToChange();                             // 值尚未变时通知压栈
            break;
        case QEvent::FocusOut:
            m_paramEditArmed = false;                                // 下次再改可再压一层
            break;
        default:
            break;
        }
        return QWidget::eventFilter(watched, event);                 // 不吞事件，控件照常改值
    }

    if (watched != m_iconLabel && watched != m_titleLabel)
        return QWidget::eventFilter(watched, event);                 // 非标题栏：交给基类

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto *e = static_cast<QMouseEvent *>(event);
        if (e->button() == Qt::LeftButton)
            m_dragStartPos = e->pos();                               // 记录拖拽起点
        break;
    }
    case QEvent::MouseMove: {
        auto *e = static_cast<QMouseEvent *>(event);
        if (!(e->buttons() & Qt::LeftButton))
            break;
        if ((e->pos() - m_dragStartPos).manhattanLength()
            < QApplication::startDragDistance())
            break;                                                   // 未超过系统拖拽阈值
        startBlockDrag();                                            // 发起块换序 DnD
        return true;
    }
    default:
        break;
    }
    return QWidget::eventFilter(watched, event);
}

void BaseBlock::startBlockDrag()
{
    auto *mime = new QMimeData;
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << quintptr(this);
    mime->setData(QLatin1String(AppConfig::MIME_BLOCK_REORDER), bytes);

    auto *drag = new QDrag(this);
    drag->setMimeData(mime);
    // 缩略预览，方便对准插入位置
    const QPixmap preview = grab().scaled(160, 48, Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);
    drag->setPixmap(preview);
    drag->setHotSpot(QPoint(preview.width() / 2, 8));
    drag->exec(Qt::MoveAction);
}

void BaseBlock::addSeparator()
{
    QFrame *line = new QFrame(this);
    line->setObjectName(QStringLiteral("blockSeparator"));
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    m_contentLayout->addWidget(line);
}

void BaseBlock::initStyle()
{
    // 样式由全局 theme_*.qss 的 #BaseBlock 规则统一控制
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);
    // Minimum：高度不低于内容 sizeHint，多块时由外层 QScrollArea 滚动，不被挤扁
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
}
