#ifndef AES_H
#define AES_H

#define AESKEY	"ZTINFO----ZTINFO"
/**
 * ���� p: ���ĵ��ַ������顣
 * ���� plen: ���ĵĳ���,���ȱ���Ϊ16�ı�����
 * ���� key: ��Կ���ַ������顣
 */
void aes(char *p, int plen, char *key);

/**
 * ���� c: ���ĵ��ַ������顣
 * ���� clen: ���ĵĳ���,���ȱ���Ϊ16�ı�����
 * ���� key: ��Կ���ַ������顣
 */
char deAes(char *c, int clen, char *key);


void encryptData(char *encrypt,unsigned char *encryptlen,char *data,unsigned char len);
char dencryptData(char *dencrypt, unsigned char *dencryptlen, char *data, unsigned char len);

#endif

