#include "binarizationblock.h"
#include "../utils/imageconverter.h"
#include "../algorithms/binarization.h"
#include "../algorithms/otsu.h"

BinarizationBlock::BinarizationBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupTitle("🔲", "二值化处理");
    setupUI();
}

void BinarizationBlock::setupUI()
{
    addSeparator();

    QString style = spinBoxStyle();

    // ---------- 下限值 ----------
    auto *lowerLayout = new QHBoxLayout();
    auto *lowerLabel = new QLabel("下限值：", this);
    lowerLabel->setStyleSheet("font-size: 12px; color: #666; border: none;");
    lowerLabel->setFixedWidth(55);

    m_lowerSpin = new QSpinBox(this);
    m_lowerSpin->setRange(0, 255);
    m_lowerSpin->setValue(AppConfig::DEFAULT_BINARY_LOWER);
    m_lowerSpin->setFixedWidth(80);
    m_lowerSpin->setStyleSheet(style);

    lowerLayout->addWidget(lowerLabel);
    lowerLayout->addWidget(m_lowerSpin);
    lowerLayout->addStretch();
    contentLayout()->addLayout(lowerLayout);

    // ---------- 上限值 ----------
    auto *upperLayout = new QHBoxLayout();
    auto *upperLabel = new QLabel("上限值：", this);
    upperLabel->setStyleSheet("font-size: 12px; color: #666; border: none;");
    upperLabel->setFixedWidth(55);

    m_upperSpin = new QSpinBox(this);
    m_upperSpin->setRange(0, 255);
    m_upperSpin->setValue(AppConfig::DEFAULT_BINARY_UPPER);
    m_upperSpin->setFixedWidth(80);
    m_upperSpin->setStyleSheet(style);

    upperLayout->addWidget(upperLabel);
    upperLayout->addWidget(m_upperSpin);
    upperLayout->addStretch();
    contentLayout()->addLayout(upperLayout);

    // ---------- 自动设置按钮 ----------
    m_autoBtn = new QPushButton("自动设置", this);
    m_autoBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 6px 12px;"
        "    font-size: 12px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #1976D2; }"
        );
    contentLayout()->addWidget(m_autoBtn);

    // ---------- 信号连接 ----------
    connect(m_lowerSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BinarizationBlock::onLowerChanged);
    connect(m_upperSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BinarizationBlock::onUpperChanged);
    connect(m_autoBtn, &QPushButton::clicked,
            this, &BinarizationBlock::onAutoThresholdClicked);
}

QPixmap BinarizationBlock::process(const QPixmap &input, const QRectF &roi)
{
    if (input.isNull()) return input;

    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    // pixmapToMatRGB 输出的是 RGB 顺序，但 OpenCV 算法默认 BGR
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);

    cv::Mat result;

    if (roi.isEmpty() || roi.isNull()) {
        // 全图二值化
        cv::Mat gray;
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
        cv::Mat binary = BinarizationAlgorithm::applyRangeThreshold(
            gray, m_lowerSpin->value(), m_upperSpin->value());
        cv::cvtColor(binary, result, cv::COLOR_GRAY2BGR);
    } else {
        // ROI 区域二值化（坐标转换：QRectF 转 cv::Rect）
        cv::Rect cvRoi(
            qMax(0, qRound(roi.x())),
            qMax(0, qRound(roi.y())),
            qMax(0, qRound(roi.width())),
            qMax(0, qRound(roi.height()))
            );
        result = BinarizationAlgorithm::applyRoiRangeThreshold(
            src, cvRoi, m_lowerSpin->value(), m_upperSpin->value());
    }

    // BGR → RGB → QPixmap
    cv::cvtColor(result, result, cv::COLOR_BGR2RGB);
    return ImageConverter::matToPixmap(result);
}

void BinarizationBlock::onAutoThresholdClicked()
{
    // 注意：自动阈值需要原图，这里由外部调用 updateAutoThreshold
    // 简化处理：发出信号，由主窗口计算后设置值
    // 为了保持封装性，暂时用固定值触发，实际计算放在 ImageProcessor
    emit paramsChanged();
}

void BinarizationBlock::onLowerChanged(int value)
{
    // 下限不超过上限
    if (value > m_upperSpin->value()) {
        m_upperSpin->blockSignals(true);
        m_upperSpin->setValue(value);
        m_upperSpin->blockSignals(false);
    }
    emit paramsChanged();
}

void BinarizationBlock::onUpperChanged(int value)
{
    // 上限不低于下限
    if (value < m_lowerSpin->value()) {
        m_lowerSpin->blockSignals(true);
        m_lowerSpin->setValue(value);
        m_lowerSpin->blockSignals(false);
    }
    emit paramsChanged();
}

QString BinarizationBlock::spinBoxStyle()
{
    return
        "QSpinBox {"
        "    padding: 3px 6px;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    font-size: 12px;"
        "}"
        "QSpinBox::up-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: top right;"
        "    width: 20px;"
        "    height: 12px;"
        "    border-left: 1px solid #ddd;"
        "    border-bottom: 1px solid #ddd;"
        "    border-top-right-radius: 4px;"
        "}"
        "QSpinBox::down-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: bottom right;"
        "    width: 20px;"
        "    height: 12px;"
        "    border-left: 1px solid #ddd;"
        "    border-bottom-right-radius: 4px;"
        "}"
        "QSpinBox::up-button:hover { background-color: #e0e0e0; }"
        "QSpinBox::down-button:hover { background-color: #e0e0e0; }"
        "QSpinBox::up-button:pressed { background-color: #ccc; }"
        "QSpinBox::down-button:pressed { background-color: #ccc; }"
        "QSpinBox::up-arrow { width: 6px; height: 6px; }"
        "QSpinBox::down-arrow { width: 6px; height: 6px; }";
}
