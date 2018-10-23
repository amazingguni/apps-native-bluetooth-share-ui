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

#ifndef __DEF_BT_SHARE_UI_VIEW_H_
#define __DEF_BT_SHARE_UI_VIEW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bt-share-ui-main.h"
#include "bt-share-ui-ipc.h"

#include <bluetooth-share-api.h>
#include <bluetooth.h>

#define BT_GLOBALIZATION_STR_LENGTH	256
#define BT_FILE_NAME_LEN_MAX 255
#define BT_ANI_UPLOAD		"bt_share_upload"
#define BT_ANI_DOWNLOAD		"bt_share_download"

typedef enum {
	BT_FILE_IMAGE,	/**<IMAGE */
	BT_FILE_VCARD,	/**<VCARD */
	BT_FILE_VCAL,	/**<VCAL */
	BT_FILE_VBOOKMARK,	/**<VBOOKMARK */
	BT_FILE_VMEMO,
	BT_FILE_DOC,	/**<DOC, */
	BT_FILE_OTHER	/**<OTHER*/
} bt_file_type_e;

Evas_Object *_bt_create_win(const char *name);
void _bt_terminate_app(void);
void _bt_genlist_prepend_tr_data_item(bt_share_appdata_t *ad,
				Evas_Object *genlist, bt_tr_data_t *info, int tr_type);
void _bt_genlist_append_tr_data_item(bt_share_appdata_t *ad,
			bt_tr_data_t *info, int tr_type);
int  _bt_create_transfer_view(bt_share_appdata_t *ad);
void _bt_cb_state_changed(int result,
			bt_adapter_state_e adapter_state,
			void *user_data);
void _bt_share_ui_handle_transfer_disconnected(bt_share_appdata_t *ad,
		bt_share_tr_type_e type);
void _bt_share_ui_handle_transfer_complete(bt_share_appdata_t *ad,
		char *address, bt_share_tr_type_e type);
void _bt_share_ui_handle_transfer_started(bt_share_appdata_t *ad,
		char *address, char *file_name, unsigned long size, int transfer_id,
		bt_share_tr_type_e type);
void _bt_delete_selected_notification(bt_share_tr_type_e tr_type,
		int noti_id, const char *opp_role);
int _bt_share_enable_bt(bt_share_appdata_t *ad);
Evas_Object * _bt_share_create_toolbar_button(bt_share_appdata_t *ad,
		char *text);
void _bt_share_delete_toolbar_button(bt_share_appdata_t *ad);

#ifdef __cplusplus
}
#endif
#endif				/* __DEF_BT_SHARE_UI_VIEW_H_ */
