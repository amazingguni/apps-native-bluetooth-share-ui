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

#include <stdio.h>
#include <app.h>
#include <app_control.h>
#include <app_control_internal.h>
#if 0
#include <Ecore_X.h>
#include <utilX.h>
#endif
#include <vconf.h>
#include <vconf-keys.h>
#include <E_DBus.h>
#if 0
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#endif
#include <aul.h>
#include <efl_extension.h>
#include "applog.h"
#include "bt-share-ui-main.h"
#include "bt-share-ui-view.h"
#include "bt-share-ui-popup.h"
#include "bt-share-ui-ipc.h"
#include "bt-share-ui-resource.h"
#include "bt-share-ui-widget.h"
#include <bluetooth-share-api.h>
#include <bluetooth.h>
#include <bluetooth_internal.h>
#include <bundle_internal.h>

bt_share_appdata_t *app_state = NULL;
bt_share_appdata_t app_data = {0,};

static void __bt_lang_changed_cb(app_event_info_h event_info, void *data)
{
	DBG("+");
	ret_if(data == NULL);

	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;

	if (ad->tr_genlist)
		elm_genlist_realized_items_update(ad->tr_genlist);

	if (ad->toolbar_button) {
		DBG("Changing language for toolbar_button: ad->launch_mode: %d",
				ad->launch_mode);

		if ((ad->tr_type == BT_TR_INBOUND) || (ad->launch_mode == BT_LAUNCH_ONGOING))
			elm_object_text_set(ad->toolbar_button, BT_STR_STOP);
		else
			elm_object_text_set(ad->toolbar_button, BT_STR_RESEND_FAILED_FILES);
	}

}

static bt_share_launch_mode_t __bt_parse_launch_mode(bt_share_appdata_t *ad, bundle *b)
{
	const char *launch_type = NULL;
	bt_share_launch_mode_t launch_mode = BT_LAUNCH_NONE;
	retvm_if(ad == NULL, -1, "Invalid param");
	retvm_if(b == NULL, -1, "Invalid param");

	launch_type = bundle_get_val(b, "launch-type");
	retv_if(!launch_type, -1);

	if (!strcasecmp(launch_type, "ongoing"))
		launch_mode = BT_LAUNCH_ONGOING;
	else if (!strcasecmp(launch_type, "transfer_list"))
		launch_mode = BT_LAUNCH_TRANSFER_LIST;
	else
		ERR("Invalid bundle value");

	return launch_mode;
}

static void __bt_share_free_tr_data(bt_share_transfer_data_t *tr_data)
{
	FN_START;
	ret_if(tr_data == NULL);

	g_free((char *) tr_data->device_name);
	g_free((char *) tr_data->device_address);
	g_free((char *) tr_data->transfer_type);
	g_free((char *) tr_data->db_sid);
	g_free((char *) tr_data->filename);
	g_free(tr_data);
	FN_END;
}

static int __bt_share_launch_handler(bt_share_appdata_t *ad, bundle *b,
						bt_share_launch_mode_t launch_mode)
{
	FN_START;
	const char *transfer_type = NULL;
	const char *temp = NULL;
	char *stop = NULL;
	unsigned char tranferred;
	int tr_type = 0;
	sqlite3 *db = NULL;

	retvm_if(ad == NULL, -1, "Invalid param");
	retvm_if(b == NULL, -1, "Invalid param");

	if (launch_mode == BT_LAUNCH_TRANSFER_LIST ||
		launch_mode == BT_LAUNCH_ONGOING) {
		INFO("%s", launch_mode == BT_LAUNCH_TRANSFER_LIST ?
				"BT_LAUNCH_TRANSFER_LIST" : "BT_LAUNCH_ONGOING");
		bt_share_transfer_data_t *transfer_data = NULL;

		elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
		elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);

		transfer_type = bundle_get_val(b, "transfer_type");
		retvm_if(!transfer_type, -1, "Invalid transfer type!");

		if (!strcmp(transfer_type, "outbound"))
			tr_type = BT_TR_OUTBOUND;
		else
			tr_type = BT_TR_INBOUND;

		/* Open the DB */
		db = bt_share_open_db();
		retvm_if(!db, -1, "fail to open db!");

		/* Create Transfer Data */
		transfer_data = g_new0(bt_share_transfer_data_t, 1);
		transfer_data->device_name = g_strdup(bundle_get_val(b, "device_name"));
		transfer_data->device_address = g_strdup(bundle_get_val(b, "device_addr"));
		transfer_data->transfer_type = g_strdup(bundle_get_val(b, "transfer_type"));
		transfer_data->db_sid = g_strdup(bundle_get_val(b, "db_sid"));

		INFO("Create Transfer data - Name: %s, Address :%s, type: %s, SID: %s",
				transfer_data->device_name, transfer_data->device_address,
				transfer_data->transfer_type, transfer_data->db_sid);

		if (launch_mode == BT_LAUNCH_TRANSFER_LIST) {
			temp = bundle_get_val(b, "transfer_id");
			if (temp != NULL) {
				transfer_data->transfer_id = atoi(temp);
				INFO_SECURE("Transfer ID: %d", transfer_data->transfer_id);

				if (tr_type == BT_TR_INBOUND) {
					int noti_id;
					DBG("INBOUND TRANSFER");

					/* Check if noti_id is updated or not. If noti_id is not updated it means
					 * it's an ongoing transfer. If its an ongoing transfer then redirect it to
					 * progress view, otherwise just launch the transfer list.
					 */
					noti_id = bt_share_get_noti_id(db, BT_DB_INBOUND, transfer_data->db_sid);

					DBG("noti_id: [%d]", noti_id);
					/* Get transfer progress only if its an ongoing transfer. */
					if (!noti_id && bt_opp_get_transfer_progress(BT_TRANSFER_INBOUND,
							transfer_data->transfer_id, &tranferred) == 0) {
						DBG("INBOUND ONGOING TRANSFER");
						launch_mode = BT_LAUNCH_ONGOING;
						transfer_data->percentage = tranferred;

						transfer_data->filename = g_strdup(bundle_get_val(b, "filename"));
						INFO_SECURE("File Name: %s", transfer_data->filename);
					}
				}
			}
			_bt_set_opc_launched_session(FALSE);
		} else {
			/* Create Progress Data */
			transfer_data->filename = g_strdup(bundle_get_val(b, "filename"));
			INFO_SECURE("Create Progress Data : File Name: %s", transfer_data->filename);
			temp = bundle_get_val(b, "size");
			if (temp != NULL)
				transfer_data->size = atol(temp);
			temp = bundle_get_val(b, "transfer_id");
			if (temp != NULL) {
				transfer_data->transfer_id = atoi(temp);
				INFO_SECURE("Transfer ID: %d", transfer_data->transfer_id);
			}

			temp = bundle_get_val(b, "progress_cnt");
			if (temp != NULL) {
				int current, total;
				INFO("PROGRESS TEXT: %s", temp);
				current = strtol(temp + 1, &stop, 10);
				total = strtol(stop + 1, &stop, 10);
				transfer_data->current_file = current;
				transfer_data->total_files = total;
			}
			INFO("File Progress: [%d/%d]", transfer_data->current_file,
					transfer_data->total_files);

			if (tr_type == BT_TR_OUTBOUND) {
				bt_opp_get_transfer_progress(BT_TRANSFER_OUTBOUND,
						-1, &tranferred);
				_bt_set_opc_launched_session(TRUE);
			} else {
				bt_opp_get_transfer_progress(BT_TRANSFER_INBOUND,
						transfer_data->transfer_id, &tranferred);
			}
			transfer_data->percentage = tranferred;
		}

		if (ad->tr_view && ad->tr_type == tr_type && ad->launch_mode == launch_mode &&
					g_strcmp0(ad->db_sid, transfer_data->db_sid) == 0) {
			ERR("Same view. no need to create transfer view!");

			/* Show the TOP of List */
			elm_genlist_item_show(ad->device_item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
			__bt_share_free_tr_data(transfer_data);
			bt_share_close_db(db);
			return 0;
		}

		__bt_share_free_tr_data(ad->transfer_info);
		ad->transfer_info = transfer_data;
		transfer_data->success = 0;
		transfer_data->failed = 0;
		ad->launch_mode = launch_mode;

		ad->tr_type = tr_type;

		/* Get data from share DB */
		if (ad->db_sid) {
			g_free(ad->db_sid);
			ad->db_sid = NULL;
		}
		ad->db_sid = g_strdup(transfer_data->db_sid);
		INFO_SECURE("Device Address: %s", transfer_data->device_address);
		ad->tr_data_list = bt_share_get_all_tr_data_by_sid(db,
				ad->tr_type, transfer_data->device_address,
				transfer_data->db_sid);

		bt_share_close_db(db);

		_bt_create_transfer_view(ad);

	} else {
		ERR("Invalid bundle value");
		return -1;
	}

	FN_END;
	return 0;
}

static bool __app_create(void *data)
{
	FN_START;
	bt_share_appdata_t *ad = data;
	Evas_Object *win = NULL;
	int ret;
	bt_adapter_state_e status = BT_ADAPTER_DISABLED;

	elm_app_base_scale_set(1.8);

	/* create window */
	win = _bt_create_win(PACKAGE);
	retv_if(win == NULL, false);
	ad->win = win;

	bindtextdomain(BT_COMMON_PKG, BT_COMMON_RES);

	if (bt_initialize() != BT_ERROR_NONE)
		ERR("bt_initialize() failed");

	if (bt_adapter_get_state(&status) != BT_ERROR_NONE)
		ERR("bt_adapter_get_state() failed!");
	DBG("bt_status : %d", status);

	ad->bt_status = status;

	/* Set event callbacks */
	ret =
	    bt_adapter_set_state_changed_cb(_bt_cb_state_changed, (void *)ad);
	if (ret != BT_ERROR_NONE)
		ERR("bt_adapter_set_state_changed_cb failed");
	FN_END;
	return true;
}

static void __app_service(app_control_h app_control, void *user_data)
{
	INFO("__app_service");
	bt_share_appdata_t *ad = user_data;
	int ret;
	bundle *b = NULL;
	bt_share_launch_mode_t launch_mode;
	ret_if(ad == NULL);

	ret = app_control_export_as_bundle(app_control, &b);

	if (ad->dbus_conn == NULL)
		_bt_signal_init(ad);

	launch_mode = __bt_parse_launch_mode(ad, b);
	if (launch_mode == BT_LAUNCH_NONE) {
		if (b)
			bundle_free(b);
		_bt_terminate_app();
		return;
	}

	ret = __bt_share_launch_handler(ad, b, launch_mode);
	if (ret < 0)
		_bt_terminate_app();

	evas_object_show(ad->win);
	elm_win_activate(ad->win);
	bundle_free(b);
}

static void __app_terminate(void *data)
{
	FN_START;
	bt_share_appdata_t *ad = data;
	int err;

	_bt_destroy_info_popup(ad);
	_bt_signal_deinit(ad);

	if (ad->tr_device_itc) {
		elm_genlist_item_class_free(ad->tr_device_itc);
		ad->tr_device_itc = NULL;
	}
	if (ad->tr_status_itc) {
		elm_genlist_item_class_free(ad->tr_status_itc);
		ad->tr_status_itc = NULL;
	}
	if (ad->tr_progress_itc) {
		elm_genlist_item_class_free(ad->tr_progress_itc);
		ad->tr_progress_itc = NULL;
	}
	if (ad->tr_file_title_itc) {
		elm_genlist_item_class_free(ad->tr_file_title_itc);
		ad->tr_file_title_itc = NULL;
	}
	if (ad->tr_data_itc) {
		elm_genlist_item_class_free(ad->tr_data_itc);
		ad->tr_data_itc = NULL;
	}
	if (ad->transfer_info) {
		__bt_share_free_tr_data(ad->transfer_info);
		ad->transfer_info = NULL;
	}

	if (ad->idler) {
		ecore_idler_del(ad->idler);
		ad->idler = NULL;
	}
	if (ad->db_sid) {
		g_free(ad->db_sid);
		ad->db_sid = NULL;
	}
	err = bt_adapter_unset_state_changed_cb();
	if (err != BT_ERROR_NONE)
		ERR("unset of state change cb  failed: %d", err);

	err = bt_deinitialize();
	if (err != BT_ERROR_NONE)
		ERR("bt_deinitialize failed: %d", err);
	FN_END;
}

static void __app_pause(void *data)
{
	INFO("__app_pause ");
}

static void __app_resume(void *data)
{
	INFO("__app_resume");
}

EXPORT int main(int argc, char *argv[])
{
	DBG("Start bluetooth-share-ui main()");

	ui_app_lifecycle_callback_s callback = {0,};
	app_event_handler_h lang_changed_handler;
	app_state = &app_data;

	callback.create = __app_create;
	callback.terminate = __app_terminate;
	callback.pause = __app_pause;
	callback.resume = __app_resume;
	callback.app_control = __app_service;

	ui_app_add_event_handler(&lang_changed_handler, APP_EVENT_LANGUAGE_CHANGED, __bt_lang_changed_cb, &app_data);

	DBG("ui_app_main() is called.");
	int ret = ui_app_main(argc, argv, &callback, &app_data);
	if (ret != APP_ERROR_NONE)
		ERR("ui_app_main() is failed. err = %d", ret);

	DBG("End bluetooth-share-ui main()");
	return ret;
}

