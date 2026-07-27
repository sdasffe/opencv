#ifndef APPLOGGER_H
#define APPLOGGER_H

#include "../blocksdk/blocksdk_global.h"
#include <QString>

/**
 * @file applogger.h
 * @brief 简易文件日志（无 UI，横切基础设施）
 */
class BLOCKSDK_EXPORT AppLogger
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
