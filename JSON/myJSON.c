#include "myJSON.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cJSON.h"
#include "key.h"
#include "sim800c.h"
#include "stmflash.h"
#include "uart.h"
#include "rtc.h"
//#include "DSHCHO.h"

void SendDataMode_group(cJSON *root);
void ConnectionMode_group(cJSON *root);
void NoPress_Connection_group(cJSON *root);
void PressMode_group(cJSON *root);
void GprsRecordMode_group(cJSON *root);

//���ⲿ���õ��ڲ���
extern volatile int Conce_PM2_5;       // PM2.5Ũ��
extern volatile int Conce_PM10;        // PM10Ũ��
extern volatile int AQI_Max;
extern char http_buf[1];
extern unsigned int current_interval;
extern char mqtt_mode[2];
extern volatile unsigned char fan_level;
extern char payload[MQTT_SEND_SIZE];

//int CSQ[CLOSE_INTERVAL / COUNT_INTERVAL] = {0};
//int HCHO[CLOSE_INTERVAL / COUNT_INTERVAL] = {0};
int C1[CLOSE_INTERVAL / COUNT_INTERVAL] = {0};
int C2[CLOSE_INTERVAL / COUNT_INTERVAL] = {0};
int AQI1[CLOSE_INTERVAL / COUNT_INTERVAL] = {0};
int AQI2[CLOSE_INTERVAL / COUNT_INTERVAL] = {0};
int AQI[CLOSE_INTERVAL / COUNT_INTERVAL] = {0};
int L[CLOSE_INTERVAL / COUNT_INTERVAL] = {0};

void SendDataMode_group(cJSON *root)
{
//  int i;
  int trans_size;
  cJSON *aqi = NULL;
//  cJSON *j_csq = NULL;
//  cJSON *j_c1 = NULL;
//  cJSON *j_c2 = NULL;
//  cJSON *j_aqi1 = NULL;
//  cJSON *j_aqi2 = NULL;
//  cJSON *j_aqi = NULL;
//  cJSON *j_l = NULL;

  trans_size = current_interval / COUNT_INTERVAL;

  cJSON_AddItemToObject(root, "aqi", aqi = cJSON_CreateObject());
//		cJSON_AddItemToObject(root, "CSQ", cJSON_CreateIntArray((const int *)CSQ, trans_size));
  cJSON_AddStringToObject(aqi, "mode", (const char *)mqtt_mode);

//  cJSON_AddItemToObject(root, "CSQ", j_csq = cJSON_CreateArray());
//  cJSON_AddItemToObject(aqi, "C1", j_c1 = cJSON_CreateArray());
//  cJSON_AddItemToObject(aqi, "C2", j_c2 = cJSON_CreateArray());
//  cJSON_AddItemToObject(aqi, "AQI1", j_aqi1 = cJSON_CreateArray());
//  cJSON_AddItemToObject(aqi, "AQI2", j_aqi2 = cJSON_CreateArray());
//  cJSON_AddItemToObject(aqi, "AQI", j_aqi = cJSON_CreateArray());
//  cJSON_AddItemToObject(aqi, "L", j_l = cJSON_CreateArray());
//  for(i = 0; i < trans_size; i++)
//  {
//    cJSON_AddItemToArray(j_csq, cJSON_CreateNumber(CSQ[i]));
//    cJSON_AddItemToArray(j_c1, cJSON_CreateNumber(C1[i]));
//    cJSON_AddItemToArray(j_c2, cJSON_CreateNumber(C2[i]));
//    cJSON_AddItemToArray(j_aqi1, cJSON_CreateNumber(AQI1[i]));
//    cJSON_AddItemToArray(j_aqi2, cJSON_CreateNumber(AQI2[i]));
//    cJSON_AddItemToArray(j_aqi, cJSON_CreateNumber(AQI[i]));
//    cJSON_AddItemToArray(j_l, cJSON_CreateNumber(L[i]));
//  }

	//cJSON_AddItemToObject(root, "CSQ", cJSON_CreateIntArray(CSQ, trans_size));
	//cJSON_AddItemToObject(aqi, "HCHO", cJSON_CreateIntArray(HCHO, trans_size));
	cJSON_AddItemToObject(aqi, "C1", cJSON_CreateIntArray(C1, trans_size));
	cJSON_AddItemToObject(aqi, "C2", cJSON_CreateIntArray(C2, trans_size));
	cJSON_AddItemToObject(aqi, "AQI1", cJSON_CreateIntArray(AQI1, trans_size));
	cJSON_AddItemToObject(aqi, "AQI2", cJSON_CreateIntArray(AQI2, trans_size));
	cJSON_AddItemToObject(aqi, "AQI", cJSON_CreateIntArray(AQI, trans_size));
	cJSON_AddItemToObject(aqi, "L", cJSON_CreateIntArray(L, trans_size));



  if(fan_level == 0) current_interval = CLOSE_INTERVAL;
  else current_interval = OPEN_INTERVAL;

  memset(C1, 0, sizeof(C1));	//ÿ�η�����֮����Ҫ����������
  memset(C2, 0, sizeof(C2));	//ÿ�η�����֮����Ҫ����������
  memset(AQI1, 0, sizeof(AQI1));	//ÿ�η�����֮����Ҫ����������
  memset(AQI2, 0, sizeof(AQI2));	//ÿ�η�����֮����Ҫ����������
  memset(AQI, 0, sizeof(AQI));	//ÿ�η�����֮����Ҫ����������
  memset(L, 0, sizeof(L));	//ÿ�η�����֮����Ҫ����������
}

void ConnectionMode_group(cJSON *root)
{
  cJSON *j_connection = NULL;

  cJSON_AddItemToObject(root, "connection", j_connection = cJSON_CreateObject());

  if(wait_send_press)
  {
    PressMode_group(j_connection);
    wait_send_press = 0;
  }
  else
  {
    NoPress_Connection_group(j_connection);
  }
}

void NoPress_Connection_group(cJSON *root)
{
//	cJSON *j_hcho = NULL;
  cJSON *j_c1 = NULL;
  cJSON *j_c2 = NULL;
  cJSON *j_aqi = NULL;

  cJSON_AddStringToObject(root, "mode", (const char *)mqtt_mode);

	//cJSON_AddItemToObject(root, "HCHO", j_hcho = cJSON_CreateArray());
  cJSON_AddItemToObject(root, "C1", j_c1 = cJSON_CreateArray());
  cJSON_AddItemToObject(root, "C2", j_c2 = cJSON_CreateArray());
  cJSON_AddItemToObject(root, "AQI", j_aqi = cJSON_CreateArray());
	//cJSON_AddItemToArray(j_hcho, cJSON_CreateNumber(Conce_HCHO));
  cJSON_AddItemToArray(j_c1, cJSON_CreateNumber(Conce_PM2_5));
  cJSON_AddItemToArray(j_c2, cJSON_CreateNumber(Conce_PM10));
  cJSON_AddItemToArray(j_aqi, cJSON_CreateNumber(AQI_Max));
}

void PressMode_group(cJSON *root)
{
//  int i;
  cJSON *j_press = NULL;
  cJSON *j_press_time = NULL;
//	cJSON *j_hcho = NULL;
  cJSON *j_c1 = NULL;
  cJSON *j_c2 = NULL;
  cJSON *j_aqi = NULL;

  cJSON_AddItemToObject(root, "press", j_press = cJSON_CreateArray());
  cJSON_AddItemToObject(root, "time", j_press_time = cJSON_CreateArray());
	//cJSON_AddItemToObject(root, "HCHO", j_hcho = cJSON_CreateArray());
  cJSON_AddItemToObject(root, "C1", j_c1 = cJSON_CreateArray());
  cJSON_AddItemToObject(root, "C2", j_c2 = cJSON_CreateArray());
  cJSON_AddItemToObject(root, "AQI", j_aqi = cJSON_CreateArray());

	cJSON_AddItemToArray(j_press, cJSON_CreateString(press_buf));
//			count2date(press_time_log[i]);
//			sprintf(somevalue, "%04d%02d%02d%02d%02d%02d\r\n", calendar_tmp.w_year, calendar_tmp.w_month, calendar_tmp.w_date, calendar_tmp.hour, calendar_tmp.min, calendar_tmp.sec);
//			cJSON_AddItemToArray(press_time, cJSON_CreateString(somevalue));
	cJSON_AddItemToArray(j_press_time, cJSON_CreateNumber(press_time_log));
	//cJSON_AddItemToArray(j_hcho, cJSON_CreateNumber(press_HCHO[i]));
	cJSON_AddItemToArray(j_c1, cJSON_CreateNumber(press_C1));
	cJSON_AddItemToArray(j_c2, cJSON_CreateNumber(press_C2));
	cJSON_AddItemToArray(j_aqi, cJSON_CreateNumber(press_AQI));

//  memset(press_buf, 0, sizeof(press_buf));	//ÿ�η�����֮����Ҫ����������
//  memset(press_time_log, 0, sizeof(press_time_log));	//ÿ�η�����֮����Ҫ����������
//	//memset(press_HCHO, 0, sizeof(press_HCHO));	//ÿ�η�����֮����Ҫ����������
//  memset(press_C1, 0, sizeof(press_C1));	//ÿ�η�����֮����Ҫ����������
//  memset(press_C2, 0, sizeof(press_C2));	//ÿ�η�����֮����Ҫ����������
//  memset(press_AQI, 0, sizeof(press_AQI));	//ÿ�η�����֮����Ҫ����������
//  press_len = 0;
}

void group_json(unsigned char mode)
{
  cJSON *root = NULL;
//  cJSON *http_root = NULL;
  char *out = NULL;
  int JSON_len = 0;

  root = cJSON_CreateObject();

  switch(mode)
  {
    case CONNECTION_MODE:
      ConnectionMode_group(root);
      break;
//    case GEO_MODE:
//      http_root = cJSON_Parse(http_buf);
//      if(http_root != NULL)	cJSON_AddItemReferenceToObject(root, "geo", http_root);
//      break;
    case SENDDATA_MODE:
      SendDataMode_group(root);
      break;
    case PRESS_MODE:
      PressMode_group(root);
      break;
    default:
      printf("group_json 's mode is error!\r\n");
      cJSON_Delete(root);
      return;
  }

  out = cJSON_Print(root);
  JSON_len = strlen(out);
  strcpy(payload, out);

  printf("JSON_len = %d\r\n", JSON_len);
  //printf("%s\r\n", out);

  free(out);
//  if(mode == GEO_MODE) cJSON_Delete(http_root);
  cJSON_Delete(root);
}





