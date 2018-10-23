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

#ifndef __DEF_BT_SHARE_UI_IPC_H_
#define __DEF_BT_SHARE_UI_IPC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bluetooth-share-api.h"

#define BT_SHARE_UI_INTERFACE "User.BluetoothShare.UI"
#define BT_SHARE_UI_SIGNAL_OPPABORT "opp_abort"
#define BT_SHARE_UI_SIGNAL_SEND_FILE "send_file"
#define BT_SHARE_UI_SIGNAL_INFO_UPDATE "info_update"

#define BT_SHARE_ENG_INTERFACE "User.BluetoothShare.Engine"
#define BT_SHARE_ENG_OBJECT "/org/projectx/transfer_info"
#define BT_SHARE_ENG_SIGNAL_UPDATE_VIEW "update_view"
#define BT_INBOUND_TABLE	"inbound"
#define BT_OUTBOUND_TABLE	"outbound"

#define BT_EVENT_SERVICE "org.projectx.bt_event"
#define BT_OPP_CLIENT_PATH "/org/projectx/bt/opp_client"
#define BT_OPP_SERVER_PATH "/org/projectx/bt/opp_server"
#define BT_OPP_CONNECTED "OppConnected"
#define BT_OPP_CLIENT_DISCONNECTED "ShareOppClientDisconnected"
#define BT_OPP_SERVER_DISCONNECTED "ShareOppServerDisconnected"
#define BT_TRANSFER_STARTED "TransferStarted"
#define BT_TRANSFER_PROGRESS "TransferProgress"
#define BT_TRANSFER_COMPLETED "TransferCompleted"

#define BT_SHARE_UI_EVENT_BASE            ((int)(0x0000))		/**< No event */
#define BT_SHARE_UI_EVENT_GAP_BASE        ((int)(BT_SHARE_UI_EVENT_BASE + 0x0010))
								/**< Base ID for GAP Event */
#define BT_SHARE_UI_EVENT_SDP_BASE        ((int)(BT_SHARE_UI_EVENT_GAP_BASE + 0x0020))
								/**< Base ID for SDP events */
#define BT_SHARE_UI_EVENT_RFCOMM_BASE     ((int)(BT_SHARE_UI_EVENT_SDP_BASE + 0x0020))
								/**< Base ID for RFCOMM events */
#define BT_SHARE_UI_EVENT_NETWORK_BASE     ((int)(BT_SHARE_UI_EVENT_RFCOMM_BASE + 0x0020))
								/**< Base ID for NETWORK events */
#define BT_SHARE_UI_EVENT_HDP_BASE     ((int)(BT_SHARE_UI_EVENT_NETWORK_BASE + 0x0020))
								/**< Base ID for HDP events */
#define BT_SHARE_UI_EVENT_OPC_BASE  ((int)(BT_SHARE_UI_EVENT_HDP_BASE + 0x0020))
								/**< Base ID for OPC events */
#define BT_SHARE_UI_EVENT_OBEX_SERVER_BASE ((int)(BT_SHARE_UI_EVENT_OPC_BASE + 0x0020))
								/**< Base ID for Obex Server events */

#define BT_SHARE_UI_ERROR_BASE                   ((int)0)		/**< Error code base */
#define BT_SHARE_UI_ERROR_NONE                   ((int)0)		/**< No error #0 */
#define BT_SHARE_UI_ERROR_CANCEL                 ((int)BT_SHARE_UI_ERROR_BASE - 0x01)
#define BT_SHARE_UI_ERROR_NOT_CONNECTED          ((int)BT_SHARE_UI_ERROR_BASE - 0x15)
#define BT_SHARE_UI_ERROR_CANCEL_BY_USER         ((int)BT_SHARE_UI_ERROR_BASE - 0x1b)

/**
 * Bluetooth share event type
 */
typedef enum {
	BT_SHARE_UI_EVENT_OPC_CONNECTED = BT_SHARE_UI_EVENT_OPC_BASE,
								/* OPC Connected event */
	BT_SHARE_UI_EVENT_OPC_DISCONNECTED,		/* OPC Disonnected event */
	BT_SHARE_UI_EVENT_OPC_TRANSFER_STARTED,	/* OPC Transfer started event */
	BT_SHARE_UI_EVENT_OPC_TRANSFER_PROGRESS,	/* OPC Transfer progress event */
	BT_SHARE_UI_EVENT_OPC_TRANSFER_COMPLETE,	/* OPC Transfer Complete event */

	BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_AUTHORIZE = BT_SHARE_UI_EVENT_OBEX_SERVER_BASE,
								/* Obex server authorize event*/
	BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_STARTED,	/* Obex Server transfer started event*/
	BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_PROGRESS,/* Obex Server transfer progress event*/
	BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_COMPLETED,/* Obex Server transfer complete event*/
	BT_SHARE_UI_EVENT_OBEX_SERVER_CONNECTION_AUTHORIZE,
	BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_CONNECTED, /* Obex Transfer connected event */
	BT_SHARE_UI_EVENT_OBEX_SERVER_TRANSFER_DISCONNECTED, /* Obex Transfer disconnected event */
} bt_share_ui_event_type_t;

/**
 * Server type
 */
typedef enum {
	OPP_SERVER = 0x0,
	FTP_SERVER
} bt_share_server_type_t;

/**
 * Stucture to hold event information
 */
typedef struct {
	int event;	/**< event type */
	int result;	/**< Success or error value */
	void *param_data;
			/**<parameter data pointer */
	void *user_data;
} bt_share_event_param_t;

/**
 * Stucture to OPP client transfer information
 */
typedef struct {
	char *device_addr;
	char *filename;
	unsigned long size;
	int percentage;
} bt_share_transfer_info_t;

/**
 * Stucture to OPP/FTP server transfer information
 */
typedef struct {
	char *filename;
	char *device_name;
	char *file_path;
	char *type;
	int transfer_id;
	unsigned long file_size;
	int percentage;
	bt_share_server_type_t server_type;
	char *address;
	unsigned char *contact_auth_info;
} bt_share_server_transfer_info_t;


void _bt_signal_init(bt_share_appdata_t *ad);
void _bt_signal_deinit(bt_share_appdata_t *ad);
int _bt_share_ui_ipc_info_update(bt_share_appdata_t *ad, int uid);
int _bt_abort_signal_send(bt_share_appdata_t *ad,
		bt_share_abort_data_t *abort_data);
void _bt_share_ui_event_handler(int event, bt_share_event_param_t *param,
			       void *user_data);
void  _bt_share_ui_handle_update_view(bt_share_appdata_t *ad,
						char *table);
int _bt_share_ui_retry_failed(bt_share_appdata_t *ad);
void _bt_set_opc_launched_session(gboolean value);
#ifdef __cplusplus
}
#endif
#endif				/* __DEF_BT_SHARE_UI_IPC_H_ */
