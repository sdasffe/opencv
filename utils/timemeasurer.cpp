/**
 * @file timemeasurer.cpp
 * @brief TimeMeasurer 实现 —— 构造计时、析构打印
 *
 * 【调用链】
 *   开发者代码创建 TimeMeasurer → 构造 start → 执行被测代码 → 析构 optional qDebug
 *
 * 【输出格式】
 *   [标签] 耗时: N ms
 */

#include "timemeasurer.h"
#include <QDebug>

/**
 * @brief 构造：保存参数并立即 start 计时
 */
TimeMeasurer::TimeMeasurer(const QString &label, bool autoPrint)
    : m_label(label)
    , m_autoPrint(autoPrint)
{
    m_timer.start();
}

/**
 * @brief 析构：RAII 自动报告
 *
 * 条件：autoPrint && label 非空
 * 为什么 label 空时不打：避免匿名计时器刷屏
 */
TimeMeasurer::~TimeMeasurer()
{
    if (m_autoPrint && !m_label.isEmpty()) {
        qDebug() << QString("[%1] 耗时: %2 ms").arg(m_label).arg(elapsedMs());
    }
}

/** 重置起点；同作用域内测多段时先构造再多次 start() */
void TimeMeasurer::start()
{
    m_timer.start();
}

/** 毫秒，与析构日志及 QElapsedTimer::elapsed 一致 */
qint64 TimeMeasurer::elapsedMs() const
{
    return m_timer.elapsed();
}

/** 微秒，适合单次算法调用等短耗时细测 */
qint64 TimeMeasurer::elapsedUs() const
{
    return m_timer.nsecsElapsed() / 1000;
}
