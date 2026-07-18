#include "graytransformblock.h"
#include "../utils/imageconverter.h"
#include "../utils/roiprocess.h"

GrayTransformBlock::GrayTransformBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupTitle(QStringLiteral("🎚"), AppConfig::BLOCK_NAME_GRAYTRANSFORM);
    setupUI();
}

void GrayTransformBlock::setupUI()
{
    addSeparator();

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(QStringLiteral("转灰度"), int(GrayTransformAlgorithm::Type::ToGray));
    m_typeCombo->addItem(QStringLiteral("亮度/对比度"), int(GrayTransformAlgorithm::Type::BrightContrast));
    m_typeCombo->addItem(QStringLiteral("反转"), int(GrayTransformAlgorithm::Type::Invert));
    m_typeCombo->addItem(QStringLiteral("对数变换"), int(GrayTransformAlgorithm::Type::Log));
    m_typeCombo->addItem(QStringLiteral("伽马变换"), int(GrayTransformAlgorithm::Type::Gamma));
    m_typeCombo->addItem(QStringLiteral("直方图均衡"), int(GrayTransformAlgorithm::Type::Equalize));
    m_typeCombo->addItem(QStringLiteral("归一化"), int(GrayTransformAlgorithm::Type::Normalize));
    contentLayout()->addWidget(m_typeCombo);

    auto *bRow = new QHBoxLayout();
    auto *bLb = new QLabel(QStringLiteral("亮度"), this);
    bLb->setObjectName(QStringLiteral("blockFieldLabel"));
    bLb->setFixedWidth(40);
    m_brightSpin = new QSpinBox(this);
    m_brightSpin->setRange(-100, 100);
    m_brightSpin->setValue(0);
    m_brightSpin->setFixedWidth(64);
    bRow->addWidget(bLb);
    bRow->addWidget(m_brightSpin);
    bRow->addStretch();
    contentLayout()->addLayout(bRow);

    auto *cRow = new QHBoxLayout();
    auto *cLb = new QLabel(QStringLiteral("对比度"), this);
    cLb->setObjectName(QStringLiteral("blockFieldLabel"));
    cLb->setFixedWidth(40);
    m_contrastSpin = new QSpinBox(this);
    m_contrastSpin->setRange(0, 300);
    m_contrastSpin->setValue(100);
    m_contrastSpin->setFixedWidth(64);
    cRow->addWidget(cLb);
    cRow->addWidget(m_contrastSpin);
    cRow->addStretch();
    contentLayout()->addLayout(cRow);

    auto *gRow = new QHBoxLayout();
    auto *gLb = new QLabel(QStringLiteral("伽马"), this);
    gLb->setObjectName(QStringLiteral("blockFieldLabel"));
    gLb->setFixedWidth(40);
    m_gammaSpin = new QDoubleSpinBox(this);
    m_gammaSpin->setRange(0.1, 5.0);
    m_gammaSpin->setSingleStep(0.1);
    m_gammaSpin->setValue(1.0);
    m_gammaSpin->setFixedWidth(64);
    gRow->addWidget(gLb);
    gRow->addWidget(m_gammaSpin);
    gRow->addStretch();
    contentLayout()->addLayout(gRow);

    auto emitChangeInt = [this](int) { emit paramsChanged(); };
    auto emitChangeD = [this](double) { emit paramsChanged(); };
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        updateParamEnabled();
        emit paramsChanged();
    });
    connect(m_brightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChangeInt);
    connect(m_contrastSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChangeInt);
    connect(m_gammaSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, emitChangeD);

    updateParamEnabled();
}

void GrayTransformBlock::updateParamEnabled()
{
    const auto t = currentType();
    const bool bc = (t == GrayTransformAlgorithm::Type::BrightContrast);
    const bool gm = (t == GrayTransformAlgorithm::Type::Gamma);
    m_brightSpin->setEnabled(bc);
    m_contrastSpin->setEnabled(bc);
    m_gammaSpin->setEnabled(gm);
}

GrayTransformAlgorithm::Type GrayTransformBlock::currentType() const
{
    return static_cast<GrayTransformAlgorithm::Type>(m_typeCombo->currentData().toInt());
}

QPixmap GrayTransformBlock::process(const QPixmap &input, const RoiInfo &roi)
{
    if (input.isNull()) return input;
    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);
    cv::Mat out = RoiProcess::apply(src, roi, [&](const cv::Mat &m) {
        return GrayTransformAlgorithm::apply(
            m, currentType(),
            m_brightSpin->value(), m_contrastSpin->value(),
            m_gammaSpin->value());
    });
    return ImageConverter::matToPixmap(out);
}
