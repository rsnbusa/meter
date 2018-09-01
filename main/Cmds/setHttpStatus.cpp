using namespace std;
#include "setHttpStatus.h"

extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);
extern string makeDateString(time_t t);


void set_HttpStatus(void * pArg){
	char textl[300];
	arg *argument=(arg*)pArg;
	time_t t;
	struct tm tm;
	tcpip_adapter_ip_info_t ip_info;
	u8 meter;
	string state,logStr,finalStr,hourStr,totalStr;
	u8 mac[6];

	time(&t);
	localtime_r(&t, &tm);

	if(!set_commonCmd(argument,false))
		return;


	state=getParameter(argument,"password");
	if(state!="zipo")
	{
		state="Not authorized";
		sendResponse( argument->pComm,argument->typeMsg, state,state.length(),NOERROR,false,false);            // send to someones browser when asked
		goto exit;
	}

	state=getParameter(argument,"meter");
	meter=atoi(state.c_str());
	if(meter>MAXDEVS-1)
	{
		sprintf(textl,"Meter out of range %d[%d]",meter,MAXDEVS-1);
		totalStr=string(textl);
		goto sale;
	}

	sprintf(textl,"<!DOCTYPE html><html><head><title>%s Status</title></head><style>table, th,td {border: 1px solid black; border-collapse: collapse;} th,td {padding: 5px; text-align:center;}</style><body><h1",aqui.meterName);
	state=string(textl);
	sprintf(textl," style=\"color:red;\">%s Status</h1><p>Date:%d-%d-%d %02d:%02d:%02d<br>",aqui.meterName,tm.tm_mday,tm.tm_mon,tm.tm_year,
			tm.tm_hour,tm.tm_min,tm.tm_sec);
	state+=string(textl);
	sprintf(textl,"<img style=\"width: 225px; height: 225px;\" alt=\"Ejemplo\" src=\"http://feediot.co.nf/meter.txt\"><br>");
	state+=string(textl);

	if (string(aqui.meterName)!="")
	{
		sprintf(textl," <span style=\"color:blue;\">RTC:%s SNTP:%s<br>",rtcf?"Y":"N",timef?"Y":"N");
		state+=string(textl);
		sprintf(textl,"Last Boot: %s Count:%d ResetCode:0x%02x<br>",makeDateString(aqui.lastTime).c_str(),aqui.bootcount,aqui.lastResetCode);
		state+=string(textl);
		sprintf(textl,"SSID:%s<br>",aqui.ssid[0]);
		state+=string(textl);

		if (aqui.ssid[0][0]!=0)
			tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
		else
			tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info);
		esp_wifi_get_mac(WIFI_IF_STA, mac);
		sprintf(textl,"IP: %d.%d.%d.%d Gateway:%d.%d.%d.%d MAC:%2x%2x<br>",IP2STR(&ip_info.ip),IP2STR(&ip_info.gw),mac[4],mac[5]);
		state+=string(textl);
		sprintf(textl,"Firmware Update Server:%s<br>",aqui.domain);
		state+=string(textl);
		sprintf(textl,"MQTT Server:%s Connected:%s Cmd:%s User:%s<br>",aqui.mqtt,mqttf?"Y":"N",cmdTopic.c_str(),aqui.mqttUser);
		state+=string(textl);

		nameStr=string(APP)+".bin";

		sprintf(textl,"Version OTA-Updater %s<br>",aqui.actualVersion);
		state+=string(textl);

		sprintf(textl,"Firmware %s - %s %s</br>",nameStr.c_str(),__DATE__,__TIME__);
		state+=string(textl);

		sprintf(textl,"Last Firmware Update %s<br>",makeDateString(aqui.lastUpload).c_str());
		state+=string(textl);



		sprintf(textl,"Email %s address %s {%s}<br>",aqui.emailName[0],aqui.email[0],aqui.except[0]?"EXCEPTION":"ALWAYS");
		state+=string(textl);

		sprintf(textl,"Current temperature %.02fC<br>",DS_get_temp(NULL));
		finalStr=string(textl);
		uint32_t ll;
		float pago;
		if(xSemaphoreTake(framSem, 1000))
		{
			fram.read_beat(meter,(u8*)&ll);
			fram.read_pago(meter,(u8*)&pago);
			time_t t;
			fram.read_corte(meter,(u8*)&t);
			xSemaphoreGive(framSem);
		}
		sprintf(textl,"Meter[%d]=%s<br>",meter,aqui.medidor_id[meter]);
		finalStr+=string(textl);
		sprintf(textl,"LifetBeat[%d]-[%d] last %s<br>",ll,theMeters[meter].currentBeat,makeDateString(theMeters[meter].lastKwHDate).c_str());
		finalStr+=string(textl);
		sprintf(textl,"LifeKWH[%d] Month[%d]=%d Day[%d]=%d Hour[%d]=%d Beats<br>",theMeters[meter].curLife,mesg,theMeters[meter].curMonth,
				diag,theMeters[meter].curDay,horag,theMeters[meter].curHour);
		finalStr+=string(textl);
		sprintf(textl,"MinTime %d (%s) MaxTime %d (%s) Corte %d<br>", minbeatTime[meter],makeDateString(minTime[meter]).c_str(), maxbeatTime[meter], makeDateString(maxTime[meter]).c_str()
				,aqui.diaDeCorte[meter]);
		finalStr+=string(textl);
		sprintf(textl,"Pago $%.02f @ %s<br>",pago,makeDateString(t).c_str());
		finalStr+=string(textl);

		int largo=state.length()+logStr.length()+finalStr.length()+hourStr.length();
		if(largo<MAXHTTP+50)
			totalStr=state+logStr+hourStr+finalStr;
		else
		{
			if ((largo-hourStr.length())<MAXHTTP+50)
				totalStr=state+logStr+finalStr;
			else
				totalStr=state+finalStr;
		}
	}
	else
		totalStr="System Not Configured yet";

	sale:

#ifdef DEBUGMQTT
	if(aqui.traceflag & (1<<CMDD))
		PRINT_MSG("FinalSize %d\n",totalStr.length());
#endif
	sendResponse( argument->pComm,argument->typeMsg, totalStr,totalStr.length(),NOERROR,true,false);            // send to someones browser when asked
#ifdef DEBUGMQTT
	if(aqui.traceflag & (1<<CMDD))
		printf("HttpStatus\n");                  // A general status condition for display. See routine for numbers.
#endif
	exit:
	state=logStr=finalStr=totalStr="";

//	free(pArg);
//	vTaskDelete(NULL);
}



