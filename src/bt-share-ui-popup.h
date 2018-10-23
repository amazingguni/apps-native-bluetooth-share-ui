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

#ifndef __DEF_BT_SHARE_UI_POPUP_H_
#define __DEF_BT_SHARE_UI_POPUP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <bluetooth-share-api.h>
#include "bt-share-ui-main.h"

#define BT_FILE_SIZE_STR 10
#define BT_INFO_POPUP_TIMEOUT_IN_SEC 2

Evas_Object *_bt_share_add_turning_on_popup(bt_share_appdata_t *ad);

int _bt_update_progressbar(bt_share_appdata_t *ad, int transfer_id,
					const char *name, int percentage);

Evas_Object *_bt_create_info_popup(bt_share_appdata_t *ad, const char *text);

int _bt_destroy_info_popup(bt_share_appdata_t *ad);

#ifdef __cplusplus
}
#endif
#endif				/* __DEF_BT_SHARE_UI_POPUP_H_ */
