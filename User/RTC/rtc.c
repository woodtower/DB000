#include "delay.h"
//#include "usart.h"
#include "rtc.h"
#include "stdio.h"
#include "uart.h"
//#include "exti.h"
#include "key.h"
#include "stmflash.h"

//Mini STM32������
//RTCʵʱʱ�� ��������
//����ԭ��@ALIENTEK
//2010/6/6

_calendar_obj calendar;//ʱ�ӽṹ��
_calendar_obj calendar_tmp;//��ʱʱ�ӽṹ��

static void RTC_NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;		//RTCȫ���ж�
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//��ռ���ȼ�1λ,�����ȼ�3λ
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	//��ռ���ȼ�0λ,�����ȼ�4λ
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//ʹ�ܸ�ͨ���ж�
  NVIC_Init(&NVIC_InitStructure);		//����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
}

//ʵʱʱ������
//��ʼ��RTCʱ��,ͬʱ���ʱ���Ƿ�������
//BKP->DR1���ڱ����Ƿ��һ�����õ�����
//����0:����
//����:�������

u8 RTC_Init(u32 year, u8 month, u8 day, u8 hour, u8 min, u8 sec)
{
  //����ǲ��ǵ�һ������ʱ��
  u8 temp = 0;
	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��
  PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ�������
  printf("%x\r\n", BKP_ReadBackupRegister(BKP_DR1));
  if (BKP_ReadBackupRegister(BKP_DR1) != 0x5050)		//��ָ���ĺ󱸼Ĵ����ж�������:��������д���ָ�����ݲ����
  {
    BKP_DeInit();	//��λ��������
#ifdef	RCC_LSE
/*LSE*/
		RCC_LSEConfig(RCC_LSE_ON);	//�����ⲿ���پ���(LSE),ʹ��������پ���
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET&&temp<250)	//���ָ����RCC��־λ�������,�ȴ����پ������
			{
			temp++;
			delay_ms(10);
			}
		if(temp>=250)return 1;//��ʼ��ʱ��ʧ��,����������
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//����RTCʱ��(RTCCLK),ѡ��LSI��ΪRTCʱ��
#elif defined RCC_LSI
/*LSI*/
    RCC_LSICmd(ENABLE);  //�����ڲ����پ���(LSI)
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET && temp < 250)	//���ָ����RCC��־λ�������,�ȴ����پ������
    {
      temp++;
      delay_ms(10);
    }
    if(temp >= 250)return 1; //��ʼ��ʱ��ʧ��,����������
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);		//����RTCʱ��(RTCCLK),ѡ��LSI��ΪRTCʱ��
#elif defined RCC_HSE
/*HSE*/
		RCC_HSEConfig(RCC_HSE_ON);	//�����ⲿ���پ���(HSE),ʹ��������پ���
		while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET&&temp<250)	//���ָ����RCC��־λ�������,�ȴ����پ������
			{
			temp++;
			delay_ms(10);
			}
		if(temp>=250)return 1;//��ʼ��ʱ��ʧ��,����������
		RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);		//����RTCʱ��(RTCCLK),ѡ��LSI��ΪRTCʱ��
#endif
    RCC_RTCCLKCmd(ENABLE);	//ʹ��RTCʱ��
    RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
    RTC_WaitForSynchro();		//�ȴ�RTC�Ĵ���ͬ��
    RTC_ITConfig(RTC_IT_SEC, ENABLE);		//ʹ��RTC���ж�
    RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
    RTC_EnterConfigMode();/// ��������
#ifdef	RCC_LSE
    RTC_SetPrescaler(32767); //����RTCԤ��Ƶ��ֵ-LSE
#elif defined RCC_LSI
		RTC_SetPrescaler(39999); //����RTCԤ��Ƶ��ֵ-LSI
#elif defined RCC_HSE
		RTC_SetPrescaler(62499); //����RTCԤ��Ƶ��ֵ-HSE
#endif
    RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
    RTC_Set(year, month, day, hour, min, sec); //����ʱ��
    RTC_ExitConfigMode(); //�˳�����ģʽ
    BKP_WriteBackupRegister(BKP_DR1, 0X5050);	//��ָ���ĺ󱸼Ĵ�����д���û���������
  }
  else//ϵͳ������ʱ
  {

    RTC_WaitForSynchro();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
    RTC_ITConfig(RTC_IT_SEC, ENABLE);	//ʹ��RTC���ж�
    RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
  }
  RTC_NVIC_Config();//RCT�жϷ�������
  RTC_Get();//����ʱ��
  return 0; //ok

}
//RTCʱ���ж�
//ÿ�봥��һ��
//extern u16 tcnt;
void RTC_IRQHandler(void)
{
//	u32 curruent_time;
//	u32 timeout_time;
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)//�����ж�
  {
    RTC_Get();//����ʱ��

//		if(Flash_Read_Number(FLASH_SAVE_ADDR+4) == 0xaaaaaaaa)	//�ж������ʱ���޲���ȫf�Ļ�˵����ʱʱ�䱻���ù�
//		{
//			curruent_time = RTC_GetCounter();
//			timeout_time = Flash_Read_Number(FLASH_SAVE_ADDR);
//			//timeout_time = BKP_ReadBackupRegister(BKP_DR2) +  (BKP_ReadBackupRegister(BKP_DR3)<<16);
//			
//			if((curruent_time >= timeout_time) && (s_Powerkey.timeout_flag == 0))
//			{
//				printf("time out!\r\n");
//				s_Powerkey.timeout_flag = 1;
//				//EXTI_DeInit();
//			}
//			else if((curruent_time < timeout_time) && (s_Powerkey.timeout_flag == 1))
//			{
//				s_Powerkey.timeout_flag = 0;
//				//EXTIX_Init();	
//			}
//		}
//		else
//		{
//			s_Powerkey.timeout_flag = 1;
//		}
  }
  if(RTC_GetITStatus(RTC_IT_ALR) != RESET) //�����ж�
  {
    RTC_ClearITPendingBit(RTC_IT_ALR);		//�������ж�
    RTC_Get();				//����ʱ��
    //printf("Alarm Time:%d-%d-%d %d:%d:%d\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//�������ʱ��

  }
  RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_OW);		//�������ж�
  RTC_WaitForLastTask();
}
//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//����:���
//���:������ǲ�������.1,��.0,����
u8 Is_Leap_Year(u16 year)
{
  if(year % 4 == 0) //�����ܱ�4����
  {
    if(year % 100 == 0)
    {
      if(year % 400 == 0)return 1; //�����00��β,��Ҫ�ܱ�400����
      else return 0;
    }
    else return 1;
  }
  else return 0;
}
//����ʱ��
//�������ʱ��ת��Ϊ����
//��1970��1��1��Ϊ��׼
//1970~2099��Ϊ�Ϸ����
//����ֵ:0,�ɹ�;����:�������.
//�·����ݱ�
u8 const table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5}; //���������ݱ�
//ƽ����·����ڱ�
const u8 mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
u8 RTC_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
  u16 t;
  u32 seccount = 0;
  if(syear < 1970 || syear > 2099)return 1;
  for(t = 1970; t < syear; t++)	//��������ݵ��������
  {
    if(Is_Leap_Year(t))seccount += 31622400; //�����������
    else seccount += 31536000;			 //ƽ���������
  }
  smon -= 1;
  for(t = 0; t < smon; t++)	 //��ǰ���·ݵ����������
  {
    seccount += (u32)mon_table[t] * 86400; //�·����������
    if(Is_Leap_Year(syear) && t == 1)seccount += 86400; //����2�·�����һ���������
  }
  seccount += (u32)(sday - 1) * 86400; //��ǰ�����ڵ����������
  seccount += (u32)hour * 3600; //Сʱ������
  seccount += (u32)min * 60;	 //����������
  seccount += sec; //�������Ӽ���ȥ
#ifdef UNIX_timestamp
	seccount -= 28800;	//UNIXʱ����Ǵ�1970.1.1 08:00:00��ʼ��
#endif

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��
  PWR_BackupAccessCmd(ENABLE);	//ʹ��RTC�ͺ󱸼Ĵ�������
  RTC_SetCounter(seccount);	//����RTC��������ֵ

  RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
  return 0;
}

//��ʼ������
//��1970��1��1��Ϊ��׼
//1970~2099��Ϊ�Ϸ����
//syear,smon,sday,hour,min,sec�����ӵ�������ʱ����
//����ֵ:0,�ɹ�;����:�������.
u8 RTC_Alarm_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
  u16 t;
  u32 seccount = 0;
  if(syear < 1970 || syear > 2099)return 1;
  for(t = 1970; t < syear; t++)	//��������ݵ��������
  {
    if(Is_Leap_Year(t))seccount += 31622400; //�����������
    else seccount += 31536000;			 //ƽ���������
  }
  smon -= 1;
  for(t = 0; t < smon; t++)	 //��ǰ���·ݵ����������
  {
    seccount += (u32)mon_table[t] * 86400; //�·����������
    if(Is_Leap_Year(syear) && t == 1)seccount += 86400; //����2�·�����һ���������
  }
  seccount += (u32)(sday - 1) * 86400; //��ǰ�����ڵ����������
  seccount += (u32)hour * 3600; //Сʱ������
  seccount += (u32)min * 60;	 //����������
  seccount += sec; //�������Ӽ���ȥ
  //����ʱ��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��
  PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ�������
  //���������Ǳ����!

  RTC_SetAlarm(seccount);

  RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������

  return 0;
}


//�õ���ǰ��ʱ��
//����ֵ:0,�ɹ�;����:�������.
u8 RTC_Get(void)
{
  static u16 daycnt = 0;
  u32 timecount = 0;
  u32 temp = 0;
  u16 temp1 = 0;
  timecount = RTC_GetCounter();
#ifdef UNIX_timestamp
	timecount += 28800;
#endif
  temp = timecount / 86400; //�õ�����(��������Ӧ��)
  if(daycnt != temp) //����һ����
  {
    daycnt = temp;
    temp1 = 1970;	//��1970�꿪ʼ
    while(temp >= 365)
    {
      if(Is_Leap_Year(temp1))//������
      {
        if(temp >= 366)temp -= 366; //�����������
        else
        {
          temp1++;
          break;
        }
      }
      else temp -= 365;	 //ƽ��
      temp1++;
    }
    calendar.w_year = temp1; //�õ����
    temp1 = 0;
    while(temp >= 28) //������һ����
    {
      if(Is_Leap_Year(calendar.w_year) && temp1 == 1) //�����ǲ�������/2�·�
      {
        if(temp >= 29)temp -= 29; //�����������
        else break;
      }
      else
      {
        if(temp >= mon_table[temp1])temp -= mon_table[temp1]; //ƽ��
        else break;
      }
      temp1++;
    }
    calendar.w_month = temp1 + 1;	//�õ��·�
    calendar.w_date = temp + 1;  	//�õ�����
  }
  temp = timecount % 86400;     		//�õ�������
  calendar.hour = temp / 3600;     	//Сʱ
  calendar.min = (temp % 3600) / 60; 	//����
  calendar.sec = (temp % 3600) % 60; 	//����
  calendar.week = RTC_Get_Week(calendar.w_year, calendar.w_month, calendar.w_date); //��ȡ����
  return 0;
}
//������������ڼ�
//��������:���빫�����ڵõ�����(ֻ����1901-2099��)
//�������������������
//����ֵ�����ں�
u8 RTC_Get_Week(u16 year, u8 month, u8 day)
{
  u16 temp2;
  u8 yearH, yearL;

  yearH = year / 100;
  yearL = year % 100;
  // ���Ϊ21����,�������100
  if (yearH > 19)yearL += 100;
  // ����������ֻ��1900��֮���
  temp2 = yearL + yearL / 4;
  temp2 = temp2 % 7;
  temp2 = temp2 + day + table_week[month - 1];
  if (yearL % 4 == 0 && month < 3)temp2--;
  return(temp2 % 7);
}

//��countֵת�������ڸ�ʽ
//����ֵ:0,�ɹ�;����:�������.
u8 count2date(u32 current_count)
{
  static u16 daycnt = 0;
  u32 timecount = 0;
  u32 temp = 0;
  u16 temp1 = 0;
  timecount = current_count;
#ifdef UNIX_timestamp
	timecount += 28800;
#endif
  temp = timecount / 86400; //�õ�����(��������Ӧ��)
  if(daycnt != temp) //����һ����
  {
    daycnt = temp;
    temp1 = 1970;	//��1970�꿪ʼ
    while(temp >= 365)
    {
      if(Is_Leap_Year(temp1))//������
      {
        if(temp >= 366)temp -= 366; //�����������
        else
        {
          temp1++;
          break;
        }
      }
      else temp -= 365;	 //ƽ��
      temp1++;
    }
    calendar_tmp.w_year = temp1; //�õ����
    temp1 = 0;
    while(temp >= 28) //������һ����
    {
      if(Is_Leap_Year(calendar_tmp.w_year) && temp1 == 1) //�����ǲ�������/2�·�
      {
        if(temp >= 29)temp -= 29; //�����������
        else break;
      }
      else
      {
        if(temp >= mon_table[temp1])temp -= mon_table[temp1]; //ƽ��
        else break;
      }
      temp1++;
    }
    calendar_tmp.w_month = temp1 + 1;	//�õ��·�
    calendar_tmp.w_date = temp + 1;  	//�õ�����
  }
  temp = timecount % 86400;     		//�õ�������
  calendar_tmp.hour = temp / 3600;     	//Сʱ
  calendar_tmp.min = (temp % 3600) / 60; 	//����
  calendar_tmp.sec = (temp % 3600) % 60; 	//����
  calendar_tmp.week = RTC_Get_Week(calendar_tmp.w_year, calendar_tmp.w_month, calendar_tmp.w_date); //��ȡ����
  return 0;
}

int timeout_SET(u32 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
	u16 t;
  u32 seccount = 0;
  if(syear < 1970 || syear > 2099)return 0;
  for(t = 1970; t < syear; t++)	//��������ݵ��������
  {
    if(Is_Leap_Year(t))seccount += 31622400; //�����������
    else seccount += 31536000;			 //ƽ���������
  }
  smon -= 1;
  for(t = 0; t < smon; t++)	 //��ǰ���·ݵ����������
  {
    seccount += (u32)mon_table[t] * 86400; //�·����������
    if(Is_Leap_Year(syear) && t == 1)seccount += 86400; //����2�·�����һ���������
  }
  seccount += (u32)(sday - 1) * 86400; //��ǰ�����ڵ����������
  seccount += (u32)hour * 3600; //Сʱ������
  seccount += (u32)min * 60;	 //����������
  seccount += sec; //�������Ӽ���ȥ
	
	PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ�������
	BKP_WriteBackupRegister(BKP_DR2, seccount & 0xffff);	//��󱸼Ĵ���BKP_DR2��д�볬ʱʱ��ĵ�16λ
	BKP_WriteBackupRegister(BKP_DR3, seccount>>16 & 0xffff);	//��󱸼Ĵ���BKP_DR2��д�볬ʱʱ��ĸ�16λ
	
	return 1;
}

int UNIXtime2date(u32 UNIXtime)
{
	int result_date = 0;
	count2date(UNIXtime);
	//result_date += calendar_tmp.w_year*10000000000;
	result_date += calendar_tmp.w_month*100000000;
	result_date += calendar_tmp.w_date*1000000;
	result_date += calendar_tmp.hour*10000;
	result_date += calendar_tmp.min*100;
	result_date += calendar_tmp.sec;
	return result_date;
}












