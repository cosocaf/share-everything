/**
 * @file logger.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-26
 *
 * ロギングユーティリティ。
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SHARE_EVERYTHING_LOGGER_H_
#define SHARE_EVERYTHING_LOGGER_H_

#include "pch.h"

#include "tchar_util.h"

namespace share_everything {
  /**
   * @brief アプリケーションのログ取りをするユーティリティクラス。
   * 基本的には下で定義するマクロを経由してログを取る。
   *
   */
  class Logger {
    bool consoleEnabled;
    HANDLE hFile;
    void log(WORD colorFlag,
             tstring_view logLevel,
             tstring_view message,
             tstring_view group,
             tstring_view func,
             tstring_view file,
             int line);

  public:
    /**
     * @brief コンストラクタ。Logger::initを呼ばないと何もしないので注意。
     *
     */
    Logger();
    ~Logger() noexcept;

    /**
     * @brief コンソールを初期化する。
     *
     * @return true 初期化に成功した場合
     * @return false 初期化に失敗した場合
     */
    bool init();

    /**
     * @brief レベルをDebugとしてログ取りする。
     *
     * @param message ログの内容
     * @param group ログのグループ
     * @param func ログが呼ばれた場所
     * @param file ログが呼ばれたファイル
     * @param line ログが呼ばれたファイルの行数
     */
    void debug(tstring_view message,
               tstring_view group,
               tstring_view func,
               tstring_view file,
               int line);
    /**
     * @brief レベルをInfoとしてログ取りする。
     *
     * @param message ログの内容
     * @param group ログのグループ
     * @param func ログが呼ばれた場所
     * @param file ログが呼ばれたファイル
     * @param line ログが呼ばれたファイルの行数
     */
    void info(tstring_view message,
              tstring_view group,
              tstring_view func,
              tstring_view file,
              int line);
    /**
     * @brief レベルをWarnとしてログ取りする。
     *
     * @param message ログの内容
     * @param group ログのグループ
     * @param func ログが呼ばれた場所
     * @param file ログが呼ばれたファイル
     * @param line ログが呼ばれたファイルの行数
     */
    void warn(tstring_view message,
              tstring_view group,
              tstring_view func,
              tstring_view file,
              int line);
    /**
     * @brief レベルをErrorとしてログ取りする。
     *
     * @param message ログの内容
     * @param group ログのグループ
     * @param func ログが呼ばれた場所
     * @param file ログが呼ばれたファイル
     * @param line ログが呼ばれたファイルの行数
     */
    void error(tstring_view message,
               tstring_view group,
               tstring_view func,
               tstring_view file,
               int line);
    /**
     * @brief レベルをFatalとしてログ取りする。
     * このログが出るときは即座にアプリを終了しなければならないようなとき。
     * そんな状況でログ取りできるかは知らんけど。
     *
     * @param message ログの内容
     * @param group ログのグループ
     * @param func ログが呼ばれた場所
     * @param file ログが呼ばれたファイル
     * @param line ログが呼ばれたファイルの行数
     */
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