
/*
 * set_statusSend.cpp

 *
 *  Created on: Apr 16, 2017
 *      Author: RSN
 */
using namespace std;
#include "framCmds.h"
#include "framSPI.h"
#include "framDef.h"

extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);
extern string makeDateString(time_t t);


typedef enum {	GETMONTH,GETDAY,GETHOUR,GETMONTHALL,GETDAYALL,GETDAYSINMONTH,GETHOURSINDAY,GETHOURSINMONTH,GETCYCLE,GETCYCLEDATE} framCmdType;

bool get_stats_params(arg *argument,uint8_t* cualMeter,u8* cualMonth,u16*cualDay, u8* cualHora)
{
	string state;

	state=getParameter(argument,"meter");
	*cualMeter=atoi(state.c_str());
	if(*cualMeter>MAXDEVS-1)
	{
		state="";
		return false;
	}

	state=getParameter(argument,"month");
	if(state!="")
	{
		*cualMonth=atoi(state.c_str());
		if(*cualMonth<1 || *cualMonth>12)
		{
			state="";
			return false;
		}
	}
	state=getParameter(argument,"day");
	if(state!="")
	{
		*cualDay=atoi(state.c_str());
		if(*cualDay>31)
		{
			state="";
			return false;
		}
	}
	state=getParameter(argument,"hour");
	if(state!="")
	{
		*cualHora=atoi(state.c_str());
		if(*cualHora>23)
		{
			state="";
			return false;
		}
	}
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<FRAMD))
		printf("Meter %d Year %d Month %d Day %d Hour %d\n",*cualMeter,yearg,*cualMonth,*cualDay,*cualHora);
#endif

	state="";
	return true;
}

void fram_command(arg * argument,framCmdType cual)
{
	char textl[60];
	string algo,state;
	u8 cualMeter,cualMonth,cualHora,val;
	u16 valor,total,cualDay,ret;
	u32 valor32;

	if(!set_commonCmd(argument,false))
		return;

	if(!get_stats_params(argument,&cualMeter,&cualMonth,&cualDay,&cualHora))
	{
		algo="Bad parameters";
		sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);
		return;
	}

	switch(cual)
	{
	case GETMONTH:
		if(xSemaphoreTake(framSem, portMAX_DELAY))
		{
			fram.read_month(cualMeter, cualMonth, (u8*)&valor);
			xSemaphoreGive(framSem);
			sprintf(textl,"%d!",valor);
			algo=string(textl);
		}
		break;
	case GETDAY:
		if(xSemaphoreTake(framSem, portMAX_DELAY))		{
			fram.read_day(cualMeter, yearg,cualMonth, cualDay, (u8*)&valor);
			xSemaphoreGive(framSem);
			sprintf(textl,"%d!",valor);
			algo=string(textl);
		}
		break;
	case GETHOUR:
		if(xSemaphoreTake(framSem, portMAX_DELAY))		{
			fram.read_hour(cualMeter, yearg,cualMonth, cualDay, cualHora, (u8*)&valor);
			xSemaphoreGive(framSem);
			val=(u8)valor;
			sprintf(textl,"%d!",val);
			algo=string(textl);
		}
		break;
	case GETMONTHALL:
		total=0;
		if(xSemaphoreTake(framSem, portMAX_DELAY))		{
			for (int a=0;a<12;a++)
			{
				fram.read_month(cualMeter, a, (u8*)&valor);
				sprintf(textl,"%d!",valor);
				algo+=string(textl);
				total+=valor;
			}
			xSemaphoreGive(framSem);
			sprintf(textl,"%d!",total);
			algo+=string(textl);
		}
		break;
	case GETDAYALL:
		total=0;
		algo="";
		if(xSemaphoreTake(framSem, portMAX_DELAY))
		{
			for (int a=0;a<12;a++)
			{
				for(int b=0;b<daysInMonth[a];b++)
				{
					fram.read_day(cualMeter,yearg, a, b,(u8*) &valor);
					sprintf(textl,"%d!",valor);
					algo+=string(textl);
					total+=valor;
				}
			}
				xSemaphoreGive(framSem);
		//		return;
			sprintf(textl,"%d!",total);
			algo+=string(textl);
		}
		break;
	case GETDAYSINMONTH:
		algo="";
		total=0;
		if(xSemaphoreTake(framSem, portMAX_DELAY))		{
			for (int a=0;a<daysInMonth[cualMonth];a++)
			{
				fram.read_day(cualMeter,yearg,cualMonth, a, (u8*)&valor);
				sprintf(textl,"%d!",valor);
				algo+=string(textl);
				total+=valor;
			}
			xSemaphoreGive(framSem);
			sprintf(textl,"%d!",total);
			algo+=string(textl);
		}
		break;
	case GETHOURSINDAY:
		algo="";
		total=0;
		if(xSemaphoreTake(framSem, portMAX_DELAY))		{
			for (int a=0;a<24;a++)
			{
				fram.read_hour(cualMeter, yearg,cualMonth, cualDay, a, (u8*)&valor);
				sprintf(textl,"%d!",valor);
				algo+=string(textl);
				total+=valor;
			}
			xSemaphoreGive(framSem);
			sprintf(textl,"%d!",total);
			algo+=string(textl);
		}
		break;
	case GETHOURSINMONTH:
		algo="";
		total=0;
		if(xSemaphoreTake(framSem, portMAX_DELAY))		{
			for(int b=0;b<daysInMonth[cualMonth];b++)
			{
				for (int a=0;a<24;a++)
				{
					fram.read_hour(cualMeter, yearg,cualMonth, b, a, (u8*)&valor);
					sprintf(textl,"%d!",valor);
					algo+=string(textl);
					total+=valor;
				}
			}
			xSemaphoreGive(framSem);
			sprintf(textl,"%d!",total);
			algo+=string(textl);
		}
		break;
	case GETCYCLE:
		if(xSemaphoreTake(framSem, portMAX_DELAY))		{
			fram.read_cycle(cualMeter, cualMonth, (u8*)&valor);
			xSemaphoreGive(framSem);
			sprintf(textl,"%d!",valor);
			algo=string(textl);
		}
		break;
	case GETCYCLEDATE:
		if(xSemaphoreTake(framSem, portMAX_DELAY))		{
			fram.read_cycledate(cualMeter, cualMonth, (u8*)&valor32);
			xSemaphoreGive(framSem);
			sprintf(textl,"%s",makeDateString(valor32).c_str());
			algo=string(textl);
		}
		break;
	default:
		break;
	}
	sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);            // send to someones browser when asked
	algo="";
	state="";
}

void set_getMonth(void * pArg){
	arg *argument=(arg*)pArg;

	fram_command(argument,GETMONTH);         // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<FRAMD))
		printf("getMonth\n");                  // A general status condition for display. See routine for numbers.
#endif
//	free(pArg);
//	vTaskDelete(NULL);
}

void set_getDay(void * pArg){
	arg *argument=(arg*)pArg;

	fram_command(argument,GETDAY);         // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<FRAMD))
		printf("getDay\n");                  // A general status condition for display. See routine for numbers.
#endif
//	free(pArg);
//	vTaskDelete(NULL);
}

void set_getHour(void * pArg){
	arg *argument=(arg*)pArg;

	fram_command(argument,GETHOUR);         // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<FRAMD))
		printf("getHour\n");                  // A general status condition for display. See routine for numbers.
#endif
//	free(pArg);
//	vTaskDelete(NULL);
}

void set_getMonthAll(void * pArg){
	arg *argument=(arg*)pArg;

	fram_command(argument,GETMONTHALL);         // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<FRAMD))
		printf("getMonthAll\n");                  // A general status condition for display. See routine for numbers.
#endif
//	free(pArg);
//	vTaskDelete(NULL);
}

void set_getDayAll(void * pArg){
	arg *argument=(arg*)pArg;

	fram_command(argument,GETDAYALL);         // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<FRAMD))
		printf("getDayAll\n");                  // A general status condition for display. See routine for numbers.
#endif
//	free(pArg);
//	vTaskDelete(NULL);
}

void set_getDaysInMonth(void * pArg){
	arg *argument=(arg*)pArg;

	fram_command(argument,GETDAYSINMONTH);         // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<FRAMD))
		printf("getDaysInMonth\n");                  // A general status condition for display. See routine for numbers.
#endif
//	free(pArg);
//	vTaskDelete(NULL);
}

void set_getHoursInDay(void * pArg){
	arg *argument=(arg*)pArg;

	fram_command(argument,GETHOURSINDAY);         // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<FRAMD))
		printf("getDayAll\n");                  // A general status condition for display. See routine for numbers.
#endif
//	free(pArg);
//	vTaskDelete(NULL);
}

void set_getHoursInMonth(void * pArg){
	arg *argument=(arg*)pArg;

	fram_command(argument,GETHOURSINMONTH);         // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<FRAMD))
		printf("getHoursInMonth\n");                  // A general status condition for display. See routine for numbers.
#endif
//	free(pArg);
//	vTaskDelete(NULL);
}

void set_getCycle(void * pArg){
	arg *argument=(arg*)pArg;

	fram_command(argument,GETCYCLE);         // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<FRAMD))
		printf("getCyclel\n");                  // A general status condition for display. See routine for numbers.
#endif
//	free(pArg);
//	vTaskDelete(NULL);
}
void set_getCycleDate(void * pArg){
	arg *argument=(arg*)pArg;

	fram_command(argument,GETCYCLEDATE);         // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<FRAMD))
		printf("getCycleDate\n");                  // A general status condition for display. See routine for numbers.
#endif
//	free(pArg);
//	vTaskDelete(NULL);
}

