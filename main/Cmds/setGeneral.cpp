
/*
 * setGeneral.cpp

 *
 *  Created on: Apr 16, 2017
 *      Author: RSN
 */
using namespace std;
#include "setGeneral.h"
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);
extern void delay(uint16_t a);
extern void postLog(int code, int code1, string que);


void set_generalap(void * pArg){
	arg *argument=(arg*)pArg; //set pet name and put name in bonjour list if changed

	string s1,webString;
	set_commonCmd(argument,false);
	s1=getParameter(argument,"meter");
	if (s1 !="")
	{
		memcpy(&aqui.meterName,s1.c_str(),s1.length()+1);
#ifdef DEBUGMQQT
		if(aqui.traceflag & (1<<GEND))
			PRINT_MSG("[GEND]Name %s\n",aqui.meterName);
#endif
		webString="General Info set";
		//    sendResponse( webString);            // send to someones browser when asked
#ifdef DEBUGMQQT
		if(aqui.traceflag & (1<<GEND))
			PRINT_MSG("[GEND]%s\n",webString.c_str());
#endif
		s1=getParameter(argument,"group");
		if (s1 == "" )
			memcpy(&aqui.groupName,&aqui.meterName,sizeof(aqui.meterName));
		else
			memcpy(&aqui.groupName,s1.c_str(),s1.length()+1);
#ifdef DEBUGMQQT
		if(aqui.traceflag & (1<<GEND))
			PRINT_MSG("[GEND]Name %s\n",aqui.groupName);
#endif

//		s1=getParameter(argument,"email");
//		memcpy(&aqui.email[0],s1.c_str(),s1.length()+1);
//		memcpy(&aqui.emailName[0],"Owner",5);
//		aqui.emailName[0][5]=0;
//		aqui.ecount=1;

		s1=getParameter(argument,"ap");

		if (s1=="")
		{// it the update cmd not a AP option
			write_to_flash();
			webString="General Erased SSID";
			printf("invalid ap\n");
			sendResponse( argument->pComm,argument->typeMsg, webString,webString.length(),NOERROR,false,false);            // send to someones browser when asked
		//	free(pArg);
			return;
		}
		memcpy(&aqui.ssid[0],s1.c_str(),s1.length()+1);
#ifdef DEBUGMQQT
		if(aqui.traceflag & (1<<GEND))
			PRINT_MSG("[GEND]Ap %s\n",aqui.ssid[0]);
#endif
		s1=getParameter(argument,"pass");
		memcpy(&aqui.pass[0],s1.c_str(),s1.length()+1);
#ifdef DEBUGMQQT
		if(aqui.traceflag & (1<<GEND))
			PRINT_MSG("[GEND]Pass %s\n",aqui.pass[0]);
		if(aqui.traceflag & (1<<GEND))
				PRINT_MSG("[GEND]Ap %s Pass %s\n",aqui.ssid[0],aqui.pass[0]);
#endif
		s1=getParameter(argument,"epoch");
		if(s1!="") //set the RTC clock
		{
			char *ptr;
			unsigned long  longg=strtol(s1.c_str(),&ptr,10);
			if(xSemaphoreTake(I2CSem, portMAX_DELAY))
				{
					rtc.setEpoch(longg); //minus local time GMT -5 hours =18000
					xSemaphoreGive(I2CSem);
				}
#ifdef DEBUGMQQT
				if(aqui.traceflag & (1<<GEND))
					PRINT_MSG("[GEND]Epoch %lu %s string [%s]\n",longg,s1.c_str(),ptr);
#endif
		}
		write_to_flash();
		sendResponse( argument->pComm,argument->typeMsg, webString,webString.length(),NOERROR,false,false);            // send to someones browser when asked
		postLog(APIL,0,"AP Set");
		int son=10;
		while (son--)
			mg_mgr_poll(&mgr, 10);
		delay(3000);
		esp_restart();

	}
	s1=webString="";
//	free(pArg);
//	vTaskDelete(NULL);
}





