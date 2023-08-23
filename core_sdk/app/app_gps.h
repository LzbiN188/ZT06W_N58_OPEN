#ifndef APP_GPS
#define APP_GPS

#include "nwy_osi_api.h"


#define GPSFIXED			1
#define GPSINVALID			0

#define GPSITEMCNTMAX		24
#define GPSITEMSIZEMAX		20

#define GPSFIFOSIZE			7

typedef struct
{
    unsigned char item_cnt;
    char item_data[GPSITEMCNTMAX][GPSITEMSIZEMAX];
} gpsitem_s;

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    int8_t hour;	//必须为有符号的
    uint8_t minute;
    uint8_t second;
} datetime_s;

typedef struct
{
    datetime_s datetime;
    char NS;
    char EW;
    uint8_t fixmode;
    uint8_t fixAccuracy;
    uint8_t used_star;
    uint8_t gpsviewstart;
    uint8_t beidouviewstart;
    uint8_t glonassviewstart;
    uint8_t hadupload;
    uint8_t beidouCn[30];
    uint8_t gpsCn[30];
    uint8_t fixstatus;
    uint16_t course;
    float pdop;
    float hight;
    double latitude;	//纬度
    double longtitude;	//经度
    double speed;
} gpsinfo_s;

typedef struct
{
    uint8_t currentindex;
    gpsinfo_s gpsinfohistory[GPSFIFOSIZE];
    gpsinfo_s lastfixgpsinfo;
} gpsfifo_s;

typedef struct
{
    datetime_s datetime;
    double latitude;
    double longtitude;
    uint8_t init;
} lastUploadPosition_s;

typedef struct
{
	datetime_s datetime;
    double latitude;
    double longtitude;
    uint8_t init;
}lastMilePosition_s;

typedef enum
{
    NMEA_RMC = 1,
    NMEA_GSA,
    NMEA_GGA,
    NMEA_GSV,
} nmeatype_e;


void nmeaParser(uint8_t *buf, uint16_t len);

void gpsClearCurrentGPSInfo(void);
gpsinfo_s *getCurrentGPSInfo(void);
gpsinfo_s *getLastFixedGPSInfo(void);
gpsfifo_s *getGSPfifo(void);

datetime_s changeUTCTimeToLocalTime(datetime_s utctime, int8_t localtimezone);
void portUpdateLocalTime(uint8_t y, uint8_t m, uint8_t d, uint8_t hh, uint8_t mm, uint8_t ss, int8_t utc);

void gpsUploadPointToServer(void);

void ClearLastMilePoint(void);
void gpsMileRecord(void);


#endif
