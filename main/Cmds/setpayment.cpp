
/*
 * set_statusSend.cpp

 *
 *  Created on: Apr 16, 2017
 *      Author: RSN
 */
using namespace std;
#include "setpayment.h"

extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);
extern string makeDateString(time_t t);
extern void postLog(int code,int code1,string mensaje);

void set_payment(void * pArg){
	char textl[100];
	arg *argument=(arg*)pArg;
	string algo,state;
	int cualmeter,dia,mes,year,desde=0,donde=0;

	float pago=0.0;
	struct tm tml;
	time_t t;

	if(!set_commonCmd(argument,false))
		return;

	algo=getParameter(argument,"password");
	if(algo!="zipo")
	{
		algo="Not authorized";
		sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);            // send to someones browser when asked
		goto exit;
	}

	state=getParameter(argument,"meter");
	if(state!="")
	{
		cualmeter=atoi(state.c_str());
		if(cualmeter>MAXDEVS)
		{
			sprintf(textl,"Meter %d Out of Range[%d]",cualmeter,MAXDEVS-1);
			goto sale;
		}

		state=getParameter(argument,"pay");
		if(state!="")
		{
			pago=atof(state.c_str());

			state=getParameter(argument,"day");
			if(state!="")
			{
				sprintf(textl,"Wrong Date format %s (dd/mm/yyyy)",state.c_str());
				donde=state.find('/',desde);
				if(donde<0)
				{
					donde=state.find('-',desde);
					if(donde<0)
						goto sale;
				}
				dia=atoi(state.substr(desde,donde).c_str());
				desde=donde+1;
				donde=state.find('/',desde);
				if(donde<0)
				{
					donde=state.find('-',desde);
					if(donde<0)
						goto sale;
				}
				mes=atoi(state.substr(desde,donde).c_str());
				desde=donde+1;
				year=atoi(state.substr(desde).c_str());
				tml.tm_mday=dia;
				tml.tm_mon=mes-1;
				tml.tm_year=year-1900;
				tml.tm_hour=tml.tm_min=tml.tm_sec=0;
				t=mktime(&tml);
			}
			else
				time(&t); //Use now as date. Very stupid
			xSemaphoreTake(framSem, portMAX_DELAY);//portMAX_DELAY
			fram.write_corte(cualmeter, (u32)t);
			fram.write_pago(cualmeter, pago);
			xSemaphoreGive(framSem);
			displayMode=DISPLAYUSER;

		}
		sprintf(textl,"Meter %d pago $%.02f @ %s",cualmeter,pago,makeDateString(t).c_str());
	}
	else
		sprintf(textl,"No meter given");
	postLog(PAYL,0,string(textl));

	sale:    	    algo=string(textl);

	sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);            // send to someones browser when asked
#ifdef DEBUGMQTT
	if(aqui.traceflag & (1<<CMDD))
		printf("Payment\n");                  // A general status condition for display. See routine for numbers.
#endif
	exit:
	algo=state="";
//	free(pArg);
//	vTaskDelete(NULL);
}



