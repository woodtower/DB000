/******************************************************************************
*  Include Files
******************************************************************************/
#include "buzzer.h"
#include "delay.h"

/********************************************************************
* Macros
********************************************************************/

/********************************************************************
* Types
********************************************************************/

/********************************************************************
* Extern Variables (Extern /Global)
********************************************************************/
beep_play_t beep_play;

/********************************************************************
* Local Variables:  STATIC
********************************************************************/
static unsigned long beep_start_tick = 0;
static u8 beep_note_id = 0;


/********************************************************************
* External Functions declaration
********************************************************************/

/********************************************************************
* Local Function declaration
********************************************************************/

/********************************************************************
* Local Function
********************************************************************/
void TIM1_Int_Init(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
//	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;

  //ʹ��TIM1��ʱ��
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = BEEP_TIMER_PERIOD - 1; // 1000kHz
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
  TIM_ARRPreloadConfig(TIM1, ENABLE);

  /* PWM1 Mode configuration: Channel4 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_Pulse = BEEP_TIMER_PWM_PLUSE;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
  TIM_OC1Init(TIM1, &TIM_OCInitStructure);

//	 //��������
//	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;
//	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;
//	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
//	TIM_BDTRInitStructure.TIM_DeadTime = 0x90;  //�������������С0-0xff
//	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;
//	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;
//	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
//	TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);

  TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

  //TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable); //ʹ��Ԥװ�ؼĴ���
  //TIM_SetCompare4(TIM1,300);
  //TIM_CtrlPWMOutputs(TIM1, ENABLE);	//TIM1_OCͨ�����PWM��һ��Ҫ�ӣ�
	TIM_CtrlPWMOutputs(TIM1, DISABLE);	//�ȹر�PWM���
  TIM_Cmd(TIM1, ENABLE); //ʹ�� TIM1
}

void beep_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//TIM1_CHN1 GPIO��ʼ��������BUZZER_PIN_F
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//PA12 GPIO��ʼ��������BUZZER_PIN_V
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	BUZZER_PIN_F = 0;
	BUZZER_PIN_V = 0;
	
	beep_play.beep_status = BEEP_STATUS_STOP;
}

void beep_on(beep_tpye_enum beep_type)
{
#ifdef ACTIVE_BEEP
  BUZZER_PIN_F = 1;
  delay_ms(time);
  BUZZER_PIN_F = 0;
#else
	switch(beep_type)
	{
		case BEEP_ON:
			beep_play.note_num = 3;
			beep_play.note[0].freq = F2_3;
			beep_play.note[0].Tv = 200;
			beep_play.note[0].Tf = 300;
			beep_play.note[1].freq = F2_6;
			beep_play.note[1].Tv = 200;
			beep_play.note[1].Tf = 300;
			beep_play.note[2].freq = F2_9;
			beep_play.note[2].Tv = 200;
			beep_play.note[2].Tf = 300;
			break;
		case BEEP_OFF:
			beep_play.note_num = 3;
			beep_play.note[0].freq = F2_9;
			beep_play.note[0].Tv = 200;
			beep_play.note[0].Tf = 300;
			beep_play.note[1].freq = F2_6;
			beep_play.note[1].Tv = 200;
			beep_play.note[1].Tf = 300;
			beep_play.note[2].freq = F2_3;
			beep_play.note[2].Tv = 200;
			beep_play.note[2].Tf = 300;
			break;
		case BEEP_CMD:
			beep_play.note_num = 1;
			beep_play.note[0].freq = F4;
			beep_play.note[0].Tv = 200;
			beep_play.note[0].Tf = 300;
			break;
		case BEEP_CONNECT:
			beep_play.note_num = 2;
			beep_play.note[0].freq = F2_3;
			beep_play.note[0].Tv = 200;
			beep_play.note[0].Tf = 300;
			beep_play.note[1].freq = F4;
			beep_play.note[1].Tv = 200;
			beep_play.note[1].Tf = 300;
			break;
	}
	beep_play.type = beep_type;
	beep_play.beep_status = BEEP_STATUS_START;
#endif
}

void beep_handle(unsigned long current_tick)
{
	if(beep_play.beep_status == BEEP_STATUS_START)
	{
		beep_start_tick = current_tick;
		beep_note_id = 0;
		BUZZER_PIN_V = 1; //�򿪷����������
		TIM_PrescalerConfig(TIM1, beep_play.note[beep_note_id].freq, TIM_PSCReloadMode_Update); //���÷��������ź������PWM��Ƶ��
		TIM_CtrlPWMOutputs(TIM1, ENABLE);	//�򿪷��������ź������PWM
		beep_play.beep_status = BEEP_STATUS_KEEP;
	}
	else if(beep_play.beep_status == BEEP_STATUS_KEEP)
	{
		if(beep_note_id+1 > beep_play.note_num) //�ж��Ƿ���������Ҫ����
		{
			beep_play.beep_status = BEEP_STATUS_STOP;
		}
		else
		{
			if(current_tick == beep_start_tick+beep_play.note[beep_note_id].Tv)
			{
				BUZZER_PIN_V = 0; //�رշ����������
			}
			else if(current_tick >= beep_start_tick+beep_play.note[beep_note_id].Tf)
			{
				beep_note_id++;
				if(beep_note_id+1 > beep_play.note_num) //beep_note_id��һ���ж��Ƿ���������Ҫ����
				{
					beep_play.beep_status = BEEP_STATUS_STOP;
					BUZZER_PIN_V = 0; //�رշ����������
					TIM_CtrlPWMOutputs(TIM1, DISABLE);	//�رշ��������ź������PWM
				}
				else
				{
					beep_start_tick = current_tick;
					BUZZER_PIN_V = 1; //�򿪷����������
					TIM_PrescalerConfig(TIM1, beep_play.note[beep_note_id].freq, TIM_PSCReloadMode_Update); //���÷��������ź������PWM��Ƶ��
				}
			}
		}
	}
}


