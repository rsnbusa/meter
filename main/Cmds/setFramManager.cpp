using namespace std;
#include "setFramManager.h"

extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);
extern void postLog(int code, int code1, string que);
extern void load_from_fram(uint8_t a);

void set_framManager(void * pArg){
	char textl[100];
	arg *argument=(arg*)pArg;
	string algo,state;
	int ret,cualmeter;

	if(!set_commonCmd(argument,false))
		return;

	algo=getParameter(argument,"password");
	if(algo!="zipo")
	{
		algo="Not authorized";
		sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);            // send to someones browser when asked
		goto exit;
	}

	state=getParameter(argument,"ALL");// the whole chip
	if(state=="y")
	{
		printf("Format all %s\n",state.c_str());
		xSemaphoreTake(framSem, portMAX_DELAY);//portMAX_DELAY
		ret=fram.format(0,ota_write_data,2000,true);
		xSemaphoreGive(framSem);
		if(ret!=0)
		{
			sprintf(textl,"Format failed %d",ret);
			postLog(FRAMFL,ret,string(textl));
		}
		else
		{
			sprintf(textl,"FRAM Formatted");
			postLog(FRAMFL,0,string(textl));
		//	memset(&currentBeat,0,sizeof(currentBeat)); //counter to zero
			memset(&theMeters,0,sizeof(theMeters)); //counter to zero
			for (int a=0;a<MAXDEVS;a++)
			{
				load_from_fram(a); //why they are in zero everything
//				oldTime[a]=0;
//				minTime[a]=maxTime[a]=maxbeatTime[a]=0;
//				minbeatTime[a]=99999;
//				comofue[a]=0;
//				maxPower[a]=0.0;
//				msPower[a]=99999;
//				theMeters[a].beat=0;
//				theMeters[a].curBeat=0;
//				theMeters[a].meterid=a;
//				theMeters[a].saveit=false;
//				theMeters[a].timestamp=0;
				theMeters[a].minamps=0xffffffff;
			}
		}
		goto sale;
	}


	state=getParameter(argument,"METER");//one Meter data structure to erase
	if(state!="")
	{
		cualmeter=atoi(state.c_str());
		state=getParameter(argument,"full"); //the whole meter
		if(state=="y")
		{
			xSemaphoreTake(framSem, portMAX_DELAY);//portMAX_DELAY
		//	fram.formatMeter(cualmeter,buffer,2000);
			fram.formatMeter(cualmeter,ota_write_data,2000);
			xSemaphoreGive(framSem);
			sprintf(textl,"Meter %d formatted",cualmeter);
			postLog(FRAMFL,0,string(textl));
			goto sale;
		}
		state=getParameter(argument,"MON"); //just a month
		if(state!="")
		{
			int como=atoi(state.c_str());
			if(como>11)
			{
				sprintf(textl,"Month %d out of range[0-11]",como);
				goto sale;
			}
			xSemaphoreTake(framSem, portMAX_DELAY);//portMAX_DELAY
			for (int a=0;a<daysInMonth[como];a++) // ALl days and hours in month to 0
			{
				for (int b=0;b<24;b++)
					fram.write_hour(cualmeter, yearg,como, a, b, 0); //could be done by chuncks offset + size
			}
			fram.write_month(cualmeter, como, 0);
			xSemaphoreGive(framSem);

			sprintf(textl,"Month %d of meter %d formatted",como,cualmeter);
			postLog(FRAMFL,0,string(textl));

			goto sale;
		}
		state=getParameter(argument,"DAY"); //just a day
		if(state!="")
		{
			int como=atoi(state.c_str());
			state=getParameter(argument,"month");//of this month
			if (state!="")
			{
				int mes=atoi(state.c_str());
				if(mes>11)
				{
					sprintf(textl,"Month %d out of range[0-11]",como);
					goto sale;
				}
				if(como>daysInMonth[mes])
				{
					sprintf(textl,"Day %d out of range[0-%d]",como,daysInMonth[mes]);
					goto sale;
				}
				xSemaphoreTake(framSem, portMAX_DELAY);//portMAX_DELAY
				for (int a=0;a<24;a++)
					fram.write_hour(cualmeter, yearg,mes, como, a, 0); // all 24 hours
				fram.write_day(cualmeter,yearg, mes , como,0); //this day
				xSemaphoreGive(framSem);
				sprintf(textl,"Day %d of Month %d formatted",como,mes);
				postLog(FRAMFL,0,string(textl));

			}
			goto sale;
		}
		state=getParameter(argument,"HOUR"); //just one hour
		if(state!="")
		{
			int como=atoi(state.c_str());
			if(como>23)
			{
				sprintf(textl,"Hour %d out of range[0-23]",como);
				goto sale;
			}
			state=getParameter(argument,"month");//of this month
			if (state!="")
			{
				int mes=atoi(state.c_str());
				if(mes>11)
				{
					sprintf(textl,"Month %d out of range[0-11]",como);
					goto sale;
				}
				state=getParameter(argument,"mday");//and this day
				if (state!="")
				{
					int dia=atoi(state.c_str());
					if(dia>daysInMonth[mes])
					{
						sprintf(textl,"Day %d out of range[0-%d]",dia,daysInMonth[mes]);
						goto sale;
					}
					xSemaphoreTake(framSem, portMAX_DELAY);//portMAX_DELAY
					fram.write_hour(cualmeter, yearg,mes , dia , como, 0);
					xSemaphoreGive(framSem);
					sprintf(textl,"Hour %d of Month %d of Day %d formatted",como,mes,dia);
					postLog(FRAMFL,0,string(textl));

				}

			}
			goto sale;
		}
	}
	else
		sprintf(textl,"Meter not specified");

	sale:	algo=string(textl);
	sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);            // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<CMDD))
		printf("framManager\n");                  // A general status condition for display. See routine for numbers.
#endif
	exit:
	state=algo="";
//	free(pArg);
//	vTaskDelete(NULL);
}



