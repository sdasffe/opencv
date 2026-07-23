#ifndef APPLOGGER_H
#define APPLOGGER_H

#include <QString>

/**
 * @file applogger.h
 * @brief 简易文件日志（无 UI，横切基础设施）
 *
 * 日志目录：可执行文件旁 logs/；文件名 app_yyyyMMdd.log（按天滚动）
 */
class AppLogger
{
public:
    enum class Level { Info, Warn, Error }; // 日志级别：一般信息 / 可恢复警告 / 明确错误

    static void init(); // 程序启动时调用一次：创建 logs/ 目录并写入会话头
    static void info(const QString &event, const QString &detail = QString());  // 记录一般操作（打开图片、添加块、保存等）
    static void warn(const QString &event, const QString &detail = QString());  // 记录可恢复异常或用户误操作
    static void error(const QString &event, const QString &detail = QString()); // 记录明确失败（加载/保存/导出错误等）
    static void log(Level level, const QString &event, const QString &detail = QString()); // 统一写日志入口；未 init 时会懒加载 init()
    static QString currentLogPath(); // 当前日志文件绝对路径（init 之后有效，供「打开日志目录」菜单使用）

private:
    AppLogger() = delete; // 纯静态工具类，禁止实例化
};

#endif // APPLOGGER_H
