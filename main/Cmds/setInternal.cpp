
/*
 * set_statusSend.cpp

 *
 *  Created on: Apr 16, 2017
 *      Author: RSN
 */
using namespace std;
#include "setInternal.h"

extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);
extern void write_to_flash();
extern void delay(uint16_t a);
extern void postLog(int code, int code1, string mensaje);

void set_internal(void * pArg){
	arg *argument=(arg*)pArg;
	string webstring,state;
	int meter;
	bool res=false;

	printf("Internal\n");
	webstring="";

	if(!set_commonCmd(argument,false))
		return;


	state=getParameter(argument,"password");
	if(state!="zipo")
	{
		state="Not authorized ";
		sendResponse( argument->pComm,argument->typeMsg, state,state.length(),NOERROR,false,false);            // send to someones browser when asked
		goto exit;
	}

	// Set SSID and Password
	state=getParameter(argument,"ssss"); //this command can will end call. If more need do not use this option
	if(state!="")
	{
		webstring+="Internal SSID/Pass-";
		memcpy((void*)&aqui.ssid[0][0],(void*)state.c_str(),state.length()+1);
		res=true;

		state=getParameter(argument,"pppp");
		if(state!="")
		{
			memcpy(&aqui.pass[0][0],state.c_str(),state.length()+1);
			postLog(WIFIL,0,"SSID/PASS set");
		}
		else
			webstring+="Missing Password ";
		goto sale;
	}

	// Set MQTT Server and Port
	state=getParameter(argument,"qqqq");
	if(state!="")
	{
		memcpy(&aqui.mqtt,state.c_str(),state.length()+1);
		res=true;
		state=getParameter(argument,"port");
		if(state!="")
			aqui.mqttport=atoi(state.c_str());
		else
			aqui.mqttport=1883; //default
		state=getParameter(argument,"ssl");
		if(state!="")
			aqui.ssl=atoi(state.c_str());
		webstring+="Internal MQtt-";
	}

	// Set MQTT User and Password
	state=getParameter(argument,"uupp");
	if(state!="")
	{
		memcpy(&aqui.mqttUser,state.c_str(),state.length()+1);
		state=getParameter(argument,"passq");
		if(state!="")
		{
			memcpy(&aqui.mqttPass,state.c_str(),state.length()+1);
			res=true; //restart
		}
		webstring+="Internal MQttUser-";
	}

	// Set Meter Name
	state=getParameter(argument,"nnnn");
	if(state!="")
	{
		memcpy(&aqui.meterName,state.c_str(),state.length()+1);
		memcpy(&aqui.groupName,state.c_str(),state.length()+1);
		webstring+="Internal Name-";
	}

	// Set Meter Internals
	state=getParameter(argument,"meter");
	if(state!="")
	{
		meter=atoi(state.c_str());
		if (meter>MAXDEVS-1)
		{
			webstring="Meter Out Of Range";
			goto sale;
		}

		state=getParameter(argument,"xxxx");
		if(state!="")
		{
			minTime[meter]=maxTime[meter]=maxbeatTime[meter]=0;
			minbeatTime[meter]=99999;
			webstring+="Internal XXXX-";

		}

		state=getParameter(argument,"beats");
		if(state!="")
		{
			aqui.beatsPerKw[meter]=atoi(state.c_str());
			webstring+="Beats-";
		}


		state=getParameter(argument,"mmmm");
		if(state!="")
		{
			memcpy(&aqui.medidor_id[meter][0],state.c_str(),state.length()+1);
			time(&aqui.bornDate[meter]);
			state=getParameter(argument,"born");
			if(state!="")
			{
				aqui.bornKwh[meter]=atoi(state.c_str());//In KWH
				printf("Format Meter...");
				xSemaphoreTake(framSem, portMAX_DELAY);//portMAX_DELAY
		//		fram.formatMeter(meter,buffer,2000);
				fram.formatMeter(meter,ota_write_data,2000);
				xSemaphoreGive(framSem);

				printf("done\n");
				//reset general counters

				theMeters[meter].curLife=aqui.bornKwh[meter];
				theMeters[meter].curMonth=0;
				theMeters[meter].curDay=0;
				theMeters[meter].curHour=0;
				theMeters[meter].beatSave=0;
				theMeters[meter].maxamps=0;
				theMeters[meter].lastKwHDate=0;
				theMeters[meter].minamps=9999;
				theMeters[meter].currentBeat=0;
				theMeters[meter].oldbeat=theMeters[meter].currentBeat;
				xSemaphoreTake(framSem, portMAX_DELAY);//portMAX_DELAY
				fram.write_minamps(meter,theMeters[meter].minamps);
				fram.write_beat(meter,theMeters[meter].currentBeat);
				fram.write_lifekwh(meter,aqui.bornKwh[meter
													  ]);
				xSemaphoreGive(framSem);

			}
			webstring+="Meter Born-";

		}

		state=getParameter(argument,"dddd");
		if(state!="")
		{
			aqui.diaDeCorte[meter]=atoi(state.c_str());
			aqui.corteSent[meter]=false;
			webstring+="Internal Corte Day "+state+"-";
		}

		state=getParameter(argument,"bbbb");
		if(state!="")
		{
			aqui.beatsPerKw[meter]=atoi(state.c_str());
			if(aqui.beatsPerKw[meter]==0)
				aqui.beatsPerKw[meter]=800;
			webstring+="Internal Beats-";
		}

		state=getParameter(argument,"onoff");
		if(state!="")
		{
			aqui.breakers[meter]=atoi(state.c_str());
			webstring+="Breaker Set-";
		}
	}

	state=getParameter(argument,"zzzz");
	if(state=="Y")
	{
		fram.format(0,ota_write_data,2000);
		webstring+="Internal Format Fram";
	}

	write_to_flash();
#ifdef DEBUGMQTT
	if(aqui.traceflag & (1<<CMDD))
		PRINT_MSG("Internal setup\n");
#endif
	postLog(WIFIL,0,webstring);

	sale:
#ifdef DEBUGMQTT
	if(aqui.traceflag & (1<<CMDD))
		printf("Internal\n");
#endif

	sendResponse( argument->pComm,argument->typeMsg, webstring,webstring.length(),NOERROR,false,false);            // send to someones browser when asked
	if(res)
	{
		delay(2000);
		esp_restart();
	}

	exit:
	webstring=state="";
//	free(pArg);
//	vTaskDelete(NULL);
}



