#include "morphologyblock.h"
#include "../utils/imageconverter.h"
#include "../utils/roiprocess.h"

MorphologyBlock::MorphologyBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupTitle(QStringLiteral("🧩"), AppConfig::BLOCK_NAME_MORPHOLOGY);
    setupUI();
}

void MorphologyBlock::setupUI()
{
    addSeparator();

    m_opCombo = new QComboBox(this);
    m_opCombo->addItem(QStringLiteral("膨胀"), int(MorphologyAlgorithm::Op::Dilate));
    m_opCombo->addItem(QStringLiteral("腐蚀"), int(MorphologyAlgorithm::Op::Erode));
    m_opCombo->addItem(QStringLiteral("开运算"), int(MorphologyAlgorithm::Op::Open));
    m_opCombo->addItem(QStringLiteral("闭运算"), int(MorphologyAlgorithm::Op::Close));
    contentLayout()->addWidget(m_opCombo);

    auto addSpin = [&](const QString &label, int minV, int maxV, int def) {
        auto *row = new QHBoxLayout();
        auto *lb = new QLabel(label, this);
        lb->setObjectName(QStringLiteral("blockFieldLabel"));
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

    auto emitChange = [this](int) { emit paramsChanged(); };
    connect(m_opCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChange);
    connect(m_kxSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    connect(m_kySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    connect(m_iterSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
}

MorphologyAlgorithm::Op MorphologyBlock::currentOp() const
{
    return static_cast<MorphologyAlgorithm::Op>(m_opCombo->currentData().toInt());
}

QPixmap MorphologyBlock::process(const QPixmap &input, const RoiInfo &roi)
{
    if (input.isNull()) return input;
    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);
    cv::Mat out = RoiProcess::apply(src, roi, [&](const cv::Mat &m) {
        return MorphologyAlgorithm::apply(m, currentOp(),
                                          m_kxSpin->value(), m_kySpin->value(),
                                          m_iterSpin->value());
    });
    return ImageConverter::matToPixmap(out);
}
