#include "sys.h"

//group_json��ģʽѡ��
#define CONNECTION_MODE 0
#define GEO_MODE 1
#define SENDDATA_MODE 2
#define PRESS_MODE 3

//�������ݵ�ʱ����
#define CLOSE_INTERVAL 600
#define OPEN_INTERVAL 300
#define COUNT_INTERVAL 30
#define PING_SET 120
#define PING_INTERVAL 120
#define AUTOMODE_INTERVAL 5
//#define CLOSE_INTERVAL 120
//#define OPEN_INTERVAL 60
//#define COUNT_INTERVAL 6
//#define PING_SET 120
//#define PING_INTERVAL 24
//#define AUTOMODE_INTERVAL 5

//�������Լ�¼
#define GPRS_STATE_TIME_SIZE 5


//���ڲ����õ��ⲿ��
//extern int CSQ[CLOSE_INTERVAL/COUNT_INTERVAL];
//extern int HCHO[CLOSE_INTERVAL/COUNT_INTERVAL];
extern int C1[CLOSE_INTERVAL/COUNT_INTERVAL];
extern int C2[CLOSE_INTERVAL/COUNT_INTERVAL];
extern int AQI1[CLOSE_INTERVAL/COUNT_INTERVAL];
extern int AQI2[CLOSE_INTERVAL/COUNT_INTERVAL];
extern int AQI[CLOSE_INTERVAL/COUNT_INTERVAL];
extern int L[CLOSE_INTERVAL/COUNT_INTERVAL];

void group_json(unsigned char mode);

