#ifndef SHARE_EVERYTHING_LOGGER_H_
#define SHARE_EVERYTHING_LOGGER_H_

#include "pch.h"

#include "tchar_util.h"

namespace share_everything {
  class Logger {
    bool consoleEnabled;
    void log(tstring_view logLevel,
             tstring_view message,
             tstring_view group,
             tstring_view func,
             tstring_view file,
             int line);

  public:
    Logger();
    ~Logger() noexcept;

    bool init();

    void debug(tstring_view message,
               tstring_view group,
               tstring_view func,
               tstring_view file,
               int line);
    void info(tstring_view message,
              tstring_view group,
              tstring_view func,
              tstring_view file,
              int line);
    void warn(tstring_view message,
              tstring_view group,
              tstring_view func,
              tstring_view file,
              int line);
    void error(tstring_view message,
               tstring_view group,
               tstring_view func,
               tstring_view file,
               int line);
    void fatal(tstring_view message,
               tstring_view group,
               tstring_view func,
               tstring_view file,
               int line);
  };
  extern Logger logger;
} // namespace share_everything

#if defined(_MSC_VER)
#define LOG_DEBUG(msg, group)     \
  share_everything::logger.debug( \
    msg, group, _T(__FUNCSIG__), _T(__FILE__), __LINE__)
#define LOG_INFO(msg, group)     \
  share_everything::logger.info( \
    msg, group, _T(__FUNCSIG__), _T(__FILE__), __LINE__)
#define LOG_WARN(msg, group)     \
  share_everything::logger.warn( \
    msg, group, _T(__FUNCSIG__), _T(__FILE__), __LINE__)
#define LOG_ERROR(msg, group)     \
  share_everything::logger.error( \
    msg, group, _T(__FUNCSIG__), _T(__FILE__), __LINE__)
#define LOG_FATAL(msg, group)     \
  share_everything::logger.fatal( \
    msg, group, _T(__FUNCSIG__), _T(__FILE__), __LINE__)
#elif defined(__GNUC__)
#define LOG_DEBUG(msg, group)     \
  share_everything::logger.debug( \
    msg, group, _T(__PRETTY_FUNCTION__), _T(__FILE__), __LINE__)
#define LOG_INFO(msg, group)     \
  share_everything::logger.info( \
    msg, group, _T(__PRETTY_FUNCTION__), _T(__FILE__), __LINE__)
#define LOG_WARN(msg, group)     \
  share_everything::logger.warn( \
    msg, group, _T(__PRETTY_FUNCTION__), _T(__FILE__), __LINE__)
#define LOG_ERROR(msg, group)     \
  share_everything::logger.error( \
    msg, group, _T(__PRETTY_FUNCTION__), _T(__FILE__), __LINE__)
#define LOG_FATAL(msg, group)     \
  share_everything::logger.fatal( \
    msg, group, _T(__PRETTY_FUNCTION__), _T(__FILE__), __LINE__)
#else
#define LOG_DEBUG(msg, group)     \
  share_everything::logger.debug( \
    msg, group, _T(__func__), _T(__FILE__), __LINE__)
#define LOG_INFO(msg, group)     \
  share_everything::logger.info( \
    msg, group, _T(__func__), _T(__FILE__), __LINE__)
#define LOG_WARN(msg, group)     \
  share_everything::logger.warn( \
    msg, group, _T(__func__), _T(__FILE__), __LINE__)
#define LOG_ERROR(msg, group)     \
  share_everything::logger.error( \
    msg, group, _T(__func__), _T(__FILE__), __LINE__)
#define LOG_FATAL(msg, group)     \
  share_everything::logger.fatal( \
    msg, group, _T(__func__), _T(__FILE__), __LINE__)
#endif // defined(_MSC_VER)

#endif // SHARE_EVERYTHING_LOGGER_H_