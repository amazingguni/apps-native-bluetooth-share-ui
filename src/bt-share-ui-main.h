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

#ifndef __DEF_BT_SHARE_UI_MAIN_H_
#define __DEF_BT_SHARE_UI_MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <E_DBus.h>
#include <Elementary.h>
#include <dlog.h>
#include <glib.h>
#include <bluetooth-share-api.h>
#include <efl_extension.h>
#include <bluetooth.h>
#define EXPORT __attribute__((visibility("default")))

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.bluetooth-share-ui"
#endif

#if !defined(EDJFILE)
#define EDJDIR	"/usr/apps/org.tizen.bluetooth-share-ui/res/edje"
#define	EDJFILE	EDJDIR"/bt-share-layout.edj"
#define EDJ_IMAGES	EDJDIR "/images.edj"
#define EDJ_BT_ICON_ANIMATION EDJDIR "/bt_share_icon_animation.edj"
#endif


typedef enum {
	BT_TR_OUTBOUND,
	BT_TR_INBOUND
} bt_share_tr_type_e;

typedef struct {
	const char *transfer_type;
	int transfer_id;
} bt_share_abort_data_t;

typedef struct {
	/* common fields */
	const char *device_name;
	const char *device_address;
	const char *transfer_type;
	const char *db_sid;
	/* Progress specific */
	const char *filename;
	char percentage;
	int current_file;
	int total_files;
	unsigned long size;
	int transfer_id;
	/* Transfer specific */
	int success;
	int failed;
} bt_share_transfer_data_t;

typedef struct {
	bt_tr_data_t *tr_data;
	gboolean highlighted;
	gboolean tr_inbound;
} bt_gl_data_t;

typedef enum {
	BT_LAUNCH_NONE = -1,
	BT_LAUNCH_ONGOING,
	BT_LAUNCH_TRANSFER_LIST
} bt_share_launch_mode_t;

typedef struct {
	bt_share_launch_mode_t launch_mode;
	bt_adapter_state_e bt_status;
	Evas_Object *win;
	Eina_List *table;
	Evas_Object *bg;
	Evas_Object *conform;
	Evas_Object *tr_view;
	Evas_Object *navi_fr;
	Elm_Object_Item *navi_it;
	Evas_Object *toolbar_button;
	Evas_Object *toolbar_ly;

	/* Transfer Information */
	bt_share_transfer_data_t *transfer_info;

	Evas_Object *progress_layout;
	Evas_Object *progressbar;
	Elm_Object_Item *device_item;
	Elm_Object_Item *progress_item;
	Elm_Object_Item *status_item;
	Elm_Object_Item *file_title_item;
	Elm_Object_Item *current_item;

	/* Turning on popup */
	Evas_Object *turning_on_popup;

	/* Information Popup */
	Evas_Object *info_popup;

	/* Transfer data list*/
	Evas_Object *tr_genlist;
	Elm_Genlist_Item_Class *tr_device_itc;
	Elm_Genlist_Item_Class *tr_status_itc;
	Elm_Genlist_Item_Class *tr_progress_itc;
	Elm_Genlist_Item_Class *tr_file_title_itc;
	Elm_Genlist_Item_Class *tr_data_itc;

	GSList *tr_data_list;

	/* Idler */
	Ecore_Idler *idler;

	E_DBus_Connection *dbus_conn;
	/* Events from Service for OPP Client*/
	E_DBus_Signal_Handler *client_connected_sh;
	E_DBus_Signal_Handler *client_started_sh;
	E_DBus_Signal_Handler *client_progress_sh;
	E_DBus_Signal_Handler *client_completed_sh;
	/* Events from Service for OPP Server*/
	E_DBus_Signal_Handler *server_started_sh;
	E_DBus_Signal_Handler *server_progress_sh;
	E_DBus_Signal_Handler *server_completed_sh;
	/* Events from Bluetooth-Share*/
	E_DBus_Signal_Handler *update_sh;
	E_DBus_Signal_Handler *client_disconnected_sh;
	E_DBus_Signal_Handler *server_disconnected_sh;
	/* Events from Appcore*/
	E_DBus_Signal_Handler *app_core_sh;

	bt_share_tr_type_e tr_type;
	char *db_sid;

	int inbound_latest_id;
	int outbound_latest_id;
	bool opp_transfer_abort;
	bool send_after_turning_on;
} bt_share_appdata_t;

#ifdef __cplusplus
}
#endif
#endif				/* __DEF_BT_SHARE_UI_MAIN_H_ */
