/**
 * @file timemeasurer.cpp
 * @brief TimeMeasurer 实现 —— 构造计时、析构打印
 *
 * 输出格式：[标签] 耗时: N ms
 */

#include "timemeasurer.h"
#include <QDebug>

/** @brief 构造：保存参数并立即开始计时 */
TimeMeasurer::TimeMeasurer(const QString &label, bool autoPrint)
    : m_label(label)
    , m_autoPrint(autoPrint)
{
    m_timer.start(); // 启动计时器
}

/** @brief 析构：RAII 自动报告耗时（label 非空且 autoPrint 为 true 时） */
TimeMeasurer::~TimeMeasurer()
{
    if (m_autoPrint && !m_label.isEmpty()) {
        qDebug() << QString("[%1] 耗时: %2 ms").arg(m_label).arg(elapsedMs()); // 打印标签与毫秒数
    }
}

/** @brief 重置计时起点，用于同作用域内多段测量 */
void TimeMeasurer::start()
{
    m_timer.start(); // 重新计时
}

/** @brief 返回自构造或最近一次 start() 起的毫秒数 */
qint64 TimeMeasurer::elapsedMs() const
{
    return m_timer.elapsed(); // QElapsedTimer 毫秒接口
}

/** @brief 返回自构造或最近一次 start() 起的微秒数 */
qint64 TimeMeasurer::elapsedUs() const
{
    return m_timer.nsecsElapsed() / 1000; // 纳秒转微秒
}
