#ifndef TIMEMEASURER_H
#define TIMEMEASURER_H

#include <QElapsedTimer>
#include <QString>

/**
 * @file timemeasurer.h
 * @brief 耗时统计工具 —— 开发期性能分析
 *
 * 【在整条链路中的位置】
 *   可在 ImageProcessor::reprocess、各 Algorithm::apply 或 Block::process 内
 *   用 RAII 方式包裹一段代码，析构时自动 qDebug 打印毫秒数
 *
 * 【设计模式】
 *   RAII（Resource Acquisition Is Initialization）：
 *   构造 = 开始计时，析构 = 可选自动输出，无需手动 stop
 *
 * 【使用示例】
 * @code
 * {
 *     TimeMeasurer t("二值化处理");
 *     // ... 处理代码 ...
 * } // 离开作用域自动打印耗时
 * @endcode
 *
 * 【与 QElapsedTimer 的关系】
 *   本类是对 QElapsedTimer 的薄封装，增加标签与自动打印
 */

class TimeMeasurer
{
public:
    /**
     * @brief 构造并开始计时
     *
     * @param label     日志前缀，如「MorphologyBlock::process」
     * @param autoPrint 析构时是否 qDebug 输出（默认 true）
     *
     * 若 label 为空且 autoPrint 为 true，析构时不打印（避免无意义日志）
     */
    explicit TimeMeasurer(const QString &label = QString(), bool autoPrint = true);

    /**
     * @brief 析构，若 autoPrint 为 true 且 label 非空则输出耗时
     */
    ~TimeMeasurer();

    /**
     * @brief 重新开始计时（用于同作用域内多段测量）
     *
     * 典型用法：
     *   TimeMeasurer t("总耗时", false);
     *   ... 段1 ...
     *   t.start(); ... 段2 ...
     *   qDebug() << t.elapsedMs();
     */
    void start();

    /**
     * @brief 获取已流逝时间（毫秒）
     * @return 自构造或最近一次 start() 起的毫秒数
     */
    qint64 elapsedMs() const;

    /**
     * @brief 获取已流逝时间（微秒）
     * @return 纳秒精度 / 1000，适合短函数细测
     */
    qint64 elapsedUs() const;

private:
    QString m_label;       ///< 日志标签
    bool m_autoPrint;      ///< 析构是否自动打印
    QElapsedTimer m_timer; ///< Qt 高精度计时器（单调递增，不受系统时间调整影响）
};

#endif // TIMEMEASURER_H
