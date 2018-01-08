#include "stdio.h"
#include "24tft.h"
#include "ASCII.h"
#include "GB1616.h"	//16*16������ģ
#include "oled_number.h"
#include "oled_picture.h"

#define SPILCD_W 176
#define SPILCD_H 240

void ILI9325_CMO24_Initial(void);	//OLEDģ���ʼ������
void OLED_display_init(void);			//OLED�ṹ���ʼ��

OLED_display_t OLED_display;

void Delayms(unsigned short time)
{
	unsigned short i,j;
	for(i=0;i<time;i++)
		for(j=0;j<2600;j++)	;
}

void LCD_WriteByteSPI(unsigned char byte)
{
	  unsigned char buf;
    unsigned char i;
    for(i=0;i<8;i++) 
    {
        buf=(byte>>(7-i))&0x1;
        SPI_SDA(buf);
				SPI_DCLK(0);
        SPI_DCLK(1);
    }	
}
void LCD_WriteoneSPI(unsigned char byte)
{
	  unsigned char buf;
    unsigned char i;
	
    for(i=0;i<4;i++) 
    {
        buf=(byte>>(3-i))&0x1;
        SPI_SDA(buf);
				SPI_DCLK(0);
        SPI_DCLK(1);
    }	

}
void WriteComm(unsigned char dat)
{
	SPI_CS(0);
	lcd_RS(0);
	LCD_WriteByteSPI(dat);	//upper eight bits
	lcd_RS(1);
	SPI_CS(1);
}
void LCD_WriteRegIndex(unsigned char Index)
{
	lcd_RS(0);
	LCD_WriteByteSPI(Index);	//upper eight bits
	lcd_RS(1);
}
void LCD_WriteData(unsigned short dat)
{
		lcd_RS(1);
    LCD_WriteByteSPI(dat);
}

//��������LCD_WR_REG
//��  ������
//        ׼����ʼд��GRAM
void SPILCD_WriteRAM_Prepare(void)
{
	LCD_WriteRegIndex(0x22);   //дRAM
}	 

/*************************************************
��������Lcd�����㶨λ����
���ܣ�ָ��320240Һ���ϵ�һ����Ϊд���ݵ���ʼ��
��ڲ�����x ���� 0~239
          y ���� 0~319
����ֵ����
*************************************************/
void Lcd_SetCursor(u16 x,u16 y)
{ 
  LCD_WriteRegIndex(0x20);
  LCD_WriteData(x);//ˮƽ����
  LCD_WriteRegIndex(0x21);
  LCD_WriteData(y);//��ֱ���� 
} 

//��������LCD_WR_REG
//��  ������
//        дGRAM����
void SPILCD_WriteRAM(unsigned short RGB_Code)
{							    
    LCD_WriteData(RGB_Code); 
}
//��������LCD_SetWindow
//��  ����Xpos:������
//				Ypos:������
void SPILCD_SetWindow(unsigned short xstat,unsigned short xend,unsigned short ystat,unsigned short yend)
{
	//HX8367-A

	//Set GRAM Area 
	LCD_WR_REG(0x02,xstat>>8); 
	LCD_WR_REG(0x03,xstat&0xff);     //Column Start 
	LCD_WR_REG(0x04,xend>>8); 
	LCD_WR_REG(0x05,xend&0xff);     //Column End 
	 
	LCD_WR_REG(0x06,ystat>>8); 
	LCD_WR_REG(0x07,ystat&0xff);     //Row Start 
	LCD_WR_REG(0x08,yend>>8); 
	LCD_WR_REG(0x09,yend&0xff);     //Row End 
	  
	 LCD_WriteRegIndex(0x22);

}
//������ɫ��ʾ
void LCD_InvDisplayOn()
{
  LCD_WriteRegIndex(0x21);  
}
//�رշ�ɫ��ʾ
void LCD_InvDisplayOff()
{
  LCD_WriteRegIndex(0x20); 
}

/******************************************
��������Lcdд�����
���ܣ���Lcdָ��λ��д��Ӧ�����������
��ڲ�����Index ҪѰַ�ļĴ�����ַ
          ConfigTemp д������ݻ�����ֵ
******************************************/
void LCD_WR_REG(u16 Index,u16 CongfigTemp)
{
	LCD_WriteRegIndex(Index);
	LCD_WriteByteSPI(CongfigTemp);
}

void ILI9325_CMO24_Initial(void)
{
	//��λ
	GPIO_ResetBits(GPIOA, GPIO_Pin_2);Delayms(100);
	GPIO_SetBits(GPIOA, GPIO_Pin_2);Delayms(100);
	
	SPI_CS(0);
	Delayms(25);

	WriteComm(0xae);

	WriteComm(0x15);    //   �����е�ַ
	WriteComm(0x00);    //  start column   0
	WriteComm(0x3f);    //  end column   127

	WriteComm(0x75);    //   �����е�ַ
	WriteComm(0x00);    //  start row   0
	WriteComm(0x7f);    //  end row   127

	WriteComm(0x81);	 // ���öԱȶ�
	WriteComm(0x80);


	WriteComm(0xa0);    //segment remap
	WriteComm(0x51);	  //

	WriteComm(0xa1);	 // start line
	WriteComm(0x00);

	WriteComm(0xa2);	 // display offset
	WriteComm(0x00);

	WriteComm(0xa4);    //normal display

	WriteComm(0xa8);    // set multiplex ratio
	WriteComm(0x7f);

	WriteComm(0xb1);	 // set phase leghth
	WriteComm(0xf1);

	WriteComm(0xb3);  // set dclk
	WriteComm(0x00);  //80Hz:0xc1	90Hz:0xe1  100Hz:0x00	110Hz:0x30 120Hz:0x50 130Hz:0x70

	WriteComm(0xab);	 // 
	WriteComm(0x01);	 //


	/*WriteComm(0xb8);
	WriteComm(0x00);
	WriteComm(0x08);
	WriteComm(0x10);
	WriteComm(0x18);
	WriteComm(0x20);
	WriteComm(0x28);
	WriteComm(0x30);
	WriteComm(0x38);*/

	WriteComm(0xb6);	 // set phase leghth
	WriteComm(0x0f);

	WriteComm(0xbe);
	WriteComm(0x0f);

	WriteComm(0xbc);
	WriteComm(0x08);

	WriteComm(0xd5);
	WriteComm(0x62);

	WriteComm(0xfd);
	WriteComm(0x12);

	WriteComm(0xaf); 
}

//��������SPILCD_Init
//��  ����X Y ���� 
//        ��X Y�ϴ��
void SPILCD_DrawPoint(unsigned short x,unsigned short y,unsigned short color)
{
	SPILCD_SetWindow(x,x+1,y,y+1);//���ù��λ�� 
	SPILCD_WriteRAM_Prepare();     //��ʼд��GRAM	 
	SPILCD_WriteRAM(color);
}
//void SPILCD_Fill(unsigned short xsta,unsigned short ysta,unsigned short xend,unsigned short yend,unsigned short color)
//{                    
//	unsigned short i,j;
//	//���ô���		
//	SPILCD_SetWindow(xsta,xend,ysta,yend);
//	for(i=xsta;i<=xend;i++)
//		for(j=ysta;j<=yend;j++)
//	{
//		   	SPILCD_WriteRAM(color);	  //��ʾ��������ɫ. 
//// 		k=40000;while(k--);
//	}
//	//�ָ�����
//	SPILCD_SetWindow(0,SPILCD_W-1,0,SPILCD_H-1);	    
//}
//��������SPILCD_Clear
//��  ����Color ��ɫ      
void SPILCD_Clear(unsigned short Color)
{
	unsigned short x,y;  
	WriteComm(0x15);//SET COLUMN ADDR 
	WriteComm(0x00); 
	WriteComm(127); 
	WriteComm(0x75);//SET ROW ADDR 
	WriteComm(0x00); 
	WriteComm(95); 	
//	SPILCD_SetWindow(0,SPILCD_W-1,0,SPILCD_H-1);	
SPI_CS(0);	
	for(x=0;x<128;x++)
		for(y=0;y<48;y++)
	{
		SPILCD_WriteRAM(Color);//��ʾ��������ɫ. 
// 		i=40000;while(i--);
	}
	SPI_CS(1);
} 
//��������SPILCD_Clear_Fast
//��  ����single_Color ��ɫ  ֻ����0��1��0�Ǻ�ɫ��1Ϊ��ɫ      
void SPILCD_Clear_Fast(unsigned char single_Color)
{
	unsigned int x,y;
  int temp;	
	SPILCD_SetWindow(0,SPILCD_W-1,0,SPILCD_H-1);	
	SPI_SDA(single_Color);
	lcd_RS(1);
	temp=SPILCD_W<<4;
	for(x=0;x<temp;x++)
		for(y=0;y<SPILCD_H;y++)
	{
    SPI_DCLK(0);
    SPI_DCLK(1);
	}
} 
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//SPILCD DRIVER  LV2��    ----   LV1 ��LV0�Ļ�����ʵ��
//���ַ���ʾ
//�ַ�����ʾ
//������ʾ

//��������SPILCD_ShowChar
//��  ����
//(x,y): 
//num:Ҫ��ʾ���ַ�:" "--->"~"
//size:�����С 12/16
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
void SPILCD_ShowChar(unsigned char x,unsigned char y,unsigned char num)
{       
	unsigned char temp;
	unsigned int pos,t,i;  
	unsigned char size; 
	size=16;	//�ҵ������С
  WriteComm(0x15);//SET COLUMN ADDR 
	WriteComm(x); 
	WriteComm(x+3); 
	WriteComm(0x75);//SET ROW ADDR 
	WriteComm(0x0+y); 
	WriteComm(0x0+y+15); 
	lcd_RS(1);
	SPI_CS(0);  
	num=num-' ';//�õ�ƫ�ƺ��ֵ
	i=num*16;

		for(pos=0;pos<size;pos++)
		{

			temp=nAsciiDot[i+pos];	//��ͨ������������
			for(t=0;t<8;t++)
		   {                 
		      if(temp&0x80)
						LCD_WriteoneSPI(0xff);
					else 
						LCD_WriteoneSPI(0x00);
		      temp<<=1; 
		    }
		}	 
}  
void DrawPixel(u16 x, u16 y, int Color)
{
	SPILCD_SetWindow(x,x,y,y); 
  SPILCD_WriteRAM(Color);							  
}
void PutGB1616(unsigned char x, unsigned char  y, unsigned char c[2])
{
	unsigned int i,j,k;
	unsigned short m;
	WriteComm(0x15);//SET COLUMN ADDR 
	WriteComm(0x0+x); 
	WriteComm(0x0+x+7); 
	WriteComm(0x75);//SET ROW ADDR 
	WriteComm(0x0+y); 
	WriteComm(0x0+y+15); 
//	SPILCD_SetWindow(0,SPILCD_W-1,0,SPILCD_H-1);	
	lcd_RS(1);
SPI_CS(0);	

// 	SPILCD_SetWindow(0,SPILCD_H-1,0,SPILCD_W-1);
	for (k=0;k<64;k++) { //64��ʾ�Խ����ֿ��еĸ�����ѭ����ѯ����
	  if ((codeGB_16[k].Index[0]==c[0])&&(codeGB_16[k].Index[1]==c[1]))
			{ 
    	for(i=0;i<32;i++) 
			{
				m=codeGB_16[k].Msk[i];
				for(j=0;j<8;j++) 
				{		
					if((m&0x80)==0x80) {
						LCD_WriteoneSPI(0xff);
						}
					else {
						LCD_WriteoneSPI(0x00);
						}
					m=m<<1;
				} 
				if(i%2){y++;x=x-8;}
				else x=x+8;
		  }
		}  
	  }	
	}
void LCD_PutString(unsigned char x, unsigned char y, unsigned char *s) 
{
	unsigned char l=0;
	while(*s) 
		{
			if( *s < 0x80) 
				{
					SPILCD_ShowChar(x+l,y,*s);
					s++;l+=4;
				}
			else
				{
					PutGB1616(x+l,y,(unsigned char*)s);
					s+=2;l=l+8;
				}
		}
}

void SPILCD_WriteArray(unsigned int start, unsigned int size, const unsigned char * array)
{
	int pos, t;
	unsigned char temp;
	
	for(pos=0;pos<size;pos++)
	{
		temp=array[start+pos];	//��ͨ������������
		for(t=0;t<8;t++)
		{                 
			if(temp&0x80)
				LCD_WriteoneSPI(0xff);
			else 
				LCD_WriteoneSPI(0x00);
			temp<<=1; 
		}
	}
}

void SPILCD_ShowNum(unsigned char x,unsigned char y,unsigned char num, OLED_NUM_TYPE_ENUM type)
{       
	unsigned int i;  
	unsigned char width, high;
	unsigned int size = 0;
	
	if(type == NUM_40_64)
	{
		width = 40;
		high = 64;
	}
	else if(type == NUM_16_32)
	{
		width = 16;
		high = 32;
	}
	else if(type == NUM_32_45)
	{
		width = 32;
		high = 45;
	}
	else if(type == NUM_16_24)
	{
		width = 16;
		high = 24;
	}
	
	size = high*(width/8);
	
  WriteComm(0x15);//SET COLUMN ADDR 
	WriteComm(x/2); 
	WriteComm((x+width)/2-1); 
	WriteComm(0x75);//SET ROW ADDR 
	WriteComm(0x0+y); 
	WriteComm(0x0+y+high-1); 
	lcd_RS(1);
	SPI_CS(0);  
	//num=num-'0';//�õ�ƫ�ƺ��ֵ
	i=num*size;

	if(type == NUM_40_64)
	{
		SPILCD_WriteArray(i, size, NumberDot4060);
	}
	else if(type == NUM_16_32)
	{
		SPILCD_WriteArray(i, size, NumberDot1632);
	}
	else if(type == NUM_32_45)
	{
		SPILCD_WriteArray(i, size, NumberDot3245);
	}
	else if(type == NUM_16_24)
	{
		SPILCD_WriteArray(i, size, NumberDot1624);
	}
}  

void LCD_PutNumber(unsigned char x, unsigned char y, int number, OLED_NUM_TYPE_ENUM type) 
{
	unsigned char l=0;
	unsigned char i=0;
	unsigned char temp[3];
	unsigned char width;
	
	if(number > 999)						//�����ʾ��ֵΪ999
		number = 999;
	
	if(type == NUM_40_64)
		width = 40;
	else if(type == NUM_16_32)
		width = 16;
	else if(type == NUM_32_45)
		width = 32;
	else if(type == NUM_16_24)
		width = 16;
	
	temp[0] = number/100%10;
	temp[1] = number/10%10;
	temp[2] = number%10;
	
	for(i = 0; i < 3; i++)
	{
		SPILCD_ShowNum(x+l, y, temp[i], type);
		l += width;
	}
}

void SPILCD_ShowPicture(unsigned char x,unsigned char y,OLED_PICTURE_ENUM picture, OLED_PICTURE_TYPE_ENUM type)
{       
	unsigned int i;  
	unsigned char width, high;
	unsigned int size = 0;
	
	if(type == PICTURE_16_32)
	{
		width = 16;
		high = 32;
	}
	else if(type == PICTURE_32_32)
	{
		width = 32;
		high = 32;
	}
	
	size = high*(width/8);
	
  WriteComm(0x15);//SET COLUMN ADDR 
	WriteComm(x/2); 
	WriteComm((x+width)/2-1); 
	WriteComm(0x75);//SET ROW ADDR 
	WriteComm(0x0+y); 
	WriteComm(0x0+y+high-1); 
	lcd_RS(1);
	SPI_CS(0);  
	//num=num-'0';//�õ�ƫ�ƺ��ֵ
	i=picture*size;

	if(type == PICTURE_16_32)
	{
		SPILCD_WriteArray(i, size, PictureDot1632);
	}
	else if(type == PICTURE_32_32)
	{
		SPILCD_WriteArray(i, size, PictureDot3232);
	}
}

/******************************************
��������Lcdͼ�����100*100
���ܣ���Lcdָ��λ�����ͼ��
��ڲ�����
******************************************/
void LCD_Fill_Pic(u16 x, u16 y,u16 pic_H, u16 pic_V, const unsigned char* pic)
{
  unsigned long i;
//	unsigned int j;
	SPILCD_SetWindow(x,x+pic_H-1,y,y+pic_V-1);

// 	lcd_RS(1);
	for (i = 0; i < pic_H*pic_V*2; i++)
	{
    LCD_WriteByteSPI(pic[i]);
	}
// 	SPILCD_SetWindow(0,319,0,239);//д��ͼƬ��ָ�������ʾ����

}

void OLED_init(void)
{
	//GPIO��ʼ����main������
	ILI9325_CMO24_Initial();				//OLEDģ���ʼ������
	OLED_display_init();
}

void OLED_display_init(void)
{
	OLED_display.ui_type = UI_WELCOME;
	OLED_display.ui_clear = 0;
	OLED_display.screen_light = 1;
	OLED_display.light_time = OLED_LIGHT_TIME;
	OLED_display.switch_time = OLED_SWITCH_TIME;
	OLED_display.ui_main.pm2_5 = 0;
	OLED_display.ui_main.air_volum = OLED_AIR_0;
	OLED_display.ui_main.wifi_status = 0;
	OLED_display.ui_main.mode = OLED_AUTO_MODE;
}

void OLED_uitype_change(OLED_UI_ENUM ui_type)
{
	OLED_display.ui_type = ui_type;
	OLED_display.ui_clear = 1;				//��Ҫ��������
}

void OLED_mode_change(OLED_PICTURE_ENUM mode)
{
	OLED_display.ui_main.mode = mode;
}

void OLED_air_set(OLED_AIR_ENUM volum)
{
	OLED_display.ui_main.air_volum = volum;
}

void OLED_display_handle(void)
{
	static unsigned char wifi_flag = 0;
	static int count = 0;
	
	if(OLED_display.ui_clear)
	{
		OLED_display.ui_clear = 0;
		SPILCD_Clear(0x00);							//OLED����
	}
	
	switch(OLED_display.ui_type)
	{
		case UI_CLOSE:
			break;
		case UI_MAIN:
			LCD_PutNumber(16,5,count,NUM_32_45);
			SPILCD_Fill(2,54,124,1,0xff);
			//LCD_PutNumber(0,64,2,NUM_16_32);
			LCD_PutNumber(0,64,OLED_display.ui_main.air_volum,NUM_16_24);
			SPILCD_ShowPicture(48,64,OLED_AIR_VOLUM,PICTURE_16_32);
			if(OLED_display.ui_main.wifi_status == 1)
				SPILCD_ShowPicture(64,64,OLED_WIFI,PICTURE_32_32);
			else
			{
				if(wifi_flag == 0)
				{
					wifi_flag = 1;
					//���wifi�����ͼ��
					SPILCD_Fill(64,64,32,32,0x00);
				}
				else
				{
					wifi_flag = 0;
					SPILCD_ShowPicture(64,64,OLED_WIFI,PICTURE_32_32);
				}
			}
			SPILCD_ShowPicture(96,64,OLED_display.ui_main.mode,PICTURE_32_32);
			break;
		case UI_WELCOME:
			LCD_PutString(16,40,"����Ƽ�");
			break;
		case UI_CONNECTING:
			SPILCD_Clear(0x00);							//OLED����
			LCD_PutString(16,40,"��������");
			break;
		case UI_WIFI_CONFIG:
			SPILCD_Clear(0x00);							//OLED����
			LCD_PutString(16,40,"��������");
			break;
		case UI_MODE:
			break;
	}
	
	count++;
}

void SPILCD_Fill(unsigned short xsta,unsigned short ysta,unsigned short xlen,unsigned short ylen,unsigned short color)
{       
	int i,j;
	
  WriteComm(0x15);//SET COLUMN ADDR 
	WriteComm(xsta/2); 
	WriteComm((xsta+xlen)/2-1); 
	WriteComm(0x75);//SET ROW ADDR 
	WriteComm(ysta); 
	WriteComm(ysta+ylen-1);  

	SPI_CS(0);	
	for(i=0/2;i<(xlen/2);i++)
	{
		for(j=0;j<ylen;j++)
		{
			SPILCD_WriteRAM(color);//��ʾ��������ɫ. 
//	 		i=40000;while(i--);
		}
	}		
	SPI_CS(1);
}
