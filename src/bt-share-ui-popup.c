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
#include <bluetooth-share-api.h>
#include <notification.h>

#include "applog.h"
#include "bt-share-ui-view.h"
#include "bt-share-ui-popup.h"
#include "bt-share-ui-ipc.h"
#include "bt-share-ui-resource.h"
#include "bt-share-ui-widget.h"
#include "bt-share-ui-main.h"

extern bt_share_appdata_t *app_state;

int _bt_update_progressbar(bt_share_appdata_t *ad,
			int transfer_id, const char *name, int percentage)
{
	retvm_if(ad == NULL, 0, "Invalid argument: ad is NULL");
	retvm_if(ad->transfer_info == NULL, 0, "Invalid argument: transfer_info is NULL");
	retvm_if(ad->transfer_info->transfer_id != transfer_id, 0, "Invalid transfer_id!");

	ad->transfer_info->percentage = percentage;
	if (ad->progress_item && ad->progressbar) {
		float i;
		i = (float)(percentage) / (float)100.0;
		elm_progressbar_value_set(ad->progressbar, i);

		elm_object_part_text_set(ad->progressbar, "elm.text.bottom.left",
				g_strdup_printf("%d%%", percentage));
	}

	return 0;
}

static gboolean __bt_info_popup_timer_cb(void *data, Evas_Object *obj,
				    void *event_info)
{
	DBG("+");
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	retv_if(ad == NULL, FALSE);

	_bt_destroy_info_popup(ad);

	if (ad->tr_view == NULL)
		_bt_terminate_app();

	DBG("-");
	return FALSE;
}

Evas_Object *_bt_create_info_popup(bt_share_appdata_t *ad,
					const char *text)
{
	FN_START;
	Evas_Object *popup = NULL;

	retv_if((!ad || !text), NULL);
	retv_if(ad->info_popup != NULL, NULL);

	popup = elm_popup_add(ad->win);
	elm_object_style_set(popup, "toast");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK,
			(Evas_Smart_Cb)__bt_info_popup_timer_cb, ad);
	elm_object_part_text_set(popup, "elm.text", text);
	evas_object_smart_callback_add(popup, "block,clicked",
			(Evas_Smart_Cb)__bt_info_popup_timer_cb, ad);

	if (!elm_config_access_get()) {
		elm_popup_timeout_set(popup, BT_INFO_POPUP_TIMEOUT_IN_SEC);
		evas_object_smart_callback_add(popup, "timeout",
			(Evas_Smart_Cb) __bt_info_popup_timer_cb, ad);
	} else {
		evas_object_smart_callback_add(popup, "access,read,stop",
			(Evas_Smart_Cb) __bt_info_popup_timer_cb, ad);
	}

	evas_object_show(popup);
	ad->info_popup = popup;

	FN_END;
	return popup;
}

static void
__bt_share_popup_block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_popup_dismiss(obj);
}

static void
__bt_share_popup_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_popup_dismiss(obj);
}

static void
__bt_share_popup_hide_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
}

static void
__bt_share_progressbar_popup_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Ecore_Timer *timer = data;
	ecore_timer_del(timer);
}

static Eina_Bool
__bt_share_progressbar_timer_cb(void *data)
{
	retv_if(!data, ECORE_CALLBACK_CANCEL);

	Evas_Object *popup = data;
	Evas_Object *progressbar = evas_object_data_get(popup, "progressbar");
	double value;

	value = elm_progressbar_value_get(progressbar);
	if (value == 1.0) {
		evas_object_data_del(popup, "timer");
		evas_object_del(popup);
		return ECORE_CALLBACK_CANCEL;
	}
	value = value + 0.01;
	elm_progressbar_value_set(progressbar, value);

	return ECORE_CALLBACK_RENEW;
}

Evas_Object *_bt_share_add_turning_on_popup(bt_share_appdata_t *ad)
{
	FN_START;
	retv_if(ad == NULL, NULL);
	Evas_Object *popup = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *progressbar = NULL;
	Ecore_Timer *timer = NULL;

	popup = elm_popup_add(ad->win);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __bt_share_popup_hide_cb, NULL);
	evas_object_smart_callback_add(popup, "dismissed", __bt_share_popup_hide_finished_cb, NULL);
	evas_object_smart_callback_add(popup, "block,clicked", __bt_share_popup_block_clicked_cb, NULL);

	/* layout */
	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, EDJFILE, "turning_on_view_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(layout, "elm.text", BT_STR_TURNING_ON_BLUETOOTH_ING);

	progressbar = elm_progressbar_add(layout);
	elm_object_style_set(progressbar, "process_medium");
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	elm_object_part_content_set(layout, "processing", progressbar);
	timer = ecore_timer_add(0.1, __bt_share_progressbar_timer_cb, popup);

	elm_object_content_set(popup, layout);

	evas_object_data_set(popup, "progressbar", progressbar);
	evas_object_data_set(popup, "timer", timer);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL,
			__bt_share_progressbar_popup_del_cb, timer);

	evas_object_show(popup);

	FN_END;
	return popup;
}

int _bt_destroy_info_popup(bt_share_appdata_t *ad)
{
	DBG("+");
	retvm_if(ad == NULL, 0,
		 "Invalid argument: ad is NULL\n");

	if (ad->info_popup) {
		DBG("delete popup");
		evas_object_del(ad->info_popup);
		ad->info_popup = NULL;
	}

	DBG("-");
	return 0;
}

