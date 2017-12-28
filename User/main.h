#include "sys.h"

//#define SWIO_DEBUG

//transmission type ��������Ϊȫ�ֺ꣬�����Option for Target��������
//#define TRANS_GPRS 1
#define TRANS_WIFI 1

//ALL_Status
#define initialWAITOK -1
#define initialTCP -2
#define initialMQTT -3
#define sendPM -4

//���Ƶ���ٶ�
#define MOTORSPEED0 0x0
#define MOTORSPEED1 0x6
#define MOTORSPEED2 0xa
#define MOTORSPEED3 0xc
#define MOTORSPEED4 0x1

//�Զ������ٽ�ֵ
#define PM2_5_LEVEL1 50
#define PM2_5_LEVEL2 100
#define PM2_5_LEVEL3 150
#define LEVEL_OFFSET 10

//LED
//#define SIGNAL_LED PBout(3)
#define AUTO_LED PBout(5)
#define SLEEP_LED PBout(6)
#define SPEED1_LED PBout(7)
#define SPEED2_LED PBout(8)
#define SPEED3_LED PBout(9)

void TRANS_USART(u32 baudRate);
void RCC_Configuration(void);
void NVIC_Configuration(void);
void GPIO_Configuration(void);
void TIM3_Int_Init(u16 arr,u16 psc);

//ͨ��״̬���ķ�ʽ����������������
void restart_MCU(void);
void Initial_trans_module(void);
void Initial_MQTT(void);
void MQTT_Sub0Pub1(void);
int Public_Open(int time);	//��������ָ��
void Transmission_State(void);
//int fifo1readdata(unsigned char* s, int maxlen);
//int fifo3readdata(unsigned char* s, int maxlen);
void SendJson(u8 mode);
void SendPress(void);
int SendPingPack(int times);
void recv_mqtt(unsigned char* recv_data, int data_len, char* return_data, int* return_len);	//����mqtt������

void ModeCountrol(void);
void LedCountrol(unsigned short mode); //����5��ģʽLED��
void SetMotorLevel(int cmd);
void MotorCountrol(unsigned char level); //�����ƺ���
void FanTest(char * fan_test);
void auto_mode(unsigned char *level); //�Զ�����ģʽ
void AirLEDControl(void);	//���ƿ���������
