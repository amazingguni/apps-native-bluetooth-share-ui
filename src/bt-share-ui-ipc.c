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

#include <dbus/dbus.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <notification.h>
#include <bluetooth-share-api.h>
#include "applog.h"
#include "bt-share-ui-main.h"
#include "bt-share-ui-ipc.h"
#include "bt-share-ui-view.h"
#include "bt-share-ui-popup.h"
#include "bt-share-ui-resource.h"
#include "bt-share-ui-widget.h"

#define BT_ADDRESS_LENGTH_MAX	6
#define DBUS_CORE_APPS_PATH "/Org/Tizen/Coreapps/home/raise"
#define DBUS_CORE_APPS_INTERFACE "org.tizen.coreapps.home.raise"
#define DBUS_CORE_APPS_MEMBER "homeraise"

static gboolean opc_launched_session;

void _bt_set_opc_launched_session(gboolean value)
{
	opc_launched_session = value;
}

static void __handle_opp_client_signal(void *data, DBusMessage *msg)
{
	bt_share_event_param_t event_info = { 0, };
	int result = BT_SHARE_UI_ERROR_NONE;
	const char *member = dbus_message_get_member(msg);

	retm_if(data == NULL, "Invalid argument: data is NULL\n");
	retm_if(msg == NULL, "Invalid argument: msg is NULL\n");
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	retm_if(ad->tr_type == BT_TR_INBOUND, "Invalid tr_type: BT_TR_INBOUND");
	if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_SIGNAL)
		return;

	if (!dbus_message_has_interface(msg, BT_EVENT_SERVICE))
		return;

	if (!dbus_message_has_path(msg, BT_OPP_CLIENT_PATH))
		return;

	ret_if(member == NULL);

	if (strcasecmp(member, BT_OPP_CONNECTED) == 0) {
		char *address = NULL;
		int request_id = 0;
		DBG("BT_OPP_CONNECTED signal");
		if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &address,
			DBUS_TYPE_INT32, &request_id,
			DBUS_TYPE_INVALID)) {
			ERR("Unexpected parameters in signal");
			return;
		}
		event_info.event = BT_SHARE_UI_EVENT_OPC_CONNECTED;
		event_info.param_data = address;
		event_info.result = result;
		event_info.user_data = data;
		DBG("Event BT_SHARE_UI_EVENT_OPC_CONNECTED");
		_bt_share_ui_event_handler(BT_SHARE_UI_EVENT_OPC_CONNECTED,
				&event_info, event_info.user_data);
	} else if (strcasecmp(member, BT_TRANSFER_STARTED) == 0) {
		char *file_path = NULL;
		char *file_name = NULL;
		char *device_addr = NULL;
		int request_id = 0;
		guint64 size = 0;
		bt_share_transfer_info_t transfer_info;

		if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &device_addr,
			DBUS_TYPE_STRING, &file_path,
			DBUS_TYPE_UINT64, &size,
			DBUS_TYPE_INT32, &request_id,
			DBUS_TYPE_INVALID)) {
			ERR("Unexpected parameters in signal");
			return;
		}

		file_name = g_path_get_basename(file_path);
		DBG("file_name:%s file_path:%s", file_name, file_path);

		memset(&transfer_info, 0x00, sizeof(bt_share_transfer_info_t));

		transfer_info.device_addr = g_strdup(device_addr);
		transfer_info.filename = g_strdup(file_name);
		transfer_info.size = size;
		transfer_info.percentage = 0;

		event_info.event = BT_SHARE_UI_EVENT_OPC_TRANSFER_STARTED;
		event_info.param_data = &transfer_info;
		event_info.result = result;
		event_info.user_data = data;

		_bt_share_ui_event_handler(
				BT_SHARE_UI_EVENT_OPC_TRANSFER_STARTED,
				&event_info, event_info.user_data);

		g_free(transfer_info.device_addr);
		g_free(transfer_info.filename);
		g_free(file_name);
	} else if (strcasecmp(member, BT_TRANSFER_PROGRESS) == 0) {
		char *file_name = NULL;
		int request_id = 0;
		guint64 size = 0;
		int progress = 0;
		bt_share_transfer_info_t transfer_info;

		if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &file_name,
			DBUS_TYPE_UINT64, &size,
			DBUS_TYPE_INT32, &progress,
			DBUS_TYPE_INT32, &request_id,
			DBUS_TYPE_INVALID)) {
			ERR("Unexpected parameters in signal");
			return;
		}

		memset(&transfer_info, 0x00, sizeof(bt_share_transfer_info_t));

		transfer_info.filename = g_strdup(file_name);
		transfer_info.size = size;
		transfer_info.percentage = progress;

		event_info.event = BT_SHARE_UI_EVENT_OPC_TRANSFER_PROGRESS;
		event_info.param_data = &transfer_info;
		event_info.result = result;
		event_info.user_data = data;

		_bt_share_ui_event_handler(
				BT_SHARE_UI_EVENT_OPC_TRANSFER_PROGRESS,
				&event_info, event_info.user_data);

		g_free(transfer_info.filename);
	} else if (strcasecmp(member, BT_TRANSFER_COMPLETED) == 0) {
		char *file_name = NULL;
		char *device_addr = NULL;
		int request_id = 0;
		guint64 size = 0;
		bt_share_transfer_info_t transfer_info;

		if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &device_addr,
			DBUS_TYPE_STRING, &file_name,
			DBUS_TYPE_UINT64, &size,
			DBUS_TYPE_INT32, &request_id,
			DBUS_TYPE_INVALID)) {
			ERR("Unexpected parameters in signal");
			return;
		}

		memset(&transfer_info, 0x00, sizeof(bt_share_transfer_info_t));

		transfer_info.device_addr = g_strdup(device_addr);
		transfer_info.filename = g_strdup(file_name);
		transfer_info.size = size;

		event_info.event = BT_SHARE_UI_EVENT_OPC_TRANSFER_COMPLETE;
		event_info.param_data = &transfer_info;
		event_info.result = result;
		event_info.user_data = data;

		_bt_share_ui_event_handler(
				BT_SHARE_UI_EVENT_OPC_TRANSFER_COMPLETE,
				&event_info, event_info.user_data);

		g_free(transfer_info.device_addr);
		g_free(transfer_info.filename);
	} else if (strcasecmp(member, BT_OPP_CLIENT_DISCONNECTED) == 0) {
		DBG("BT_OPP_CLIENT_DISCONNECTED signal");
		char *device_addr = NULL;
		int request_id = 0;

		if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &device_addr,
			DBUS_TYPE_INT32, &request_id,
			DBUS_TYPE_INVALID)) {
			ERR("Unexpected parameters in signal");
			return;
		}

		event_info.event = BT_SHARE_UI_EVENT_OPC_DISCONNECTED;
		event_info.param_data = device_addr;
		event_info.result = result;
		event_info.user_data = data;

		_bt_share_ui_event_handler(
				BT_SHARE_UI_EVENT_OPC_DISCONNECTED,
				&event_info, event_info.user_data);
	}
}

static void __handle_obex_server_signal(void *data, DBusMessage *msg)
{
	bt_share_event_param_t event_info = { 0, };
	int result = BT_SHARE_UI_ERROR_NONE;
	const char *member = dbus_message_get_member(msg);

	retm_if(data == NULL, "Invalid argument: data is NULL\n");
	retm_if(msg == NULL, "Invalid argument: msg is NULL\n");
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	retm_if(ad->tr_type == BT_TR_OUTBOUND, "Invalid tr_type: BT_TR_OUTBOUND");
	if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_SIGNAL)
		return;

	if (!dbus_message_has_interface(msg, BT_EVENT_SERVICE))
		return;

	if (!dbus_message_has_path(msg, BT_OPP_SERVER_PATH))
		return;

	retm_if(member == NULL, "member value is NULL\n");
	if (strcasecmp(member, BT_TRANSFER_STARTED) == 0) {
		char *file_name = NULL;
		char *type = NULL;
		char *device_name = NULL;
		char *device_addr = NULL;
		int transfer_id = 0;
		int server_type = 0; /* bt_server_type_t */
		guint64 size = 0;
		bt_share_server_transfer_info_t transfer_info;

		if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &device_name,
			DBUS_TYPE_STRING, &file_name,
			DBUS_TYPE_STRING, &type,
			DBUS_TYPE_STRING, &device_addr,
			DBUS_TYPE_UINT64, &size,
			DBUS_TYPE_INT32, &transfer_id,
			DBUS_TYPE_INT32, &server_type,
			DBUS_TYPE_INVALID)) {
			ERR("Unexpected parameters in signal");
			return;
		}

		memset(&transfer_info, 0x00,
			sizeof(bt_share_server_transfer_info_t));

		transfer_info.device_name = g_strdup(device_name);
		transfer_info.address = g_strdup(device_addr);
		transfer_info.filename = g_strdup(file_name);
		transfer_info.type = g_strdup(type);
		transfer_info.file_size = size;
		transfer_info.transfer_id = transfer_id;
		transfer_info.percentage = 0;

		event_info.event = BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_STARTED;
		event_info.param_data = &transfer_info;
		event_info.result = result;
		event_info.user_data = data;

		_bt_share_ui_event_handler(
				BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_STARTED,
				&event_info, event_info.user_data);

		g_free(transfer_info.device_name);
		g_free(transfer_info.address);
		g_free(transfer_info.filename);
		g_free(transfer_info.type);
	} else if (strcasecmp(member, BT_TRANSFER_PROGRESS) == 0) {
		char *file_name = NULL;
		char *type = NULL;
		char *device_name = NULL;
		char *device_addr = NULL;
		int transfer_id = 0;
		int progress = 0;
		int server_type = 0; /* bt_server_type_t */
		guint64 size = 0;
		bt_share_server_transfer_info_t transfer_info;

		if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &file_name,
			DBUS_TYPE_STRING, &type,
			DBUS_TYPE_STRING, &device_name,
			DBUS_TYPE_STRING, &device_addr,
			DBUS_TYPE_UINT64, &size,
			DBUS_TYPE_INT32, &transfer_id,
			DBUS_TYPE_INT32, &progress,
			DBUS_TYPE_INT32, &server_type,
			DBUS_TYPE_INVALID)) {
			ERR("Unexpected parameters in signal");
			return;
		}

		memset(&transfer_info, 0x00,
			sizeof(bt_share_server_transfer_info_t));

		transfer_info.device_name = g_strdup(device_name);
		transfer_info.address = g_strdup(device_addr);
		transfer_info.filename = g_strdup(file_name);
		transfer_info.type = g_strdup(type);
		transfer_info.file_size = size;
		transfer_info.transfer_id = transfer_id;
		transfer_info.percentage = progress;

		event_info.event = BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_PROGRESS;
		event_info.param_data = &transfer_info;
		event_info.result = result;
		event_info.user_data = data;

		_bt_share_ui_event_handler(
				BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_PROGRESS,
				&event_info, event_info.user_data);

		g_free(transfer_info.device_name);
		g_free(transfer_info.address);
		g_free(transfer_info.filename);
		g_free(transfer_info.type);
	} else if (strcasecmp(member, BT_TRANSFER_COMPLETED) == 0) {
		char *file_name = NULL;
		char *device_name = NULL;
		char *device_addr = NULL;
		char *type = NULL;
		char *file_path;
		int transfer_id = 0;
		int server_type = 0; /* bt_server_type_t */
		guint64 size = 0;
		bt_share_server_transfer_info_t transfer_info;

		if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &file_name,
			DBUS_TYPE_STRING, &type,
			DBUS_TYPE_STRING, &device_name,
			DBUS_TYPE_STRING, &file_path,
			DBUS_TYPE_STRING, &device_addr,
			DBUS_TYPE_UINT64, &size,
			DBUS_TYPE_INT32, &transfer_id,
			DBUS_TYPE_INT32, &server_type,
			DBUS_TYPE_INVALID)) {
			ERR("Unexpected parameters in signal");
			return;
		}

		memset(&transfer_info, 0x00,
				sizeof(bt_share_server_transfer_info_t));

		transfer_info.filename = g_strdup(file_name);
		transfer_info.type = g_strdup(type);
		transfer_info.device_name = g_strdup(device_name);
		transfer_info.address = g_strdup(device_addr);
		transfer_info.file_path = g_strdup(file_path);
		transfer_info.file_size = size;
		transfer_info.transfer_id = transfer_id;

		event_info.event = BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_COMPLETED;
		event_info.param_data = &transfer_info;
		event_info.result = result;
		event_info.user_data = data;

		_bt_share_ui_event_handler(
				BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_COMPLETED,
				&event_info, event_info.user_data);

		g_free(transfer_info.filename);
		g_free(transfer_info.type);
		g_free(transfer_info.device_name);
		g_free(transfer_info.address);
		g_free(transfer_info.file_path);
	}
}

static void __handle_update_view_signal(void *data, DBusMessage * msg)
{
	bt_share_appdata_t *ad;
	char *db;

	retm_if(data == NULL, "Invalid argument: data is NULL\n");
	retm_if(msg == NULL, "Invalid argument: msg is NULL\n");

	ad = (bt_share_appdata_t *)data;

	DBG("+");

	if (!dbus_message_get_args(msg, NULL,
				DBUS_TYPE_STRING, &db,
				DBUS_TYPE_INVALID)) {
		ERR("Event handling failed");
		return;
	}

	_bt_share_ui_handle_update_view(ad, db);
}

static void __handle_opp_disconnect_signal(void *data, DBusMessage * msg)
{
	DBG("+");
	bt_share_event_param_t event_info = { 0, };
	bt_share_server_transfer_info_t transfer_info = { 0, };
	char *address = NULL;
	char *member = NULL;
	int result = BT_SHARE_UI_ERROR_NONE;
	int transfer_id = -1;

	retm_if(data == NULL, "Invalid argument: data is NULL\n");
	retm_if(msg == NULL, "Invalid argument: msg is NULL\n");

	member = (char *)dbus_message_get_member(msg);
	retm_if(member == NULL, "Invalid argument: member is NULL\n");

	if (strcasecmp(member, BT_OPP_CLIENT_DISCONNECTED) == 0) {
		DBG("BT_OPP_CLIENT_DISCONNECTED");
		event_info.event = BT_SHARE_UI_EVENT_OPC_DISCONNECTED;
	} else if (strcasecmp(member, BT_OPP_SERVER_DISCONNECTED) == 0) {
		DBG("BT_OPP_SERVER_DISCONNECTED");
		event_info.event = BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_DISCONNECTED;
	} else {
		DBG("Unhandled Signal");
		return;
	}

	if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &address,
			DBUS_TYPE_INT32, &transfer_id,
			DBUS_TYPE_INVALID)) {
		ERR("get_args failed");
		return;
	}

	transfer_info.file_path = address;
	transfer_info.transfer_id = transfer_id;

	event_info.result = result;
	event_info.user_data = data;
	event_info.param_data = &transfer_info;

	_bt_share_ui_event_handler(event_info.event, &event_info,
			event_info.user_data);

	DBG("-");
}

static void __handle_home_key_signal(void *data, DBusMessage *msg)
{
	const char *member;

	retm_if(data == NULL, "Invalid argument: data is NULL");
	retm_if(msg == NULL, "Invalid argument: msg is NULL");

	member = dbus_message_get_member(msg);
	retm_if(member == NULL, "member value is NULL");

	if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_SIGNAL)
		return;

	if (!dbus_message_has_interface(msg, DBUS_CORE_APPS_INTERFACE) ||
		!dbus_message_has_path(msg, DBUS_CORE_APPS_PATH))
		return;

	DBG("Received signal : %s", member);

	if (strcasecmp(member, DBUS_CORE_APPS_MEMBER) == 0)
		_bt_terminate_app();
}

static void __bt_add_tr_data_list(bt_share_appdata_t *ad, int transfer_type)
{
	GSList *list = NULL;
	GSList *next = NULL;
	sqlite3 *db = NULL;
	bt_tr_data_t *info = NULL;
	bt_tr_data_t *tr_data = NULL;
	int success = 0;
	int failed = 0;
	int len = 0;
	int i;
	retm_if(!ad, "ad is NULL!");

	GSList *tr_data_list = ad->tr_data_list;
	retm_if(!tr_data_list, "tr_data_list is NULL!");

	Elm_Object_Item *git = elm_genlist_item_next_get(ad->file_title_item);

	db = bt_share_open_db();
	if (!db)
		return;

	list = bt_share_get_all_tr_data_by_sid(db, transfer_type,
			ad->transfer_info->device_address, ad->transfer_info->db_sid);
	bt_share_close_db(db);
	retm_if(!list, "fail to get tr list!");

	/* Append new tr data to tr_data_list */
	len = g_slist_length(list);
	next = list;
	for (i = 0; i < len; i++) {
		info = next->data;

		if (info == NULL) {
			next = g_slist_next(next);
			tr_data_list = g_slist_next(tr_data_list);
			if (tr_data_list == NULL)
				break;

			if (git)
				git = elm_genlist_item_next_get(git);

			if (next == NULL)
				break;
			else
				continue;
		}

		if (info->tr_status == BT_TRANSFER_SUCCESS)
			success++;
		else if (info->tr_status == BT_TRANSFER_FAIL)
			failed++;

		DBG_SECURE("ID :%d Status:%d Filename :%s", info->id,
				info->tr_status, info->file_path);


		if (transfer_type == BT_TR_OUTBOUND &&
				info->id > ad->outbound_latest_id) {
			tr_data = g_new0(bt_tr_data_t, 1);
			tr_data->id = info->id;
			tr_data->sid = info->sid;
			tr_data->tr_status = info->tr_status;
			tr_data->file_path = g_strdup(info->file_path);
			tr_data->dev_name = g_strdup(info->dev_name);
			tr_data->timestamp = info->timestamp;
			tr_data->addr = g_strdup(info->addr);
			tr_data->type = g_strdup(info->type);
			tr_data->content = g_strdup(info->content);
			tr_data->size = info->size;
			ad->tr_data_list = g_slist_append(ad->tr_data_list, tr_data);
		} else if (transfer_type == BT_TR_INBOUND &&
				info->id > ad->inbound_latest_id) {
			tr_data = g_new0(bt_tr_data_t, 1);
			tr_data->id = info->id;
			tr_data->sid = info->sid;
			tr_data->tr_status = info->tr_status;
			tr_data->file_path = g_strdup(info->file_path);
			tr_data->dev_name = g_strdup(info->dev_name);
			tr_data->timestamp = info->timestamp;
			tr_data->addr = g_strdup(info->addr);
			tr_data->type = g_strdup(info->type);
			tr_data->content = g_strdup(info->content);
			tr_data->size = info->size;
			ad->tr_data_list = g_slist_append(ad->tr_data_list, tr_data);
		} else {
			// Update data in list
			bt_tr_data_t *list_info = NULL;
			list_info = tr_data_list->data;

			if (list_info) {
				if (list_info->id == info->id) {
					list_info->timestamp = info->timestamp;
					list_info->tr_status = info->tr_status;
					list_info->size = info->size;
					if (list_info->type == NULL)
						list_info->type = g_strdup(info->type);

					if (list_info->tr_status == BT_TRANSFER_ONGOING)
						ad->current_item = git;
				}
			}
		}

		next = g_slist_next(next);
		if (next == NULL)
			break;

		tr_data_list = g_slist_next(tr_data_list);

		if (git)
			git = elm_genlist_item_next_get(git);
	}

	ad->transfer_info->success = success;
	ad->transfer_info->failed = failed;
	DBG("SUCCESS:%d, FAILED:%d", success, failed);
	if (ad->status_item)
		elm_genlist_item_fields_update(ad->status_item, "*",
				ELM_GENLIST_ITEM_FIELD_TEXT);

	bt_share_release_tr_data_list(list);
}

void _bt_signal_init(bt_share_appdata_t *ad)
{
	E_DBus_Connection *conn = NULL;
	E_DBus_Signal_Handler *sh = NULL;
	e_dbus_init();

	conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
	retm_if(conn == NULL, "conn is NULL\n");

	e_dbus_request_name(conn, BT_SHARE_UI_INTERFACE, 0, NULL, NULL);

	sh = e_dbus_signal_handler_add(conn,
				       NULL,
				       BT_OPP_CLIENT_PATH,
				       BT_EVENT_SERVICE,
				       BT_OPP_CONNECTED,
				       __handle_opp_client_signal, ad);
	retm_if(sh == NULL, "Connect Event register failed\n");
	ad->client_connected_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
				       NULL,
				       BT_OPP_CLIENT_PATH,
				       BT_EVENT_SERVICE,
				       BT_TRANSFER_STARTED,
				       __handle_opp_client_signal, ad);
	retm_if(sh == NULL, "started Event register failed\n");
	ad->client_started_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
				       NULL,
				       BT_OPP_CLIENT_PATH,
				       BT_EVENT_SERVICE,
				       BT_TRANSFER_PROGRESS,
				       __handle_opp_client_signal, ad);
	retm_if(sh == NULL, "progress Event register failed\n");
	ad->client_progress_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
				       NULL,
				       BT_OPP_CLIENT_PATH,
				       BT_EVENT_SERVICE,
				       BT_TRANSFER_COMPLETED,
				       __handle_opp_client_signal, ad);
	retm_if(sh == NULL, "complete Event register failed\n");
	ad->client_completed_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
				       NULL,
				       BT_OPP_SERVER_PATH,
				       BT_EVENT_SERVICE,
				       BT_TRANSFER_STARTED,
				       __handle_obex_server_signal, ad);
	retm_if(sh == NULL, "started Event register failed\n");
	ad->server_started_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
				       NULL,
				       BT_OPP_SERVER_PATH,
				       BT_EVENT_SERVICE,
				       BT_TRANSFER_PROGRESS,
				       __handle_obex_server_signal, ad);
	retm_if(sh == NULL, "progress Event register failed\n");
	ad->server_progress_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
				       NULL,
				       BT_OPP_SERVER_PATH,
				       BT_EVENT_SERVICE,
				       BT_TRANSFER_COMPLETED,
				       __handle_obex_server_signal, ad);
	retm_if(sh == NULL, "complete Event register failed\n");
	ad->server_completed_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
					NULL,
					BT_SHARE_ENG_OBJECT,
					BT_SHARE_ENG_INTERFACE,
					BT_SHARE_ENG_SIGNAL_UPDATE_VIEW,
					__handle_update_view_signal, ad);

	retm_if(sh == NULL, "Progress Event register failed\n");
	ad->update_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
					   NULL,
					   BT_SHARE_ENG_OBJECT,
					   BT_SHARE_ENG_INTERFACE,
					   BT_OPP_CLIENT_DISCONNECTED,
					   __handle_opp_disconnect_signal, ad);
	retm_if(sh == NULL, "Disconnected Event register failed\n");
	ad->client_disconnected_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
					   NULL,
					   BT_SHARE_ENG_OBJECT,
					   BT_SHARE_ENG_INTERFACE,
					   BT_OPP_SERVER_DISCONNECTED,
					   __handle_opp_disconnect_signal, ad);
	retm_if(sh == NULL, "Disconnected Event register failed\n");
	ad->server_disconnected_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
					NULL,
					DBUS_CORE_APPS_PATH,
					DBUS_CORE_APPS_INTERFACE,
					DBUS_CORE_APPS_MEMBER,
					__handle_home_key_signal, ad);
	retm_if(sh == NULL, "Connect Event register failed");
	ad->app_core_sh = sh;

	ad->dbus_conn = conn;

	return;
}

void _bt_signal_deinit(bt_share_appdata_t *ad)
{
	ret_if(ad == NULL);

	if (ad->client_connected_sh) {
		e_dbus_signal_handler_del(ad->dbus_conn, ad->client_connected_sh);
		ad->client_connected_sh = NULL;
	}
	if (ad->client_started_sh) {
		e_dbus_signal_handler_del(ad->dbus_conn, ad->client_started_sh);
		ad->client_started_sh = NULL;
	}
	if (ad->client_progress_sh) {
		e_dbus_signal_handler_del(ad->dbus_conn, ad->client_progress_sh);
		ad->client_progress_sh = NULL;
	}
	if (ad->client_completed_sh) {
		e_dbus_signal_handler_del(ad->dbus_conn, ad->client_completed_sh);
		ad->client_completed_sh = NULL;
	}
	if (ad->server_started_sh) {
		e_dbus_signal_handler_del(ad->dbus_conn, ad->server_started_sh);
		ad->server_started_sh = NULL;
	}
	if (ad->server_progress_sh) {
		e_dbus_signal_handler_del(ad->dbus_conn, ad->server_progress_sh);
		ad->server_progress_sh = NULL;
	}
	if (ad->server_completed_sh) {
		e_dbus_signal_handler_del(ad->dbus_conn, ad->server_completed_sh);
		ad->server_completed_sh = NULL;
	}
	if (ad->client_disconnected_sh) {
		e_dbus_signal_handler_del(ad->dbus_conn, ad->client_disconnected_sh);
		ad->client_disconnected_sh = NULL;
	}
	if (ad->server_disconnected_sh) {
		e_dbus_signal_handler_del(ad->dbus_conn, ad->server_disconnected_sh);
		ad->server_disconnected_sh = NULL;
	}
	if (ad->update_sh) {
		e_dbus_signal_handler_del(ad->dbus_conn, ad->update_sh);
		ad->update_sh = NULL;
	}
	if (ad->app_core_sh) {
		e_dbus_signal_handler_del(ad->dbus_conn, ad->app_core_sh);
		ad->app_core_sh = NULL;
	}
	return;
}

int _bt_abort_signal_send(bt_share_appdata_t *ad,
		bt_share_abort_data_t *abort_data)
{
	DBG("+");
	DBusMessage *msg = NULL;

	retvm_if(abort_data == NULL, -1,
		 "progressbar data is NULL\n");
	retvm_if(ad->dbus_conn == NULL, -1,
		 "Invalid argument: ad->dbus_conn is NULL\n");

	msg = dbus_message_new_signal(BT_SHARE_ENG_OBJECT,
			BT_SHARE_UI_INTERFACE,
			BT_SHARE_UI_SIGNAL_OPPABORT);

	retvm_if(msg == NULL, -1, "msg is NULL\n");

	if (!dbus_message_append_args(msg,
			DBUS_TYPE_STRING, &abort_data->transfer_type,
			DBUS_TYPE_INT32, &abort_data->transfer_id,
			DBUS_TYPE_INVALID)) {
		ERR("Abort sending failed");
		dbus_message_unref(msg);
		return -1;
	}

	ad->opp_transfer_abort = TRUE; /* Transfer aborted by user */

	INFO("abort_data->transfer_type = %s", abort_data->transfer_type);
	INFO("abort_data->transfer_id = %d", abort_data->transfer_id);

	e_dbus_message_send(ad->dbus_conn, msg, NULL, -1, NULL);
	dbus_message_unref(msg);
	DBG("-");
	return 0;
}

static void __bt_conv_addr_string_to_addr_type(char *addr,
						  const char *address)
{
	int i;
	char *ptr = NULL;

	if (!address || !addr)
		return;

	for (i = 0; i < BT_ADDRESS_LENGTH_MAX; i++) {
		addr[i] = strtol(address, &ptr, 16);
		if (ptr != NULL) {
			if (ptr[0] != ':')
				return;

			address = ptr + 1;
		}
	}
}

int _bt_share_ui_retry_failed(bt_share_appdata_t *ad)
{
	DBG("+");
	DBusMessage *msg = NULL;
	DBusMessageIter iter;
	DBusMessageIter array_iter;
	DBusMessageIter file_iter;
	DBusMessageIter filepath_iter;
	char *bd_addr = NULL;
	char **file_path = NULL;
	int i;
	int count;
	bt_tr_data_t *info = NULL;
	int files_count;
	int valid_files_count;
	GSList *current = NULL;
	GSList *failed = NULL;

	DBG_SECURE("Device:%s SID:%s", ad->transfer_info->device_address,
			ad->transfer_info->db_sid);

	failed = ad->tr_data_list;

	retvm_if(failed == NULL, -1, "Invalid argument: info is NULL\n");

	retvm_if(ad->dbus_conn == NULL, -1,
			"Invalid argument: ad->dbus_conn is NULL\n");

	files_count = g_slist_length(failed);
	msg = dbus_message_new_signal(BT_SHARE_ENG_OBJECT,
						BT_SHARE_UI_INTERFACE,
						BT_SHARE_UI_SIGNAL_SEND_FILE);

	if (msg == NULL) {
		ERR("msg is NULL");
		return -1;
	}

	bd_addr = g_new0(char, BT_ADDRESS_LENGTH_MAX);
	__bt_conv_addr_string_to_addr_type((char *)bd_addr,
			ad->transfer_info->device_address);

	file_path = g_new0(char *, files_count);
	current = failed;
	count = 0;
	DBG("Total files: %d", files_count);

	while (current && (count < files_count)) {
		info = current->data;

		if (info->tr_status != BT_TRANSFER_FAIL) {
			current = g_slist_next(current);
			continue;
		}

		/* check for valid utf8 file*/
		if (!g_utf8_validate(info->file_path, -1, NULL)) {
			ERR_SECURE("Invalid filepath: %s", info->file_path);
			notification_status_message_post(BT_STR_UNABLE_TO_SEND);
			goto done;
		}

		/* file validation check begin*/
		if (access(info->file_path, F_OK) != 0) {
			ERR_SECURE("access failed for %s. May be file is deleted from the Device", info->file_path);
			notification_status_message_post(BT_STR_FILE_NOT_EXIST);
			goto done;
		}
		/* file validation check end*/

		file_path[count] = g_strdup(info->content);

		current = g_slist_next(current);
		count++;
	}

	dbus_message_iter_init_append(msg, &iter);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
				DBUS_TYPE_BYTE_AS_STRING, &array_iter);

	for (i = 0; i < BT_ADDRESS_LENGTH_MAX; i++) {
		dbus_message_iter_append_basic(&array_iter,
				DBUS_TYPE_BYTE, &bd_addr[i]);
	}
	dbus_message_iter_close_container(&iter, &array_iter);

	if (info) {
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING,
							&info->dev_name);
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING,
							&info->type);
	}

	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
				DBUS_TYPE_ARRAY_AS_STRING
				DBUS_TYPE_BYTE_AS_STRING, &file_iter);

	if (file_path) {
		valid_files_count = count;
		for (count = 0; count < valid_files_count; count++) {
			int file_length;
			dbus_message_iter_open_container(&file_iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_BYTE_AS_STRING, &filepath_iter);
			file_length = strlen(file_path[count]);
			for (i = 0; i < file_length; i++) {
				dbus_message_iter_append_basic(&filepath_iter, DBUS_TYPE_BYTE,
						&file_path[count][i]);
			}
			dbus_message_iter_close_container(&file_iter, &filepath_iter);
		}
	}
	dbus_message_iter_close_container(&iter, &file_iter);

	e_dbus_message_send(ad->dbus_conn, msg, NULL, -1, NULL);

done:
	dbus_message_unref(msg);
	g_free(bd_addr);
	if (file_path) {
		for (count = 0; count < files_count; count++)
			g_free(file_path[count]);
		g_free(file_path);
	}

	DBG("-");
	return 0;
}

int _bt_share_ui_ipc_info_update(bt_share_appdata_t *ad, int uid)
{
	DBG("+");
	retv_if(ad == NULL, -1);

	DBusMessage *msg = NULL;
	int info_uid = 0;
	int info_type = 0;

	info_uid = uid;
	info_type = ad->tr_type;

	INFO("info_uid = %d", info_uid);
	INFO("info_type = %d", info_type);

	retvm_if(ad->dbus_conn == NULL, -1,
			"Invalid argument: ad->dbus_conn is NULL\n");

	msg = dbus_message_new_signal(BT_SHARE_ENG_OBJECT,
				      BT_SHARE_UI_INTERFACE,
				      BT_SHARE_UI_SIGNAL_INFO_UPDATE);

	retvm_if(msg == NULL, -1, "msg is NULL\n");

	if (!dbus_message_append_args(msg,
				      DBUS_TYPE_INT32, &info_uid,
				      DBUS_TYPE_INT32, &info_type,
				      DBUS_TYPE_INVALID)) {
		ERR("Connect sending failed");
		dbus_message_unref(msg);
		return -1;
	}

	e_dbus_message_send(ad->dbus_conn, msg, NULL, -1, NULL);
	dbus_message_unref(msg);

	DBG("-");
	return 0;
}

void _bt_share_ui_handle_update_view(bt_share_appdata_t *ad,
						char *table)
{
	GSList *list_iter = NULL;
	bt_tr_data_t *info = NULL;
	int transfer_type;
	Elm_Object_Item *git;
	DBG("+");

	if (g_strcmp0(table, BT_INBOUND_TABLE) == 0)
		transfer_type = BT_TR_INBOUND;
	else if (g_strcmp0(table, BT_OUTBOUND_TABLE) == 0)
		transfer_type = BT_TR_OUTBOUND;
	else
		return;

	__bt_add_tr_data_list(ad, transfer_type);

	/* Insert new transfer result to first genlist item  */
	list_iter = ad->tr_data_list;
	if (transfer_type == BT_TR_INBOUND && ad->tr_type == BT_TR_INBOUND) {
		while (NULL != list_iter) {
			info = list_iter->data;
			if (info->id > ad->inbound_latest_id) {
				_bt_genlist_append_tr_data_item
						(ad, info, transfer_type);
			}

			list_iter = g_slist_next(list_iter);
		}
	} else if (transfer_type == BT_TR_OUTBOUND &&
		ad->tr_type == BT_TR_OUTBOUND) {
		while (NULL != list_iter) {
			info = list_iter->data;
			if (info->id > ad->outbound_latest_id) {
				_bt_genlist_append_tr_data_item
						(ad, info, transfer_type);
			}
			list_iter = g_slist_next(list_iter);
		}
	}

	/* Update Finished and Processing Items */
	if (ad->current_item) {
		_bt_share_genlist_item_content_update(ad->current_item, BT_SHARE_ITEM_PART_FILE_TRANSFER_STATUS_ICON);
		_bt_share_genlist_item_text_update(ad->current_item, BT_SHARE_ITEM_PART_FILE_TRANSFER_STATUS_TEXT);
		_bt_share_genlist_item_text_update(ad->current_item, BT_SHARE_ITEM_PART_FILE_SIZE);

		git = elm_genlist_item_prev_get(ad->current_item);
		_bt_share_genlist_item_content_update(git, BT_SHARE_ITEM_PART_FILE_TRANSFER_STATUS_ICON);
		_bt_share_genlist_item_text_update(git, BT_SHARE_ITEM_PART_FILE_TRANSFER_STATUS_TEXT);
		_bt_share_genlist_item_text_update(git, BT_SHARE_ITEM_PART_FILE_SIZE);
	}

	evas_object_show(ad->tr_genlist);
	DBG("-");
}

static void __bt_share_ui_handle_progress(bt_share_appdata_t *ad,
					int transfer_id, char *name, int percentage,
					gboolean completed, int error_type)
{

	if (completed == FALSE)
		_bt_update_progressbar(ad, transfer_id, name, percentage);
}
static void __bt_share_ui_handle_error(bt_share_appdata_t *ad, int error_type,
						bt_share_tr_type_e trans_type)
{
	DBG("Error type : %d", error_type);

	switch (error_type) {
	case BT_SHARE_UI_ERROR_NOT_CONNECTED:
		_bt_create_info_popup(ad, BT_STR_UNABLE_TO_SEND);
		break;
	case BT_SHARE_UI_ERROR_CANCEL:
		DBG("opp_transfer_abort by user:%d, trans_type = %d",
			ad->opp_transfer_abort, ad->tr_type);
		if ((trans_type == BT_TR_INBOUND) &&
			!ad->opp_transfer_abort)
			_bt_create_info_popup(ad, BT_STR_UNABLE_TO_RECEIVE);
		else if (!ad->opp_transfer_abort &&
					(trans_type == BT_TR_OUTBOUND))
			_bt_create_info_popup(ad, BT_STR_UNABLE_TO_SEND);
		ad->opp_transfer_abort = FALSE;
		break;
	default:
		break;
	}
}

void _bt_share_ui_event_handler(int event, bt_share_event_param_t *param,
			       void *user_data){

	FN_START;
	bt_share_server_transfer_info_t *transfer_info = NULL;
	bt_share_transfer_info_t *client_info = NULL;
	char *name = NULL;
	int percentage = 0;
	bt_share_appdata_t *ad = (bt_share_appdata_t *)user_data;

	switch (event) {
	case BT_SHARE_UI_EVENT_OPC_CONNECTED:
		INFO("BT_SHARE_UI_EVENT_OPC_CONNECTED");
		if (param->result != BT_SHARE_UI_ERROR_NONE) {
			__bt_share_ui_handle_error(ad, param->result,
					BT_TR_OUTBOUND);
			_bt_share_ui_handle_update_view(ad, BT_OUTBOUND_TABLE);
		}

		/* Set to false when new Outbound OPP session is started*/
		opc_launched_session = FALSE;
		break;

	case BT_SHARE_UI_EVENT_OPC_TRANSFER_STARTED:
		if (opc_launched_session == FALSE) {
			/* Do not process events for new OPP Session */
			INFO("TRANSFER_STARTED: Different Obex Session");
			return;
		}

		INFO("BT_SHARE_UI_EVENT_OPC_TRANSFER_STARTED");
		if (param->result == BT_SHARE_UI_ERROR_NONE) {
			client_info = (bt_share_transfer_info_t *)param->param_data;
			if (client_info) {
				_bt_share_ui_handle_update_view(ad, BT_OUTBOUND_TABLE);
				_bt_share_ui_handle_transfer_started(ad,
					client_info->device_addr, client_info->filename,
					client_info->size, 0, BT_TR_OUTBOUND);
			}
		}
		break;

	case BT_SHARE_UI_EVENT_OPC_TRANSFER_PROGRESS:
		if (opc_launched_session == FALSE) {
			/* Do not process events for new OPP Session */
			return;
		}

		client_info = (bt_share_transfer_info_t *)param->param_data;

		name =  strrchr(client_info->filename, '/');
		if (name)
			name++;
		else
			name = client_info->filename;

		percentage = client_info->percentage;
		__bt_share_ui_handle_progress(ad, 0, name, percentage,
					FALSE, BT_SHARE_UI_ERROR_NONE);
		break;

	case BT_SHARE_UI_EVENT_OPC_TRANSFER_COMPLETE:
		if (opc_launched_session == FALSE) {
			/* Do not process events for new OPP Session */
			INFO("TRANSFER_COMPLETE: Different Obex Session");
			return;
		}

		INFO("BT_SHARE_UI_EVENT_OPC_TRANSFER_COMPLETE ");
		client_info = (bt_share_transfer_info_t *)param->param_data;

		if (g_strcmp0(ad->transfer_info->device_address, client_info->device_addr))
			return;

		if (param->result != BT_SHARE_UI_ERROR_NONE) {
			__bt_share_ui_handle_error(ad, param->result,
					BT_TR_OUTBOUND);
			if (ad->tr_view)
				_bt_share_ui_handle_update_view(ad, BT_OUTBOUND_TABLE);
			else if (ad->info_popup == NULL)
				_bt_terminate_app();

			_bt_share_ui_handle_transfer_complete(ad, client_info->device_addr,
					BT_TR_OUTBOUND);

			return;
		}

		INFO_SECURE("client_info->filename = [%s]",
				client_info->filename);

		name =  strrchr(client_info->filename, '/');
		if (name)
			name++;
		else
			name = client_info->filename;

		INFO("name address = [%x]", name);
		__bt_share_ui_handle_progress(ad, 0, name, 100, TRUE,
				param->result);

		if (ad->tr_view == NULL)
			_bt_terminate_app();
		else
			_bt_share_ui_handle_update_view(ad, BT_OUTBOUND_TABLE);

		_bt_share_ui_handle_transfer_complete(ad, client_info->device_addr,
				BT_TR_OUTBOUND);

		break;
	case BT_SHARE_UI_EVENT_OPC_DISCONNECTED: {
		GSList *failed = NULL;
		sqlite3 *db = NULL;

		INFO("BT_SHARE_UI_EVENT_OPC_DISCONNECTED");

		db = bt_share_open_db();
		retm_if(!db, "fail to open db!");

		failed = bt_share_get_failed_tr_data_by_sid(db,
				ad->tr_type, ad->transfer_info->device_address,
				ad->transfer_info->db_sid);
		bt_share_close_db(db);

		if (failed) {
			/* Some failed items are there */
			elm_object_text_set(ad->toolbar_button,
						BT_STR_RESEND_FAILED_FILES);
			elm_object_disabled_set(ad->toolbar_button, FALSE);
			/*free list*/
			bt_share_release_tr_data_list(failed);
		} else
			_bt_share_delete_toolbar_button(ad);
		ad->launch_mode = BT_LAUNCH_TRANSFER_LIST;

		_bt_share_ui_handle_transfer_disconnected(ad, BT_TR_OUTBOUND);
		_bt_share_ui_handle_update_view(ad, BT_OUTBOUND_TABLE);

		elm_genlist_realized_items_update(ad->tr_genlist);

		break;
	}
	case BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_STARTED:
		INFO("BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_STARTED");
		if (param->result == BT_SHARE_UI_ERROR_NONE) {
			transfer_info = param->param_data;

			if (transfer_info->transfer_id != ad->transfer_info->transfer_id) {
				/* Different session */
				INFO("TRANSFER_STARTED: Different Session");
				return;
			}

			_bt_share_ui_handle_transfer_started(ad, transfer_info->address,
					transfer_info->filename, transfer_info->file_size, transfer_info->transfer_id,
					BT_TR_INBOUND);
		}
		break;
	case BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_PROGRESS:
		if (param->result == BT_SHARE_UI_ERROR_NONE) {
			transfer_info = param->param_data;

			if (transfer_info->transfer_id != ad->transfer_info->transfer_id) {
				/* Different session */
				return;
			}
			__bt_share_ui_handle_progress(ad,
					transfer_info->transfer_id,
					transfer_info->filename,
					transfer_info->percentage,
					FALSE, BT_SHARE_UI_ERROR_NONE);
		}
		break;

	case BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_COMPLETED:
		INFO("BT_SHARE_UI_EVENT_OBEX_TRANSFER_COMPLETED ");

		transfer_info = param->param_data;
		if (g_strcmp0(ad->transfer_info->device_address, transfer_info->address))
			return;

		if (transfer_info->transfer_id != ad->transfer_info->transfer_id) {
			/* Different session */
			INFO("TRANSFER_STARTED: Different Session");
			return;
		}

		if (param->result != BT_SHARE_UI_ERROR_NONE) {
			__bt_share_ui_handle_error(ad, param->result,
					BT_TR_INBOUND);
			if (ad->tr_view)
				_bt_share_ui_handle_update_view(ad,
						BT_INBOUND_TABLE);

			_bt_share_ui_handle_transfer_complete(ad, transfer_info->address,
					BT_TR_INBOUND);
			return;
		}

		if (ad->tr_view == NULL)
			_bt_terminate_app();
		else
			_bt_share_ui_handle_update_view(ad, BT_INBOUND_TABLE);

		_bt_share_ui_handle_transfer_complete(ad, transfer_info->address,
				BT_TR_INBOUND);
		break;

	case BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_DISCONNECTED: {
		DBG("BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_DISCONNECTED");
		transfer_info = param->param_data;
		INFO("transfer_info->transfer_id: [%d]", transfer_info->transfer_id);
		INFO("ad->transfer_info->transfer_id: [%d]", ad->transfer_info->transfer_id);
		if (transfer_info->transfer_id == ad->transfer_info->transfer_id) {
			DBG("Same session");
			_bt_share_delete_toolbar_button(ad);
			_bt_share_ui_handle_transfer_disconnected(ad, BT_TR_INBOUND);

			_bt_share_ui_handle_update_view(ad, BT_INBOUND_TABLE);
			elm_genlist_realized_items_update(ad->tr_genlist);
		}
	}
	break;

	default:
		DBG("Unhandled event %x", event);
		break;
	}
	FN_END;

}
