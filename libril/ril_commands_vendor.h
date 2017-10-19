/* //device/libs/telephony/ril_commands.h
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
    {SAMSUNG_REQUEST_BASE, NULL}, // 10000
    {RIL_REQUEST_DIAL_EMERGENCY_CALL, /* dispatchDial */ NULL}, // 10001
    {RIL_REQUEST_CALL_DEFLECTION, /* dispatchVoid */ NULL}, // 10002
    {RIL_REQUEST_MODIFY_CALL_INITIATE, /* dispatchVoid */ NULL}, // 10003
    {RIL_REQUEST_MODIFY_CALL_CONFIRM, /* dispatchVoid */ NULL}, // 10004
    {RIL_REQUEST_SET_VOICE_DOMAIN_PREF, /* dispatchVoid */ NULL}, // 10005
    {RIL_REQUEST_SAFE_MODE, /* dispatchVoid */ NULL}, // 10006
    {RIL_REQUEST_SET_TRANSMIT_POWER, /* dispatchVoid */ NULL}, // 10007
    {RIL_REQUEST_GET_CELL_BROADCAST_CONFIG, /* dispatchVoid */ NULL}, // 10008
    {RIL_REQUEST_GET_PHONEBOOK_STORAGE_INFO, /* dispatchVoid */ NULL}, // 10009
    {RIL_REQUEST_GET_PHONEBOOK_ENTRY, /* dispatchVoid */ NULL}, // 10010
    {RIL_REQUEST_ACCESS_PHONEBOOK_ENTRY, /* dispatchVoid */ NULL}, // 10011
    {RIL_REQUEST_USIM_PB_CAPA, /* dispatchVoid */ NULL}, // 10012
    {RIL_REQUEST_LOCK_INFO, /* dispatchVoid */ NULL}, // 10013
    {RIL_REQUEST_STK_SIM_INIT_EVENT, /* dispatchVoid */ NULL}, // 10014
    {RIL_REQUEST_SET_PREFERRED_NETWORK_LIST, /* dispatchVoid */ NULL}, // 10015
    {RIL_REQUEST_GET_PREFERRED_NETWORK_LIST, /* dispatchVoid */ NULL}, // 10016
    {RIL_REQUEST_CHANGE_SIM_PERSO, /* dispatchVoid */ NULL}, // 10017
    {RIL_REQUEST_ENTER_SIM_PERSO, /* dispatchVoid */ NULL}, // 10018
    {RIL_REQUEST_SEND_ENCODED_USSD, /* dispatchVoid */ NULL}, // 10019
    {RIL_REQUEST_CDMA_SEND_SMS_EXPECT_MORE, /* dispatchVoid */ NULL}, // 10020
    {RIL_REQUEST_HANGUP_VT, /* dispatchVoid */ NULL}, // 10021
    {RIL_REQUEST_HOLD, /* dispatchVoid */ NULL}, // 10022
    {RIL_REQUEST_SET_SIM_POWER, /* dispatchVoid */ NULL}, // 10023
    {10024, NULL}, // 10024
    {RIL_REQUEST_UICC_GBA_AUTHENTICATE_BOOTSTRAP, /* dispatchVoid */ NULL}, // 10025
    {RIL_REQUEST_UICC_GBA_AUTHENTICATE_NAF, /* dispatchVoid */ NULL}, // 10026
    {RIL_REQUEST_GET_INCOMING_COMMUNICATION_BARRING, /* dispatchVoid */ NULL}, // 10027
    {RIL_REQUEST_SET_INCOMING_COMMUNICATION_BARRING, /* dispatchVoid */ NULL}, // 10028
    {RIL_REQUEST_QUERY_CNAP, /* dispatchVoid */ NULL}, // 10029
    {RIL_REQUEST_SET_TRANSFER_CALL, /* dispatchVoid */ NULL}, // 10030
    {RIL_REQUEST_GET_DISABLE_2G, /* dispatchVoid */ NULL}, // 10031
    {RIL_REQUEST_SET_DISABLE_2G, /* dispatchVoid */ NULL}, // 10032
    {RIL_REQUEST_REFRESH_NITZ_TIME, /* dispatchVoid */ NULL}, // 10033
