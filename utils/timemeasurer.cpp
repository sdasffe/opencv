#include "timemeasurer.h"
#include <QDebug>

TimeMeasurer::TimeMeasurer(const QString &label, bool autoPrint)
    : m_label(label)
    , m_autoPrint(autoPrint)
{
    m_timer.start();
}

TimeMeasurer::~TimeMeasurer()
{
    if (m_autoPrint && !m_label.isEmpty()) {
        qDebug() << QString("[%1] 耗时: %2 ms").arg(m_label).arg(elapsedMs());
    }
}

void TimeMeasurer::start()
{
    m_timer.start();
}

qint64 TimeMeasurer::elapsedMs() const
{
    return m_timer.elapsed();
}

qint64 TimeMeasurer::elapsedUs() const
{
    return m_timer.nsecsElapsed() / 1000;
}
