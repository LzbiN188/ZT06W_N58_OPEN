#ifndef APP_DB
#define APP_DB

#include "nwy_osi_api.h"
#include "app_protocol.h"

#define DB_MAX_SIZE		20

typedef struct
{
    gpsRestore_s gpsBuff[DB_MAX_SIZE];
    uint8_t  gpsNum;
} dbSave_s;



void dbSaveInit(void);
void dbSaveAll(void);
void dbPush(gpsRestore_s *gpsresinfo);
uint8_t dbPost(void);



#endif
