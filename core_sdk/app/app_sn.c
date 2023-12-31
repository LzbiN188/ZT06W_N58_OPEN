#include "app_sn.h"
#include "app_sys.h"
#include "app_protocol.h"
#define		ENCRYPT		0
#define		DESCRYPT	1
#define     PINLEN      9

static unsigned char   KS[16][48];
static unsigned char   E[64];

static unsigned char Key1_Save[9] =  { 0x65,0x87,0x34,0x16,0x80,0x7A,0x9D,0x5B,0x00};
static unsigned char Key2_Save[9] =  { 0x3C,0x8A,0x2C,0x09,0xD8,0xBB,0x7D,0x1B,0x00};

static const unsigned char   e[] =
{
    32, 1, 2, 3, 4, 5,
    4, 5, 6, 7, 8, 9,
    8, 9,10,11,12,13,
    12,13,14,15,16,17,
    16,17,18,19,20,21,
    20,21,22,23,24,25,
    24,25,26,27,28,29,
    28,29,30,31,32, 1,
};
static const unsigned char   PC1_C[] =
{
    57,49,41,33,25,17, 9,
    1,58,50,42,34,26,18,
    10, 2,59,51,43,35,27,
    19,11, 3,60,52,44,36,
};
static const unsigned char   PC1_D[] =
{
    63,55,47,39,31,23,15,
    7,62,54,46,38,30,22,
    14, 6,61,53,45,37,29,
    21,13, 5,28,20,12, 4,
};
static const unsigned char   shifts[] =
{
    1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1,
};

// for setkey
static const unsigned char   PC2_C[] =
{
    14,17,11,24, 1, 5,
    3,28,15, 6,21,10,
    23,19,12, 4,26, 8,
    16, 7,27,20,13, 2,
};
// for setkey
static const unsigned char   PC2_D[] =
{
    41,52,31,37,47,55,
    30,40,51,45,33,48,
    44,49,39,56,34,53,
    46,42,50,36,29,32,
};

static const unsigned char   IP[] =
{
    58,50,42,34,26,18,10, 2,
    60,52,44,36,28,20,12, 4,
    62,54,46,38,30,22,14, 6,
    64,56,48,40,32,24,16, 8,
    57,49,41,33,25,17, 9, 1,
    59,51,43,35,27,19,11, 3,
    61,53,45,37,29,21,13, 5,
    63,55,47,39,31,23,15, 7,
};

/*
 * Final permutation, FP = IP^(-1)
 */

// For encrypt
static const unsigned char   FP[] =
{
    40, 8,48,16,56,24,64,32,
    39, 7,47,15,55,23,63,31,
    38, 6,46,14,54,22,62,30,
    37, 5,45,13,53,21,61,29,
    36, 4,44,12,52,20,60,28,
    35, 3,43,11,51,19,59,27,
    34, 2,42,10,50,18,58,26,
    33, 1,41, 9,49,17,57,25,
};

static const unsigned char   S[8][64] =
{
    {14, 4,13, 1, 2,15,11, 8, 3,10, 6,12, 5, 9, 0, 7,
    0,15, 7, 4,14, 2,13, 1,10, 6,12,11, 9, 5, 3, 8,
    4, 1,14, 8,13, 6, 2,11,15,12, 9, 7, 3,10, 5, 0,
    15,12, 8, 2, 4, 9, 1, 7, 5,11, 3,14,10, 0, 6,13},

    {15, 1, 8,14, 6,11, 3, 4, 9, 7, 2,13,12, 0, 5,10,
    3,13, 4, 7,15, 2, 8,14,12, 0, 1,10, 6, 9,11, 5,
    0,14, 7,11,10, 4,13, 1, 5, 8,12, 6, 9, 3, 2,15,
    13, 8,10, 1, 3,15, 4, 2,11, 6, 7,12, 0, 5,14, 9},

    {10, 0, 9,14, 6, 3,15, 5, 1,13,12, 7,11, 4, 2, 8,
    13, 7, 0, 9, 3, 4, 6,10, 2, 8, 5,14,12,11,15, 1,
    13, 6, 4, 9, 8,15, 3, 0,11, 1, 2,12, 5,10,14, 7,
    1,10,13, 0, 6, 9, 8, 7, 4,15,14, 3,11, 5, 2,12},

    {7,13,14, 3, 0, 6, 9,10, 1, 2, 8, 5,11,12, 4,15,
    13, 8,11, 5, 6,15, 0, 3, 4, 7, 2,12, 1,10,14, 9,
    10, 6, 9, 0,12,11, 7,13,15, 1, 3,14, 5, 2, 8, 4,
    3,15, 0, 6,10, 1,13, 8, 9, 4, 5,11,12, 7, 2,14},

    {2,12, 4, 1, 7,10,11, 6, 8, 5, 3,15,13, 0,14, 9,
    14,11, 2,12, 4, 7,13, 1, 5, 0,15,10, 3, 9, 8, 6,
    4, 2, 1,11,10,13, 7, 8,15, 9,12, 5, 6, 3, 0,14,
    11, 8,12, 7, 1,14, 2,13, 6,15, 0, 9,10, 4, 5, 3},

    {12, 1,10,15, 9, 2, 6, 8, 0,13, 3, 4,14, 7, 5,11,
    10,15, 4, 2, 7,12, 9, 5, 6, 1,13,14, 0,11, 3, 8,
    9,14,15, 5, 2, 8,12, 3, 7, 0, 4,10, 1,13,11, 6,
    4, 3, 2,12, 9, 5,15,10,11,14, 1, 7, 6, 0, 8,13},

    {4,11, 2,14,15, 0, 8,13, 3,12, 9, 7, 5,10, 6, 1,
    13, 0,11, 7, 4, 9, 1,10,14, 3, 5,12, 2,15, 8, 6,
    1, 4,11,13,12, 3, 7,14,10,15, 6, 8, 0, 5, 9, 2,
    6,11,13, 8, 1, 4,10, 7, 9, 5, 0,15,14, 2, 3,12},

    {13, 2, 8, 4, 6,15,11, 1,10, 9, 3,14, 5, 0,12, 7,
    1,15,13, 8,10, 3, 7, 4,12, 5, 6,11, 0,14, 9, 2,
    7,11, 4, 1, 9,12,14, 2, 0, 6,10,13,15, 3, 5, 8,
    2, 1,14, 7, 4,10, 8,13,15,12, 9, 0, 3, 5, 6,11},
};

/*
 * P is a permutation on the selected combination
 * of the current L and key.
 */
static const unsigned char   P[] =
{
    16, 7,20,21,
    29,12,28,17,
    1,15,23,26,
    5,18,31,10,
    2, 8,24,14,
    32,27, 3, 9,
    19,13,30, 6,
    22,11, 4,25,
};

static  unsigned char   C[64];
static  unsigned char   D[64];


static int setkey(unsigned char* key)
{
    register int i, j, k;
    int t;

    for (i=0; i<28; i++)
    {
        C[i] = key[PC1_C[i]-1];
        D[i] = key[PC1_D[i]-1];
    }

    for (i=0; i<16; i++)
    {
        //rotate.
        for (k=0; k<shifts[i]; k++)
        {
            t = C[0];
            for (j=0; j<28-1; j++)
                C[j] = C[j+1];
            C[27] = t;
            t = D[0];
            for (j=0; j<28-1; j++)
                D[j] = D[j+1];
            D[27] = t;
        }
        /*
         * get Ki. Note C and D are concatenated.
         */
        for (j=0; j<24; j++)
        {
            KS[i][j] = C[PC2_C[j]-1];
            KS[i][j+24] = D[PC2_D[j]-28-1];
        }
    }

    for(i=0; i<48; i++)
        E[i] = e[i];

    return 0;

}

static int encrypt(unsigned char* block, int edflag)
{

    unsigned  char  L[64], R[64];
    unsigned char   tempL[64];
    unsigned char   f[64];
    unsigned char   preS[64];

    int i, ii;
    register int t, j, k;

    for (j=0; j<64; j++)
    {
        L[j] = block[IP[j]-1];
    }

    for (j=0; j<32; j++)
    {
        R[j] = L[j+32];
    }

    for (ii=0; ii<16; ii++)
    {
        if (edflag)
            i = 15-ii;
        else
            i = ii;
        /*
         * Save the R array,
         * which will be the new L.
         */
        for (j=0; j<32; j++)
            tempL[j] = R[j];

        /*
         * Expand R to 48 bits using the E selector;
         * exclusive-or with the current key bits.
         */
        for (j=0; j<48; j++)
            preS[j] = R[E[j]-1] ^ KS[i][j];

        /*
         * The pre-select bits are now considered
         * in 8 groups of 6 bits each.
         * The 8 selection functions map these
         * 6-bit quantities into 4-bit quantities
         * and the results permuted
         * to make an f(R, K).
         * The indexing into the selection functions
         * is peculiar; it could be simplified by
         * rewriting the tables.
         */
        for (j=0; j<8; j++)
        {
            t = 6*j;
            k = S[j][(preS[t+0]<<5)+
                     (preS[t+1]<<3)+
                     (preS[t+2]<<2)+
                     (preS[t+3]<<1)+
                     (preS[t+4]<<0)+
                     (preS[t+5]<<4)];
            t = 4*j;
            f[t+0] = (k>>3)&01;
            f[t+1] = (k>>2)&01;
            f[t+2] = (k>>1)&01;
            f[t+3] = (k>>0)&01;
        }

        /*
         * The new R is L ^ f(R, K).
         * The f here has to be permuted first, though.
         */
        for (j=0; j<32; j++)
            R[j] = L[j] ^ f[P[j]-1];
        /*
         * Finally, the new L (the original R)
         * is copied back.
         */
        for (j=0; j<32; j++)
            L[j] = tempL[j];

    }
    /*
     * The output L and R are reversed.
     */
    for (j=0; j<32; j++)
    {
        t = L[j];
        L[j] = R[j];
        R[j] = t;
    }


    /*============================================== */
    for (j=32; j<64; j++)
        L[j] = R[j-32];
    /*============================================== */
    /*
     * The final output
     * gets the inverse permutation of the very original.
     */
    for (j=0; j<64; j++)
        block[j] = L[FP[j]-1];

    return 0;
}

static int expand(unsigned char* in, unsigned char* out)
{
    int	i,j;

    for (i=0; i<8; i++)
    {
        for (j=0; j<8; j++)
        {
            *out = (in[i] <<j) & 0x80;
            if (*out == 0x80)
                *out = 0x01;
            out++;

        }
    }
    return 0;
}

static int compress(unsigned char* in, unsigned char* out)
{
    int	temp;
    int	i,j;

    for(i=0; i<8; i++)
    {
        out[i] = 0;
        temp = 1;
        for (j=7; j>=0; j--)
        {
            out[i] = out[i] + ( in[i*8+j] * temp);
            temp *= 2;
        }
    }
    return 0;
}

void ztvm_encrypt_undes(unsigned char *pin, unsigned char *workkey, unsigned char *cipher_pin)
{
    int		i;
    unsigned char	clear_txt[PINLEN];
    unsigned char	pin_block[PINLEN];
    unsigned char	key_block[PINLEN];
    unsigned char	bits[64];

    for(i=0; i<8; i++)
        key_block[i]=workkey[i];
    /* get  pan_block */
    expand(key_block,bits);
    setkey(bits);
    /* set key */

    expand(pin,bits);		/* expand to bit stream */
    encrypt(bits,DESCRYPT);		/* descrypt */
    compress(bits,clear_txt);	/* compress to 8 characters */

    for (i=0; i<8; i++)
        pin_block[i] = clear_txt[i];
    memcpy(cipher_pin,pin_block, 8);
}


void ztvm_encrypt_des(unsigned char *pin, unsigned char *workkey, unsigned char *cipher_pin)
{
    int		i;
    unsigned char	clear_txt[PINLEN],cipher_txt[PINLEN];
    unsigned char	key_block[PINLEN];
    unsigned char	bits[64];
    for(i=0; i<8; i++)
        key_block[i]=workkey[i];

    for (i=0; i<8; i++)
        clear_txt[i] = pin[i];

    expand(key_block,bits);
    setkey(bits);
    expand(clear_txt,bits);		/* expand to bit stream */
    encrypt(bits,ENCRYPT);		/* encrypt */
    compress(bits,cipher_txt);	/* compress to 8 characters */

    memcpy(cipher_pin,cipher_txt,8);
}

void ztvm_encrypt_untrides(unsigned char *input, unsigned char *left_key, unsigned char *right_key, unsigned char *output)
{
    unsigned char tmpstr1[9], tmpstr2[9];

    ztvm_encrypt_undes( input, left_key, tmpstr1 );
    ztvm_encrypt_des( tmpstr1, right_key, tmpstr2 );
    ztvm_encrypt_undes( tmpstr2, left_key, output );
}

char XINGSHENG_ID_ASCII_Decoding(unsigned char c)
{

    if ( c < 10 )
    {
        return (c+'0');
    }

    if ( c >= 10 && c <= 36 )
    {
        return (c-10+'A');
    }

    if ( c >= 37 && c <= 63)
    {
        return (c-36+'a');
    }
    return '$';
}

int XINGSHENG_ID_Decoder(unsigned char *encoder,char *sn)
{
    int val;

    /* Mode type */
    val = ( (encoder[1] >> 4 )&0x0f)*10 + (encoder[1]&0x0f);
    sn[0] = XINGSHENG_ID_ASCII_Decoding(val);

    val = ( (encoder[2] >> 4 )&0x0f)*10 + (encoder[2]&0x0f);
    sn[1] = XINGSHENG_ID_ASCII_Decoding(val);

    val = ( (encoder[3] >> 4 )&0x0f)*10 + (encoder[3]&0x0f);
    sn[2] = XINGSHENG_ID_ASCII_Decoding(val);

    /* Month */
    val = ( (encoder[4] >> 4 )&0x0f)*10 + (encoder[4]&0x0f);
    sn[3] = XINGSHENG_ID_ASCII_Decoding(val);

    sn[4] = ((encoder[5]>>4)&0x0f)+'0';
    sn[5] = (encoder[5]&0x0f)+'0';

    sn[6] = ((encoder[6]>>4)&0x0f)+'0';
    sn[7] = (encoder[6]&0x0f)+'0';
    sn[8] = 0;

    return 8;
}

//0BEB3ACF8432710181D6C9F893B469D0E4FAEA4EBE4FF997C6E3
int decryptSN(unsigned char *Sn_Data, char *imei)
{
    int i;
    unsigned short CRC_K;
    unsigned char Key1[9], Key2[9],PinSN[9],SN[9];
    CRC_K =  GetCrc16((const char *)Sn_Data+2,24);
    if (  ((CRC_K>>8)&0xff) != Sn_Data[0] || ((CRC_K & 0xff) != Sn_Data[1] ) )
    {
        return -1;
    }
    //unpack Key
    ztvm_encrypt_untrides(Sn_Data+2, Key1_Save, Key2_Save, Key1);
    ztvm_encrypt_untrides(Sn_Data+10, Key1_Save, Key2_Save, Key2);
    ztvm_encrypt_untrides(Sn_Data+18, Key1_Save, Key2_Save, PinSN);
    //unpack SN
    ztvm_encrypt_untrides(PinSN, Key1, Key2, SN);
    if ( ((SN[0] >>4) &0x0f) == 1)
    {
        XINGSHENG_ID_Decoder((unsigned char *)SN, imei);
        return 0;
    }

    for( i = 0; i < 8; i++)
    {
        imei[i*2] = (SN[i] >> 4 & 0x0f) + '0';
        imei[i*2+1] = (SN[i]  & 0x0f) + '0';
    }
    imei[15] = 0;
    return 1;
}


