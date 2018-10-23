/*
* bluetooth-share-ui
*
* Copyright 2012 Samsung Electronics Co., Ltd
*
* Contact: Hocheol Seo <hocheol.seo@samsung.com>
*           GirishAshok Joshi <girish.joshi@samsung.com>
*           DoHyun Pyun <dh79.pyun@samsung.com>
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.tizenopensource.org/license
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#ifndef __APPLOG_H__
#define __APPLOG_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ret_if(), retv_if(), retm_if(), retvm_if()
 *   If expr is true, current function return.
 *   Postfix 'v' means that it has a return value and
 *   'm' means that it has output message.
 *
 */

#include <stdio.h>
#include <dlog.h>

#undef LOG_TAG
#define LOG_TAG "BLUETOOTH_SHARE_UI"

#define INFO(fmt, arg...) \
	SLOGI(fmt, ##arg)

#define ERR(fmt, arg...) \
	SLOGE(fmt, ##arg)

#define DBG(fmt, arg...) \
	SLOGD(fmt, ##arg)

#define DBG_SECURE(fmt, args...) SECURE_SLOGD(fmt, ##args)
#define ERR_SECURE(fmt, args...) SECURE_SLOGE(fmt, ##args)
#define INFO_SECURE(fmt, args...) SECURE_SLOGI(fmt, ##args)

#define	FN_START DBG("[ENTER FUNC]");
#define	FN_END DBG("[EXIT FUNC]");

#define ret_if(expr) do { \
		if (expr) { \
			return; \
		} \
	} while (0)
#define retv_if(expr, val) do { \
		if (expr) { \
			return (val); \
		} \
	} while (0)
#define retm_if(expr, fmt, arg...) do { \
		if (expr) { \
			ERR(fmt, ##arg); \
			return; \
		} \
	} while (0)
#define retvm_if(expr, val, fmt, arg...) do { \
		if (expr) { \
			ERR(fmt, ##arg); \
			return (val); \
		} \
	} while (0)

#ifdef __cplusplus
}
#endif
#endif				/* __APPLOG_H__ */
