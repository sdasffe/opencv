#ifndef TIMEMEASURER_H
#define TIMEMEASURER_H

#include <QElapsedTimer>
#include <QString>

/**
 * @file timemeasurer.h
 * @brief 耗时统计工具 —— RAII 方式构造计时、析构自动打印
 *
 * 用法：在作用域内创建 TimeMeasurer，离开作用域时自动 qDebug 输出毫秒数。
 */
class TimeMeasurer
{
public:
    explicit TimeMeasurer(const QString &label = QString(), bool autoPrint = true); // 构造并开始计时；label 为日志前缀，autoPrint 控制析构是否自动打印
    ~TimeMeasurer();           // 析构时若 autoPrint 且 label 非空则输出耗时
    void start();              // 重新开始计时（同作用域内多段测量）
    qint64 elapsedMs() const;  // 获取已流逝时间（毫秒）
    qint64 elapsedUs() const;  // 获取已流逝时间（微秒）

private:
    QString m_label;       // 日志标签（如「MorphologyBlock::process」）
    bool m_autoPrint;      // 析构时是否自动 qDebug 输出
    QElapsedTimer m_timer; // Qt 高精度单调递增计时器
};

#endif // TIMEMEASURER_H
