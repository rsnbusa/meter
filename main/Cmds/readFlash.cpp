/*
 * readFlash.cpp
 *
 *  Created on: Apr 16, 2017
 *      Author: RSN
 */

#include "readFlash.h"

string makeDateString(time_t t)
{
	char local[40];
	struct tm timeinfo;
	if (t==0)
		time(&t);
	localtime_r(&t, &timeinfo);
	sprintf(local,"%02d/%02d/%02d %02d:%02d:%02d",timeinfo.tm_mday,timeinfo.tm_mon+1,1900+timeinfo.tm_year,timeinfo.tm_hour,
			timeinfo.tm_min,timeinfo.tm_sec);
	return string(local);
}


void print_date_time(string que,time_t t)
{
	struct tm timeinfo;

	localtime_r(&t, &timeinfo);
	printf("[%s] %02d/%02d/%02d %02d:%02d:%02d\n",que.c_str(),timeinfo.tm_mday,timeinfo.tm_mon,1900+timeinfo.tm_year,timeinfo.tm_hour,
			timeinfo.tm_min,timeinfo.tm_sec);
}

void show_config(u8 meter, bool full) // read flash and if HOW display Status message for terminal
{
	char textl[100];
	time_t now = 0;
	unsigned long ll=0;

		time(&now);
		print_date_time(string("Flash Read Meter"),now );
		if(full)
		{
			printf ("Last Compile %s-%s\n",__DATE__,__TIME__);
			//	printf("Temp:%.02fC\n",DS_get_temp());

			if(aqui.centinel==CENTINEL)
				printf("Valid Centinel. ");
			printf("RTC:%s SNTP:%s\n",rtcf?"Y":"N",timef?"Y":"N");
			u32 diffd=now-aqui.lastTime;
			u16 horas=diffd/3600;
			u16 min=(diffd-(horas*3600))/60;
			u16 secs=diffd-(horas*3600)-(min*60);
			printf("[Last Boot: %s] [Elapsed %02d:%02d:%02d] [Previous Boot %s] [Count:%d ResetCode:0x%02x]\n",makeDateString(aqui.lastTime).c_str(),horas,min,secs,
					makeDateString(aqui.preLastTime).c_str(),aqui.bootcount,aqui.lastResetCode);
			for(int a=0;a<5;a++)
						if(aqui.ssid[a][0]!=0)
							printf("[SSID[%d]:%s-%s %s\n",a,aqui.ssid[a],aqui.pass[a],curSSID==a ?"*":" ");
			printf( "[IP:" IPSTR "] ", IP2STR(&localIp));

			u8 mac[6];
			esp_wifi_get_mac(WIFI_IF_STA, mac);
			sprintf(textl,"[MAC %2x%2x] ",mac[4],mac[5]);
			string mmac=string(textl);
			printf("%s",mmac.c_str());
			mmac="";
			printf("[AP Name:%s]\n",AP_NameString.c_str());
			printf("Meter Name:%s\n",aqui.meterName);
			printf("MQTT Server:[%s:%d] Client: %s Connected:%s User:[%s] Passw:[%s] SSL %s \n",aqui.mqtt,aqui.mqttport,settings.client_id,mqttf?"Yes":"No",
							aqui.mqttUser,aqui.mqttPass,aqui.ssl?"Yes":"No");
			printf("Cmd Queue:%s [%d]\n",cmdTopic.c_str(),aqui.cmdsIn);
			printf("Answer Queue:%s [%d]\n",spublishTopic.c_str(),aqui.responses);
			printf("Billing Queue:%s\n",sCollectionTopic.c_str());
			printf("Update Server:%s\n",aqui.domain);
			nameStr=string(APP)+".bin";
			printf("[Version OTA-Updater %s] ",aqui.actualVersion);
			printf("[Firmware %s @ %s]\n",nameStr.c_str(),makeDateString(aqui.lastUpload).c_str());
			if (aqui.ecount>0)
			{
				printf("Emails %d\n",aqui.ecount);
				for (int a=0;a<aqui.ecount;a++)
					if(a<MAXEMAILS) //Guard corruption
						printf("%s @ %s {%s}\n",aqui.emailName[a],aqui.email[a],aqui.except[a]?"EXCEPTION":"ALWAYS");
			}
			//          printf("Accepted Ids %d\n",aqui.ucount);
			//       print_log();
		}

		printf("[DispMgrTimer %d Resolution %d Beat@KwH %d]\n",aqui.DISPTIME,aqui.MODDISPLAY[meter],aqui.beatsPerKw[meter]);
		printf("[Amps: Min %d (%s) Max %d (%s)]\n",minbeatTime[meter],makeDateString(minTime[meter]).c_str(),maxbeatTime[meter],makeDateString(maxTime[meter]).c_str());
		time_t t;
		float pago;

		if(xSemaphoreTake(framSem, 1000))
		{
			fram.read_beat(meter,(u8*)&ll);
			fram.read_pago(meter,(u8*)&pago);
			fram.read_corte(meter,(u8*)&t);
			xSemaphoreGive(framSem);
			printf("Medidor Id[%d]=%s Born:%s Start %d Corte %d[%s]\nLifeBeat[%lu]-[%d] last %s\n",meter+1,aqui.medidor_id[meter],
					makeDateString(aqui.bornDate[meter]).c_str(),aqui.bornKwh[meter],aqui.diaDeCorte[meter],aqui.corteSent[meter]?"t":"f",
							ll,theMeters[meter].curBeat,makeDateString(theMeters[meter].lastBeatDate).c_str());
			printf("LifeKWH[%d] Month[%d]=%d Day[%d]=%d Hour[%d]=%d\n",curLife[meter],mesg,curMonth[meter],diag,curDay[meter],horag,curHour[meter]);
		}
		printf("Pago $%.02f @ %s\n",pago,makeDateString(t).c_str());
		printf("Working dates: Year %d Mes %d Day %d Hora %d day %d\n",yearg,mesg,diag,horag,daysg);
}






