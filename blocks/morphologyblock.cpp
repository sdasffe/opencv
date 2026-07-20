/**
 * @file morphologyblock.cpp
 * @brief 形态学处理块 —— UI 层封装，将用户参数传给 MorphologyAlgorithm
 *
 * 【在整条链路中的位置】
 *   Widget 左侧拖入「形态学」→ createBlockByName 构造本类
 *   → ImageProcessor::addBlock 监听 paramsChanged
 *   → reprocess() 按流水线顺序调用 process(current, roi)
 *
 * 【数据流】
 *   QPixmap(上一块输出) → cv::Mat(BGR) → RoiProcess::apply → MorphologyAlgorithm::apply
 *   → cv::Mat → QPixmap(显示/传给下一块)
 *
 * 【与算法层的关系】
 *   本文件只负责：参数控件 + 格式转换 + ROI 包装
 *   具体膨胀/腐蚀/开闭运算在 algorithms/morphology.cpp
 */

#include "morphologyblock.h"
#include "../utils/imageconverter.h"
#include "../utils/roiprocess.h"

/**
 * @brief 构造函数：初始化标题栏与参数 UI
 *
 * 谁调用：Widget::createBlockByName 在拖入「形态学」块时
 * 何时调用：块被创建时一次
 * 输出：一个可交互的形态学参数面板
 */
MorphologyBlock::MorphologyBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupTitle(QStringLiteral("🧩"), AppConfig::BLOCK_NAME_MORPHOLOGY);
    setupUI();
}

/**
 * @brief 构建形态学参数控件并连接信号
 *
 * 控件说明：
 *   - m_opCombo：运算类型（膨胀/腐蚀/开/闭），userData 存 MorphologyAlgorithm::Op 枚举值
 *   - m_kxSpin / m_kySpin：结构元素宽高（算法层会强制为奇数）
 *   - m_iterSpin：迭代次数（多次重复同一运算，效果更强）
 *
 * 任何参数变化 → emit paramsChanged() → ImageProcessor 触发整链重算
 */
void MorphologyBlock::setupUI()
{
    addSeparator();

    // ----- 运算类型下拉框 -----
    m_opCombo = new QComboBox(this);
    // addItem(显示文字, userData)；currentData() 可还原为 Op 枚举
    m_opCombo->addItem(QStringLiteral("膨胀"), int(MorphologyAlgorithm::Op::Dilate));
    m_opCombo->addItem(QStringLiteral("腐蚀"), int(MorphologyAlgorithm::Op::Erode));
    m_opCombo->addItem(QStringLiteral("开运算"), int(MorphologyAlgorithm::Op::Open));
    m_opCombo->addItem(QStringLiteral("闭运算"), int(MorphologyAlgorithm::Op::Close));
    contentLayout()->addWidget(m_opCombo);

    // Lambda 工厂：统一创建「标签 + SpinBox」行，减少重复代码
    auto addSpin = [&](const QString &label, int minV, int maxV, int def) {
        auto *row = new QHBoxLayout();
        auto *lb = new QLabel(label, this);
        lb->setObjectName(QStringLiteral("blockFieldLabel")); // QSS 统一字段标签样式
        lb->setFixedWidth(40);
        auto *sp = new QSpinBox(this);
        sp->setRange(minV, maxV);
        sp->setValue(def);
        sp->setFixedWidth(64);
        row->addWidget(lb);
        row->addWidget(sp);
        row->addStretch();
        contentLayout()->addLayout(row);
        return sp;
    };

    m_kxSpin = addSpin(QStringLiteral("核 X"), 1, 31, 3);
    m_kySpin = addSpin(QStringLiteral("核 Y"), 1, 31, 3);
    m_iterSpin = addSpin(QStringLiteral("次数"), 1, 20, 1);

    // 参数变化时通知 ImageProcessor 重算（忽略具体数值，只关心「变了」）
    auto emitChange = [this](int) { emit paramsChanged(); };
    connect(m_opCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChange);
    connect(m_kxSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    connect(m_kySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    connect(m_iterSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
}

/**
 * @brief 读取当前选中的形态学运算类型
 *
 * 谁调用：process() 内部
 * 输入：m_opCombo 的 currentData（构造时写入的 int 枚举值）
 * 输出：MorphologyAlgorithm::Op
 */
MorphologyAlgorithm::Op MorphologyBlock::currentOp() const
{
    return static_cast<MorphologyAlgorithm::Op>(m_opCombo->currentData().toInt());
}

/**
 * @brief 对输入图像执行形态学处理（BaseBlock 纯虚函数实现）
 *
 * 谁调用：ImageProcessor::reprocess() 遍历流水线时
 * 何时调用：加载图片、改参数、改 ROI、增删块等触发重算时
 *
 * @param input 上一处理块的 QPixmap 输出（或原图）
 * @param roi   当前 ROI；isEmpty() 为 true 时处理全图
 * @return 处理后的 QPixmap，供下一块或预览区使用
 *
 * 数据流步骤：
 *   1. QPixmap → cv::Mat(RGB) → BGR（OpenCV 默认通道序）
 *   2. RoiProcess::apply 在 ROI 内跑算法，ROI 外保持原像素
 *   3. cv::Mat → QPixmap 返回 Qt 侧
 */
QPixmap MorphologyBlock::process(const QPixmap &input, const RoiInfo &roi)
{
    if (input.isNull()) return input;

    // Qt 侧 RGB → OpenCV 侧 BGR（与算法层约定一致）
    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);

    // lambda 接收整图 BGR，返回整图处理结果；RoiProcess 负责按 mask 贴回
    cv::Mat out = RoiProcess::apply(src, roi, [&](const cv::Mat &m) {
        return MorphologyAlgorithm::apply(m, currentOp(),
                                          m_kxSpin->value(), m_kySpin->value(),
                                          m_iterSpin->value());
    });
    return ImageConverter::matToPixmap(out);
}
