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

#ifndef __BT_SHARE_UI_WIDGET_H__
#define __BT_SHARE_UI_WIDGET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <Elementary.h>
#include <efl_extension_events.h>

Evas_Object *_bt_create_bg(Evas_Object *parent, char *style);

Evas_Object *_bt_create_layout(Evas_Object *parent, char *edj,
				char *content);

Evas_Object *_bt_create_conformant(Evas_Object *parent,
				   Evas_Object *content);

Eina_List *_bt_color_table_set(void);

Eina_List *_bt_font_table_set(void);

void _bt_share_genlist_item_text_update(Elm_Object_Item *git,
		const char *part);

void _bt_share_genlist_item_content_update(Elm_Object_Item *git,
		const char *part);

#ifdef __cplusplus
}
#endif
#endif				/* __BT_SHARE_UI_WIDGET_H__ */
