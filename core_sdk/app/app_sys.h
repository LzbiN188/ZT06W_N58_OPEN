#ifndef	APP_SYS
#define APP_SYS

#include "nwy_osi_api.h"


#define DEBUG_NONE			0
#define DEBUG_LOW			1
#define DEBUG_NET			2
#define DEBUG_GPS			3
#define DEBUG_FACTORY		4
#define DEBUG_ALL			9

#define ITEMCNTMAX					8
#define ITEMSIZEMAX					150


typedef struct
{
    uint8_t item_cnt;
    char item_data[ITEMCNTMAX][ITEMSIZEMAX + 1];
} ITEM;



void LogMessage(uint8_t level, char *debug);
void LogMessageWL(uint8_t level, char *debug, uint16_t len);
void LogPrintf(uint8_t level, const char *debug, ...);

int getCharIndex(uint8_t *src, int src_len, char ch);
void changeByteArrayToHexString(uint8_t *src, uint8_t *dest, uint16_t srclen);
int16_t changeHexStringToByteArray(uint8_t *dest, uint8_t *src, uint16_t size);
int16_t changeHexStringToByteArray_10in(uint8_t *dest, uint8_t *src, uint16_t size);

int my_strpach(char *str1, const char *str2);
int my_strstr(char *str1, const char *str2, int str1_len);
int my_getstrindex(char *str1, const char *str2, int len);
int mycmdPatch(uint8_t *cmd1, uint8_t *cmd2);

void stringToItem(ITEM *item, uint8_t *str, uint16_t len);
void stringToUpper(char *str, uint16_t strlen);

#endif
