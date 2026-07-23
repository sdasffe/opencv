#ifndef APPLOGGER_H
#define APPLOGGER_H

#include <QString>

/**
 * @file applogger.h
 * @brief 简易文件日志（无 UI，横切基础设施）
 *
 * 【在整条链路中的位置】
 *   main 启动 → AppLogger::init()
 *   Widget / ImageProcessor 等各处 → info/warn/error 记录用户操作与异常
 *   不参与图像处理，仅作运行期诊断与行为审计
 *
 * 日志目录：可执行文件旁 logs/
 * 文件名：app_yyyyMMdd.log（按天滚动）
 */
class AppLogger
{
public:
    enum class Level { Info, Warn, Error };

    /** 程序启动时调用一次：创建 logs/ 目录、写入会话头 */
    static void init();

    /** 一般操作（打开图片、添加块、保存等） */
    static void info(const QString &event, const QString &detail = QString());
    /** 可恢复异常或用户误操作（空文件夹、会话读取失败等） */
    static void warn(const QString &event, const QString &detail = QString());
    /** 明确失败（加载/保存/导出错误等） */
    static void error(const QString &event, const QString &detail = QString());

    /**
     * 统一写日志入口；未 init 时会懒加载 init()
     * @param event  事件摘要（中文短语）
     * @param detail 可选细节（路径、数值、错误信息等）
     */
    static void log(Level level, const QString &event, const QString &detail = QString());

    /** 当前日志文件绝对路径（init 之后有效，供「打开日志目录」菜单使用） */
    static QString currentLogPath();

private:
    AppLogger() = delete;
};

#endif // APPLOGGER_H
