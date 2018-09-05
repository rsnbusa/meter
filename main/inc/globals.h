/*
 * globals.h
 *
 *  Created on: Apr 16, 2017
 *      Author: RSN
 */

#ifndef MAIN_GLOBALS_H_
#define MAIN_GLOBALS_H_

#ifdef GLOBAL
#define EXTERN extern
#else
#define EXTERN
#endif
#include "defines.h"
#include "projTypes.h"

using namespace std;

// =========================
 EXTERN portMUX_TYPE 					myMutex;
 EXTERN char 							APP[9];
 EXTERN char 							WHOAMI[23];
 EXTERN char 							MQTTSERVER[18];
 EXTERN	char                   	 		meses[12][4];
 EXTERN u8                       	  	daysInMonth [12] ;
 EXTERN char 							lookuptable[NKEYS][10];
 EXTERN cmdRecord 						cmds[MAXCMDS];
 EXTERN char						    *compile_date;
// Critical Variables
 EXTERN QueueHandle_t 					mqttQ,isrQ,isrW;
 EXTERN esp_mqtt_client_config_t  		settings;
 EXTERN esp_mqtt_client_handle_t 		clientCloud;
 EXTERN config_flash        			aqui __attribute__((aligned(4))) ;
 EXTERN u8                  			METERS[MAXDEVS];

// Devices and Services
 EXTERN	FramSPI							fram;
 EXTERN	I2C								miI2C;
#ifdef GLOBAL
 EXTERN	SSD1306             			display;
#else
 EXTERN SSD1306 						display(0x3c, &miI2C);
#endif

 EXTERN	i2ctype 						i2cp;
 EXTERN nvs_handle 						nvshandle;

 EXTERN u8                  			diaTarifa[24];      // % of Meter tariff. Ex: 800 *120=(20% cheaper).

 EXTERN displayModeType    				displayMode; //Displaymode when boot first time
 EXTERN struct rst_info   				*rtc_info; //Restart System variable. States Reason for reboot. Look first line for reasons
 EXTERN u8               				curSSID,mesg,diag,horag,oldHorag,oldDiag,oldMesg,lastalign,lastFont,currentMonth,chosenMeter;
 EXTERN u8                				breakerPin[MAXDEVS],barX[3],barH[3],userNum,sonUid;
 EXTERN u16								yearg,daysg,yearDay;
 EXTERN bool                			mqttf,tracef,showf,framf,mqttThingf;
 EXTERN bool                			firstmqtt,verbose,timef,rtcf,logdiscf,displayf;

 EXTERN string              			spublishTopic,cmdTopic,sCollectionTopic,montonUid[5];
 EXTERN string              			AP_NameString,publishString,decirlo,nameStr,uidStr,logText[18];

 EXTERN char                			AP_NameChar[MAXCHARS];
 EXTERN u32                 			framWords,uidLogin[5];

 EXTERN char   							WIFIME[9];//must be 8 chars Password by default of ESP8266 MeterIoT Access Point
 EXTERN char   							eserver[20];
 EXTERN int                				eport ,RSSI,oldRSSI,maxw;

// =========================

 EXTERN char 							TAGG[10];

 EXTERN u16 							producto;
 EXTERN SemaphoreHandle_t 				framSem,I2CSem,logSem;
 EXTERN QueueHandle_t 					uart0_queue;


 EXTERN uint8_t 						recontimes;
 EXTERN uint8_t* 						mpointer;
 EXTERN int 							cual,oldMode,oldMeter;

 EXTERN bool 							timerF,mqttflag,reconf,connf,mongf,mdnsf,timerf,sntpf,subf,loggf,firstTimeTimer;

 EXTERN ip4_addr_t 						localIp;
 EXTERN int 							cuantos;
 EXTERN struct 							timeval tvStart;
 EXTERN cJSON 							*root;
 EXTERN char 							ota_write_data[BUFFSIZE + 1] ;
/*an packet receive buffer*/
 EXTERN char 							text[BUFFSIZE + 1] ;
/* an image total length*/
 EXTERN int 							binary_file_length ;
/*socket id*/
 EXTERN int 							socket_id ;
 EXTERN char 							http_request[100] ;
/* operate handle : uninitialized value is zero ,every ota begin would exponential growth*/
 EXTERN esp_ota_handle_t 				update_handle ;
 EXTERN esp_partition_t 				operate_partition;
 EXTERN argumento 						args[MAXDEVS];
 EXTERN struct mg_mgr 					mgr;
 EXTERN RESET_REASON 					reboot;
 EXTERN int								addHTTP,llevoHTTP;
 EXTERN u32								usertime;
 EXTERN FILE 							*bitacora;
 EXTERN QueueHandle_t 					logQueue;
 EXTERN uint8_t							sensors[2][8],numsensors;
 EXTERN adc1_channel_t 					adcchannel;     //GPIO34 if ADC1, GPIO14 if ADC2
 EXTERN adc_atten_t 					atten;
 EXTERN meterType 						theMeters[MAXDEVS];
 EXTERN TimerHandle_t					hourChange;
#endif /* MAIN_GLOBALS_H_ */
