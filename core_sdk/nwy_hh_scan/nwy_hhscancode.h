/*
 * CUDW.h
 *
 *  Created on: 2018年01月19日
 *  release 0.2.19-CUDW-SDK-20190418
 *      Author: Cloud
 */

#ifndef _NWY_AT_HHSCANCODE_H_
#define _NWY_AT_HHSCANCODE_H_

#ifdef FEATURE_NWY_HHSCANCODE_SUPPORT

/* return value */
#define	RET_SUCCESS                 0
#define	RET_ERR_PARAM               -1
#define	RET_FAILED                  -2
#define RET_AUTH_FAILED_3           -3
#define RET_TIMEISEXPIRE            -4
#define RET_VERIFY_IS_NULL          -5
#define RET_NO_RESULT               -6

/* code type define */
#define SET_CODETYPE_EAN13          1
#define SET_CODETYPE_EAN8           2
#define SET_CODETYPE_UPCA           3
#define SET_CODETYPE_UPCE           4
#define SET_CODETYPE_C128           5
#define SET_CODETYPE_C39            6
#define SET_CODETYPE_ITF25          7
#define SET_CODETYPE_CBAR           8
#define SET_CODETYPE_C93            9
#define SET_CODETYPE_PDF417		    32
#define SET_CODETYPE_QR			    33
#define SET_CODETYPE_DATAMATRIX	    34



/* SetBar paramter define */
#define SET_CLASS_ENABLE            0x10001000
#define SET_CLASS_PROPERTY          0x10001001
#define SET_CLASS_LENGTH            0x10001002


/* SET_CLASS_ENABLE begin */

// 条码使能开关
#define SET_VAL_DISABLE             0           // 关闭使能
#define SET_VAL_ENABLE              1           // 开启使能

/* SET_CLASS_ENABLE end */



/* SET_CLASS_PROPERTY begin */

// 通用码制属性，种类有:CodeBar\ITF25\Matrix25\STD25\IDSTL25\DTLOGC25\NEC25
//|_________________________________________________________________________________________
//| bit0     | bit1     | bit2     | bit3     | bit4     | bit5     | bit6     | bit7      |
//|__________|__________|__________|__________|__________|__________|__________|___________|
//| checksum | undef    | undef    | strip    | undef    | undef    | undef    | undef     |
//|__________|__________|__________|__________|__________|__________|__________|___________|
#define SET_VAL_NO_CHECK            0           // Disable checksum checking (default)
#define SET_VAL_CHECKSUM            1           // Enable checksum checking
#define SET_VAL_STRIP               8           // Strip the checksum from the result string(与checksum属性组合使用)

// C39专用属性
//|_________________________________________________________________________________________
//| bit0     | bit1     | bit2     | bit3     | bit4     | bit5     | bit6     | bit7      |
//|__________|__________|__________|__________|__________|__________|__________|___________|
//| checksum | undef    | undef    | strip    | fullASC  | undef    | undef    | undef     |
//|__________|__________|__________|__________|__________|__________|__________|___________|
#define SET_VAL_C39_NO_CHECK        0x00        // Disable checksum checking (default)
#define SET_VAL_C39_CHECKSUM        0x01        // Enable checksum checking
#define SET_VAL_C39_STRIP           0x08        // Strip the checksum from the result string(与checksum属性组合使用)
#define SET_VAL_C39_FULLASCII       0x10        // Full ASCII

// QR专用属性
#define SET_VAL_QR_NORMAL           0x00        // Disable all
#define SET_VAL_QR_SMALL            0x01        // Enable QR small
#define SET_VAL_QR_LOW_CONTRAST     0x02        // Enable QR low contrast

/* SET_CLASS_PROPERTY end */




/* SET_CLASS_LENGTH begin */

// 最大最小长度定义，种类有:ITF25\Matrix25\NEC25\STD25\IDSTL25\DTLOGC25
//|____________________________________
//| byte0 byte1     | byte2 byte3      |
//|_________________|__________________|
//| min length      | max length       |
//|_________________|__________________|
#define SET_VAL_LENGTH(min, max)    ((min & 0xFFFF) + ((max & 0xFFFF) << 16))

/* SET_CLASS_LENGTH end */

/*
 * CDWiInit
 *
 * input param ：
 *               InBuf -- Verify string
 * return :
 *          =-1 is parameter error
 *          =-2 is invalid error
 *          = 0 is Ok
 */
int  CDWiInit(char *InBuf);

/*
 * CDWiDecodeBar
 *
 * input param ：
 *               nWidth    -- bmp width
 *               nHeight   -- bmp height
 *               pBmpBit   -- bmp buffer
 *               nDataSize -- pData size
 * output param:
 *               nType     -- code type
 *               pData     -- barcode info
 * return :
 *          =-1 is parameter error
 *          =-2 is invalid error
 *          = 0 is decode end
 *          > 0 is barcode length
 */
int  CDWiDecodeBar(int nWidth, int nHeight, char *pBmpBit, int *nType, char *pData, int nDataSize);

/*
 * CDWiSetBar
 *
 * input param ：
 *               nClass    -- SET_CLASS_ENABLE / SET_CLASS_PROPERTY / SET_CLASS_LENGTH
 *               nType     -- code type define
 *               nVal      -- property value
 * output param:
 * return :
 *          =-1 is parameter error
 *          =-2 is invalid error
 *          = 0 is set success
 */
int CDWiSetBar(int nClass, int nType, int nVal);

/*
 * CDWiGetVer
 *
 * input param ：
 *               nVerSize  -- pVer size, >= 36 Bytes
 * output param:
 *               pVer      -- output Version
 * return :
 *          =-1 is parameter error
 *          =-2 is invalid error
 *          >0  pVer length
 */
int  CDWiGetVer(char* pVer, unsigned int nVerSize);

int g98301_is_timeout1(void);
void g98301_reset_timeout1(void);
int g98301_is_timeout2(void);
void g98301_reset_timeout2(void);
void nwy_hh_init(void);
int Decoding_Image_HH(unsigned char *img_buffer, int width, int height);
unsigned int GetResultLength_HH(void);
int GetDecoderResult_HH(char *result);

#endif

#endif /* _NWY_AT_HHSCANCODE_H_ */

