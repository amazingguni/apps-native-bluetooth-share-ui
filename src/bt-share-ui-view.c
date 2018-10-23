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

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if 0
#include <Ecore_X.h>
#include <utilX.h>
#endif
#include <Elementary.h>
#include <efl_extension.h>
#include <aul.h>
#include <app.h>
#include <vconf.h>
#include <vconf-keys.h>
#include <bluetooth-share-api.h>
#include <bluetooth_internal.h>
#include <notification.h>
#include <notification_internal.h>
#include <notification_list.h>

#include "applog.h"
#include "bt-share-ui-view.h"
#include "bt-share-ui-popup.h"
#include "bt-share-ui-ipc.h"
#include "bt-share-ui-resource.h"
#include "bt-share-ui-widget.h"
#include "bt-share-ui-main.h"

#define NOTI_OPS_APP_ID	"bluetooth-share-opp-server"
#define NOTI_OPC_APP_ID	"bluetooth-share-opp-client"

#define BT_DEFAULT_MEM_PHONE 0
#define BT_DEFAULT_MEM_MMC 1

#define BT_DOWNLOAD_PHONE_FOLDER "/opt/usr/media/Downloads"
#define BT_DOWNLOAD_MMC_FOLDER "/opt/media/SDCardA1"
#define BT_CONTACT_SHARE_TMP_DIR BT_DOWNLOAD_PHONE_FOLDER "/.bluetooth"

extern bt_share_appdata_t *app_state;

static Evas_Object *__bt_add_tr_data_genlist(Evas_Object *parent,
						  bt_share_appdata_t *ad);

void _bt_delete_selected_notification(bt_share_tr_type_e tr_type,
					int noti_id, const char *opp_role)
{
	notification_list_h list_head;
	notification_list_h list_traverse;
	notification_h noti;
	int priv_id;

	notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &list_head);
	list_traverse = list_head;

	while (list_traverse != NULL) {
		noti = notification_list_get_data(list_traverse);
		notification_get_id(noti, NULL, &priv_id);
		if (priv_id == noti_id) {
			DBG_SECURE("Deleting Notification ID: %d", noti_id);
			if (notification_delete_by_priv_id(opp_role,
					NOTIFICATION_TYPE_NOTI, priv_id)
					!= NOTIFICATION_ERROR_NONE)
				ERR("Could Not Delete Notification");
			break;
		}
		list_traverse = notification_list_get_next(list_traverse);
	}
}

static void __bt_get_noti_id_opp_role_and_table(bt_share_appdata_t *ad, int *noti_id,
		char **opp_role, bt_tr_db_table_e *table)
{
	sqlite3 *db = bt_share_open_db();

	if (ad->tr_type == BT_TR_OUTBOUND) {
		*noti_id = bt_share_get_noti_id(db, BT_DB_OUTBOUND, ad->transfer_info->db_sid);
		*opp_role = NOTI_OPC_APP_ID;
		*table = BT_DB_OUTBOUND;
	} else {
		*noti_id = bt_share_get_noti_id(db, BT_DB_INBOUND, ad->transfer_info->db_sid);
		*opp_role = NOTI_OPS_APP_ID;
		*table = BT_DB_INBOUND;
	}

	bt_share_close_db(db);
}

void _bt_share_ui_handle_transfer_disconnected(bt_share_appdata_t *ad,
		bt_share_tr_type_e type)
{
	FN_START;
	ret_if(ad == NULL);

	if (ad->progress_item) {
		DBG("Updating Status");
		Elm_Object_Item *git = elm_genlist_item_insert_after(ad->tr_genlist,
				ad->tr_status_itc, ad, NULL,
				ad->device_item, ELM_GENLIST_ITEM_NONE,
				NULL, NULL);
		if (git == NULL) {
			ERR("elm_genlist_item_insert_after is failed!");
		} else {
			elm_genlist_item_select_mode_set(git,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			ad->status_item = git;
		}

		elm_object_item_del(ad->progress_item);
		ad->progress_item = NULL;
		ad->progressbar = NULL;
	}

	FN_END;
}

void _bt_share_ui_handle_transfer_complete(bt_share_appdata_t *ad,
		char *address, bt_share_tr_type_e type)
{
	FN_START;
	ret_if(ad == NULL || ad->transfer_info == NULL || ad->transfer_info->device_address == NULL);
	DBG_SECURE("Received Address: %s", address);
	DBG_SECURE("Device Address: %s", ad->transfer_info->device_address);

	ret_if(g_strcmp0(ad->transfer_info->device_address, address));

	FN_END;
}

void _bt_share_ui_handle_transfer_started(bt_share_appdata_t *ad,
		char *address, char *file_name, unsigned long size, int transfer_id,
		bt_share_tr_type_e type)
{
	FN_START;
	ret_if(ad == NULL || ad->transfer_info == NULL || ad->transfer_info->device_address == NULL);
	DBG_SECURE("Received Address: %s", address);
	DBG_SECURE("Device Address: %s", ad->transfer_info->device_address);
	ret_if(g_strcmp0(ad->transfer_info->device_address, address));

	ad->transfer_info->filename = g_strdup(file_name);
	DBG_SECURE("Transfer ID: %d", transfer_id);
	ad->transfer_info->size = size;
	ad->transfer_info->percentage = 0;
	ad->transfer_info->current_file++;

	if (ad->status_item) {
		elm_object_item_del(ad->status_item);
		ad->status_item = NULL;
	}

	if (ad->progress_item) {
		_bt_share_genlist_item_text_update(ad->progress_item, BT_SHARE_ITEM_PART_PROGRESSBAR_FILE_NAME);
		_bt_share_genlist_item_content_update(ad->progress_item, BT_SHARE_ITEM_PART_PROGRESSBAR_ICON);
	} else {
		/* Create Progress bar if not present */
		Elm_Object_Item *git = elm_genlist_item_insert_after(ad->tr_genlist,
				ad->tr_progress_itc, ad, NULL, ad->device_item,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
		if (git == NULL) {
			ERR("elm_genlist_item_append is failed!");
		} else {
			elm_genlist_item_select_mode_set(git,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			ad->progress_item = git;
		}
	}

	if (!ad->toolbar_button) {
		/* Create Stop Button if not present */
		ad->toolbar_button = _bt_share_create_toolbar_button(ad,
				BT_STR_STOP);
	}

	FN_END;
}

Evas_Object *_bt_create_win(const char *name)
{
	Evas_Object *eo = NULL;

	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	retv_if(!eo, NULL);

	elm_win_title_set(eo, name);
	elm_win_borderless_set(eo, EINA_TRUE);
	return eo;
}

void _bt_terminate_app(void)
{
	DBG("Terminate BT SHARE UI");
	elm_exit();
}

static void __bt_clear_view(void *data)
{
	DBG("+");

	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	ret_if(!ad);

	if (ad->tr_genlist) {
		elm_genlist_clear(ad->tr_genlist);
		ad->tr_genlist = NULL;
		ad->device_item = NULL;
		ad->progress_item = NULL;
		ad->status_item = NULL;
		ad->file_title_item = NULL;
		ad->tr_data_itc = NULL;
		ad->progressbar = NULL;
	}

	if (ad->navi_fr) {
		evas_object_del(ad->navi_fr);
		ad->navi_fr = NULL;
	}

	if (ad->tr_view) {
		evas_object_del(ad->tr_view);
		ad->tr_view = NULL;
	}
	DBG("-");
}

static Eina_Bool __bt_back_button_cb(void *data, Elm_Object_Item *it)
{
	DBG("pop current view ");
	retvm_if(!data, EINA_FALSE, "invalid parameter!");

	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	int ret;

	__bt_clear_view(data);

	if (ad->tr_data_list) {
		ret = bt_share_release_tr_data_list(ad->tr_data_list);

		if (ret != BT_SHARE_ERR_NONE)
			ERR("Transfer data release failed ");
		ad->tr_data_list = NULL;
	}

	_bt_terminate_app();

	return EINA_FALSE;
}

static Evas_Object *__bt_tr_progress_icon_get(void *data, Evas_Object *obj,
					    const char *part)
{
	FN_START;

	Evas_Object *progress_layout = NULL;
	Evas_Object *progressbar = NULL;
	bt_share_appdata_t *ad = NULL;

	retv_if(data == NULL, NULL);
	ad = (bt_share_appdata_t *)data;

	bt_share_transfer_data_t *transfer_info = ad->transfer_info;

	if (!strcmp(part, BT_SHARE_ITEM_PART_PROGRESSBAR_ICON)) {
		char buff[BT_STR_PROGRESS_MAX_LEN] = { 0, };
		char *markup_text = NULL;

		DBG("Creating new progress layout!!!");
		progress_layout = elm_layout_add(obj);
		elm_layout_file_set(progress_layout, EDJFILE, "popup_text_progressbar_view_layout");
		evas_object_size_hint_align_set(progress_layout, EVAS_HINT_FILL,
				EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(progress_layout, EVAS_HINT_EXPAND,
				EVAS_HINT_EXPAND);
		evas_object_show(progress_layout);

		DBG("Creating new progressbar!!!");
		progressbar = elm_progressbar_add(progress_layout);

		elm_progressbar_unit_format_set(progressbar, NULL);
		elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
		evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL,
				EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND,
				EVAS_HINT_EXPAND);
		elm_progressbar_pulse(progressbar, EINA_TRUE);

		evas_object_show(progressbar);
		ad->progressbar = progressbar;

		markup_text = elm_entry_utf8_to_markup(transfer_info->filename);
		DBG_SECURE("Filename: %s", markup_text);

		DBG("Filename: %s", markup_text);

		elm_object_part_text_set(progress_layout, "elm.text.description", markup_text);
		g_free(markup_text);

		elm_object_part_content_set(progress_layout, "progressbar", progressbar);

		elm_progressbar_value_set(progressbar,
				(double) transfer_info->percentage / 100);

		elm_object_signal_emit(progressbar, "elm,bottom,text,show", "elm");

		snprintf(buff, BT_STR_PROGRESS_MAX_LEN - 1 , "%d%%", transfer_info->percentage);
		elm_object_part_text_set(progressbar, "elm.text.bottom.left", buff);

		if (ad->launch_mode == BT_LAUNCH_ONGOING &&
				ad->tr_type == BT_TR_OUTBOUND) {
			memset(buff, 0, BT_STR_PROGRESS_MAX_LEN);
			snprintf(buff, BT_STR_PROGRESS_MAX_LEN - 1 , "[%d/%d]",
					transfer_info->current_file, transfer_info->total_files);
			elm_object_part_text_set(progressbar, "elm.text.bottom.right", buff);
		}
	}

	return progress_layout;
}

static Evas_Object *__bt_tr_icon_get(void *data, Evas_Object *obj,
					    const char *part)
{
	Evas_Object *ly = NULL;
	Evas_Object *icon = NULL;
	bt_tr_data_t *info = NULL;
	bt_share_appdata_t *ad = app_state;
	char *img = NULL;
	bt_gl_data_t *gl_data = NULL;

	retv_if(!data, NULL);
	gl_data = (bt_gl_data_t *)data;

	info = gl_data->tr_data;
	retv_if(!info, NULL);

	if (!strcmp(part, BT_SHARE_ITEM_PART_FILE_TRANSFER_STATUS_ICON)) {
		ly = elm_layout_add(obj);
		if (info->tr_status == BT_TRANSFER_SUCCESS ||
				info->tr_status == BT_TRANSFER_FAIL) {
			icon = elm_image_add(obj);
			if (ad->tr_type == BT_TR_OUTBOUND) {
				img = (info->tr_status == BT_TRANSFER_SUCCESS) ?
						BT_ICON_SEND_PASS : BT_ICON_SEND_FAIL;
			} else {
				img = (info->tr_status == BT_TRANSFER_SUCCESS) ?
						BT_ICON_RECV_PASS : BT_ICON_RECV_FAIL;
			}

			elm_image_file_set(icon, EDJ_IMAGES, img);
			evas_object_size_hint_align_set(icon, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND,
					EVAS_HINT_EXPAND);

			evas_object_show(icon);
			evas_object_size_hint_min_set(icon, ELM_SCALE_SIZE(40), ELM_SCALE_SIZE(40));
			return icon;
		} else if (info->tr_status == BT_TRANSFER_ONGOING) {
#if 0
			int ret;
			ret = elm_layout_file_set(ly, EDJ_BT_ICON_ANIMATION,
				(ad->tr_type == BT_TR_OUTBOUND ? BT_ANI_UPLOAD : BT_ANI_DOWNLOAD));
			if (ret == EINA_FALSE)
				ERR("Error in setting layout file");
#endif
			icon = elm_progressbar_add(obj);
			elm_object_style_set(icon, "process_medium");
			evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, 0.5);
			evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_progressbar_pulse(icon, TRUE);
			return icon;
		} else if (info->tr_status == BT_TRANSFER_PENDING) {
			icon = elm_progressbar_add(obj);
			elm_object_style_set(icon, "process_medium");
			evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, 0.5);
			evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_progressbar_pulse(icon, TRUE);
			return icon;
		}
	}
	return ly;
}

#if 0
void _bt_util_get_number(char *source, char *dest)
{
	int len = 7;
	int slen = 0;
	char buf[9];

	slen = strlen(source);
	slen--;
	buf[8] = '\0';
	while (len > -1) {
		if (slen > -1) {
			if (isdigit(source[slen])) {
				buf[len] = source[slen];
				--len;
			}
			--slen;
		} else {
			break;
		}
	}
	if (len < 0)
		len = 0;
	strcpy(dest, &buf[len]);
}

void _bt_util_get_contact_name(int lcontact_id, char **contact_name)
{
	int ret = 0;
	int count = 0;
	contacts_filter_h filter = NULL;
	contacts_query_h query = NULL;
	contacts_list_h list = NULL;
	contacts_record_h record = NULL;
	char *name = NULL;

	DBG("+");
	ret = contacts_filter_create(_contacts_contact._uri, &filter);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("Fail to create filter for contacts");
		goto fail;
	}

	ret = contacts_filter_add_int(filter, _contacts_contact.id,
			CONTACTS_MATCH_EXACTLY, lcontact_id);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("Fail to add str to filter for contacts");
		goto fail;
	}

	ret = contacts_query_create(_contacts_contact._uri, &query);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("Fail to create qurey for contacts");
		goto fail;
	}

	ret = contacts_query_set_filter(query, filter);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("Fail to set filter for contacts");
		goto fail;
	}

	ret = contacts_db_get_count_with_query(query, &count);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("Fail to get count from db");
		goto fail;
	}

	if (count < 1) {
		DBG("No match");
		goto fail;
	}
	DBG("count = %d", count);
	ret = contacts_db_get_records_with_query(query, 0, 0, &list);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("Fail to get records from db");
		goto fail;
	}
	contacts_list_first(list);

	while (ret == CONTACTS_ERROR_NONE) {
		contacts_list_get_current_record_p(list, &record);
		contacts_record_get_str_p(record,
				_contacts_contact.display_name,
				&name);
		if (name != NULL) {
			*contact_name = g_strdup(name);
			break;
		}

		ret = contacts_list_next(list);
	}

fail:
	if (filter)
		contacts_filter_destroy(filter);

	if (query)
		contacts_query_destroy(query);

	if (list)
		contacts_list_destroy(list, true);
	DBG("-");
	return;
}

unsigned int bt_crc32(const void *src, unsigned long src_sz)
{
	return (crc32(0, src, src_sz) & 0xFFFF);
}

void _bt_util_get_contact_info(unsigned char *auth_info, int *contact_id, char **contact_name)
{
	unsigned int shash = 0;
	unsigned int chash = 0;
	char pnumber[20] = {0,};
	char str_hash[7] = {0,};
	int ret = CONTACTS_ERROR_NONE;
	char *number = NULL;
	int lcontact_id = 0;
	int count = 0;

	contacts_filter_h filter = NULL;
	contacts_query_h query = NULL;
	contacts_list_h list = NULL;
	contacts_record_h record = NULL;

	retm_if(!auth_info, "auth_info is NULL");
	retm_if(!contact_id, "contact_id is NULL");
	retm_if(!contact_name, "contact_name is NULL");
	DBG("+");
	if (contacts_connect() != CONTACTS_ERROR_NONE) {
		ERR("contacts_connect failed");
		return;
	}

	memcpy(&shash, auth_info, 3);
	shash = ntohl(shash);
	shash >>= 8;

	memcpy(&chash, &auth_info[3], 2);
	chash = ntohl(chash);
	chash >>= 16;

	g_snprintf(str_hash, 7, "%X", shash);

	ret = contacts_filter_create(_contacts_quick_connect_info._uri, &filter);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("Fail to create filter for contacts");
		goto fail;
	}

	ret = contacts_filter_add_str(filter, _contacts_quick_connect_info.hash,
			CONTACTS_MATCH_EXACTLY, str_hash);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("Fail to add str to filter for contacts");
		goto fail;
	}

	ret = contacts_query_create(_contacts_quick_connect_info._uri, &query);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("Fail to create qurey for contacts");
		goto fail;
	}

	ret = contacts_query_set_filter(query, filter);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("Fail to set filter for contacts");
		goto fail;
	}

	ret = contacts_db_get_count_with_query(query, &count);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("Fail to get count from db");
		goto fail;
	}

	if (count < 1) {
		DBG("No match");
		goto fail;
	}

	ret = contacts_db_get_records_with_query(query, 0, 0, &list);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("Fail to get records from db");
		goto fail;
	}

	contacts_list_first(list);

	while (ret == CONTACTS_ERROR_NONE) {
		contacts_list_get_current_record_p(list, &record);
		contacts_record_get_str_p(record,
				_contacts_quick_connect_info.number,
				&number);
		DBG_SECURE("number: [%s]", number);
		if (number != NULL) {
			_bt_util_get_number(number, pnumber);
			DBG_SECURE("pnumber: [%s]", pnumber);

			DBG_SECURE("CRC [%X], [%X]", chash, bt_crc32(pnumber, strlen(pnumber)));

			if (bt_crc32(pnumber , strlen(pnumber)) == chash) {
				contacts_record_get_int(record,
						_contacts_quick_connect_info.contact_id,
						&lcontact_id);
				*contact_id = lcontact_id;
				_bt_util_get_contact_name(lcontact_id, contact_name);
				DBG_SECURE("contact id : %d", lcontact_id);
				break;
			}
		}
		ret = contacts_list_next(list);
	}

fail:
	if (filter)
		contacts_filter_destroy(filter);

	if (query)
		contacts_query_destroy(query);

	if (list)
		contacts_list_destroy(list, true);

	contacts_disconnect();

	DBG("-");
}
#endif

static char *__bt_tr_device_label_get(void *data, Evas_Object *obj,
				      const char *part)
{
	FN_START;
	retv_if(data == NULL, NULL);
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	char *markup_text = NULL;

	retv_if(ad->transfer_info == NULL, NULL);

	if (!strcmp(part, BT_SHARE_ITEM_PART_DEVICE_NAME_TITLE)) {
		return g_strdup(BT_STR_DEVICENAME);
	} else if (!strcmp(part, BT_SHARE_ITEM_PART_DEVICE_NAME)) {
		bt_share_transfer_data_t *transfer_info = ad->transfer_info;
		DBG_SECURE("Device : %s", transfer_info->device_name);
		if (ad->bt_status == BT_ADAPTER_ENABLED) {
			bt_error_e ret = BT_ERROR_NONE;
			bt_device_info_s *dev_info = NULL;

			/* Get the contact name from manufacturer data */
			DBG_SECURE("Address : %s", transfer_info->device_address);
			ret =  bt_adapter_get_bonded_device_info(transfer_info->device_address, &dev_info);
			if (ret == BT_ERROR_NONE) {
				DBG_SECURE("Using remote name [%s] only.", dev_info->remote_name);
				markup_text = elm_entry_utf8_to_markup(dev_info->remote_name);
#if 0
				unsigned char auth_info[5] = {0, };
				int contact_id = -1;
				char *contact_name = NULL;
				gboolean is_alias_set = FALSE;
				ret = bt_adapter_get_bonded_device_is_alias_set(dev_info->remote_address, &is_alias_set);
				if (ret != BT_ERROR_NONE)
					ERR("bt_adapter_get_bonded_device_is_alias_set() is failed!!! error: 0x%04X", ret);

				if (is_alias_set == TRUE) {
					DBG_SECURE("Device alias is set. Using remote name [%s] only.", dev_info->remote_name);
					markup_text = elm_entry_utf8_to_markup(dev_info->remote_name);
				} else if (dev_info->manufacturer_data_len >= 30 &&
						dev_info->manufacturer_data[0] == 0x00 &&
						dev_info->manufacturer_data[1] == 0x75) {
					unsigned char auth_info_null[5];
					memset(auth_info_null, 0X0, 5);
					memcpy(auth_info, &(dev_info->manufacturer_data[10]), 5);

					/* Check for validity of auth_info */
					if (memcmp(auth_info, auth_info_null, 5)) {
						_bt_util_get_contact_info(auth_info, &contact_id, &contact_name);
						DBG_SECURE("contact id : %d | contact name : %s", contact_id, contact_name);
						if (contact_name) {
							markup_text = elm_entry_utf8_to_markup(contact_name);
							g_free(contact_name);
						}
					}
				}
#endif
				bt_adapter_free_device_info(dev_info);
			} else {
				ERR("bt_adapter_get_bonded_device_info() is failed!!! error: 0x%04X", ret);
			}
		}

		if (!markup_text)
			markup_text = elm_entry_utf8_to_markup(transfer_info->device_name);

		return markup_text;
	}
	FN_END;
	return NULL;
}

static char *__bt_tr_status_label_get(void *data, Evas_Object *obj,
				      const char *part)
{
	FN_START;
	retv_if(data == NULL, NULL);
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	bt_share_transfer_data_t *transfer_info = ad->transfer_info;
	char *tmp_success = NULL;
	char *tmp_failed = NULL;
	char *ret_str = NULL;

	retv_if(transfer_info == NULL, NULL);

	if (!strcmp(part, BT_SHARE_ITEM_PART_TRANSFER_TYPE_TITLE)) {
		if (ad->tr_type == BT_TR_INBOUND)
			return (transfer_info->success > 0) ?
				g_strdup(BT_STR_FILE_RECEIVED) :
				g_strdup(BT_STR_RECEIVING_FAILED);
		else
			return (transfer_info->success > 0) ?
				g_strdup(BT_STR_FILE_SENT) :
				g_strdup(BT_STR_SENDING_FAILED);
	} else if (!strcmp(part, BT_SHARE_ITEM_PART_TRANSFER_STATUS)) {
		char *stms_str = NULL;

		DBG_SECURE("Success %d, Failed %d", transfer_info->success, transfer_info->failed);
		if (transfer_info->success == 0 &&
				transfer_info->failed == 0)
			return NULL;

		stms_str = (ad->tr_type == BT_TR_INBOUND) ? BT_STR_PD_RECEIVED : BT_STR_PD_SENT;
		tmp_success = g_strdup_printf(stms_str, transfer_info->success);

		stms_str = BT_STR_PD_FAILED;
		tmp_failed = g_strdup_printf(stms_str, transfer_info->failed);

		ret_str = g_strdup_printf("%s, %s", tmp_success, tmp_failed);

		g_free(tmp_success);
		g_free(tmp_failed);
		return ret_str;
	}
	FN_END;
	return NULL;
}

#if 0
static char *__bt_tr_progress_label_get(void *data, Evas_Object *obj,
				      const char *part)
{
	FN_START;
	retv_if(data == NULL, NULL);
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	char *markup_text = NULL;

	retv_if(ad->transfer_info == NULL, NULL);

	if (!strcmp(part, BT_SHARE_ITEM_PART_PROGRESSBAR_FILE_NAME)) {
		bt_share_transfer_data_t *transfer_info = ad->transfer_info;
		markup_text = elm_entry_utf8_to_markup(transfer_info->filename);
		DBG_SECURE("Filename: %s", markup_text);
		return markup_text;
	}

	FN_END;
	return NULL;
}
#endif

static char *__bt_tr_file_title_label_get(void *data, Evas_Object *obj,
				      const char *part)
{
	FN_START;
	if (!strcmp(part, BT_SHARE_ITEM_PART_FILE_LIST_TITLE)) {
		DBG("File List");
		return g_strdup(BT_STR_FILE_LIST);
	}
	FN_END;
	return NULL;
}

static char *__bt_tr_label_get(void *data, Evas_Object *obj,
				      const char *part)
{
	FN_START;
	bt_tr_data_t *info = NULL;
	char *name = NULL;
	char buf[BT_GLOBALIZATION_STR_LENGTH] = { 0 };
	bt_gl_data_t *gl_data = NULL;
	char *markup_text = NULL;
	retv_if(data == NULL, NULL);
	gl_data = (bt_gl_data_t *)data;
	retvm_if(gl_data == NULL, NULL, "gl_data is NULL!");

	info = gl_data->tr_data;
	retv_if(info == NULL, NULL);

	if (!strcmp(part, BT_SHARE_ITEM_PART_FILE_NAME)) {
		name = strrchr(info->file_path, '/');
		if (name != NULL)
			name++;
		else
			name = info->file_path;
		markup_text = elm_entry_utf8_to_markup(name);

		g_strlcpy(buf, markup_text, BT_GLOBALIZATION_STR_LENGTH);
	} else if (!strcmp(part, BT_SHARE_ITEM_PART_FILE_TRANSFER_STATUS_TEXT)) {
		DBG("info->tr_status : %d", info->tr_status);
		if (info->tr_status == BT_TRANSFER_SUCCESS) {
			if (gl_data->tr_inbound)
				g_strlcpy(buf, BT_STR_RECEIVED, BT_GLOBALIZATION_STR_LENGTH);
			else
				g_strlcpy(buf, BT_STR_SENT, BT_GLOBALIZATION_STR_LENGTH);
		} else if (info->tr_status == BT_TRANSFER_FAIL) {
			g_strlcpy(buf, BT_STR_FAILED, BT_GLOBALIZATION_STR_LENGTH);
		} else if (info->tr_status == BT_TRANSFER_ONGOING) {
			if (gl_data->tr_inbound)
				g_strlcpy(buf, BT_STR_RECEIVING, BT_GLOBALIZATION_STR_LENGTH);
			else
				g_strlcpy(buf, BT_STR_SENDING, BT_GLOBALIZATION_STR_LENGTH);
		} else if (info->tr_status == BT_TRANSFER_PENDING) {
			g_strlcpy(buf, BT_STR_WAITING, BT_GLOBALIZATION_STR_LENGTH);
		}
	} else if (!strcmp(part, BT_SHARE_ITEM_PART_FILE_SIZE)) {
		char *size;

		if ((info->tr_status == BT_TRANSFER_ONGOING ||
				info->tr_status == BT_TRANSFER_PENDING) && gl_data->tr_inbound)
			return NULL;

		if (info->size > 1024 * 1024 * 1024) { //GB
			size = g_strdup_printf("%.2f GB", (float)info->size / (1024 * 1024 * 1024));
		} else if (info->size > 1024 * 1024) { //MB
			size = g_strdup_printf("%.1f MB", (float)info->size / (1024 * 1024));
		} else if (info->size > 1024) { //KB
			size = g_strdup_printf("%.1f KB", (float)info->size / (1024));
		} else {
			size = g_strdup_printf("%d B", info->size);
		}

		g_strlcpy(buf, size, BT_GLOBALIZATION_STR_LENGTH);
		g_free(size);
	} else {
		DBG("empty text for label.");
		return NULL;
	}
	if (markup_text)
		free(markup_text);

	FN_END;
	return strdup(buf);
}

static void  __bt_tr_del(void *data, Evas_Object *obj)
{
	bt_gl_data_t *gl_data;
	gl_data = (bt_gl_data_t *)data;

	g_free(gl_data);
}

#if 0
static void __bt_genlist_realized_cb(void *data,
			Evas_Object *obj, void *event_info)
{
	retm_if(event_info == NULL, "Invalid param\n");

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
#ifdef KIRAN_ACCESSIBILITY
	Evas_Object *ao;
	char str[BT_STR_ACCES_INFO_MAX_LEN] = {0, };
#endif
	char *name;
	char *date;
	bt_tr_data_t *info;
	bt_gl_data_t *gl_data;

	gl_data = (bt_gl_data_t *)elm_object_item_data_get(item);
	ret_if(gl_data == NULL);

	info = gl_data->tr_data;

	ret_if(info == NULL);
	ret_if(info->dev_name == NULL);
	ret_if(info->file_path == NULL);

	name = strrchr(info->file_path, '/');
	if (name != NULL)
		name++;
	else
		name = info->file_path;

	date = __bt_get_tr_timedate((time_t)(info->timestamp));

#ifdef KIRAN_ACCESSIBILITY
	snprintf(str, sizeof(str), "%s, %s, %s, %s",
			BT_STR_ACC_ICON, name, info->dev_name, date);
	ao = elm_object_item_access_object_get(item);
	elm_access_info_set(ao, ELM_ACCESS_INFO, str);
#endif
	g_free(date);
}
#endif

static void __bt_popup_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	FN_START;
	retm_if(data == NULL, "data is NULL");
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;

	if (ad->info_popup) {
		evas_object_del(ad->info_popup);
		ad->info_popup = NULL;
	}
	FN_END;
}

void __bt_popup_del_by_timeout(void *data, Evas_Object *obj, void *event_info)
{
	FN_START;
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	ret_if(!ad);
	if (ad->info_popup) {
		evas_object_del(ad->info_popup);
		ad->info_popup = NULL;
	}
	FN_END;
}

Evas_Object *__bt_create_error_popup(bt_share_appdata_t *ad)
{
	FN_START;
	Evas_Object *popup = NULL;
	retvm_if(ad == NULL, NULL, "ad is NULL");

	popup = elm_popup_add(ad->win);
	retvm_if(popup == NULL, NULL, "popup is NULL");

	elm_object_focus_set(popup, EINA_FALSE);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_text_set(popup, BT_STR_UNABLE_TO_FIND_APPLICATION);

	elm_popup_timeout_set(popup, 3);
	evas_object_smart_callback_add(popup, "timeout", (Evas_Smart_Cb)__bt_popup_del_by_timeout, ad);
	evas_object_show(popup);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, __bt_popup_del_cb, ad);

	ad->info_popup = popup;

	FN_END;
	return popup;
}

static bt_file_type_e __get_file_type(const char *name)
{
	FN_START;

	char *extn = NULL;

	extn = strrchr(name, '.');
	if (extn != NULL)
		extn++;

	DBG("extn : %s", extn);

	if (extn != NULL) {
		if (!strcasecmp(extn, "png") || !strcasecmp(extn, "bmp") || !strcasecmp(extn, "gif") ||
			 !strcasecmp(extn, "jpg") || !strcasecmp(extn, "jpeg") || !strcasecmp(extn, "jpe") ||
			  !strcasecmp(extn, "jp2") || !strcasecmp(extn, "pjpeg") || !strcasecmp(extn, "tif") ||
			   !strcasecmp(extn, "wbmp") || !strcasecmp(extn, "wmf"))
			return BT_FILE_IMAGE;
		else if (!strcasecmp(extn, "vcf"))
			return BT_FILE_VCARD;
		else if (!strcasecmp(extn, "vcs"))
			return BT_FILE_VCAL;
		else if (!strcasecmp(extn, "vbm"))
			return BT_FILE_VBOOKMARK;
	}
	FN_END;
	return BT_FILE_OTHER;
}

static gboolean __bt_open_file(const char *path)
{
	FN_START;
	app_control_h handle;
	int ret;
	bt_file_type_e file_type;
	bt_share_appdata_t *ad = app_state;

	app_control_create(&handle);
	app_control_set_operation(handle, APP_CONTROL_OPERATION_VIEW);
	app_control_set_uri(handle, path);

	file_type = __get_file_type(path);

	if (file_type == BT_FILE_IMAGE) {
		app_control_set_mime(handle, "image/*");
		app_control_add_extra_data(handle, "Path", path);
		app_control_set_launch_mode(handle, APP_CONTROL_LAUNCH_MODE_GROUP);
	}

	ret = app_control_send_launch_request(handle, NULL, NULL);

	if (ret == APP_CONTROL_ERROR_APP_NOT_FOUND)
		__bt_create_error_popup(ad);

	app_control_destroy(handle);
	FN_END;
	return TRUE;
}

static Elm_Object_Item * __bt_add_file_title_item(bt_share_appdata_t *ad)
{
	retvm_if(!ad, NULL, "ad is NULL!");
	retvm_if(!ad->tr_genlist, NULL, "tr_genlist is NULL!");
	retvm_if(ad->file_title_item, NULL, "file_title_item is exist");

	Elm_Object_Item *git = NULL;
	git = elm_genlist_item_append(ad->tr_genlist,
			ad->tr_file_title_itc, NULL,
			NULL, ELM_GENLIST_ITEM_NONE,
			NULL, NULL);

	if (git == NULL) {
		ERR("elm_genlist_item_append is failed!");
	} else {
		elm_genlist_item_select_mode_set(git,
					 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		ad->file_title_item = git;
	}

	return git;
}

static void __bt_tr_data_item_sel(void *data, Evas_Object *obj, void *event_info)
{
	bt_share_appdata_t *ad = app_state;
	bt_gl_data_t *gl_data = (bt_gl_data_t *) data;
	bt_tr_data_t *info = NULL;
	char *path = NULL;
	char *ext = NULL;
	int default_memory = 0;

	ret_if(data == NULL);
	ret_if(event_info == NULL);

	FN_START;

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	info = gl_data->tr_data;
	retm_if(info->file_path == NULL, "File Path is NULL");

	ext = strrchr(info->file_path, '.');
	if (ext++) {
		if (0 == g_strcmp0(ext, "vcf")) {
			if (gl_data->tr_inbound == true) {
				if (info->tr_status == BT_TRANSFER_SUCCESS)
					_bt_create_info_popup(ad, BT_STR_RECEIVED_VCF_FILE_ALREADY_IMPORTED_TO_CONTACTS);
			} else {
				_bt_create_info_popup(ad, BT_STR_VCF_FILES_ARE_TEMPORARY_AND_CANT_BE_OPENED);
			}
			return;
		}
	}

	if (gl_data->tr_inbound == true) {
		if (info->tr_status == BT_TRANSFER_SUCCESS) {
			if (vconf_get_int(VCONFKEY_SETAPPL_DEFAULT_MEM_BLUETOOTH_INT, &default_memory) != 0)
				ERR("vconf get failed");

			if (default_memory == BT_DEFAULT_MEM_PHONE)
				path = g_strdup_printf("%s/%s", BT_DOWNLOAD_PHONE_FOLDER, info->file_path);
			else if (default_memory == BT_DEFAULT_MEM_MMC)
				path = g_strdup_printf("%s/%s", BT_DOWNLOAD_MMC_FOLDER, info->file_path);

			INFO_SECURE("path : %s", path);

			if (access(path, F_OK) == 0)
				__bt_open_file(path);
			else
				_bt_create_info_popup(ad, BT_STR_FILE_NOT_EXIST);

			g_free(path);
		}
	} else {
		path = info->file_path;
		INFO_SECURE("path : %s", path);

		if (access(path, F_OK) == 0) {
			if (g_str_has_prefix(path, BT_CONTACT_SHARE_TMP_DIR) == TRUE)
				/* TODO: change to proper string when UX is updated */
				_bt_create_info_popup(ad, BT_STR_FILE_NOT_EXIST);
			else
				__bt_open_file(path);
		} else {
			_bt_create_info_popup(ad, BT_STR_FILE_NOT_EXIST);
		}
	}

	FN_END;
}

void _bt_genlist_prepend_tr_data_item(bt_share_appdata_t *ad,
				Evas_Object *genlist, bt_tr_data_t *info, int tr_type)
{
	FN_START;
	retm_if(ad == NULL || info == NULL, "Invalid parameters!");
	retm_if(ad->tr_data_itc == NULL, "ad->tr_data_itc is NULL!");

	bt_gl_data_t *gl_data = NULL;

	gl_data = g_new0(bt_gl_data_t, 1);
	gl_data->tr_data = info;
	DBG("info->tr_status : %d", info->tr_status);
	if (tr_type == BT_TR_OUTBOUND || tr_type == BT_TR_INBOUND) {
		if (info->tr_status == BT_TRANSFER_SUCCESS)
			ad->transfer_info->success++;
		else if (info->tr_status == BT_TRANSFER_FAIL)
			ad->transfer_info->failed++;
	}

	if (tr_type == BT_TR_OUTBOUND) {
		gl_data->tr_inbound = false;

		if (ad->outbound_latest_id < info->id)
			ad->outbound_latest_id = info->id;
	} else if (tr_type == BT_TR_INBOUND) {
		gl_data->tr_inbound = true;

		if (ad->inbound_latest_id < info->id)
			ad->inbound_latest_id = info->id;
	}

	elm_genlist_item_append(genlist, ad->tr_data_itc, gl_data, NULL,
			ELM_GENLIST_ITEM_NONE, __bt_tr_data_item_sel, gl_data);

	evas_object_show(genlist);

	FN_END;
}

void __bt_update_transfer_count(bt_share_appdata_t *ad, bt_tr_data_t *info)
{
	if (info->tr_status == BT_TRANSFER_SUCCESS)
		ad->transfer_info->success++;
	else
		ad->transfer_info->failed++;

	if (ad->status_item)
		elm_genlist_item_fields_update(ad->status_item, "*",
				ELM_GENLIST_ITEM_FIELD_TEXT);
}

void _bt_genlist_append_tr_data_item(bt_share_appdata_t *ad,
					bt_tr_data_t *info, int tr_type)
{
	FN_START;

	ret_if(ad == NULL || info == NULL);
	ret_if(ad->tr_data_itc == NULL);
	ret_if(ad->tr_genlist == NULL);

	Elm_Object_Item *git = NULL;

	if (elm_genlist_items_count(ad->tr_genlist) == 0) {
		Evas_Object *genlist = NULL;
		Elm_Object_Item *navi_it = NULL;

		genlist = __bt_add_tr_data_genlist(ad->tr_view, ad);
		retm_if(genlist == NULL, "genlist is NULL!");

		if (ad->tr_genlist) {
			DBG("Clear the previous genlist");
			elm_genlist_clear(ad->tr_genlist);
			ad->tr_genlist = NULL;
		}

		ad->tr_genlist = genlist;

		if (ad->navi_it) {
			elm_object_item_part_content_set(ad->navi_it,
					"elm.swallow.content", genlist);
		} else {
			navi_it = elm_naviframe_item_push(ad->navi_fr, NULL,
					NULL, NULL, genlist, NULL);
			if (navi_it)
				elm_object_item_domain_translatable_text_set(navi_it,
						BT_COMMON_PKG,
						(ad->tr_type == BT_TR_INBOUND) ?
								"IDS_ST_HEADER_RECEIVE" : "IDS_ST_HEADER_SEND");
			elm_naviframe_item_pop_cb_set(navi_it, __bt_back_button_cb, ad);
			ad->navi_it = navi_it;
		}
		return;
	}

	if (!ad->file_title_item) {
		retm_if(NULL == __bt_add_file_title_item(ad),
				"__bt_add_file_title_item is failed");
	}

	bt_gl_data_t *gl_data = NULL;
	gl_data = g_new0(bt_gl_data_t, 1);
	gl_data->tr_data = info;

	if (tr_type == BT_TR_OUTBOUND) {
		gl_data->tr_inbound = false;

		if (ad->outbound_latest_id < info->id)
			ad->outbound_latest_id = info->id;
	} else if (tr_type == BT_TR_INBOUND) {
		gl_data->tr_inbound = true;

		if (ad->inbound_latest_id < info->id)
			ad->inbound_latest_id = info->id;
	}

	git = elm_genlist_item_append(ad->tr_genlist, ad->tr_data_itc, gl_data,
			NULL, ELM_GENLIST_ITEM_NONE, __bt_tr_data_item_sel, gl_data);

	__bt_update_transfer_count(ad, info);

	if (info->tr_status == BT_TRANSFER_ONGOING)
		ad->current_item = git;

	evas_object_show(ad->tr_genlist);

	FN_END;
}

static void __bt_share_gl_highlighted(void *data, Evas_Object *obj,
							void *event_info)
{
	FN_START;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	bt_gl_data_t *gl_data;

	ret_if(item == NULL);

	gl_data = (bt_gl_data_t *)elm_object_item_data_get(item);
	ret_if(gl_data == NULL);

	gl_data->highlighted = TRUE;

	elm_genlist_item_fields_update(item, "*",
				ELM_GENLIST_ITEM_FIELD_CONTENT);

	FN_END;
}

static void __bt_share_gl_unhighlighted(void *data, Evas_Object *obj,
							void *event_info)
{
	FN_START;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	bt_gl_data_t *gl_data;

	ret_if(item == NULL);

	gl_data = (bt_gl_data_t *)elm_object_item_data_get(item);
	ret_if(gl_data == NULL);
	gl_data->highlighted = FALSE;

	elm_genlist_item_fields_update(item, "*",
				ELM_GENLIST_ITEM_FIELD_CONTENT);

	FN_END;
}

static Evas_Object *__bt_add_tr_data_genlist(Evas_Object *parent,
						  bt_share_appdata_t *ad)
{
	FN_START;
	retvm_if(ad == NULL, NULL, "Inavalid parameter!");

	Evas_Object *genlist = elm_genlist_add(parent);

	evas_object_smart_callback_add(genlist, "highlighted",
				__bt_share_gl_highlighted, ad);

	evas_object_smart_callback_add(genlist, "unhighlighted",
				__bt_share_gl_unhighlighted, ad);

	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	ad->tr_device_itc = elm_genlist_item_class_new();
	if (ad->tr_device_itc) {
		ad->tr_device_itc->item_style = "type1";
		ad->tr_device_itc->func.text_get = __bt_tr_device_label_get;
		ad->tr_device_itc->func.content_get = NULL;
		ad->tr_device_itc->func.state_get = NULL;
		ad->tr_device_itc->func.del = NULL;
	}

	ad->tr_status_itc = elm_genlist_item_class_new();
	if (ad->tr_status_itc) {
		ad->tr_status_itc->item_style = "type1";
		ad->tr_status_itc->func.text_get = __bt_tr_status_label_get;
		ad->tr_status_itc->func.content_get = NULL;
		ad->tr_status_itc->func.state_get = NULL;
		ad->tr_status_itc->func.del = NULL;
	}

	ad->tr_progress_itc = elm_genlist_item_class_new();
	if (ad->tr_progress_itc) {
		ad->tr_progress_itc->item_style = "full";
		ad->tr_progress_itc->func.text_get = NULL;
		ad->tr_progress_itc->func.content_get = __bt_tr_progress_icon_get;
		ad->tr_progress_itc->func.state_get = NULL;
		ad->tr_progress_itc->func.del = NULL;
	}

	ad->tr_file_title_itc = elm_genlist_item_class_new();
	if (ad->tr_file_title_itc) {
		ad->tr_file_title_itc->item_style = "groupindex";
		ad->tr_file_title_itc->func.text_get = __bt_tr_file_title_label_get;
		ad->tr_file_title_itc->func.content_get = NULL;
		ad->tr_file_title_itc->func.state_get = NULL;
		ad->tr_file_title_itc->func.del = NULL;
	}

	ad->tr_data_itc = elm_genlist_item_class_new();
	if (ad->tr_data_itc) {
		ad->tr_data_itc->item_style = "type1";
		ad->tr_data_itc->func.text_get = __bt_tr_label_get;
		ad->tr_data_itc->func.content_get = __bt_tr_icon_get;
		ad->tr_data_itc->func.state_get = NULL;
		ad->tr_data_itc->func.del = __bt_tr_del;
	}

	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	FN_END;
	return genlist;
}

static Evas_Object * __bt_create_naviframe(bt_share_appdata_t *ad)
{
	FN_START;
	retv_if(ad == NULL, NULL);
	Evas_Object *navi_fr = NULL;
	/* Naviframe */
	navi_fr = elm_naviframe_add(ad->tr_view);
	eext_object_event_callback_add(navi_fr, EEXT_CALLBACK_BACK,
						eext_naviframe_back_cb, NULL);
	elm_object_part_content_set(ad->tr_view, "elm.swallow.content", navi_fr);
	evas_object_show(navi_fr);
	FN_END;
	return navi_fr;
}

void _bt_cb_state_changed(int result,
			bt_adapter_state_e adapter_state,
			void *user_data)
{
	FN_START;
	DBG("bluetooth %s", adapter_state == BT_ADAPTER_ENABLED ?
				"enabled" : "disabled");

	ret_if(!user_data);
	ret_if(result != BT_ERROR_NONE);

	bt_share_appdata_t *ad = (bt_share_appdata_t *)user_data;

	ad->bt_status = adapter_state;

	if (adapter_state == BT_ADAPTER_ENABLED && ad->send_after_turning_on) {
		DBG("Adapter enabled, resend pending items");
		/* close turning on popup */
		if (ad->turning_on_popup) {
			evas_object_del(ad->turning_on_popup);
			ad->turning_on_popup = NULL;
		}

		_bt_share_ui_retry_failed(ad);
		if (ad->launch_mode == BT_LAUNCH_TRANSFER_LIST) {
			int noti_id;
			char *opp_role;
			bt_tr_db_table_e table;

			__bt_get_noti_id_opp_role_and_table(ad, &noti_id, &opp_role, &table);

			DBG_SECURE("Notification ID: %d", noti_id);
			if (noti_id < 0) {
				ERR("Invalid Notification ID");
			} else {
				sqlite3 *db = bt_share_open_db();
				bt_share_remove_tr_data_by_notification(db, table, noti_id);
				bt_share_close_db(db);
				_bt_delete_selected_notification(ad->tr_type, noti_id, opp_role);
			}
		}

		ad->send_after_turning_on = FALSE;
		_bt_terminate_app();
	}
	FN_END;
}

gboolean __bt_share_is_battery_low(void)
{
	FN_START;

	int value = 0;
	int charging = 0;

	if (vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, (void *)&charging))
		ERR("Get the battery charging status fail");

	if (charging == 1)
		return FALSE;

	DBG("charging: %d", charging);

	if (vconf_get_int(VCONFKEY_SYSMAN_BATTERY_STATUS_LOW, (void *)&value)) {
		ERR("Get the battery low status fail");
		return FALSE;
	}

	if (value <= VCONFKEY_SYSMAN_BAT_POWER_OFF)
		return TRUE;

	FN_END;
	return FALSE;
}

int _bt_share_enable_bt(bt_share_appdata_t *ad)
{
	FN_START;
	int ret;
	retv_if(ad == NULL, -1);
/*
	if (__bt_share_is_battery_low() == TRUE) {
		// Battery is critical low
		_bt_main_create_information_popup(ad, BT_STR_LOW_BATTERY);
		return -1;
	}
*/

	ret = bt_adapter_enable();
	if (ret == BT_ERROR_ALREADY_DONE) {
		_bt_cb_state_changed(BT_ERROR_NONE, BT_ADAPTER_ENABLED, ad);
	} else if (ret == BT_ERROR_NOW_IN_PROGRESS) {
		ERR("Enabling in progress [%d]", ret);
	} else if (ret != BT_ERROR_NONE) {
		ERR("Failed to enable bluetooth [%d]", ret);
	} else {
		ad->turning_on_popup = _bt_share_add_turning_on_popup(ad);
		ad->send_after_turning_on = TRUE;
	}
	FN_END;
	return 0;
}

void _bt_share_toolbar_button_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	FN_START;
	ret_if(!data);
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	bt_share_abort_data_t *abort_data = g_new0(bt_share_abort_data_t, 1);

	const char *text = elm_object_text_get(obj);

	if (g_strcmp0(text, BT_STR_STOP) == 0) {
		if (ad->transfer_info) {
			abort_data->transfer_id = ad->transfer_info->transfer_id;
			abort_data->transfer_type = ad->transfer_info->transfer_type;
			_bt_abort_signal_send(ad, abort_data);
		}
	} else if (g_strcmp0(text, BT_STR_RESEND_FAILED_FILES) == 0) {
		/* for BT off case */
		if (ad->bt_status == BT_ADAPTER_DISABLED) {
			_bt_share_enable_bt(ad);
		} else {
			_bt_share_ui_retry_failed(ad);
			if (ad->launch_mode == BT_LAUNCH_TRANSFER_LIST) {
				int noti_id;
				char *opp_role;
				bt_tr_db_table_e table;

				__bt_get_noti_id_opp_role_and_table(ad, &noti_id, &opp_role, &table);

				DBG_SECURE("Notification ID: %d", noti_id);
				if (noti_id < 0) {
					ERR("Invalid Notification ID");
				} else {
					sqlite3 *db = bt_share_open_db();
					bt_share_remove_tr_data_by_notification(db, table, noti_id);
					bt_share_close_db(db);
					_bt_delete_selected_notification(ad->tr_type, noti_id, opp_role);
				}
			}
			_bt_terminate_app();
		}
	}

	g_free(abort_data);
	FN_END;
}

Evas_Object * _bt_share_create_toolbar_button(bt_share_appdata_t *ad, char *text)
{
	Evas_Object *layout = NULL;
	Evas_Object *toolbar_button = NULL;

	layout = elm_layout_add(ad->navi_fr);
	elm_layout_file_set(layout, EDJFILE, "toolbar_button_ly");

	toolbar_button = elm_button_add(layout);

	/* Use "bottom" style button */
	elm_object_style_set(toolbar_button, "bottom");
	evas_object_size_hint_weight_set(toolbar_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(toolbar_button, EVAS_HINT_FILL, 0.5);

	elm_object_text_set(toolbar_button, text);

	evas_object_smart_callback_add(toolbar_button, "clicked",
			_bt_share_toolbar_button_cb, ad);

	/* Set button into "toolbar" swallow part */
	elm_object_part_content_set(layout, "button", toolbar_button);
	elm_object_item_part_content_set(ad->navi_it, "toolbar", layout);
	ad->toolbar_ly = layout;
	return toolbar_button;
}

void _bt_share_delete_toolbar_button(bt_share_appdata_t *ad)
{
	FN_START;
	ret_if(!ad);

	if (ad->toolbar_button) {
		evas_object_del(ad->toolbar_button);
		ad->toolbar_button = NULL;
	}
	if (ad->toolbar_ly) {
		evas_object_del(ad->toolbar_ly);
		ad->toolbar_ly = NULL;
	}
	FN_END;
}

static Eina_Bool __bt_list_item_add(bt_share_appdata_t *ad)
{
	FN_START;
	int i;

	retv_if(!ad, EINA_FALSE);
	GSList *tr_data_list = ad->tr_data_list;
	retv_if(ad->launch_mode == BT_LAUNCH_TRANSFER_LIST && !tr_data_list, EINA_FALSE);

	/* Add first 5 genlist items */
	for (i = 0; NULL != tr_data_list && i < 5; i++) {
		_bt_genlist_prepend_tr_data_item(ad,  ad->tr_genlist, tr_data_list->data,
						ad->tr_type);
		tr_data_list = g_slist_next(tr_data_list);
	}

	if (ad->launch_mode == BT_LAUNCH_TRANSFER_LIST &&
			ad->tr_type == BT_TR_OUTBOUND && ad->transfer_info->failed) {
		ad->toolbar_button = _bt_share_create_toolbar_button(ad,
				BT_STR_RESEND_FAILED_FILES);
	}

	FN_END;

	return EINA_TRUE;
}

static Eina_Bool __bt_list_item_idler(void *data)
{
	FN_START;

	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;

	retv_if(!ad, EINA_FALSE);
	GSList *tr_data_list = ad->tr_data_list;
	retv_if(ad->launch_mode == BT_LAUNCH_TRANSFER_LIST && !tr_data_list, EINA_FALSE);

	DBG("Total Items in List : %d", g_slist_length(tr_data_list));
	/* Add rest of the genlist items */
	if (g_slist_length(ad->tr_data_list) >= 5) {
		tr_data_list = g_slist_nth(ad->tr_data_list, 5);

		while (NULL != tr_data_list) {
			_bt_genlist_prepend_tr_data_item(ad,  ad->tr_genlist, tr_data_list->data,
							ad->tr_type);
			tr_data_list = g_slist_next(tr_data_list);
		}
	}

	if (ad->launch_mode == BT_LAUNCH_TRANSFER_LIST &&
			ad->tr_type == BT_TR_OUTBOUND &&
			ad->transfer_info->failed &&
			ad->toolbar_button == NULL) {
		ad->toolbar_button = _bt_share_create_toolbar_button(ad,
				BT_STR_RESEND_FAILED_FILES);
	}

	/* Delete the notification */
	/* TODO: Delete Notification only if
	 * transfer(sent) is completed with no failed items or received screen related to this session */
	if (ad->launch_mode == BT_LAUNCH_TRANSFER_LIST &&
			((ad->tr_type == BT_TR_OUTBOUND &&
					ad->transfer_info->failed == 0) || ad->tr_type == BT_TR_INBOUND)) {
		int noti_id;
		char *opp_role;
		bt_tr_db_table_e table;

		__bt_get_noti_id_opp_role_and_table(ad, &noti_id, &opp_role, &table);

		DBG_SECURE("Notification ID: %d", noti_id);
		if (noti_id < 0) {
			ERR("Invalid Notification ID");
		} else {
			if (ad->bt_status == BT_ADAPTER_DISABLED) {
				sqlite3 *db = bt_share_open_db();
				bt_share_remove_tr_data_by_notification(db, table, noti_id);
				bt_share_close_db(db);
			}
			_bt_delete_selected_notification(ad->tr_type, noti_id, opp_role);
		}
	}

	if (ad->status_item)
		elm_genlist_item_fields_update(ad->status_item, "*",
				ELM_GENLIST_ITEM_FIELD_TEXT);

	FN_END;
	return EINA_FALSE;
}

int _bt_create_transfer_view(bt_share_appdata_t *ad)
{
	FN_START;
	retv_if(ad == NULL, -1);

	Elm_Object_Item *navi_it = NULL;
	Elm_Object_Item *git = NULL;
	Evas_Object *conform = NULL;
	Evas_Object *navi_fr = NULL;
	Evas_Object *bg = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *genlist = NULL;

	__bt_clear_view(ad);
	bg = _bt_create_bg(ad->win, NULL);
	retv_if(bg == NULL, -1);
	ad->bg = bg;

	conform = _bt_create_conformant(ad->win, NULL);
	retvm_if(conform == NULL, -1, "conform is NULL!");
	ad->conform = conform;

	bg = elm_bg_add(conform);
	elm_object_style_set(bg, "indicator/headerbg");
	elm_object_part_content_set(conform, "elm.swallow.indicator_bg", bg);
	evas_object_show(bg);

	layout = _bt_create_layout(conform, EDJFILE, "share_view");
	ad->tr_view = layout;

	elm_object_content_set(conform, layout);
	elm_win_conformant_set(ad->win, EINA_TRUE);

	navi_fr = __bt_create_naviframe(ad);
	retvm_if(navi_fr == NULL, -1, "navi_fr is NULL!");
	ad->navi_fr = navi_fr;

	/* Genlist */
	genlist = __bt_add_tr_data_genlist(layout, ad);
	retvm_if(genlist == NULL, -1, "genlist is NULL!");
	ad->tr_genlist = genlist;

	git = elm_genlist_item_append(genlist,
			ad->tr_device_itc, ad,
			NULL, ELM_GENLIST_ITEM_NONE,
			NULL, NULL);
	if (git == NULL) {
		ERR("elm_genlist_item_append is failed!");
	} else {
		elm_genlist_item_select_mode_set(git,
					 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		ad->device_item = git;
	}

	if (ad->launch_mode == BT_LAUNCH_TRANSFER_LIST) {
		git = elm_genlist_item_append(genlist,
				ad->tr_status_itc, ad,
				NULL, ELM_GENLIST_ITEM_NONE,
				NULL, NULL);
		if (git == NULL) {
			ERR("elm_genlist_item_append is failed!");
		} else {
			elm_genlist_item_select_mode_set(git,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			ad->status_item = git;
		}
	} else if (ad->launch_mode == BT_LAUNCH_ONGOING) {
		if (ad->progress_item) {
			elm_object_item_del(ad->progress_item);
			ad->progress_item = NULL;
			ad->progressbar = NULL;
		}

		git = elm_genlist_item_append(genlist,
				ad->tr_progress_itc, ad,
				NULL, ELM_GENLIST_ITEM_NONE,
				NULL, NULL);
		if (git == NULL) {
			ERR("elm_genlist_item_append is failed!");
		} else {
			elm_genlist_item_select_mode_set(git,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			ad->progress_item = git;
		}
	}

	__bt_add_file_title_item(ad);

	navi_it = elm_naviframe_item_push(navi_fr, NULL,
					NULL, NULL, genlist, NULL);
	if (navi_it == NULL) {
		ERR("elm_naviframe_item_push is failed!");
	} else {
		elm_object_item_domain_translatable_text_set(navi_it, BT_COMMON_PKG,
		(ad->tr_type == BT_TR_INBOUND) ? "IDS_ST_HEADER_RECEIVE" : "IDS_ST_HEADER_SEND");
		elm_naviframe_item_pop_cb_set(navi_it, __bt_back_button_cb, ad);
		ad->navi_it = navi_it;
	}

	if (ad->launch_mode == BT_LAUNCH_ONGOING) {
		ad->toolbar_button = _bt_share_create_toolbar_button(ad,
				BT_STR_STOP);
	}

	__bt_list_item_add(ad);

	ad->idler = ecore_idler_add(__bt_list_item_idler, ad);
	if (!ad->idler)
		ERR("idler can not be added");

	FN_END;
	return 0;
}
