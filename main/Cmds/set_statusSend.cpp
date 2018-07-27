
/*
 * set_statusSend.cpp

 *
 *  Created on: Apr 16, 2017
 *      Author: RSN
 */
using namespace std;
#include "set_statusSend.h"

extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);

void set_statusSend(void * pArg){
	char textl[100];
	arg *argument=(arg*)pArg;

	string webString;
	char  str_temp[10];

	time_t now = 0;
	struct tm timeptr ;
	u8 cualmeter;
	if(!set_commonCmd(argument,false))
		return;
	time(&now);
	localtime_r(&now, &timeptr);

	string algo=getParameter(argument,"meter");
	cualmeter=atoi(algo.c_str());
	float mxpow=4500.0;// /float(msPower[cualmeter])*8.33;
	char *mensaje=asctime(&timeptr);
	int son=strlen(mensaje);
	mensaje[son-1]=0;
	sprintf(textl,"%d!%d!%d!%d!%d!%d!%d!%s!%s!%d!%d!%d!%s!%d!%d!%d!", currentBeat[cualmeter],curLife[cualmeter],curMonth[cualmeter],curDay[cualmeter],curHour[cualmeter],
			curCycle[cualmeter],dia24h[horag],mensaje,aqui.medidor_id[cualmeter],aqui.breakers[cualmeter],(int)mxpow
			,msPower[cualmeter]," ",msNow[cualmeter],beatSave[cualmeter],aqui.diaDeCorte[cualmeter]);
	webString=string(textl);
	sendResponse( argument->pComm,argument->typeMsg, webString,webString.length(),NOERROR,false,false);            // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<PUBSUBD))
		printf("statussend %s\n",textl);                  // A general status condition for display. See routine for numbers.
#endif
	algo="";
	webString="";
	//free(pArg);
	//vTaskDelete(NULL);
}



