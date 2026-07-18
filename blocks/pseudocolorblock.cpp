#include "pseudocolorblock.h"
#include "../utils/imageconverter.h"
#include "../utils/roiprocess.h"

PseudoColorBlock::PseudoColorBlock(QWidget *parent)
    : BaseBlock(parent)
{
    setupTitle(QStringLiteral("🌈"), AppConfig::BLOCK_NAME_PSEUDOCOLOR);
    setupUI();
}

void PseudoColorBlock::setupUI()
{
    addSeparator();

    m_mapCombo = new QComboBox(this);
    m_mapCombo->addItem(QStringLiteral("Jet"), int(PseudoColorAlgorithm::Map::Jet));
    m_mapCombo->addItem(QStringLiteral("Hot"), int(PseudoColorAlgorithm::Map::Hot));
    m_mapCombo->addItem(QStringLiteral("Cool"), int(PseudoColorAlgorithm::Map::Cool));
    m_mapCombo->addItem(QStringLiteral("Rainbow"), int(PseudoColorAlgorithm::Map::Rainbow));
    m_mapCombo->addItem(QStringLiteral("Ocean"), int(PseudoColorAlgorithm::Map::Ocean));
    m_mapCombo->addItem(QStringLiteral("Summer"), int(PseudoColorAlgorithm::Map::Summer));
    m_mapCombo->addItem(QStringLiteral("Winter"), int(PseudoColorAlgorithm::Map::Winter));
    m_mapCombo->addItem(QStringLiteral("Autumn"), int(PseudoColorAlgorithm::Map::Autumn));
    m_mapCombo->addItem(QStringLiteral("Bone"), int(PseudoColorAlgorithm::Map::Bone));
    m_mapCombo->addItem(QStringLiteral("Pink"), int(PseudoColorAlgorithm::Map::Pink));
    contentLayout()->addWidget(m_mapCombo);

    connect(m_mapCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { emit paramsChanged(); });
}

PseudoColorAlgorithm::Map PseudoColorBlock::currentMap() const
{
    return static_cast<PseudoColorAlgorithm::Map>(m_mapCombo->currentData().toInt());
}

QPixmap PseudoColorBlock::process(const QPixmap &input, const RoiInfo &roi)
{
    if (input.isNull()) return input;
    cv::Mat src = ImageConverter::pixmapToMatRGB(input);
    cv::cvtColor(src, src, cv::COLOR_RGB2BGR);
    cv::Mat out = RoiProcess::apply(src, roi, [&](const cv::Mat &m) {
        return PseudoColorAlgorithm::apply(m, currentMap());
    });
    return ImageConverter::matToPixmap(out);
}
