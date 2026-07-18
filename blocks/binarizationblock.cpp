#include "binarizationblock.h"
#include "../utils/imageconverter.h"
#include "../utils/roiprocess.h"
#include "../algorithms/binarization.h"

#include <QStyle>
#include <QVariant>
#include <algorithm>

BinarizationBlock::BinarizationBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupTitle(QStringLiteral("🔲"), QStringLiteral("二值化处理"));
    setupUI();
}

void BinarizationBlock::setupUI()
{
    addSeparator();

    auto *lowerLayout = new QHBoxLayout();
    auto *lowerLabel = new QLabel(QStringLiteral("下限值"), this);
    lowerLabel->setObjectName(QStringLiteral("blockFieldLabel"));
    lowerLabel->setFixedWidth(40);

    m_lowerSpin = new QSpinBox(this);
    m_lowerSpin->setRange(0, 255);
    m_lowerSpin->setValue(AppConfig::DEFAULT_BINARY_LOWER);
    m_lowerSpin->setFixedWidth(64);

    lowerLayout->addWidget(lowerLabel);
    lowerLayout->addWidget(m_lowerSpin);
    lowerLayout->addStretch();
    contentLayout()->addLayout(lowerLayout);

    auto *upperLayout = new QHBoxLayout();
    auto *upperLabel = new QLabel(QStringLiteral("上限值"), this);
    upperLabel->setObjectName(QStringLiteral("blockFieldLabel"));
    upperLabel->setFixedWidth(40);

    m_upperSpin = new QSpinBox(this);
    m_upperSpin->setRange(0, 255);
    m_upperSpin->setValue(AppConfig::DEFAULT_BINARY_UPPER);
    m_upperSpin->setFixedWidth(64);

    upperLayout->addWidget(upperLabel);
    upperLayout->addWidget(m_upperSpin);
    upperLayout->addStretch();
    contentLayout()->addLayout(upperLayout);

    m_autoBtn = new QPushButton(QStringLiteral("Otsu"), this);
    m_autoBtn->setProperty("role", QVariant(QStringLiteral("accent")));
    m_autoBtn->setCursor(Qt::PointingHandCursor);
    m_autoBtn->setToolTip(QStringLiteral("自动阈值 (Otsu)"));
    contentLayout()->addWidget(m_autoBtn);
    if (m_autoBtn->style()) {
        m_autoBtn->style()->unpolish(m_autoBtn);
        m_autoBtn->style()->polish(m_autoBtn);
    }

    connect(m_lowerSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BinarizationBlock::onLowerChanged);
    connect(m_upperSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BinarizationBlock::onUpperChanged);
    connect(m_autoBtn, &QPushButton::clicked,
            this, &BinarizationBlock::onAutoThresholdClicked);
}

QPixmap BinarizationBlock::process(const QPixmap &input, const RoiInfo &roi)
{
    if (input.isNull()) return input;

    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);

    const int lower = m_lowerSpin->value();
    const int upper = m_upperSpin->value();

    // ROI 外原样保留：整图处理后按 mask 贴回
    cv::Mat result = RoiProcess::apply(src, roi, [&](const cv::Mat &m) {
        cv::Mat gray, binary, bgr;
        cv::cvtColor(m, gray, cv::COLOR_BGR2GRAY);
        binary = BinarizationAlgorithm::applyRangeThreshold(gray, lower, upper);
        cv::cvtColor(binary, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    });

    // matToPixmap 约定输入为 BGR
    return ImageConverter::matToPixmap(result);
}

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
    emit paramsChanged();
}

void BinarizationBlock::onAutoThresholdClicked()
{
    emit otsuRequested();
}

void BinarizationBlock::onLowerChanged(int value)
{
    if (value > m_upperSpin->value()) {
        m_upperSpin->blockSignals(true);
        m_upperSpin->setValue(value);
        m_upperSpin->blockSignals(false);
    }
    emit paramsChanged();
}

void BinarizationBlock::onUpperChanged(int value)
{
    if (value < m_lowerSpin->value()) {
        m_lowerSpin->blockSignals(true);
        m_lowerSpin->setValue(value);
        m_lowerSpin->blockSignals(false);
    }
    emit paramsChanged();
}
