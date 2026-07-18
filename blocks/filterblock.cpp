#include "filterblock.h"
#include "../utils/imageconverter.h"
#include "../utils/roiprocess.h"

FilterBlock::FilterBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupTitle(QStringLiteral("🌀"), AppConfig::BLOCK_NAME_FILTER);
    setupUI();
}

void FilterBlock::setupUI()
{
    addSeparator();

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(QStringLiteral("均值滤波"), int(FilterAlgorithm::Type::Mean));
    m_typeCombo->addItem(QStringLiteral("高斯滤波"), int(FilterAlgorithm::Type::Gaussian));
    m_typeCombo->addItem(QStringLiteral("中值滤波"), int(FilterAlgorithm::Type::Median));
    m_typeCombo->addItem(QStringLiteral("Sobel"), int(FilterAlgorithm::Type::Sobel));
    m_typeCombo->addItem(QStringLiteral("Laplacian"), int(FilterAlgorithm::Type::Laplacian));
    m_typeCombo->addItem(QStringLiteral("Prewitt"), int(FilterAlgorithm::Type::Prewitt));
    m_typeCombo->addItem(QStringLiteral("Roberts"), int(FilterAlgorithm::Type::Roberts));
    contentLayout()->addWidget(m_typeCombo);

    auto addSpin = [&](const QString &label, int def) {
        auto *row = new QHBoxLayout();
        auto *lb = new QLabel(label, this);
        lb->setObjectName(QStringLiteral("blockFieldLabel"));
        lb->setFixedWidth(40);
        auto *sp = new QSpinBox(this);
        sp->setRange(1, 31);
        sp->setValue(def);
        sp->setFixedWidth(64);
        row->addWidget(lb);
        row->addWidget(sp);
        row->addStretch();
        contentLayout()->addLayout(row);
        return sp;
    };

    m_kxSpin = addSpin(QStringLiteral("核 X"), 3);
    m_kySpin = addSpin(QStringLiteral("核 Y"), 3);
    m_iterSpin = addSpin(QStringLiteral("次数"), 1);
    m_iterSpin->setRange(1, 20);

    auto emitChange = [this](int) { emit paramsChanged(); };
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChange);
    connect(m_kxSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    connect(m_kySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    connect(m_iterSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
}

FilterAlgorithm::Type FilterBlock::currentType() const
{
    return static_cast<FilterAlgorithm::Type>(m_typeCombo->currentData().toInt());
}

QPixmap FilterBlock::process(const QPixmap &input, const RoiInfo &roi)
{
    if (input.isNull()) return input;
    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);
    cv::Mat out = RoiProcess::apply(src, roi, [&](const cv::Mat &m) {
        return FilterAlgorithm::apply(m, currentType(),
                                      m_kxSpin->value(), m_kySpin->value(),
                                      m_iterSpin->value());
    });
    return ImageConverter::matToPixmap(out);
}
