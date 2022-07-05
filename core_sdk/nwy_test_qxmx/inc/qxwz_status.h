/*------------------------------------------------------------------------------
* qxwz_status.h : SDK status codes 
*          Copyright (C) 2015-2017 by QXSI, All rights reserved.
*/
#ifndef __QXWZ_STATUS_CODES_H__
#define __QXWZ_STATUS_CODES_H__

enum _status_{
    QXWZ_MRTK_STATUS_NONE = -1,
     
    QXWZ_MRTK_STATUS_SUCCESS = 100,
    QXWZ_MRTK_STATUS_START = 101,
    QXWZ_MRTK_STATUS_STOP = 102,

    QXWZ_STATUS_AUTH_SUCCESS = 200,
    QXWZ_STATUS_AUTH_LOCAL_FAIL,
    QXWZ_STATUS_AUTH_NETWORK_FAIL,
    QXWZ_STATUS_AUTH_ACCOUNT_ERROR,
    QXWZ_STATUS_AUTH_TIMESTAMP_TIMEOUT,

    QXWZ_STATUS_NTRIP_CONNECTED = 1000, //已连接到ntrip服务器
    QXWZ_STATUS_NTRIP_DISCONNECTED = 1001, //已断开与ntrip服务器的连接
    QXWZ_STATUS_APPKEY_IDENTIFY_FAILURE = 1002, //APP KEY认证失败
    QXWZ_STATUS_APPKEY_IDENTIFY_SUCCESS = 1003, //APP KEY认证成功
    QXWZ_STATUS_NETWORK_UNAVAILABLE = 1004, //网络异常
    QXWZ_STATUS_NTRIP_USER_MAX = 1005, //APP KEY用户已经达到上限
    QXWZ_STATUS_NTRIP_USER_NOT_EXIST = 1006, //Ntrip用户不存在
    QXWZ_STATUS_NTRIP_USER_IDENTIFY_SUCCESS = 1007, //Ntrip认证成功
    QXWZ_STATUS_ILLEGAL_GGA = 1011,      //非法GGA
    QXWZ_STATUS_GGA_SEND_TIMEOUT = 1012, //发送GGA超时
    QXWZ_STATUS_NTRIP_CONNECTING = 1013, //正在连接ntrip服务器
    QXWZ_STATUS_NTRIP_RECEIVING_DATA = 1014, //正在接收ntrip服务器数据
    QXWZ_STATUS_ILLEGAL_APP_KEY = 1015, //非法APP KEY
    QXWZ_STATUS_ILLEGAL_APP_SECRET = 1016, //非法APP SECRET
    QXWZ_STATUS_ILLEGAL_DEVICE_TYPE = 1017, //非法Device type
    QXWZ_STATUS_ILLEGAL_DEVICE_ID = 1018, //非法Device id
    QXWZ_STATUS_ACQUIRE_NTRIP_USER_FAILURE = 1019, //无法获取差分用户


    QXWZ_STATUS_SDK_INTERNAL_ERROR = 1020,      // SDK内部错误
    QXWZ_STATUS_NTRIP_RTCM_SUCCESS = 1021,      // Ntrip播发数据正常
    QXWZ_STATUS_NTRIP_UNAUTHORIZED = 1022,      // Ntrip认证失败
    QXWZ_STATUS_NULL_APP_KEY = 1023,            //APP KEY不能为空
    QXWZ_STATUS_NULL_APP_SECRET =1024,          //APP SECRET不能为空
    QXWZ_STATUS_NULL_DEVICE_TYPE = 1025,        //Device type不能为空
    QXWZ_STATUS_NULL_DEVICE_ID = 1026,          //Device id不能为空
    QXWZ_STATUS_API_FORBIDDEN           = 1034,
    QXWZ_STATUS_CONFIG_NULL = 1035,                //Config为空
    QXWZ_STATUS_NO_SETTING_INIT_FUNCTION = 1036,   //开发者没有调用setting函数

    QZWZ_STATUS_RTCM_SERVER_DOWN        = 1039,
    QXWZ_STATUS_GGA_OUT_OF_SERVICE_AREA = 1040,
    QXWZ_STATUS_GGA_OUT_OF_CONTROL_AREA = 1041,
    QXWZ_STATUS_NETWORK_CONGESTION      = 1042,
    QXWZ_STATUS_SERVICE_TIMEOUT         = 1043,
    QXWZ_STATUS_SERVICE_UNAVALIABLE     = 1044,
    QXWZ_STATUS_OSS_CLIENT_ALLOCATED    = 1045,
    QXWZ_STATUS_OSS_CLIENT_RELEASED     = 1046,
    QXWZ_STATUS_OSS_CLIENT_UNCONNECTED  = 1047,
    QXWZ_STATUS_OSS_CLIENT_CONNECTING   = 1048,
    QXWZ_STATUS_OSS_CLIENT_CONNECTED    = 1049,
    QXWZ_STATUS_OSS_CLIENT_DISCONNECTING = 1050,

    QXWZ_STATUS_OPENAPI_PARAM_MISSING = 2001, //缺少参数
    QXWZ_STATUS_OPENAPI_ACCOUNT_NOT_EXIST = 2002, //账号不存在
    QXWZ_STATUS_OPENAPI_DUPLICATE_ACCOUNT = 2003, //账号重复
    QXWZ_STATUS_OPENAPI_INCORRECT_PASSWORD = 2004, //错误密码
    QXWZ_STATUS_OPENAPI_DISABLED_ACCOUNT = 2005, //账号未激活
    QXWZ_STATUS_OPENAPI_NO_AVAILABLE_ACCOUNT = 2006, //没有有效的账号
    QXWZ_STATUS_OPENAPI_NO_RELATED_POPUSER = 2007, //POPUser不存在
    QXWZ_STATUS_OPENAPI_SYSTEM_ERROR = 2008, //服务端内部错误
    QXWZ_STATUS_NTRIP_SERVER_DISCONNECTED = 2009,//Ntrip服务器断开Socket连接
    QXWZ_STATUS_OPENAPI_ACCOUNT_EXPIRED = 2010, //账号已过期，需续费
    QXWZ_STATUS_OPENAPI_ACCOUNT_TOEXPIRE = 2011, //账号即将过期
    QXWZ_STATUS_OPENAPI_BINDMODEMISMATCH_EXPIRE = 2012, //当前账号无法自动绑定
    QXWZ_STATUS_OPENAPI_PARAMETER_ERROR,
    QXWZ_STATUS_OPENAPI_UNKNOWN_ERROR,

    QXWZ_STATUS_SERVER_INTERNAL_ERROR   = 9997,
    QXWZ_STATUS_INVALID_REQUEST         = 9998,
    QXWZ_STATUS_UNKNOWN                 = 9999,

    QXWZ_MRTK_STATUS_WORK = 10000,
    //SOCKET 
    QXWZ_SOC_ACCOUNT_ID_NOT_VAILD = 10001,
    QXWZ_SOC_GET_ACCOUNT_ID_ERROR = 10002,
    QXWZ_SOC_CREATE_ERROR = 10003,
    QXWZ_SOC_CONNECT_ERROR = 10004,
    QXWZ_SOC_SEND_ERROR = 10005,
    QXWZ_SOC_CLOSE_ERROR = 10006,
    QXWZ_SOC_GPRS_TIMEOUT = 10007,
    QXWZ_SOC_CONNECT_TIMEOUT = 10008,
    QXWZ_SOC_SEND_TIMEOUT = 10009,
    QXWZ_SOC_RECV_TIMEOUT = 10010,
    QXWZ_SOC_RESPONSE_TIMEOUT = 10011,
    QXWZ_SOC_PACKET_NOT_COMPLETE = 10012,
    QXWZ_SOC_PACKET_TOO_BIG = 10013,
    QXWZ_SOC_SESSION_REQUEST_TOO_LONG = 10014,
    QXWZ_SOC_CLOSE_TIMEOUT = 10015,
    QXWZ_SOC_GET_HOST_NAME_ERROR = 10016,

    //RESOURCES
    QXWZ_MEM_POOL_CREATE_ERROR = 20001,   //ABORT
    QXWZ_MEM_ALLOC_ERROR = 20002,         //ABORT
    QXWZ_TIMER_CREATE_ERROR = 20003,      //ABORT
    QXWZ_EVENT_SCHEDULER_NULL_PTR = 20004,
    QXWZ_TIMER_ID_NOT_IN_RANGE = 20005,
    QXWZ_TAKE_MUTEX_ERROR = 20006,
    QXWZ_GIVE_MUTEX_ERROR = 20007,

    //RTK Algo
    QXWZ_RTK_MEM_ALLOC_FAILED = 30001, //ABORT
    QXWZ_RTK_OPEN_FILE_FAILED = 30002,
    QXWZ_RTK_NULL_PTR = 30003,
    QXWZ_RTK_REF_DATA_REPEATED = 30004,
    QXWZ_RTK_ROV_DATA_REPEATED = 30005,
    QXWZ_RTK_ROV_OBS_DATA_NULL = 30006,
    QXWZ_RTK_REF_OBS_DATA_NULL = 30007,
    QXWZ_RTK_EPOCH_DATA_ALREADY_PROCESSED = 30008,
    QXWZ_RTK_ROV_SAT_NUM_NOT_ENOUGH = 30009,
    QXWZ_RTK_REF_SAT_NUM_NOT_ENOUGH = 30010,
    QXWZ_RTK_BRDC_EPH_NOT_ENOUGH_FOR_REF = 30011,
    QXWZ_RTK_BRDC_EPH_NOT_ENOUGH_FOR_ROV = 30012,
    QXWZ_RTK_BRDC_EPH_EMPTY = 30013,
    QXWZ_RTK_BRDC_GEPH_EMPTY = 30014,
    QXWZ_RTK_SAME_SAT_NOT_ENOUGH = 30015,
    QXWZ_RTK_NOT_ENOUGH_VALID_GEO_DATA = 30016,
    QXWZ_RTK_EPOCH_TIMESTAMP_INVALID = 30017,   
    QXWZ_RTK_SPP_FAILED = 30018,
    QXWZ_RTK_SPP_OBS_DATA_NULL = 30019,
    QXWZ_RTK_SPP_SAT_NUM_NOT_ENOUGH = 30020,
    QXWZ_RTK_MATRIX_NULL = 30021,
    QXWZ_RTK_MATRIX_IDX_OUT_OF_RANGE = 30022,
    QXWZ_RTK_SAT_NOT_FOUND_OR_DISABLED = 30023,

    //AGNSS
    QXWZ_GET_IMSI_ERROR = 40001,
    QXWZ_GET_CELLID_ERROR = 40002,
    QXWZ_AGNSS_DECODE_ERROR = 40003,
    QXWZ_AGNSS_EPH_ERROR = 40004,

    //RAW DATA
    QXWZ_GNSS_UART_INIT_ERROR = 50001,
    QXWZ_GNSS_UART_SHUTDOWN_ERROR = 50002,
    QXWZ_GNSS_UART_RAW_DATA_NOT_AVAIL = 50003,
    QXWZ_GNSS_UART_NEMA_BROKEN = 50004,
    QXWZ_GNSS_UART_CHECKSUM_ERROR = 50005,
    QXWZ_GNSS_NO_GNSS_SIGNAL = 50006,

    //RTCM 
    QXWZ_RTCM_DECODE_ERROR = 60001,
};

#endif /*__QXWZ_STATUS_CODES_H__*/
