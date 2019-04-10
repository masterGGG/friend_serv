#ifndef _MONITOR_API_H_
#define _MONITOR_API_H_

#include "log.h"

#define RP_ERROR_CODE_DB			"DB_ERR"
#define RP_ERROR_CODE_NETWORK		"NET_ERR"
#define RP_ERROR_CODE_SYSCALL		"SYS_ERR"
#define RP_ERROR_CODE_LOGICAL		"LOGI_ERR"
#define RP_ERROR_CODE_EXCEPTION		"EXCE_ERR"

#ifndef MONITOR_DETAIL 
#define MONITOR_DETAIL(level, key, error_type, fmt, args...) \
		write_log(level, key, "[%s][%d][%s]%s: " fmt "\n", __FILE__, __LINE__, error_type, __FUNCTION__, ##args)
#endif

#define ERROR_LOG_RP(error_type, fmt, args...) \
		MONITOR_DETAIL(log_lvl_crit, 0, error_type, fmt, ##args)
		
#endif
