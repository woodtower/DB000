#include "key.h"
#include "delay.h"
#include "uart.h"
#include "string.h"
#include "main.h"
//#include "MyFifo.h"
#include "stdio.h"
#include "aqi.h"
//#include "DSHCHO.h"
#include "24tft.h"

//unsigned char wait_send_press;
//int press_len;
//char press_buf[PRESS_SIZE][2];
//u32 press_time_log[PRESS_SIZE];
////u32 press_HCHO[PRESS_SIZE];
//u32 press_C1[PRESS_SIZE];
//u32 press_C2[PRESS_SIZE];
//u32 press_AQI[PRESS_SIZE];

KEY_FLAG_T key_flag;	//������ر�ʾ�ṹ��
BUTTON_T s_Powerkey;	//power�����Ľṹ��
BUTTON_T s_Modekey;		//mode�����Ľṹ��

//�Ƿ���POWER�������½ӿں���
unsigned char  IsKeyDownPower(void)
{
  if (0 == KEY_POWER) return 1;
  return 0;
}

//�Ƿ���MODE�������½ӿں���
unsigned char  IsKeyDownMode(void)
{
  if (0 == KEY_MODE) return 1;
  return 0;
}

void  PanakeyHard_Init(void)
{
  //GPIO_Init (POWER_KEY_PORT, POWER_KEY_PIN, GPIO_MODE_IN_FL_NO_IT);//power key
  //GPIO�ĳ�ʼ����main�����в���
}

void  PanakeyVar_Init(void)
{
	key_flag.ChildLock_flag = 0;	//ͯ����־��0Ϊͯ����Ч��1Ϊͯ����Ч
	key_flag.Comb_flag = 0;				//��ϰ�����ʾ
	
  /* ��ʼ��POWER����������֧�ְ��¡����𡢳��� */
	s_Powerkey.key_type = KEY_TPYE_POWER;
  s_Powerkey.IsKeyDownFunc = IsKeyDownPower;                /* �жϰ������µĺ��� */
  s_Powerkey.FilterTime = BUTTON_FILTER_TIME;                /* �����˲�ʱ�� */
  s_Powerkey.LongTime = BUTTON_LONG_TIME;                        /* ����ʱ�� */
  s_Powerkey.Count = s_Powerkey.FilterTime / 2;                /* ����������Ϊ�˲�ʱ���һ�� */
  s_Powerkey.State = 0;                                                        /* ����ȱʡ״̬��0Ϊδ���� */
  s_Powerkey.KeyCodeDown = KEY_DOWN_Power;                        /* �������µļ�ֵ���� */
  s_Powerkey.KeyCodeUp = KEY_UP_Power;                               /* ��������ļ�ֵ���� */
  s_Powerkey.KeyCodeLong = KEY_LONG_Power;                        /* �������������µļ�ֵ���� */
  s_Powerkey.RepeatSpeed = 0;                                                /* �����������ٶȣ�0��ʾ��֧������ */
  s_Powerkey.RepeatCount = 0;                                                /* ���������� */
	s_Powerkey.IsLong = 0;
	s_Powerkey.IsComb = 0;
	
	/* ��ʼ��MODE����������֧�ְ��¡����𡢳��� */
	s_Modekey.key_type = KEY_TPYE_MODE;
  s_Modekey.IsKeyDownFunc = IsKeyDownMode;                /* �жϰ������µĺ��� */
  s_Modekey.FilterTime = BUTTON_FILTER_TIME;                /* �����˲�ʱ�� */
  s_Modekey.LongTime = BUTTON_LONG_TIME;                        /* ����ʱ�� */
  s_Modekey.Count = s_Powerkey.FilterTime / 2;                /* ����������Ϊ�˲�ʱ���һ�� */
  s_Modekey.State = 0;                                                        /* ����ȱʡ״̬��0Ϊδ���� */
  s_Modekey.KeyCodeDown = KEY_DOWN_Power;                        /* �������µļ�ֵ���� */
  s_Modekey.KeyCodeUp = KEY_UP_Power;                               /* ��������ļ�ֵ���� */
  s_Modekey.KeyCodeLong = KEY_LONG_Power;                        /* �������������µļ�ֵ���� */
  s_Modekey.RepeatSpeed = 0;                                                /* �����������ٶȣ�0��ʾ��֧������ */
  s_Modekey.RepeatCount = 0;                                                /* ���������� */
	s_Modekey.IsLong = 0;
	s_Modekey.IsComb = 0;
}

void Panakey_Init(void)
{
  PanakeyHard_Init();                /* ��ʼ���������� */
  PanakeyVar_Init();                /* ��ʼ������Ӳ�� */
}

//void SavePressLog(void)
//{
//	int tmp_press_time = 0;
//	
//	//ɸѡ�ڶ����ڼ�������������������������������������������1S�����һ�ε�����������ǰһ�ε�������
//	tmp_press_time = RTC_GetCounter();
//	if(press_len > 0)
//	{
//		if((tmp_press_time - press_time_log[press_len-1]) <= 1)
//		{
//			press_len--;
//		}
//	}
//	
//	//if (Fifo_canPush(&recv_fifo1)) Fifo_Push(&recv_fifo1, *mqtt_mode);
//	//�����°������״̬�Ͱ��°�����ʱ���¼�������������
//	if(All_State == sendPM) press_len = 0;
//	else if(press_len >= PRESS_SIZE) press_len = PRESS_SIZE-1;	//����������ʱ���µ�����ֻ�滻ĩβ��һ������
//	strcpy(press_buf[press_len], mqtt_mode);
//	press_time_log[press_len] = RTC_GetCounter();
//	//press_HCHO[press_len] = Conce_HCHO;
//	press_C1[press_len] = Conce_PM2_5;
//	press_C2[press_len] = Conce_PM10;			
//	press_AQI[press_len] = AQI_Max;			
//	press_len++;
//	wait_send_press = 1;
//}

void Pannelkey_Put(KEY_TPYE_ENUM key_type, unsigned char KeyCode)
{
	if(key_type == KEY_TPYE_POWER)
	{
		// ����һ������ ���밴��ֵ
		if(KeyCode == KEY_DOWN_Power)
		{
			printf("POWER press!\r\n");
		}
		else if(KeyCode == KEY_UP_Power)
		{
			printf("POWER key up\r\n");
			if(OLED_screenlight_get())
			{
				if(*mqtt_mode == '0')
					PowerOnOff(1);
				else
					PowerOnOff(0);
//				SavePressLog();
			}
			else	//�������Ϣ��״̬�£����°�������ת�����˵�
			{
				OLED_uitype_change(UI_MAIN);
			}
		}
	}
	else if(key_type == KEY_TPYE_MODE)
	{
		// ����һ������ ���밴��ֵ
		if(KeyCode == KEY_DOWN_Power)
		{
			printf("MODE press!\r\n");
		}
		else if(KeyCode == KEY_UP_Power)
		{
			printf("MODE key up\r\n");
			if(OLED_screenlight_get())
			{
				switch(*mqtt_mode)
				{
					case 'A':
						strcpy(mqtt_mode,"1");
						ModeCountrol();
						break;
					case '1':
						strcpy(mqtt_mode,"2");
						ModeCountrol();
						break;
					case '2':
						strcpy(mqtt_mode,"3");
						ModeCountrol();
						break;
					case '3':
						strcpy(mqtt_mode,"4");
						ModeCountrol();
						break;
					case '4':
						strcpy(mqtt_mode,"A");
						ModeCountrol();
						break;
				}
//				ModeCountrol();
//				SavePressLog();
			}
			else	//�������Ϣ��״̬�£����°�������ת�����˵�
			{
				OLED_uitype_change(UI_MAIN);
			}
		}
	}
	
	//ֻ�������������������ܴ�����ϼ�
	if(KeyCode == KEY_LONG_Power && s_Powerkey.IsLong == 1 && s_Modekey.IsLong == 1)
	{
		//������ϼ���flag
		if(OLED_screenlight_get())
		{
			s_Powerkey.IsComb = 1;
			s_Modekey.IsComb = 1;
			printf("Combine key - wifi restore!\r\n");
			key_flag.Comb_flag = 1;
		}
		else	//�������Ϣ��״̬�£����°�������ת�����˵�
		{
			OLED_uitype_change(UI_MAIN);
		}
	}
}

/*
*********************************************************************************************************
*        �� �� ��: bsp_DetectButton
*        ����˵��: ���һ��������������״̬�����뱻�����Եĵ��á�
*        ��    �Σ������ṹ����ָ��
*        �� �� ֵ: ��
*********************************************************************************************************
*/
void Button_Detect(BUTTON_T *_pBtn)
{
  if (_pBtn->IsKeyDownFunc())
  {
    if (_pBtn->Count < _pBtn->FilterTime)
    {
      _pBtn->Count = _pBtn->FilterTime;
    }
    else if(_pBtn->Count < 2 * _pBtn->FilterTime)
    {
      _pBtn->Count++;
    }
    else
    {
      if (_pBtn->State == 0)
      {
        _pBtn->State = 1;

        /* ���Ͱ�ť���µ���Ϣ */
        if (_pBtn->KeyCodeDown > 0)
        {
          /* ��ֵ���밴��FIFO */
          Pannelkey_Put(_pBtn->key_type, _pBtn->KeyCodeDown);// ��¼�������±�־���ȴ��ͷ�

        }
      }

      if (_pBtn->LongTime > 0)
      {
        if (_pBtn->LongCount < _pBtn->LongTime)
        {
          /* ���Ͱ�ť�������µ���Ϣ */
          if (++_pBtn->LongCount == _pBtn->LongTime)
          {
            /* ��ֵ���밴��FIFO */
						_pBtn->IsLong = 1;
            Pannelkey_Put(_pBtn->key_type, _pBtn->KeyCodeLong);
          }
        }
        else
        {
          if (_pBtn->RepeatSpeed > 0)
          {
            if (++_pBtn->RepeatCount >= _pBtn->RepeatSpeed)
            {
              _pBtn->RepeatCount = 0;
              /* ��������ÿ��10ms����1������ */
              Pannelkey_Put(_pBtn->key_type, _pBtn->KeyCodeDown);

            }
          }
        }
      }
    }
  }
  else
  {
    if(_pBtn->Count > _pBtn->FilterTime)
    {
      _pBtn->Count = _pBtn->FilterTime;
    }
    else if(_pBtn->Count != 0)
    {
      _pBtn->Count--;
    }
    else
    {
      if (_pBtn->State == 1)
      {
        _pBtn->State = 0;

        /* ���Ͱ�ť�������Ϣ */
        if (_pBtn->KeyCodeUp > 0) /*�����ͷ�*/
        {
          /* ��ֵ���밴��FIFO */
					if(_pBtn->IsComb != 1) Pannelkey_Put(_pBtn->key_type, _pBtn->KeyCodeUp);
					_pBtn->IsComb = 0;
					_pBtn->IsLong = 0;
        }
				_pBtn->LongCount = 0;
				_pBtn->RepeatCount = 0;
      }
    }
  }
}


//����˵��: ������а�����10MS ����һ��
void Pannelkey_Polling(void)
{
  Button_Detect(&s_Powerkey);                /* POWER �� */
	Button_Detect(&s_Modekey);                /* MODE �� */
}

