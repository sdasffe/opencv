#ifndef TIMEMEASURER_H
#define TIMEMEASURER_H

#include <QElapsedTimer>
#include <QString>

/**
 * @brief 耗时统计工具
 *
 * RAII 风格的计时器，构造时开始计时，析构时自动输出耗时。
 * 也支持手动 start/elapsed 用于多段计时。
 *
 * 使用示例：
 * @code
 * {
 *     TimeMeasurer t("二值化处理");
 *     // ... 处理代码 ...
 * } // 离开作用域自动打印耗时
 * @endcode
 */
class TimeMeasurer
{
public:
    /**
     * @brief 构造并开始计时
     * @param label 计时标签，用于输出标识
     * @param autoPrint 析构时是否自动打印耗时（默认 true）
     */
    explicit TimeMeasurer(const QString &label = QString(), bool autoPrint = true);

    /**
     * @brief 析构，若 autoPrint 为 true 则输出耗时
     */
    ~TimeMeasurer();

    /**
     * @brief 重新开始计时
     */
    void start();

    /**
     * @brief 获取已流逝时间（毫秒）
     * @return 毫秒数
     */
    qint64 elapsedMs() const;

    /**
     * @brief 获取已流逝时间（微秒）
     * @return 微秒数
     */
    qint64 elapsedUs() const;

private:
    QString m_label;
    bool m_autoPrint;
    QElapsedTimer m_timer;
};

#endif // TIMEMEASURER_H
