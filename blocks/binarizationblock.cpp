/**
 * @file binarizationblock.cpp
 * @brief 二值化处理块 —— 【强烈建议作为“如何写一个算法块”的范例精读】
 *
 * =============================================================================
 * 一个 Block 在系统中的完整生命周期
 * =============================================================================
 *
 *   1. 用户从左侧拖“二值化处理”到右侧
 *   2. Widget::createBlockByName
 *        → new BinarizationBlock
 *        → wireBinarizationOtsu（给 Otsu 按钮接主窗口逻辑）
 *        → addBlockToPanel → ImageProcessor::addBlock
 *   3. 之后每次重算：
 *        ImageProcessor::reprocess
 *          → BinarizationBlock::process(inputPixmap, roi)
 *   4. 用户改上下限 / 点 Otsu / 开关启用
 *        → emit paramsChanged / otsuRequested
 *        → 最终再次 reprocess
 *   5. 用户点 ✕ → removeRequested → Widget 拆掉本块
 *
 * =============================================================================
 * process() 标准五步（其它块几乎一模一样）
 * =============================================================================
 *   ① 空图防护
 *   ② QPixmap → RGB Mat → BGR Mat   （ImageConverter）
 *   ③ 读 UI 参数（SpinBox 等）
 *   ④ RoiProcess::apply(src, roi, lambda) 里调用纯算法
 *   ⑤ BGR Mat → QPixmap 返回给下一环
 *
 * 算法与 UI 分离：
 *   - 本文件：控件 + 参数收集 + 格式转换
 *   - algorithms/binarization.cpp：真正的阈值计算（无 Qt 依赖）
 */

#include "binarizationblock.h"
#include "../utils/imageconverter.h"
#include "../utils/roiprocess.h"
#include "../algorithms/binarization.h"

#include <QJsonObject>
#include <QStyle>
#include <QVariant>
#include <algorithm>

/**
 * @brief 构造：先搭基类标题栏，再搭自己的参数区
 *
 * setupTitle 来自 BaseBlock：设置图标文字和标题。
 * setupUI 在本类：下限、上限、Otsu 按钮。
 */
BinarizationBlock::BinarizationBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupUI();
    retranslateUi();
}

/**
 * @brief 创建参数控件，并接到“一变就 paramsChanged”的槽上
 *
 * 布局全部塞进 BaseBlock::contentLayout()，这样标题栏样式统一。
 *
 * 信号约定：
 *   - 下限变化 → onLowerChanged / onUpperChanged → emit paramsChanged
 *   - Otsu 点击 → onAutoThresholdClicked → emit otsuRequested
 *     （真正算阈值的人是 Widget，见 wireBinarizationOtsu）
 */
void BinarizationBlock::setupUI()
{
    addSeparator();  // 标题栏和参数区之间的细线

    // ========== 下限阈值行 ==========
    auto *lowerLayout = new QHBoxLayout();
    m_lowerLabel = new QLabel(this);
    m_lowerLabel->setObjectName(QStringLiteral("blockFieldLabel"));  // 供 QSS 美化
    m_lowerLabel->setFixedWidth(AppConfig::BLOCK_FIELD_LABEL_WIDTH);

    m_lowerSpin = new QSpinBox(this);
    m_lowerSpin->setRange(0, 255);  // 灰度范围
    m_lowerSpin->setValue(AppConfig::DEFAULT_BINARY_LOWER);  // 默认 127
    m_lowerSpin->setFixedWidth(AppConfig::BLOCK_SPIN_WIDTH);

    lowerLayout->addWidget(m_lowerLabel);
    lowerLayout->addWidget(m_lowerSpin);
    lowerLayout->addStretch();  // 右侧留白，控件靠左
    contentLayout()->addLayout(lowerLayout);

    // ========== 上限阈值行 ==========
    auto *upperLayout = new QHBoxLayout();
    m_upperLabel = new QLabel(this);
    m_upperLabel->setObjectName(QStringLiteral("blockFieldLabel"));
    m_upperLabel->setFixedWidth(AppConfig::BLOCK_FIELD_LABEL_WIDTH);

    m_upperSpin = new QSpinBox(this);
    m_upperSpin->setRange(0, 255);
    m_upperSpin->setValue(AppConfig::DEFAULT_BINARY_UPPER);  // 默认 255
    m_upperSpin->setFixedWidth(AppConfig::BLOCK_SPIN_WIDTH);

    upperLayout->addWidget(m_upperLabel);
    upperLayout->addWidget(m_upperSpin);
    upperLayout->addStretch();
    contentLayout()->addLayout(upperLayout);

    // ========== Otsu 自动阈值按钮 ==========
    // role=accent 让 app.qss 把它画成强调色按钮
    m_autoBtn = new QPushButton(QStringLiteral("Otsu"), this);
    m_autoBtn->setProperty("role", QVariant(QStringLiteral("accent")));
    m_autoBtn->setCursor(Qt::PointingHandCursor);
    contentLayout()->addWidget(m_autoBtn);
    // 动态属性改完后强制刷新样式，否则 accent 可能不生效
    if (m_autoBtn->style()) {
        m_autoBtn->style()->unpolish(m_autoBtn);
        m_autoBtn->style()->polish(m_autoBtn);
    }

    // SpinBox::valueChanged 有重载，必须用 QOverload<int> 指明是 int 版本
    connect(m_lowerSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BinarizationBlock::onLowerChanged);
    connect(m_upperSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BinarizationBlock::onUpperChanged);
    connect(m_autoBtn, &QPushButton::clicked,
            this, &BinarizationBlock::onAutoThresholdClicked);
    trackParamWidget(m_lowerSpin);
    trackParamWidget(m_upperSpin);
    trackParamWidget(m_autoBtn);
}

void BinarizationBlock::retranslateUi()
{
    setupTitle(QStringLiteral("🔲"), tr("二值化处理"));
    BaseBlock::retranslateUi();
    if (m_lowerLabel)
        m_lowerLabel->setText(tr("下限值"));
    if (m_upperLabel)
        m_upperLabel->setText(tr("上限值"));
    if (m_autoBtn)
        m_autoBtn->setToolTip(tr("自动阈值 (Otsu)"));
}

/**
 * @brief 【核心】对输入图做范围二值化，支持 ROI
 *
 * @param input 上一块的输出（第一块则是原图），QPixmap
 * @param roi   当前感兴趣区域；None 表示全图
 * @return      处理后的 QPixmap，交给下一块或作为最终结果
 *
 * 范围二值化含义：
 *   灰度 ∈ [lower, upper] → 白色(255)
 *   否则                 → 黑色(0)
 *
 * ROI 行为（由 RoiProcess::apply 保证）：
 *   - 有 ROI：只把 ROI 内的二值化结果贴回，ROI 外保持 input 原样
 *   - 无 ROI：整图都是二值化结果
 */
QPixmap BinarizationBlock::process(const QPixmap &input, const QList<RoiInfo> &rois)
{
    // ① 空图直接返回，避免后面 OpenCV 断言崩溃
    if (input.isNull()) return input;

    // ② Qt → OpenCV
    // pixmapToMatRGB 得到 RGB 三通道；项目约定算法层统一用 BGR，所以再转一次
    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);

    // ③ 读当前 UI 参数（每次 process 都读最新值，所以调参后重算能立刻生效）
    const int lower = m_lowerSpin->value();
    const int upper = m_upperSpin->value();

    // ④ 带 ROI 的处理
    // RoiProcess::apply 会：
    //   - 先对整图调用我们的 lambda 得到 processed
    //   - 若有 ROI，用 mask 把 processed 贴回原图副本（ROI 外不变）
    cv::Mat result = RoiProcess::apply(src, rois, [&](const cv::Mat &m) {
        cv::Mat gray, binary, bgr;

        // 彩色 → 灰度（二值化基于单通道强度）
        cv::cvtColor(m, gray, cv::COLOR_BGR2GRAY);

        // 纯算法：algorithms/binarization.cpp
        binary = BinarizationAlgorithm::applyRangeThreshold(gray, lower, upper);

        // 灰度结果再变回 BGR 三通道：
        // 后续块（形态学、滤波等）都按三通道 BGR 来写，通道数保持一致很重要
        cv::cvtColor(binary, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    });

    // ⑤ OpenCV → Qt；matToPixmap 约定输入是 BGR，内部会转 RGB 再进 QImage
    return ImageConverter::matToPixmap(result);
}

QJsonObject BinarizationBlock::saveParams() const
{
    QJsonObject obj = BaseBlock::saveParams();
    obj.insert(QStringLiteral("lower"), m_lowerSpin->value());
    obj.insert(QStringLiteral("upper"), m_upperSpin->value());
    return obj;
}

void BinarizationBlock::loadParams(const QJsonObject &obj)
{
    BaseBlock::loadParams(obj);
    const int lower = obj.value(QStringLiteral("lower")).toInt(m_lowerSpin->value());
    const int upper = obj.value(QStringLiteral("upper")).toInt(m_upperSpin->value());
    m_lowerSpin->blockSignals(true);
    m_upperSpin->blockSignals(true);
    m_lowerSpin->setValue(lower);
    m_upperSpin->setValue(upper);
    m_lowerSpin->blockSignals(false);
    m_upperSpin->blockSignals(false);
}

/**
 * @brief 外部设置上下限（目前由 Widget 的 Otsu 回调使用）
 *
 * 细节：
 *   - qBound 限制在 0~255
 *   - 若 lower>upper 则交换，保证区间合法
 *   - blockSignals(true) 期间改值，避免 setValue 触发 valueChanged
 *     从而避免“改两个框触发两次重算”；最后统一 emit paramsChanged() 一次
 */
void BinarizationBlock::setThresholds(int lower, int upper)
{
    lower = qBound(0, lower, 255);
    upper = qBound(0, upper, 255);
    if (lower > upper)
        std::swap(lower, upper);

    m_lowerSpin->blockSignals(true);
    m_upperSpin->blockSignals(true);
    m_lowerSpin->setValue(lower);
    m_upperSpin->setValue(upper);
    m_lowerSpin->blockSignals(false);
    m_upperSpin->blockSignals(false);

    // 通知 ImageProcessor：参数变了，请请求 Widget 重算
    emit paramsChanged();
}

/**
 * @brief Otsu 按钮点击：自己不算，把请求抛给主窗口
 *
 * 主窗口有原图和 ROI，算完再回调 setThresholds。
 * 这种“UI 块发请求、主窗口供数据”的拆分，避免 Block 依赖 Widget 头文件。
 */
void BinarizationBlock::onAutoThresholdClicked()
{
    notifyParamsAboutToChange();
    emit otsuRequested();
}

/**
 * @brief 下限变了：维持 lower ≤ upper，然后通知重算
 *
 * 若用户把下限拧到比上限还大，就自动把上限跟着抬上去。
 * 改上限时 blockSignals，避免上限的 valueChanged 再进来造成递归。
 */
void BinarizationBlock::onLowerChanged(int value)
{
    if (value > m_upperSpin->value()) {
        m_upperSpin->blockSignals(true);
        m_upperSpin->setValue(value);
        m_upperSpin->blockSignals(false);
    }
    emit paramsChanged();
}

/**
 * @brief 上限变了：维持 lower ≤ upper，然后通知重算
 */
void BinarizationBlock::onUpperChanged(int value)
{
    if (value < m_lowerSpin->value()) {
        m_lowerSpin->blockSignals(true);
        m_lowerSpin->setValue(value);
        m_lowerSpin->blockSignals(false);
    }
    emit paramsChanged();
}
