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

#ifndef __DEF_BLUETOOTH_SHARE_UI_RES_H_
#define __DEF_BLUETOOTH_SHARE_UI_RES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libintl.h>

/*==============  String ================= */
#define BT_COMMON_PKG		"ug-setting-bluetooth-efl"
#define BT_COMMON_RES		"/usr/ug/res/locale/"

#define BT_PKGNAME "org.tizen.bluetooth-share-ui"
#define BT_PREFIX "/usr/apps/"BT_PKGNAME

#define BT_ICON_RECV_FAIL	"bluetooth_download_failed.png"
#define BT_ICON_RECV_PASS	"bluetooth_download_complete.png"
#define BT_ICON_SEND_FAIL	"bluetooth_upload_failed.png"
#define BT_ICON_SEND_PASS	"bluetooth_upload_complete.png"

#define BT_STR_UNABLE_TO_SEND	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_SENDINGFAIL")

#define BT_STR_SENDING \
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_SENDING_ING")
#define BT_STR_RECEIVING	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_RECEIVING_ING")
#define BT_STR_FILE_NOT_EXIST	\
	dgettext(BT_COMMON_PKG, "IDS_BT_TPOP_FILE_NOT_FOUND")
#define BT_STR_FILE_NOT_RECV	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_FILE_NOT_RECEIVED")
#define BT_STR_FILE_S	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_FILE_C_PS")
#define BT_STR_FAIL_S	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_FAILURE_REASON_C_PS")
#define BT_STR_FILE_SIZE_S	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_FILE_SIZE_C_PS")
#define BT_STR_FROM_S	\
	dgettext(BT_COMMON_PKG, "IDS_BT_HEADER_FROM_C_PS")
#define BT_STR_UNABLE_TO_RECEIVE	\
	dgettext(BT_COMMON_PKG, "IDS_BT_BODY_UNABLE_TO_RECEIVE")
#define BT_STR_SENT_FILES	\
	dgettext(BT_COMMON_PKG, "IDS_BT_HEADER_SENT_FILES")
#define BT_STR_RECEIVED_FILES	\
	dgettext(BT_COMMON_PKG, "IDS_BT_OPT_RECEIVED_FILES")
#define BT_STR_UNABLE_TO_RECEIVED_FILES	\
	dgettext(BT_COMMON_PKG, "IDS_BT_HEADER_UNABLE_TO_RECEIVE_FILES_ABB")
#define BT_STR_UNABLE_TO_FIND_APPLICATION	\
	dgettext(BT_COMMON_PKG, "IDS_MF_TPOP_UNABLE_TO_FIND_APPLICATION_TO_PERFORM_THIS_ACTION")
#define BT_STR_TR_1FILE_COPIED_STATUS \
	dgettext(BT_COMMON_PKG, "IDS_BT_MBODY_1_FILE_COPIED_PD_FAILED_ABB")
#define BT_STR_DEVICENAME   dgettext(BT_COMMON_PKG, "IDS_BT_BODY_DEVICENAME")
#define BT_STR_RECEIVING_FAILED   dgettext(BT_COMMON_PKG, "IDS_BT_MBODY_RECEIVING_FAILED")
#define BT_STR_FILE_RECEIVED   dgettext(BT_COMMON_PKG, "IDS_BT_MBODY_FILE_RECEIVED")
#define BT_STR_SENDING_FAILED   dgettext(BT_COMMON_PKG, "IDS_BT_MBODY_SENDING_FAILED")
#define BT_STR_FILE_SENT   dgettext(BT_COMMON_PKG, "IDS_BT_MBODY_FILE_SENT")
#define BT_STR_FILE_LIST dgettext(BT_COMMON_PKG, "IDS_FV_HEADER_FILE_LIST")
#define BT_STR_RECEIVED dgettext(BT_COMMON_PKG, "IDS_MSG_BUTTON_RECEIVED_M_MESSAGE")
#define BT_STR_SENT dgettext(BT_COMMON_PKG, "IDS_MSG_BODY_SENT_ABB")
#define BT_STR_FAILED dgettext(BT_COMMON_PKG, "IDS_EMAIL_BODY_FAILED_M_STATUS")
#define BT_STR_STOP dgettext(BT_COMMON_PKG, "IDS_BT_SK4_STOP")
#define BT_STR_WAITING dgettext(BT_COMMON_PKG, "IDS_ST_BODY_WAITING_ING")
#define BT_STR_RESEND_FAILED_FILES dgettext(BT_COMMON_PKG, "IDS_WIFI_POP_RESEND_FAILED_FILES")
#define BT_STR_VCF_FILES_ARE_TEMPORARY_AND_CANT_BE_OPENED dgettext(BT_COMMON_PKG, "IDS_BT_TPOP_VCF_FILES_ARE_TEMPORARY_AND_CANT_BE_OPENED")
#define BT_STR_RECEIVED_VCF_FILE_ALREADY_IMPORTED_TO_CONTACTS dgettext(BT_COMMON_PKG, "IDS_BT_TPOP_RECEIVED_VCF_FILE_ALREADY_IMPORTED_TO_CONTACTS")
#define BT_STR_PD_RECEIVED dgettext(BT_COMMON_PKG, "IDS_SAS_SBODY_PD_RECEIVED")
#define BT_STR_PD_SENT dgettext(BT_COMMON_PKG, "IDS_WIFI_SBODY_PD_SENT_M_STATUS")
#define BT_STR_PD_FAILED dgettext(BT_COMMON_PKG, "IDS_WIFI_SBODY_PD_FAILED")
#define BT_STR_TURNING_ON_BLUETOOTH_ING \
	dgettext(BT_COMMON_PKG, "IDS_BT_BODY_TURNING_ON_BLUETOOTH_ING")

/* Access information */
#define BT_STR_ACCES_INFO_MAX_LEN 512
#define BT_STR_ACC_ICON "Icon"

/* Length of Strings*/
#define BT_STR_PROGRESS_MAX_LEN 30

/* Genlist item parts */
#define BT_SHARE_ITEM_PART_DEVICE_NAME_TITLE "elm.text"
#define BT_SHARE_ITEM_PART_DEVICE_NAME "elm.text.sub"

#define BT_SHARE_ITEM_PART_TRANSFER_TYPE_TITLE "elm.text"
#define BT_SHARE_ITEM_PART_TRANSFER_STATUS "elm.text.sub"

#define BT_SHARE_ITEM_PART_PROGRESSBAR_FILE_NAME "elm.text.main"
#define BT_SHARE_ITEM_PART_PROGRESSBAR_ICON "elm.swallow.content"

#define BT_SHARE_ITEM_PART_FILE_LIST_TITLE "elm.text"

#define BT_SHARE_ITEM_PART_FILE_TRANSFER_STATUS_ICON "elm.swallow.icon"
#define BT_SHARE_ITEM_PART_FILE_TRANSFER_STATUS_TEXT "elm.text.sub"
#define BT_SHARE_ITEM_PART_FILE_NAME "elm.text"
#define BT_SHARE_ITEM_PART_FILE_SIZE "elm.text.sub.end"

#ifdef __cplusplus
}
#endif
#endif				/* __DEF_BLUETOOTH_SHARE_UI_RES_H_ */
