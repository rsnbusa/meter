using namespace std;
#include "setDisplayMgr.h"

extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);
extern string makeDateString(time_t t);
extern void drawString(int x, int y, string que, int fsize, int align,displayType showit,overType erase);

void set_displayManager(void * pArg){
	char textl[100];
	arg *argument=(arg*)pArg;
	string algo,state;
	u16 val;
	//	int cualmeter,dia,mes,year,desde=0,donde=0;

	//	float pago=0.0;
	//struct tm tml;
	//	time_t t;

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
		if(state!=""){
			oldMeter=100;
			chosenMeter=atoi(state.c_str());
			sprintf(textl,"Meter Selected %d",chosenMeter);
			state=getParameter(argument,"bounce");
			if(state!=""){
				aqui.bounce[chosenMeter]=atoi(state.c_str());
				sprintf(textl,"Meter %d Bounce %d",chosenMeter,aqui.bounce[chosenMeter]);
			}

		}
		state=getParameter(argument,"int");
			if(state!=""){
				oldMeter=100;
				aqui.MODDISPLAY[chosenMeter]=atoi(state.c_str());
				sprintf(textl,"Meter %d Interval %d",chosenMeter,aqui.MODDISPLAY[chosenMeter]);
			}


	state=getParameter(argument,"mode");
	if(state!=""){
		oldMeter=100;
		displayMode=(displayModeType)atoi(state.c_str());
		sprintf(textl,"Display Mode %d",displayMode);

	}

	state=getParameter(argument,"st");
		if(state!=""){
			val=atoi(state.c_str());
			aqui.pollGroup=val;
			if(!val)
				display.displayOff();
			else{
				if(xSemaphoreTake(I2CSem, portMAX_DELAY))
				{
					display.init();
					display.flipScreenVertically();
					display.clear();
					drawString(64,10,"EEQ",24,TEXT_ALIGN_CENTER,DISPLAYIT,NOREP);
					xSemaphoreGive(I2CSem);
				}
			}
			sprintf(textl,"Display %s",val?"on":"Off");

		}

	algo=string(textl);
	sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);            // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<CMDD))
		printf("DisplayMode\n");                  // A general status condition for display. See routine for numbers.
#endif
	exit:
	write_to_flash();
	algo=state="";
	//free(pArg);
//	vTaskDelete(NULL);
}



