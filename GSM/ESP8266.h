#ifndef __ESP8266_H__
#define __ESP8266_H__

#include "sys.h"

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;

extern volatile unsigned long sys_tick;
extern unsigned char smartconfig_flag;	//��־���Ƿ���Ҫ����smartconfig

#define TRANS_MODE 1 //͸��ģʽ

#define HOST_NAME   "47.92.81.9"
#ifdef SSL_MODE
#define HOST_PORT   1883
#else
#define HOST_PORT   8883
#endif

#define MQTT_RECVBUF_SIZE 256
#define MQTT_SEND_SIZE 1024
#define WIFI_RST_PIN PBout(0)

#define RECV_BUF_SIZE  256//������յĻ��棬�����ܵĴ󣬷�ֹ���
#define TIME_OUT 100

#define AI_LINK 0
#define ESP_TOUCH 1
#define AIR_LINK 2
#define ESP_AIR 3

#define STATUS_GETIP 2 //��ȡ��IP
#define STATUS_GETLINK 3 //��������
#define STATUS_LOSTLINK 4 //ʧȥ����
#define STATUS_LOSTIP 5 //δ��ȡ��IP

//function
int AutoLink(void);
int SmartConfig(void);
int WifiInit(const char *addr, uint32_t port, char *http_data);
void timer1msINT(void);
unsigned long millis(void);
void delay(unsigned int ms);
int SetBaud(uint32_t baud);
int restart(void);
int setOprToStationSoftAP(void);
int smartLink(uint8_t  type,char *link_msg);
int stopSmartLink(void);
int getSystemStatus(void);
int disableMUX(void);
int createTCP(const char *addr, uint32_t port);
int wifi_send(const uint8_t *buffer, uint32_t len);
int wifi_recv(uint8_t *buffer, uint32_t buffer_size, uint32_t timeout);
void rx_empty(void);

int eATRESTORE(void);
void WIFI_restart(void);
int esp8266_send(const uint8_t *buffer, uint32_t len);
int esp8266_recv(uint8_t *buffer, uint32_t buffer_size, uint32_t timeout);
int mqtt_recv(uint8_t *buffer, uint32_t buffer_size, uint32_t timeout);

#endif
