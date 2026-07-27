#ifndef TIMEMEASURER_H
#define TIMEMEASURER_H

#include "../blocksdk/blocksdk_global.h"
#include <QElapsedTimer>
#include <QString>

/**
 * @file timemeasurer.h
 * @brief 耗时统计工具 —— RAII 方式构造计时、析构自动打印
 */
class BLOCKSDK_EXPORT TimeMeasurer
{
public:
    explicit TimeMeasurer(const QString &label = QString(), bool autoPrint = true);
    ~TimeMeasurer();
    void start();
    qint64 elapsedMs() const;
    qint64 elapsedUs() const;

private:
    QString m_label;
    bool m_autoPrint;
    QElapsedTimer m_timer;
};

#endif // TIMEMEASURER_H
