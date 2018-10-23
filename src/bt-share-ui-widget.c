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

#include <app_extension.h>

#include "applog.h"
#include "bt-share-ui-widget.h"
#include "bt-share-ui-main.h"
#include <app_extension.h>

Evas_Object *_bt_create_bg(Evas_Object *parent, char *style)
{
	retvm_if(parent == NULL, NULL, "Invalid argument: parent is NULL\n");

	Evas_Object *bg = elm_bg_add(parent);

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,
				EVAS_HINT_EXPAND);

	if (style)
		elm_object_style_set(bg, style);

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, bg);
	evas_object_show(bg);

	return bg;
}

Evas_Object *_bt_create_layout(Evas_Object *parent, char *edj, char *content)
{
	Evas_Object *layout;

	retvm_if(parent == NULL, NULL, "Invalid argument: parent is NULL\n");

	layout = elm_layout_add(parent);

	if (edj != NULL && content != NULL)
		elm_layout_file_set(layout, edj, content);
	else {
		elm_layout_theme_set(layout, "layout", "application",
				     "default");
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND,
						 EVAS_HINT_EXPAND);
	}

	evas_object_show(layout);

	return layout;
}

Evas_Object *_bt_create_conformant(Evas_Object *parent, Evas_Object *content)
{
	Evas_Object *conform = NULL;

	elm_win_conformant_set(parent, 1);
	conform = elm_conformant_add(parent);

	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, conform);
	/* elm_object_style_set(conform, "internal_layout"); */
	evas_object_show(conform);

	if (content)
		elm_object_content_set(conform, content);

	return conform;
}

void _bt_share_genlist_item_text_update(Elm_Object_Item *git,
								const char *part) {
	ret_if(git == NULL || part == NULL);
	elm_genlist_item_fields_update(git, part, ELM_GENLIST_ITEM_FIELD_TEXT);
}

void _bt_share_genlist_item_content_update(Elm_Object_Item *git,
								const char *part) {
	ret_if(git == NULL || part == NULL);
	elm_genlist_item_fields_update(git, part, ELM_GENLIST_ITEM_FIELD_CONTENT);
}
