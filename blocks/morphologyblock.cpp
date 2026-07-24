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

#include <QJsonObject>

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
    setupUI();
    retranslateUi();
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
    addSeparator();                                                  // 标题栏与参数区细线

    m_opCombo = new QComboBox(this);
    for (int op : {int(MorphologyAlgorithm::Op::Dilate), int(MorphologyAlgorithm::Op::Erode),
                   int(MorphologyAlgorithm::Op::Open), int(MorphologyAlgorithm::Op::Close),
                   int(MorphologyAlgorithm::Op::TopHat), int(MorphologyAlgorithm::Op::ButtonHat),
                   int(MorphologyAlgorithm::Op::MorphologicalGradient)}) {
        m_opCombo->addItem(QString(), op);                           // userData 存 Op 枚举
    }
    contentLayout()->addWidget(m_opCombo);

    auto addSpin = [&](QLabel *&labelOut, int minV, int maxV, int def) {
        auto *row = new QHBoxLayout();
        labelOut = new QLabel(this);
        labelOut->setObjectName(QStringLiteral("blockFieldLabel"));
        labelOut->setFixedWidth(AppConfig::BLOCK_FIELD_LABEL_WIDTH);
        auto *sp = new QSpinBox(this);
        sp->setRange(minV, maxV);
        sp->setValue(def);
        sp->setFixedWidth(AppConfig::BLOCK_SPIN_WIDTH);
        sp->setSingleStep(2);                                        // 结构元尺寸保持奇数步进
        row->addWidget(labelOut);
        row->addWidget(sp);
        row->addStretch();
        contentLayout()->addLayout(row);
        return sp;
    };

    m_kxSpin = addSpin(m_kxLabel, 1, 31, 3);                         // 结构元宽
    m_kySpin = addSpin(m_kyLabel, 1, 31, 3);                         // 结构元高
    m_iterSpin = addSpin(m_iterLabel, 1, 20, 1);                     // 迭代次数

    auto emitChange = [this](int) { emit paramsChanged(); };
    connect(m_opCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChange);
    connect(m_kxSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    connect(m_kySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    connect(m_iterSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    trackParamWidget(m_opCombo);                                     // 换算子前压撤销
    trackParamWidget(m_kxSpin);                                      // 改结构元宽前压撤销
    trackParamWidget(m_kySpin);                                      // 改结构元高前压撤销
    trackParamWidget(m_iterSpin);                                    // 改次数前压撤销
}

void MorphologyBlock::retranslateUi()
{
    setupTitle(QStringLiteral("🧩"), tr("形态学处理"));
    BaseBlock::retranslateUi();
    if (!m_opCombo || m_opCombo->count() < 7)
        return;
    m_opCombo->setItemText(0, tr("膨胀"));
    m_opCombo->setItemText(1, tr("腐蚀"));
    m_opCombo->setItemText(2, tr("开运算"));
    m_opCombo->setItemText(3, tr("闭运算"));
    m_opCombo->setItemText(4, tr("顶帽"));
    m_opCombo->setItemText(5, tr("底帽"));
    m_opCombo->setItemText(6, tr("形态学梯度"));
    if (m_kxLabel)
        m_kxLabel->setText(tr("核 X"));
    if (m_kyLabel)
        m_kyLabel->setText(tr("核 Y"));
    if (m_iterLabel)
        m_iterLabel->setText(tr("次数"));
}

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
QPixmap MorphologyBlock::process(const QPixmap &input, const QList<RoiInfo> &rois)
{
    if (input.isNull()) return input;

    // Qt 侧 RGB → OpenCV 侧 BGR（与算法层约定一致）
    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);

    // lambda 接收整图 BGR，返回整图处理结果；RoiProcess 负责按 mask 贴回
    cv::Mat out = RoiProcess::apply(src, rois, [&](const cv::Mat &m) {
        return MorphologyAlgorithm::apply(m, currentOp(),
                                          m_kxSpin->value(), m_kySpin->value(),
                                          m_iterSpin->value());
    });
    return ImageConverter::matToPixmap(out);
}

/** @brief 导出运算类型、核尺寸、迭代次数到 JSON */
QJsonObject MorphologyBlock::saveParams() const
{
    QJsonObject obj = BaseBlock::saveParams();
    obj.insert(QStringLiteral("op"), m_opCombo->currentData().toInt());
    obj.insert(QStringLiteral("kx"), m_kxSpin->value());
    obj.insert(QStringLiteral("ky"), m_kySpin->value());
    obj.insert(QStringLiteral("iterations"), m_iterSpin->value());
    return obj;
}

/** @brief 从 JSON 恢复参数；blockSignals 避免恢复过程中触发重算 */
void MorphologyBlock::loadParams(const QJsonObject &obj)
{
    BaseBlock::loadParams(obj);
    const int op = obj.value(QStringLiteral("op")).toInt(m_opCombo->currentData().toInt());
    const int idx = m_opCombo->findData(op);
    m_opCombo->blockSignals(true);
    m_kxSpin->blockSignals(true);
    m_kySpin->blockSignals(true);
    m_iterSpin->blockSignals(true);
    if (idx >= 0)
        m_opCombo->setCurrentIndex(idx);
    m_kxSpin->setValue(obj.value(QStringLiteral("kx")).toInt(m_kxSpin->value()));
    m_kySpin->setValue(obj.value(QStringLiteral("ky")).toInt(m_kySpin->value()));
    m_iterSpin->setValue(obj.value(QStringLiteral("iterations")).toInt(m_iterSpin->value()));
    m_opCombo->blockSignals(false);
    m_kxSpin->blockSignals(false);
    m_kySpin->blockSignals(false);
    m_iterSpin->blockSignals(false);
}
