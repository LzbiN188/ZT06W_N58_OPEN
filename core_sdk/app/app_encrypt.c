/*
 * app_encrypt.c
 *
 *  Created on: May 8, 2022
 *      Author: idea
 */
#include "app_encrypt.h"
#include "app_port.h"
#include "app_sys.h"
#include <stdlib.h>
/*
 * @����
 */
static void encrypt(uint8_t *enc, uint8_t seed, uint8_t *denc)
{
    uint8_t i;
    uint8_t encryptData[4];
    //Դ����
    //����1
    for (i = 0; i < 4; i++)
    {
        encryptData[i] = (enc[i] + seed + i) % 0x100;
    }
    //�Ի�
    encryptData[0] = encryptData[0] ^ encryptData[3];
    encryptData[3] = encryptData[0] ^ encryptData[3];
    encryptData[0] = encryptData[0] ^ encryptData[3];
    encryptData[1] = encryptData[1] ^ encryptData[2];
    encryptData[2] = encryptData[1] ^ encryptData[2];
    encryptData[1] = encryptData[1] ^ encryptData[2];
    //����2
    for (i = 0; i < 4; i++)
    {
        encryptData[i] = encryptData[i] ^ (seed + i);
    }
    //����3
    encryptData[0] = encryptData[0] ^ encryptData[2];
    encryptData[1] = encryptData[1] ^ encryptData[3];
    memcpy(denc, encryptData, 4);
}
/*
 * @����
 */
static void decrypt(uint8_t *data, uint8_t seed, uint8_t *denc)
{
    uint8_t i;
    uint8_t dencryptData[4];
    //Դ����
    data[0] = data[0] ^ data[2];
    data[1] = data[1] ^ data[3];
    for (i = 0; i < 4; i++)
    {
        dencryptData[i] = data[i] ^ (seed + i);
    }

    dencryptData[0] = dencryptData[0] ^ dencryptData[3];
    dencryptData[3] = dencryptData[0] ^ dencryptData[3];
    dencryptData[0] = dencryptData[0] ^ dencryptData[3];
    dencryptData[1] = dencryptData[1] ^ dencryptData[2];
    dencryptData[2] = dencryptData[1] ^ dencryptData[2];
    dencryptData[1] = dencryptData[1] ^ dencryptData[2];
    //�Ի�

    for (i = 0; i < 4; i++)
    {
        dencryptData[i] = (dencryptData[i] + 0x100 - (seed + i)) % 0x100;
    }
    memcpy(denc, dencryptData, 4);
}

/*
 * @��������
 * @bref    �����������Ҫ����250���ֽڣ��������ݺ���4���ֽڵģ�����0����
 * @param
 *  seed    �������
 *  data    ����������
 *  len     ���������ݳ���
 *  encData �������ݴ�ŵ�ַ
 *  encLen  �������ݳ���
 * @return
 *  1       �ɹ�
 *  0       ʧ��
 */
uint8_t encryptStr(uint8_t seed, uint8_t *data, uint8_t len, uint8_t *encData, uint8_t *encLen)
{
    uint8_t usableSize, used, myEncDataLen;
    uint8_t en4[4], den4[4];
    if (data == NULL || len == 0 || encData == NULL || encLen == NULL)
    {
        return  0;
    }
    myEncDataLen = 0;
    encData[myEncDataLen++] = seed;
    used = 0;
    do
    {
        usableSize = (len - used) > 4 ? 4 : (len - used);
        memset(en4, 0, 4);
        memcpy(en4, data + used, usableSize);
        encrypt(en4, seed, den4);
        memcpy(encData + myEncDataLen, den4, 4);
        myEncDataLen += 4;
        used += usableSize;
    }
    while (used < len);
    *encLen = myEncDataLen;
    return 1;
}
/*
 * @��������
 * @bref    ���ڽ������ݺ���4�ֽڲ�����0���䣬���Խ��ܺ�0�����ݶ���
 * @param
 *  data    ����������
 *  len     ���������ݳ���
 *  encData �������ݴ�ŵ�ַ
 *  encLen  �������ݳ���
 * @return
 *  1       �ɹ�
 *  0       ʧ��
 */
uint8_t dencryptStr(uint8_t *data, uint8_t len, uint8_t *encData, uint8_t *encLen)
{
    uint8_t usableSize, used, myEncDataLen;
    uint8_t en4[4], den4[4], seed;
    if (data == NULL || len == 0 || encData == NULL || encLen == NULL)
    {
        return  0;
    }
    myEncDataLen = 0;
    seed = data[0];
    used = 1;
    do
    {
        usableSize = (len - used) > 4 ? 4 : (len - used);
        memset(en4, 0, 4);
        memcpy(en4, data + used, usableSize);
        decrypt(en4, seed, den4);
        memcpy(encData + myEncDataLen, den4, 4);
        myEncDataLen += 4;
        used += usableSize;
    }
    while (used < len);
    *encLen = myEncDataLen;
    return 1;
}




void createEncrypt(uint8_t *mac, uint8_t *encBuff, uint8_t *encLen)
{
    uint8_t debug[50];
    uint8_t src[50];
    uint8_t srcLen;
    uint8_t dec[50];
    uint8_t decLen;
    uint8_t i;

    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second; 

    portGetRtcDateTime(&year, &month, &date, &hour, &minute, &second);

    srcLen = 0;
    //����MAC��ַ
    for (i = 0; i < 6; i++)
    {
        src[srcLen++] = mac[i];
    }
    //��ʼ��Ч�ڣ�������ʱ��
    src[srcLen++] = year % 100;
    src[srcLen++] = month;
    src[srcLen++] = date;
    src[srcLen++] = hour;
    src[srcLen++] = minute;
    //��Чʱ��������
    src[srcLen++] = 5;
    i = rand();
	//������Կ
    encryptStr(i, src, srcLen, dec, &decLen);
	
    changeByteArrayToHexString(mac, debug, 6);
    debug[12] = 0;
    //LogPrintf(DEBUG_ALL, "MAC:[%s]", debug);

    changeByteArrayToHexString(dec, debug, decLen);
    debug[decLen * 2] = 0;
    //LogPrintf(DEBUG_ALL, "DEC:[%s],DECLEN:%d", debug, decLen);
    //LogPrintf(DEBUG_ALL, "LimiteTime:[%02d/%02d/%02d %02d:%02d],Limit Minute:%d", year, month, date, hour, minute, 5);

    memcpy(encBuff, dec, decLen);
    *encLen = decLen;
}

