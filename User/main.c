#define DEBUG_GSM_U3 1
//#define DEBUG_PM 1

#include "main.h"
#include "delay.h"
//#include "gsmlib.h"
//#include "gsmlib.cpp"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTPacket.h"  
#include "MyFifo.h"
#include "aqi.h"
//add for transmission
#include "uart.h"
//#include "uart3.h"
#include "interface.h"
#include "serialportAPI.h"
#ifdef TRANS_GPRS
#include "sim800C.h"
#elif TRANS_WIFI
#include "ESP8266.h"
#endif
//add for transmission
#include "cJSON.h"
#include "myJSON.h"
//add for ������
//#include "exti.h"
#include "key.h"
#include "rtc.h"
#include "stmflash.h"
//add for ����������
//#include "WS2812B.h"
#include "WS2812BNOP.h"
//add for ������
#include "buzzer.h"
//add for ��ȩ������
//#include "DSHCHO.h"
//add for PM2.5������
#include "PMS7003.h"
//add for OLED
#include "24tft.h"

//ָ�뺯������
void (*trans_module_restart)(void);
int (*trans_module_init)(const char *addr, uint32_t port, char *http_data);
int (*trans_module_send)(const uint8_t *buffer, uint32_t len);
int (*trans_module_recv)(uint8_t *buffer, uint32_t buffer_size, uint32_t timeout);

u16 U_ID[6];
char USART2_RX_BUF[10];
char power_state[4];
char mqtt_mode[2] = {"0"}; //ͨ��mqtt���յ���ָ��
unsigned int tim3_cnt = 1;	//Ϊ��ʵ��5���Ӷ�ʱ
unsigned int current_interval = CLOSE_INTERVAL;
volatile unsigned char fan_level = 0; //�Զ�ģʽ�µ��ٶȵ�λ


CONNECT_STATUS_ENUM All_State = initialWAITOK;
char http_buf[1];	//GPRSģ��ͨ��httpЭ���ȡ������
char topic_group[30];
char deviceID[20] = "200025";
MQTTString topicString = MQTTString_initializer;
unsigned char mqtt_buf[MQTT_SEND_SIZE];   
int mqtt_buflen = sizeof(mqtt_buf); 
int len = 0;
int rc;
char payload[MQTT_SEND_SIZE] = "testmessage\n";	//MQTT������ȥ������
int payloadlen;

unsigned char send_flag = 0;
unsigned char ping_flag = 0;
unsigned char auto_flag = 0;
unsigned char internal_flag = 0;
unsigned char opencover_flag = 0; //�Ƿ�򿪺�ǵ�ȫ�ֱ�־

int main()
{		
	delay_init();						//��ʱ������ʼ��
  Fifo_All_Initialize();	//fifo��ʼ��
  RCC_Configuration(); 		//ʱ������
  NVIC_Configuration(); 	//�ж�Դ����
  GPIO_Configuration(); 	//io����

  UartBegin(115200, &TRANS_USART, &U1_PutChar);				//����1����
	USART2Conf(9600, 0, 1);  												    //����2����
  USART3Conf(9600, 1, 1);															//����3����
	//DSHCHO_Init(115200, &USART3Conf, &U3_PutChar);
#ifndef ACTIVE_BEEP
	TIM1_Int_Init();								//�򿪶�ʱ��TIM1��������Դ��������PWM
#endif
  TIM2_Init();										//ÿ1ms�ж�һ�εĶ�ʱ����������¼ʱ��
	TIM3_Int_Init(9999, 7199);			//�򿪶�ʱ����ָ��ʱ�䷢�ʹ��������ݵ�������
	
	beep_init();										//��������ʼ��
	Panakey_Init();									//�������ⲿ�жϳ�ʼ��
	RTC_Init(2017, 1, 1, 0, 0, 0);	//ʵʱʱ�ӳ�ʼ�������������û��������ڲ���ʹ�á�
	OLED_init();										//OLEDģ���ʼ��

	RGB_Set(WS2812B_CUTDOWN, 4);		//�رտ���������
	SPILCD_Clear(0x00);							//OLED����
	beep_on(BEEP_ON);								//����������
	
	OLED_display_handle();					//OLED��Ļ��ʾ����
	OLED_uitype_change(UI_CONNECTING);		//OLED�л����������ӽ���
	printf("APP V1.0!!!!\r\n");
	printf("\r\n########### ��¼����: "__DATE__" - "__TIME__"\r\n");
	STMFLASH_Read(0x1ffff7e8,(u16*)U_ID,6);	//��ȡMCU_ID��
	printf("U_ID = %.4x-%.4x-%.4x-%.4x-%.4x-%.4x\r\n", U_ID[5],U_ID[4],U_ID[3],U_ID[2],U_ID[1],U_ID[0]);		
	
	delay_ms(1000);

  while(1)
  {
		OLED_display_handle();					//OLED��Ļ��ʾ����
		
		if(auto_flag == 1)
		{
			if(*mqtt_mode == 'A')
			{
				auto_mode((unsigned char *)&fan_level);
			}
			
			auto_flag = 0;
		}
		
		//��⵽��ϰ���
		if(key_flag.Comb_flag == 1)
		{
			key_flag.Comb_flag = 0;
			All_State = initialWAITOK;
			smartconfig_flag = 1;
			trans_module_restart();
			if(eATRESTORE()) printf("eATRESTORE is OK \r\n");
			else printf("eATRESTORE is Fail \r\n");
		}
		
		if(All_State != DISCNNECT)	//��������ʧ�ܲŻ�������������ݴ���
		{
			switch (All_State)
			{
				case initialWAITOK:
					if(!Initial_trans_module()) break;
					if(!Initial_MQTT()) break;
					if(!MQTT_Sub0Pub1()) break;
					wifi_status_set(1);
					break;
				case sendPM:
					Transmission_State();
					break;
				default:
					break;
			}
		}
		else
			delay_ms(1000);	//��Ϊ���ӳɹ�״̬�»���1S��ʱ
  }
}

void TRANS_USART(u32 baudRate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;     //GPRSģ��POWERKEY ���� WIFIģ���RST
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //�������
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOB, GPIO_Pin_0); //PB0�ϵ�͵�ƽ

#ifdef TRANS_GPRS	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;     //GPRSģ��VBAT
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //�������
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(GPIOA, GPIO_Pin_1); //PA1�ϵ�͵�ƽ
#endif
	
	USART1Conf(baudRate, 0, 0);
	
#ifdef TRANS_GPRS	
  trans_module_init = GSMInit;
	trans_module_restart = GSM_restart;
	trans_module_send = sim800C_send;
	trans_module_recv = sim800C_recv; //sim800C_recv��ʵ���˽����ݴ���fifo3�Ĺ���
#elif TRANS_WIFI
	trans_module_init = WifiInit;
	trans_module_restart = WIFI_restart;
	trans_module_send = esp8266_send;
	trans_module_recv = esp8266_recv;
#endif
}

void RCC_Configuration(void)
{
//  SystemInit();
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, ENABLE);
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE);
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3, ENABLE); //�򿪴��ڹܽ�ʱ��
}

void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

#ifndef SWIO_DEBUG	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	// �ı�ָ���ܽŵ�ӳ�� GPIO_Remap_SWJ_Disable SWJ ��ȫ���ã�JTAG+SW-DP��
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
	// �ı�ָ���ܽŵ�ӳ�� GPIO_Remap_SWJ_JTAGDisable ��JTAG-DP ���� + SW-DP ʹ��
#endif

	/*   OLED��ʾ��   */
																   //RS            CS          SCK           SDA    
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_13 | GPIO_Pin_15;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*   OLED��ʾ��   */
	
	/*   ������   */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;     //���������ػ�--PB8
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;     //��������
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;     //������ģʽ�л�--PB9
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;     //��������
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*   ������   */
	
	/*   ������   */
	//��beep_init()������
  //BUZZER_PIN_FΪPA11
	//BUZZER_PIN_VΪPA12
	/*   ������   */

	/*   ����������   */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;     //2����������
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);
	/*   ����������   */

	/*   ����̵���   */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;     //A4
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //�������
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;     //A5
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //�������
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;     //A6
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //�������
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;     //A7
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //�������
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*   ����̵���   */

	/*   ��������ģ��   */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;     //GPRSģ��POWERKEY ���� WIFIģ���RST
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //�������
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOB, GPIO_Pin_0); //PB0�ϵ�͵�ƽ

#ifdef TRANS_GPRS
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;     //GPRSģ��VBAT
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //�������
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(GPIOA, GPIO_Pin_1); //PA1�ϵ�͵�ƽ
#endif

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;   //USART1 TX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;   //USART1 RX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*   ��������ģ��   */

	/*   PM2.5������   */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //USART2 TX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;      //�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;      //USART2 RX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   //��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*   PM2.5������   */

	/*   ���Դ���   */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//USART3 TX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;//��������ٶ�50MHz
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//���츴�����
  GPIO_Init(GPIOB, &GPIO_InitStructure); //����ʼ���õĽṹ��װ��Ĵ���
	/*   ���Դ���   */

}

void NVIC_Configuration(void)  //�ж����ȼ�NVIC����
{
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;  //��USART1�ж�ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //���ȼ�
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  //ʹ���ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;  //��USART2�ж�ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //���ȼ�
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //�����ȼ�
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  //ʹ���ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//�򿪸��ж�
	NVIC_Init(&NVIC_InitStructure);

  /* Enable the TIM2 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //��ռ���ȼ�0��
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�3��
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
  NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���

#ifdef EXTI_GPIOB_4
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			//ʹ�ܰ���WK_UP���ڵ��ⲿ�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//��ռ���ȼ�2�� 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;					//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure); 
#else
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;			//ʹ�ܰ���WK_UP���ڵ��ⲿ�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//��ռ���ȼ�2�� 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;					//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure); 
#endif
}

void TIM3_Int_Init(u16 arr, u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��

  //��ʱ��TIM3��ʼ��
  TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
  TIM_TimeBaseStructure.TIM_Prescaler = psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

  TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx
}

void wifi_status_set(unsigned char status)
{
	if(status)
		All_State = sendPM;
	else
		All_State = DISCNNECT;
	
	OLED_wifi_status_set(status);
}

int Initial_trans_module()
{
  printf("test!\r\n");

	//while(0 == trans_module_init(HOST_NAME, HOST_PORT, http_buf)) trans_module_restart();
	if(trans_module_init(HOST_NAME, HOST_PORT, http_buf))
	{
		printf("trans_module_init is OK!");
		All_State = initialTCP;
	}
	else
	{
		printf("trans_module_init is FAIL!");
		wifi_status_set(0);
		return 0;
	}
	return 1;
}

int Initial_MQTT()
{

  MQTTPacket_connectData mqtt_data = MQTTPacket_connectData_initializer;
  mqtt_data.clientID.cstring = deviceID;
  mqtt_data.keepAliveInterval = PING_SET;
  mqtt_data.cleansession = 1;
  mqtt_data.username.cstring = deviceID;
  mqtt_data.password.cstring = "testpassword";
  //for will message
  mqtt_data.willFlag = 1;
  sprintf(topic_group, "clients/%s/state", deviceID);
  printf("willtopic = %s\r\n", topic_group);
  mqtt_data.will.topicName.cstring = topic_group;
  mqtt_data.will.message.cstring = "0";
  mqtt_data.will.qos = 1;
  mqtt_data.will.retained = 1;

  len = MQTTSerialize_connect(mqtt_buf, mqtt_buflen, &mqtt_data);  //��仰��ʼMQTT�����ӣ����ǲ�ֱ�Ӻͷ��ͺ������������Ǵ浽һ��buf���棬�ٴ�buf���淢��
  trans_module_send(mqtt_buf, len);

  mqtt_recv(mqtt_buf, sizeof(mqtt_buf), 1000);
  rc = MQTTPacket_read(mqtt_buf, mqtt_buflen, fifo3readdata);
	printf("rc = %d\r\n", rc);
  if ( rc == CONNACK)   //����ѻ�ȡ���ݵ�ָ�봫�˽�ȥ������
  {
    unsigned char sessionPresent, connack_rc;

    if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, mqtt_buf, mqtt_buflen) != 1 || connack_rc != 0)
    {
      printf("MQTT CONNACK1 FAILED!\r\n");
      wifi_status_set(0);
			return 0;
    }
    else
    {
      All_State = initialMQTT;
      printf("MQTT CONNACK OK!\r\n");
    }
  }
  else
  {
    //failed ???
    printf("MQTT CONNACK2 FAILED!\r\n");
    wifi_status_set(0);
		return 0;
  }
	
	return 1;
}

int MQTT_Sub0Pub1()
{

  int msgid = 1;
  int req_qos = 0;

  //��������
  sprintf(topic_group, "SHAir/%s/get", deviceID);
  printf("subtopic = %s\r\n", topic_group);
  topicString.cstring = topic_group;
  len = MQTTSerialize_subscribe(mqtt_buf, mqtt_buflen, 0, msgid, 1, &topicString, &req_qos);
  //������Щ������ֱ�ӷ��ͣ�����ͨ���Ȼ�ȡbuffer���������ֶ����ͳ�ȥ
  trans_module_send(mqtt_buf, len);

  mqtt_recv(mqtt_buf, sizeof(mqtt_buf), 1000);
  rc = MQTTPacket_read(mqtt_buf, mqtt_buflen, fifo3readdata);
  if (rc == SUBACK)  /* wait for suback */ //��������������
  {
    unsigned short submsgid;
    int subcount;
    int granted_qos;

    rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, mqtt_buf, mqtt_buflen);
    if (granted_qos != 0)
    {
      //wrong
      printf("MQTT SUBACK1 FAILED!\r\n");
      wifi_status_set(0);
			return 0;
    }
    else
    {
      printf("MQTT SUBACK OK!\r\n");
    }
  }
  else
  {
    printf("MQTT SUBACK2 FAILED!\r\n");
    wifi_status_set(0);
		return 0;
  }
	
	//����retain���ݣ�expiresAt��childLock
	mqtt_recv(mqtt_buf, sizeof(mqtt_buf), 1000);
  rc = MQTTPacket_read(mqtt_buf, mqtt_buflen, fifo3readdata);
	if (rc == PUBLISH)
  {
    unsigned char dup;
    int qos;
    unsigned char retained;
    unsigned short msgid;
    int payloadlen_in;
    unsigned char* payload_in;
    MQTTString receivedTopic;

		printf("recive retain publish\r\n");
    rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
                                 &payload_in, &payloadlen_in, mqtt_buf, mqtt_buflen);
    //handle "payload_in" as data from the server
		printf("retained = %d, msgid = %d, receivedTopic = %s", retained, msgid, receivedTopic.cstring);

    recv_mqtt(payload_in, payloadlen_in, payload, &payloadlen);
  }

  //����������ʾ
  if(!Public_Open(5))
  {
    printf("PUBLIC OPEN ERROR");
    wifi_status_set(0);
		return 0;
  }
  printf("PUBLIC OPEN OK!\r\n");

  //��������
  sprintf(topic_group, "SHAir/%s/update", deviceID);
  printf("pubtopic = %s", topic_group);
  topicString.cstring = topic_group;
	
	//���������������
	beep_on(BEEP_CONNECT);

	//��¼����ʱ��
	if(gprs_connect_cnt == 0)
		gprs_connect_time[gprs_connect_cnt] = UNIXtime2date(RTC_GetCounter());
	else
		gprs_connect_time[gprs_connect_cnt] = RTC_GetCounter() - break_time;
	gprs_connect_cnt++;
	if(gprs_connect_cnt >= GPRS_STATE_TIME_SIZE)	gprs_connect_cnt = 0;
	
	//���Ͷ����ڼ����������¼�����û�в�������������mode���ݡ�
	SendJson(CONNECTION_MODE);
//	//�����ŷ��͵���λ����Ϣ��
//	SendJson(GEO_MODE);
	
	return 1;
}

int Public_Open(int time)
{
  int i = 0;
	unsigned char dup = 0;
  int qos = 1;
  unsigned char retain = 1;
	unsigned short packedid = 1;	//PUBLISH��QoS ���� 0�����Ʊ��� �������һ������� 16 λ���ı�ʶ����Packet Identifier��

  for(i = 0; i < time; i++)
  {
    sprintf(topic_group, "clients/%s/state", deviceID);
    printf("opentopic = %s\r\n", topic_group);
    topicString.cstring = topic_group;
    sprintf(payload, "1");
    //strcpy(payload, http_buf);
    payloadlen = strlen(payload);
    len = MQTTSerialize_publish(mqtt_buf, mqtt_buflen, dup, qos, retain, packedid, topicString, (unsigned char*)payload, payloadlen);
    trans_module_send(mqtt_buf, len);

    mqtt_recv(mqtt_buf, sizeof(mqtt_buf), 1000);
    rc = MQTTPacket_read(mqtt_buf, mqtt_buflen, fifo3readdata);
    printf("PUBLIC OPEN rc = %d\r\n", rc);
    if(rc == PUBACK) return 1;
		else dup = 1;	//��� DUP ��־������Ϊ 1����ʾ�������һ����ǰ����������ط���
  }

  return 0;
}

void Transmission_State()
{
//	//���Ͷ����ڼ���������¼
//	if(wait_send_press == 1)
//	{
//		wait_send_press = 0;
//		printf("send press!\r\n");
//		SendJson(PRESS_MODE);
//	}
	
	//����������
	if(ping_flag == 1)
	{
		ping_flag = 0;
		if(!SendPingPack(5))
		{
			printf("pingpack is FAILED!\r\n");
			//��¼����ʱ��
			break_time = RTC_GetCounter();
			gprs_break_time[gprs_break_cnt] = UNIXtime2date(break_time);
			//gprs_break_time[gprs_break_cnt] = RTC_GetCounter();
			
			gprs_break_cnt++;
			if(gprs_break_cnt >= GPRS_STATE_TIME_SIZE)	gprs_break_cnt = 0;
			//����GPRSģ��
			wifi_status_set(0);
			return;
		}
	}

  /* transport_getdata() has a built-in 1 second timeout,
  your mileage will vary */
  trans_module_recv(mqtt_buf, sizeof(mqtt_buf), 1000);
  rc = MQTTPacket_read(mqtt_buf, mqtt_buflen, fifo3readdata);
  if (rc == PUBLISH)
  {
    unsigned char dup;
    int qos;
    unsigned char retained;
    unsigned short msgid;
    int payloadlen_in;
    unsigned char* payload_in;
    MQTTString receivedTopic;

    rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
                                 &payload_in, &payloadlen_in, mqtt_buf, mqtt_buflen);
    //handle "payload_in" as data from the server

    recv_mqtt(payload_in, payloadlen_in, payload, &payloadlen);
		//���ؽ��յ�������
		len = MQTTSerialize_publish(mqtt_buf, mqtt_buflen, 0, 0, 0, 0, topicString, (unsigned char*)payload, payloadlen);
    trans_module_send(mqtt_buf, len);
  }

//	if(internal_flag == 1)	//internal_flag������send_flag֮ǰ���������������һλ��Ϊ0
//	{
//		internal_flag = 0;
//				
//		//eATCSQ(&sim_csq);	//��ȡ��ǰcsqֵ		
//		
//		if(send_flag == 1) //��Ϊtim3_cnt�����ж��м���ģ���send_flag=1ʱtim3_cnt�������㣬���ֻ����current_interval��ֵ����λ���һ�����ݵ�λ��
//		{
//			CSQ[(current_interval / COUNT_INTERVAL) - 1] = sim_csq;
//		}
//		else 
//		{
//			CSQ[(tim3_cnt / COUNT_INTERVAL) - 1] = sim_csq;
//		}
//	}
	
  if(send_flag == 1)
  {
		send_flag = 0;
		SendJson(SENDDATA_MODE);
  }
}

//��ʱ��3�жϷ������
void TIM3_IRQHandler(void)   //TIM3�ж�
{
//  static unsigned int tim3_cnt = 0;	//Ϊ��ʵ��5���Ӷ�ʱ

  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
  {
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx�����жϱ�־
		
//		//GPRS�ź�ָʾ��
//		if(All_State == sendPM)	SIGNAL_LED = 0;
//		else	SIGNAL_LED = !SIGNAL_LED;
		
		if(tim3_cnt % AUTOMODE_INTERVAL == 0)
		{
			auto_flag = 1;
			AirLEDControl();
		}
		
//		if((tim3_cnt+1) % COUNT_INTERVAL == 0)	//Ҫ��ǰ1S���ͼ�ȩ������������
//		{
//			Send_DSHCHO_Cmd(); //����ȩ��������������
//		}

    if(tim3_cnt % COUNT_INTERVAL == 0)
    {
//			internal_flag = 1;	//���и�CSQ�����ݲ����ж��н��д�����Ϊֻ�е�����������ܻ�ȡCSQֵ�����һ�ȡCSQ�ĺ��������ӳپ�����Ҫ���ж���ִ��
			
			//HCHO[(tim3_cnt / COUNT_INTERVAL) - 1] = Conce_HCHO;
			C1[(tim3_cnt / COUNT_INTERVAL) - 1] = Conce_PM2_5;
			C2[(tim3_cnt / COUNT_INTERVAL) - 1] = Conce_PM10;
			AQI1[(tim3_cnt / COUNT_INTERVAL) - 1] = AQI_2_5;
			AQI2[(tim3_cnt / COUNT_INTERVAL) - 1] = AQI_10;
			AQI[(tim3_cnt / COUNT_INTERVAL) - 1] = AQI_Max;
			L[(tim3_cnt / COUNT_INTERVAL) - 1] = fan_level;
    }
 
		if((tim3_cnt % PING_INTERVAL == 0) && (All_State == sendPM))
		{
			ping_flag = 1;
		}
		
    if((tim3_cnt >= current_interval) && (All_State == sendPM))
    {
      tim3_cnt = 0;
      send_flag = 1;
    }

    tim3_cnt++;
  }
}

void SendJson(u8 mode)
{
	group_json(mode);
	payloadlen = strlen(payload);
	len = MQTTSerialize_publish(mqtt_buf, mqtt_buflen, 0, 0, 0, 0, topicString, (unsigned char*)payload, payloadlen);
	trans_module_send(mqtt_buf, len);
	
	printf("mode = %d, SendJson len = %d\r\n", mode, len);
}

int SendPingPack(int times)
{
	int i = 0;
	for(i=0;i<times;i++)
	{
		memset(mqtt_buf, 0, sizeof(mqtt_buf));
		len = MQTTSerialize_pingreq(mqtt_buf, 100);
		trans_module_send(mqtt_buf, len);

		mqtt_recv(mqtt_buf, sizeof(mqtt_buf), 1000);
		rc = MQTTPacket_read(mqtt_buf, mqtt_buflen, fifo3readdata);
		if ( rc == PINGRESP)   //����ѻ�ȡ���ݵ�ָ�봫�˽�ȥ������
		{
			printf("rc = %d, pingreq is successful!\r\n", rc);
			return 1;
		}
		else
		{
			printf("rc = %d, pingreq is failed!\r\n", rc);
		}
	}
	return 0;
}

void recv_mqtt(unsigned char* recv_data, int data_len, char* return_data, int* return_len)
{
  char * mode_tmp;
  char * power_tmp;
	char * firmware_cmd;
	int rtc_count;
	int timeout_count;
	char *ftp_server;
	char *ftp_username;
	char *ftp_passwd;
	char *ftp_filename;
	char *ftp_path;
  cJSON *mqtt_recv_root = NULL;
	cJSON *RTC_obj = NULL;
	cJSON *firmware_obj = NULL;
	char * out;
	
	//for debug
	u32 test_flag = 0;

  printf("MQTT recive data: %s, data_len: %d\r\n", recv_data, data_len);

  mqtt_recv_root = cJSON_Parse((const char *)recv_data);
  if(mqtt_recv_root != NULL)
  {
    printf("mqtt_recv_root is ok!\r\n");
		
		//��expiresAt��
		if(cJSON_GetObjectItem(mqtt_recv_root, "expiresAt") != NULL)
    {
			printf("expiresAt is OK\r\n");
			if(cJSON_IsNumber(cJSON_GetObjectItem(mqtt_recv_root, "expiresAt")))
			{
				printf("expiresAt is Number\r\n");
				//��ʱ��������ݸ�ʽ
				timeout_count = cJSON_GetObjectItem(mqtt_recv_root, "expiresAt")->valueint;
				if(timeout_count > 1502883485 && timeout_count < 2147483647)
				{
					printf("expiresAt = %d\r\n", timeout_count);
//					Flash_Write_Number(timeout_count, FLASH_SAVE_ADDR); //����ʱʱ��д��stm32��flash�У�д���ַ����ȵ�ǰ����Ĵ�СҪ��
//					Flash_Write_Number(0xaaaaaaaa, FLASH_SAVE_ADDR+4);
				}
				else printf("expiresAt number is Error\r\n");
			}
			else printf("expiresAt not Number\r\n");
    }
		
		//��childLock��
		if(cJSON_GetObjectItem(mqtt_recv_root, "childLock") != NULL)
    {
			if(All_State == sendPM) beep_on(BEEP_CMD);
			printf("childLock is OK\r\n");
			if(cJSON_IsTrue(cJSON_GetObjectItem(mqtt_recv_root, "childLock")))
			{
				printf("childLock is true\r\n");
				key_flag.ChildLock_flag = 1;
			}
			else if(cJSON_IsFalse(cJSON_GetObjectItem(mqtt_recv_root, "childLock")))
			{
				printf("childLock is false\r\n");
				key_flag.ChildLock_flag = 0;
			}
			else	printf("childLock ERROR!!!\r\n");
    }

    //����powerָ��յ�power on�����Զ�ģʽ���յ�power off�رշ�����յ�������ϢҲ�رշ��
    if(cJSON_GetObjectItem(mqtt_recv_root, "power") != NULL)
    {			
      power_tmp = cJSON_GetObjectItem(mqtt_recv_root, "power")->valuestring;
      strcpy(power_state, power_tmp);
      printf("recive power is %s, power_state is %s\r\n", power_tmp, power_state);
      if(!strcmp(power_state, "on"))
      {
				PowerOnOff(1);
      }
      else if(!strcmp(power_state, "off"))
      {
				PowerOnOff(0);
      }
      else
      {
        PowerOnOff(0);
      }
    }

    //����mode����
    if(cJSON_GetObjectItem(mqtt_recv_root, "mode") != NULL)
    {
      mode_tmp = cJSON_GetObjectItem(mqtt_recv_root, "mode")->valuestring;
      strcpy(mqtt_mode, mode_tmp);
      printf("mode_tmp = %s, mqtt_mode = %s\r\n", mode_tmp, mqtt_mode);
			ModeCountrol();
    }
		
    //����fan_test����
    if(cJSON_GetObjectItem(mqtt_recv_root, "fan_test") != NULL)
    {
			char * fan_test;
      fan_test = cJSON_GetObjectItem(mqtt_recv_root, "fan_test")->valuestring;
      printf("fan_test = %s\r\n", fan_test);
			FanTest(fan_test);
    }
		
		//����RTC����
		RTC_obj = cJSON_GetObjectItem(mqtt_recv_root, "RTC");
		if(RTC_obj != NULL)
    {
			//��ʱ��������ݸ�ʽ
			rtc_count = cJSON_GetObjectItem(mqtt_recv_root, "RTC")->valueint;
			//rtc_count += 28800;	//ʱ����Ǵ�1970-1-1 08:00:00��ʼ�ģ������ǵ��㷨�Ǵ�1970-1-1 00:00:00�������Ҫ������Ӧ������
			PWR_BackupAccessCmd(ENABLE);	//ʹ��RTC�ͺ󱸼Ĵ�������
			RTC_SetCounter(rtc_count);	//����RTC��������ֵ
			RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
    }
		
		//���ض�����¼����
		if(cJSON_GetObjectItem(mqtt_recv_root, "GPRS_RECORD") != NULL)
		{
			SendJson(GPRS_RECORD_MODE);
		}
		
		//����firmware update����
		firmware_obj = cJSON_GetObjectItem(mqtt_recv_root, "firmware");
		if(firmware_obj != NULL)
    {
			if(cJSON_GetObjectItem(firmware_obj, "cmd") != NULL)
			{
				firmware_cmd = cJSON_GetObjectItem(firmware_obj, "cmd")->valuestring;
				if(!strcmp(firmware_cmd, "update"))
				{
					//��ftp�Ĳ���д�뵽Ƭ��flash��
					if(cJSON_GetObjectItem(firmware_obj, "server") != NULL &&
					cJSON_GetObjectItem(firmware_obj, "username") != NULL &&
					cJSON_GetObjectItem(firmware_obj, "passwd") != NULL &&
					cJSON_GetObjectItem(firmware_obj, "filename") != NULL &&
					cJSON_GetObjectItem(firmware_obj, "path") != NULL)
					{
						ftp_server = cJSON_GetObjectItem(firmware_obj, "server")->valuestring;
						ftp_username = cJSON_GetObjectItem(firmware_obj, "username")->valuestring;
						ftp_passwd = cJSON_GetObjectItem(firmware_obj, "passwd")->valuestring;
						ftp_filename = cJSON_GetObjectItem(firmware_obj, "filename")->valuestring;
						ftp_path = cJSON_GetObjectItem(firmware_obj, "path")->valuestring;
						printf("strlen = %d, %d, %d, %d, %d\r\n", strlen(ftp_server), strlen(ftp_username), strlen(ftp_passwd), strlen(ftp_filename), strlen(ftp_path));
						if(strlen(ftp_server)<32&&strlen(ftp_username)<32&&strlen(ftp_passwd)<32&&strlen(ftp_filename)<32&&strlen(ftp_path)<32)
						{
							Flash_Write_Str(FLASH_FTP_SERVER, (u8 *)ftp_server, FTP_PARAM_SIZE);
							Flash_Write_Str(FLASH_FTP_USERNAME, (u8 *)ftp_username, FTP_PARAM_SIZE);
							Flash_Write_Str(FLASH_FTP_PASSWD, (u8 *)ftp_passwd, FTP_PARAM_SIZE);
							Flash_Write_Str(FLASH_FTP_FILENAME, (u8 *)ftp_filename, FTP_PARAM_SIZE);
							Flash_Write_Str(FLASH_FTP_PATH, (u8 *)ftp_path, FTP_PARAM_SIZE);
												
							Flash_Write_Number(0, FLASH_FIRMWARE_FLAG);	//д0��ʾҪ���¹̼�
							test_flag = Flash_Read_Number(FLASH_FIRMWARE_FLAG);
							printf("test_flag = %d\r\n", test_flag);
							//����
							__disable_fault_irq();   
							NVIC_SystemReset();
							while(1);
						}
						else printf("parame size >= 32!\r\n");
					}
					else printf("parame is error!\r\n");
				}
				else printf("cmd != update\r\n");
			}
			else printf("cJSON_GetObjectItem(firmware_obj, \"cmd\") == NULL\r\n");

    }
		
		//���յ���JSON�����������أ����ƻ�Ӧ��Ϣ
		out = cJSON_Print(mqtt_recv_root);			
		strcpy(return_data, out);
		
		//�����ͷ�out�Ŀռ䣬��������
		free(out);
  }
	else
	{
		printf("mqtt_recv_root is NULL!\r\n");
		strcpy(return_data, "recive data is error!");
	}
	
	*return_len = strlen(payload);
	printf("return_data = %s�� return_len = %d\r\n", return_data, *return_len);

  //�����ͷ�json�Ŀռ䣬��������
  cJSON_Delete(mqtt_recv_root);
}

void FanTest(char * fan_test)
{
	int fan_tmp = 0;
	switch(*fan_test){
		case '0': fan_tmp = 0x0; break;
		case '1': fan_tmp = 0x1; break;
		case '2': fan_tmp = 0x2; break;
		case '3': fan_tmp = 0x3; break;
		case '4': fan_tmp = 0x4; break;
		case '5': fan_tmp = 0x5; break;
		case '6': fan_tmp = 0x6; break;
		case '7': fan_tmp = 0x7; break;
		case '8': fan_tmp = 0x8; break;
		case '9': fan_tmp = 0x9; break;
		case 'a': fan_tmp = 0xa; break;
		case 'b': fan_tmp = 0xb; break;
		case 'c': fan_tmp = 0xc; break;
		case 'd': fan_tmp = 0xd; break;
		case 'e': fan_tmp = 0xe; break;
		case 'f': fan_tmp = 0xf; break;
		default: fan_tmp = 0x0; printf("fan_test is error!\r\n");
	}
	printf("fan_tmp = %d, fan_test = %s\r\n", fan_tmp, fan_test);
	SetMotorLevel(fan_tmp);
}

void PowerOnOff(unsigned char on_off)
{
	if(on_off)
	{
		strcpy(mqtt_mode, "A");
    printf("power on, mqtt_mode = %s\r\n", mqtt_mode);
		OLED_uitype_change(UI_MAIN);	//OLED�л���������
	}
	else
	{
		strcpy(mqtt_mode, "0");
    printf("power messge is error, mqtt_mode = %s\r\n", mqtt_mode);
		OLED_uitype_change(UI_CLOSE);	//OLED�л���Ϣ��
	}
	
	ModeCountrol();
}

void ModeCountrol(void)
{
  if(*mqtt_mode == '0')
  {		
		fan_level = 0;
  }
  else if((*mqtt_mode > '0' && *mqtt_mode <= '9') || (*mqtt_mode == 'A'))
  {
    if(*mqtt_mode == 'A') 	//�Զ�ģʽ
    {
      if(Conce_PM2_5 >= 0 && Conce_PM2_5 < PM2_5_LEVEL1) fan_level = 1;
      else if(Conce_PM2_5 >= PM2_5_LEVEL1 & Conce_PM2_5 < PM2_5_LEVEL2) fan_level = 2;
      else if(Conce_PM2_5 >= PM2_5_LEVEL2) fan_level = 3;
      else
      {
        fan_level = 0;
        printf("Conce_PM2_5 is error\r\n");
      }
    }
    else    //����ģʽ
    {
      fan_level = 0;  //�ر��Զ�ģʽ
      switch(*mqtt_mode)
      {
        case '1':
          fan_level = 1;
          break;
        case '2':
          fan_level = 2;
          break;
        case '3':
          fan_level = 3;
          break;
        case '4':
          fan_level = 4;
          break;
        case '0':
          fan_level = 0;
          break;
        default:
          fan_level = 0;
          break;
      }
    }
  }
	
	MotorCountrol(fan_level);	//���Ʒ��
	
	//�ı�OLEDģʽ��ʾ
	switch(*mqtt_mode)
	{
		case 'A':
			OLED_mode_change(OLED_AUTO_MODE);
			break;
		case '1':
			OLED_mode_change(OLED_SLEEP_MODE);
			break;
		case '2':
			OLED_mode_change(OLED_SPEED1_MODE);
			break;
		case '3':
			OLED_mode_change(OLED_SPEED2_MODE);
			break;
		case '4':
			OLED_mode_change(OLED_SPEED3_MODE);
			break;
//		case '0':
//			OLED_mode_change(OLED_AUTO_MODE);
//			break;
//		default:
//			OLED_mode_change(OLED_AUTO_MODE);
//			break;
	}
	
	//���Ʒ�����
	beep_on(BEEP_CMD);
	
	//������ڴ���״̬��˯��ģʽ�¹رտ���������
	AirLEDControl();
}

void SetMotorLevel(int cmd)
{
  GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)((cmd >> 0) & 1));
  GPIO_WriteBit(GPIOA, GPIO_Pin_5, (BitAction)((cmd >> 1) & 1));
  GPIO_WriteBit(GPIOA, GPIO_Pin_6, (BitAction)((cmd >> 2) & 1));
  GPIO_WriteBit(GPIOA, GPIO_Pin_7, (BitAction)((cmd >> 3) & 1));
}

void MotorCountrol(unsigned char level)
{
  switch(level)
  {
    case 1:
      SetMotorLevel(MOTORSPEED1);
			OLED_air_set(OLED_AIR_S);
      printf("Set Motor speed is 1\r\n");
      break;
    case 2:
      SetMotorLevel(MOTORSPEED2);
			OLED_air_set(OLED_AIR_1);
      printf("Set Motor speed is 2\r\n");
      break;
    case 3:
      SetMotorLevel(MOTORSPEED3);
			OLED_air_set(OLED_AIR_2);
      printf("Set Motor speed is 3\r\n");
      break;
    case 4:
      SetMotorLevel(MOTORSPEED4);
			OLED_air_set(OLED_AIR_3);
      printf("Set Motor speed is 4\r\n");
      break;
    default:
      SetMotorLevel(MOTORSPEED0);
			OLED_air_set(OLED_AIR_0);
      printf("Set Motor is power down\r\n");
  }
}

void auto_mode(unsigned char *level)
{
  switch(*level)
  {
    case 1:
      if(AQI_Max > PM2_5_LEVEL1 + LEVEL_OFFSET)
      {
        (*level)++;
        MotorCountrol(*level);
        printf("Auto Motor speed is 2\r\n");
      }
      break;
    case 2:
      if(AQI_Max > PM2_5_LEVEL2 + LEVEL_OFFSET)
      {
        (*level)++;
        MotorCountrol(*level);
        printf("Auto Motor speed is 3\r\n");
      }
      else if(AQI_Max < PM2_5_LEVEL1 - LEVEL_OFFSET)
      {
        (*level)--;
        MotorCountrol(*level);
        printf("Auto Motor speed is 1\r\n");
      }
      break;
    case 3:
      if(AQI_Max < PM2_5_LEVEL2 - LEVEL_OFFSET)
      {
        (*level)--;
        MotorCountrol(*level);
        printf("Auto Motor speed is 2\r\n");
      }
      break;
  }
}

void AirLEDControl(void)
{
	if(PM2_5_OK == 0)
	{
		RGB_Set(WS2812B_WHITE, 4);
		return;	//����ж�pm2.5������û�������������ÿ�����������ʾ��ɫ���������ú���
	}
	if(*mqtt_mode == '0' || *mqtt_mode == '1')
	{
		RGB_Set(WS2812B_CUTDOWN, 4);
		//printf("sleep mode close the airled!\r\n");
	}
	else if(AQI_Max <= PM2_5_LEVEL1)
	{
		//WS2812_send(colors[4], 4);
		RGB_Set(WS2812B_GREEN, 4);
		//printf("GREEN\r\n");
	}
	else if(AQI_Max > PM2_5_LEVEL1 && AQI_Max <= PM2_5_LEVEL2)
	{
		//WS2812_send(colors[1], 4);
		RGB_Set(WS2812B_ORANGE, 4);
		//printf("ORANGE\r\n");
	}
	else if(AQI_Max > PM2_5_LEVEL2)
	{
		//WS2812_send(colors[0], 4);
		RGB_Set(WS2812B_RED, 4);
		//printf("RED\r\n");
	}
	else
	{
		RGB_Set(WS2812B_WHITE, 4);
		printf("airled control is error!\r\n");
	}
}


