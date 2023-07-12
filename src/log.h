#ifndef PEAR_LOG__
#define PEAR_LOG__

#include <stdio.h>

#define LEVEL_ERROR 0x00
#define LEVEL_WARN 0x01
#define LEVEL_INFO 0x02
#define LEVEL_DEBUG 0x03

#define ERROR_TAG "ERROR"
#define WARN_TAG "WARN"
#define INFO_TAG "INFO"
#define DEBUG_TAG "DEBUG"

#ifndef LOG_LEVEL
#define LOG_LEVEL LEVEL_DEBUG
#endif


#if defined(CONFIG_USE_ALIOS)

#include <aos/cli.h>
// configure log
#define CONFIG_LANGO_DBG_LEVEL LANGO_DBG_LOG
#define LANGO_DBG_TAG "PEAR"
#include <lango_log.h>
// ------------------

#undef LOGD
#undef LOGI
#undef LOGW
#undef LOGE

#if LOG_LEVEL >= LEVEL_DEBUG
#define LOGD(fmt, ...) LANGO_LOG_DBG(fmt, ##__VA_ARGS__)
#define LOGD_DUMP_HEX(head, str, count) LANGO_LOG_DBG_DUMP_HEX(head, str, count)
#else
#define LOGD(fmt, ...)
#define LOGD_DUMP_HEX(head, str, count)
#endif

#if LOG_LEVEL >= LEVEL_INFO
#define LOGI(fmt, ...) LANGO_LOG_INFO(fmt, ##__VA_ARGS__)
#define LOGI_DUMP_HEX(head, str, count) LANGO_LOG_INFO_DUMP_HEX(head, str, count)
#else
#define LOGI(fmt, ...)
#define LOGI_DUMP_HEX(str, count, fmt, ...)
#endif

#if LOG_LEVEL >= LEVEL_WARN
#define LOGW(fmt, ...) LANGO_LOG_WRN(fmt, ##__VA_ARGS__)
#define LOGW_DUMP_HEX(head, str, count) LANGO_LOG_WRN_DUMP_HEX(head, str, count)
#else
#define LOGW(fmt, ...)
#define LOGW_DUMP_HEX(head, str, count)
#endif

#if LOG_LEVEL >= LEVEL_ERROR
#define LOGE(fmt, ...)  LANGO_LOG_ERR(fmt, ##__VA_ARGS__)
#define LOGE_DUMP_HEX(head, str, count) LANGO_LOG_ERR_DUMP_HEX(head, str, count)
#else
#define LOGE(fmt, ...)
#define LOGE_DUMP_HEX(head, str, count)
#endif
#else //!CONFIG_USE_ALIOS

#define LOG_PRINT(level_tag, fmt, ...) \
  fprintf(stdout, "%s\t%s\t%d\t" fmt"\n", level_tag, __FILE__, __LINE__, ##__VA_ARGS__)

#if LOG_LEVEL >= LEVEL_DEBUG
#define LOGD(fmt, ...) LOG_PRINT(DEBUG_TAG, fmt, ##__VA_ARGS__)
#else
#define LOGD(fmt, ...)
#endif

#if LOG_LEVEL >= LEVEL_INFO
#define LOGI(fmt, ...) LOG_PRINT(INFO_TAG, fmt, ##__VA_ARGS__)
#else
#define LOGI(fmt, ...)
#endif

#if LOG_LEVEL >= LEVEL_WARN
#define LOGW(fmt, ...) LOG_PRINT(WARN_TAG, fmt, ##__VA_ARGS__)
#else
#define LOGW(fmt, ...)
#endif

#if LOG_LEVEL >= LEVEL_ERROR
#define LOGE(fmt, ...) LOG_PRINT(ERROR_TAG, fmt, ##__VA_ARGS__)
#else
#define LOGE(fmt, ...)
#endif

#endif // CONFIG_USE_ALIOS

void _ssl_debug(void *ctx, int level, const char *file, int line, const char *str);

#endif //PEAR_LOG__