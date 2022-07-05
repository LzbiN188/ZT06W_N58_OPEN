#include "nwy_osi_api.h"
#include "osi_log.h"
#include "nwy_pm.h"
#include "nwy_file.h"
#include "nwy_lcd_bus.h"

#define ROW 128
#define COL 128

#define WIDTH 128
#define HEIGHT 128

#define ASC12_FILE_NAME    "/ASC12"
#define ASC16_FILE_NAME    "/ASC16"
#define HZK12_FILE_NAME    "/HZK12"
#define HZK16_FILE_NAME    "/HZK16"

#define LCD_DataWrite_ST7735(Data)                    \
    {                                                 \
        while (nwy_lcd_bus_write_data(Data) != true) \
            ;                                         \
    }
#define LCD_CtrlWrite_ST7735(Cmd)                   \
    {                                               \
        while (nwy_lcd_bus_write_cmd(Cmd) != true) \
            ;                                       \
    }

static unsigned short display_buf[WIDTH*HEIGHT];

void nwy_write_data_buf(void *buf, unsigned int len)
{
    int i;
    unsigned char *data = (unsigned char *)buf;

    for(i=0;i<len;i++)
        LCD_DataWrite_ST7735(data[i]);

    OSI_LOGI(0, "nwy_write_data_buf!");
}

void nwy_lcd_block_write(unsigned char startx, unsigned char starty, unsigned char endx, unsigned char endy)
{
    unsigned char buf[4];
    LCD_CtrlWrite_ST7735(0x2a);
    buf[0] = 0;
    buf[1] = startx;
    buf[2] = 0;
    buf[3] = endx;
    nwy_write_data_buf((unsigned short*)buf,sizeof(buf));
    
    LCD_CtrlWrite_ST7735(0x2B);
    buf[0] = 0;
    buf[1] = starty;
    buf[2] = 0;
    buf[3] = endy;
    nwy_write_data_buf((unsigned short*)buf,sizeof(buf));
    
    LCD_CtrlWrite_ST7735(0x2C);

    OSI_LOGI(0, "nwy LCD block write");

}

static void GetFontSize(unsigned int fontSize,unsigned int* GBK_W,
    unsigned int* GBK_H,unsigned int* ASC_W,unsigned int* ASC_H)
{
    if(fontSize== 12)
    {
        if(GBK_W!=NULL)
            *GBK_W=12;
        if(GBK_H!=NULL)
            *GBK_H=12;
        if(ASC_W!=NULL)
            *ASC_W=6;
        if(ASC_H!=NULL)
            *ASC_H=12;
    }
    else if(fontSize==16)
    {
        if(GBK_W!=NULL)
            *GBK_W=16;
        if(GBK_H!=NULL)
            *GBK_H=16;
        if(ASC_W!=NULL)
            *ASC_W=8;
        if(ASC_H!=NULL)
            *ASC_H=16;
    }
}

static void GetGbkOneBuf( unsigned int fontSize, unsigned char *gbk, unsigned short *gbk_buf,unsigned short TextColor,unsigned short BackColor)
{
    int qh; 
    int wh;
    int offset = 0;
    int i,j,k;
    int flag;
    int n =0;

    unsigned int GBK_W = 0;
    unsigned int GBK_H = 0;
    unsigned int WORD_SIZE = 0;
    char *filename =NULL;

    GetFontSize(fontSize, &GBK_W, &GBK_H,NULL, NULL);
    
    unsigned char buf[100]={0};
    unsigned char key[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
    
    qh = (int)(gbk[0] - 0xa0);
    wh = (int)(gbk[1] - 0xa0);

    if(fontSize== 12)
    {
        filename = HZK12_FILE_NAME;
        WORD_SIZE = 24;
        offset = (int)(94 * (qh - 1) + (wh - 1)) * WORD_SIZE;
    }
    else if(fontSize== 16)
    {
        filename = HZK16_FILE_NAME;
        WORD_SIZE = 32;
        offset = (int)(94 * (qh - 1) + (wh - 1)) * WORD_SIZE;
    }
    else
    {
        OSI_LOGE(0, "unsupport font size:%d", fontSize);
        return;
    }

    int fp = nwy_sdk_fopen(filename, NWY_RDONLY);
    if(0 > fp)
    {
        OSI_LOGE(0, "%s open fail\n", filename);
        return;
    }
	
    nwy_sdk_fseek(fp, offset, NWY_SEEK_SET);
    int read_len = nwy_sdk_fread(fp, buf, WORD_SIZE);

    if(read_len != WORD_SIZE)
    {
        OSI_LOGE(0, "%s read fail read_len=%d\n", filename, read_len);
    }

    nwy_sdk_fclose(fp);

    int total_word_size = WORD_SIZE/GBK_H;
    for (k = 0; k<GBK_H; k++)
    {
        for (j = 0; j<total_word_size; j++)
        {
            for (i = 0; i<8; i++)
            {
                if(j*8+i >= GBK_W)
                    break;
                flag = buf[k * total_word_size + j] & key[i];
                if(flag)
                {
                    gbk_buf[n++] = TextColor;
                }
                else
                {
                    gbk_buf[n++] = BackColor;
                }
            }
        }
    }

//LCD_BlockWrite(0,0, COL-1,ROW-1);
}

static void GetOneCharBuf(unsigned int fontSize,unsigned char ord,unsigned short *char_buf, unsigned short TextColor,unsigned short BackColor)	 // ord:0~95
{
    unsigned char i,j,k;
    unsigned char dat;
    int n=0;

    unsigned int ASC_W = 0;
    unsigned int ASC_H = 0;
    unsigned int WORD_SIZE = 0;
    char *filename =NULL;
    unsigned char buf[100]={0};
    
    GetFontSize(fontSize, NULL, NULL,&ASC_W, &ASC_H);
    
    if(fontSize== 12)
    {
        filename = ASC12_FILE_NAME;
        WORD_SIZE = 12;
    }
    else if(fontSize== 16)
    {
        filename = ASC16_FILE_NAME;
        WORD_SIZE = 16;
    }
    else
    {
        OSI_LOGE(0, "unsupport font size:%d", fontSize);
        return;
    }

    int fp = nwy_sdk_fopen(filename, NWY_RDONLY);
    if(0 > fp)
    {
        OSI_LOGE(0, "%s open fail\n", filename);
        return;
    }
    
    nwy_sdk_fseek(fp, ord*WORD_SIZE, NWY_SEEK_SET);
    int read_len = nwy_sdk_fread(fp,buf,WORD_SIZE);
    if(read_len != WORD_SIZE)
    {
        OSI_LOGE(0, "ASC16 read fail read_len=%d\n", read_len);
    }

    nwy_sdk_fclose(fp);

    int total_word_size = WORD_SIZE/ASC_H;
    
    for(k=0;k<ASC_H;k++)
    {
        for (j = 0; j<total_word_size; j++)
        {
            dat=buf[k * total_word_size + j];
            for(i=0;i<8;i++)
            {
                if(j*8+i >= ASC_W)
                    break;
                if((dat<<i)&0x80)
                {
                    char_buf[n++] = TextColor;
                }
                else
                {
                    char_buf[n++] = BackColor;
                }
            }
        }
    }
}

void nwy_dispstrline(unsigned int fontSize, unsigned char *str,unsigned int Xstart,unsigned int Ystart,unsigned short TextColor,unsigned short BackColor)
{
    int i,j;
    static unsigned short tmp_buf[256];
    int line_len;
    int str_index =0;
    unsigned int Xend = Xstart;
    unsigned int GBK_W = 0;
    unsigned int GBK_H = 0;
    unsigned int ASC_W = 0;
    unsigned int ASC_H = 0;

    GetFontSize(fontSize, &GBK_W, &GBK_H,&ASC_W, &ASC_H);
    line_len = strlen((char *)str)*ASC_W;
    
    if(line_len + Xstart > WIDTH)
    {
        OSI_LOGE(0, "ERROR:::: line_len is too lone %d\r\n", line_len);
        return;
    }

    while(!(*str=='\0'))
    {
        if( *str > 0x80)
        {
            GetGbkOneBuf(fontSize, str, tmp_buf, TextColor,BackColor);
            for(i=0;i<GBK_H;i++)
            {
                for(j=0;j<GBK_W;j++)
                {
                    display_buf[i*line_len+str_index + j ] = tmp_buf[i*GBK_W+j];
                }
            }
            str_index += GBK_W;
            
            if(Xstart>((COL)-GBK_W))
            {
                Xstart=(COL)-GBK_W;
            }
            else
            {
                Xend=Xend+GBK_W;
            }

            if(Ystart>((ROW)-GBK_H))
            {
                break;
            }
            
            str += 2;
        }
        else
        {
            GetOneCharBuf(fontSize, *str++, tmp_buf, TextColor,BackColor);
            for(i=0;i<ASC_H;i++)
            {
                for(j=0;j<ASC_W;j++)
                {
                    display_buf[i*line_len+str_index + j ] = tmp_buf[i*ASC_W+j];
                }
            }
            str_index += ASC_W;
            
            if(Xstart>((COL)-ASC_W*2))
            {
                Xstart=(COL)-ASC_W*2;
            }
            else
            {
                Xend=Xend+ASC_H;
            }

            if(Ystart>((ROW)-ASC_H))
            {
                break;
            }
        }
    }

    nwy_lcd_block_write(Xstart,Ystart,Xstart+line_len-1,Ystart+(GBK_H-1));
    nwy_write_data_buf(display_buf, line_len*GBK_H*2);
}


static inline void prvFillBufferWhiteScreen(uint16_t *buffer, unsigned width, unsigned height)
{
    memset(buffer, 0xff, width * height * sizeof(uint16_t));
}

static void prvLcdClear(void)
{
    prvFillBufferWhiteScreen(display_buf, WIDTH, HEIGHT);
    nwy_lcd_block_write(0,0,WIDTH-1,HEIGHT-1);
    nwy_write_data_buf(display_buf, WIDTH*HEIGHT*2);
}

/**************************************************************************************/
// Description: initialize all LCD with LCDC MCU MODE and LCDC mcu mode
/**************************************************************************************/
static void _st7735Init(void)
{
    OSI_LOGI(0, "lcd:    st7735Init ");
    nwy_sleep(200);
    LCD_CtrlWrite_ST7735(0x11);
    nwy_sleep(200);

    LCD_CtrlWrite_ST7735(0xB1);
    LCD_DataWrite_ST7735(0x05);
    LCD_DataWrite_ST7735(0x3C);
    LCD_DataWrite_ST7735(0x3C);

    LCD_CtrlWrite_ST7735(0xB2);
    LCD_DataWrite_ST7735(0x05);
    LCD_DataWrite_ST7735(0x3C);
    LCD_DataWrite_ST7735(0x3C);

    LCD_CtrlWrite_ST7735(0xB3);
    LCD_DataWrite_ST7735(0x05);
    LCD_DataWrite_ST7735(0x3C);
    LCD_DataWrite_ST7735(0x3C);
    LCD_DataWrite_ST7735(0x05);
    LCD_DataWrite_ST7735(0x3C);
    LCD_DataWrite_ST7735(0x3C);

    LCD_CtrlWrite_ST7735(0xB4);
    LCD_DataWrite_ST7735(0x03);

    LCD_CtrlWrite_ST7735(0xC0);
    LCD_DataWrite_ST7735(0x62);
    LCD_DataWrite_ST7735(0x02);
    LCD_DataWrite_ST7735(0x04);

    LCD_CtrlWrite_ST7735(0xC1);
    LCD_DataWrite_ST7735(0xC0);

    LCD_CtrlWrite_ST7735(0xC2);
    LCD_DataWrite_ST7735(0x0D);
    LCD_DataWrite_ST7735(0x00);

    LCD_CtrlWrite_ST7735(0xC3);
    LCD_DataWrite_ST7735(0x8D);
    LCD_DataWrite_ST7735(0x6A);

    LCD_CtrlWrite_ST7735(0xC4);
    LCD_DataWrite_ST7735(0x8D);
    LCD_DataWrite_ST7735(0xEE);

    LCD_CtrlWrite_ST7735(0xC5);
    LCD_DataWrite_ST7735(0x12);
    //turn right 90
    LCD_CtrlWrite_ST7735(0x36);
    LCD_DataWrite_ST7735(0x60);

    LCD_CtrlWrite_ST7735(0xE0);
    LCD_DataWrite_ST7735(0x03);
    LCD_DataWrite_ST7735(0x1B);
    LCD_DataWrite_ST7735(0x12);
    LCD_DataWrite_ST7735(0x11);
    LCD_DataWrite_ST7735(0x3F);
    LCD_DataWrite_ST7735(0x3A);
    LCD_DataWrite_ST7735(0x32);
    LCD_DataWrite_ST7735(0x34);
    LCD_DataWrite_ST7735(0x2F);
    LCD_DataWrite_ST7735(0x2B);
    LCD_DataWrite_ST7735(0x30);
    LCD_DataWrite_ST7735(0x3A);
    LCD_DataWrite_ST7735(0x00);
    LCD_DataWrite_ST7735(0x01);
    LCD_DataWrite_ST7735(0x02);
    LCD_DataWrite_ST7735(0x05);

    LCD_CtrlWrite_ST7735(0xE1);
    LCD_DataWrite_ST7735(0x03);
    LCD_DataWrite_ST7735(0x1B);
    LCD_DataWrite_ST7735(0x12);
    LCD_DataWrite_ST7735(0x11);
    LCD_DataWrite_ST7735(0x32);
    LCD_DataWrite_ST7735(0x2F);
    LCD_DataWrite_ST7735(0x2A);
    LCD_DataWrite_ST7735(0x2F);
    LCD_DataWrite_ST7735(0x2E);
    LCD_DataWrite_ST7735(0x2C);
    LCD_DataWrite_ST7735(0x35);
    LCD_DataWrite_ST7735(0x3F);
    LCD_DataWrite_ST7735(0x00);
    LCD_DataWrite_ST7735(0x00);
    LCD_DataWrite_ST7735(0x01);
    LCD_DataWrite_ST7735(0x05);

    LCD_CtrlWrite_ST7735(0xFC);
    LCD_DataWrite_ST7735(0x8C);

    LCD_CtrlWrite_ST7735(0x3A);
    LCD_DataWrite_ST7735(0x05);

    LCD_CtrlWrite_ST7735(0x29);
}

#define LCD_BACK_LIGHT_POWER NWY_POWER_RGB_IB0
#define LCD_MAIN_POWER NWY_POWER_LCD
#define LCD_BUS_CLK_FREQ (20000000)

static bool lcd_init = false;
void nwy_lcd_init(void)
{
    if(lcd_init)
        return;

    nwy_subpower_switch(LCD_MAIN_POWER, true, true);
    nwy_subpower_switch(NWY_POWER_BACK_LIGHT, true, true);
    nwy_subpower_switch(LCD_BACK_LIGHT_POWER, true, true);
    nwy_sleep(256);
    nwy_lcd_bus_config_t lcd_bus_config = 
    {
      .cs = NWY_LCD_BUS_CS_0,
      .cs0Polarity = false,
      .cs1Polarity = false,
      .resetb = true,
      .rsPolarity = false,
      .wrPolarity = false,
      .rdPolarity = false,
      .highByte = false,
      .clk = LCD_BUS_CLK_FREQ,
    };
    nwy_lcd_bus_init(&lcd_bus_config);
    nwy_sleep(32);
    _st7735Init();
    nwy_sleep(32);
    prvLcdClear();

    lcd_init = true;
}

void nwy_lcd_deinit(void)
{
    nwy_lcd_bus_deinit();
    nwy_subpower_switch(NWY_POWER_BACK_LIGHT, false, false);
    nwy_subpower_switch(LCD_BACK_LIGHT_POWER, false, false);
    nwy_subpower_switch(LCD_MAIN_POWER, false, false);

    lcd_init = false;
}

