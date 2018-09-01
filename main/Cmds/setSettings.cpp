using namespace std;
#include "setSettings.h"

extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);
extern string makeDateString(time_t t);

void set_settingsStatus(void * pArg){
	char textl[100];
	arg *argument=(arg*)pArg;
	string algo,state;
	int cualmeter;

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
		chosenMeter=cualmeter=atoi(state.c_str());
		if(cualmeter>MAXDEVS-1)
		{
			sprintf(textl,"Meter out of range %d[%d]",cualmeter,MAXDEVS-1);
			goto sale;
		}


		sprintf(textl,"%d!%d!%d!%s!%d!%d!%d!%d!%d!%d!%d!%d",cualmeter,(int)displayMode,aqui.pollGroup,aqui.medidor_id[cualmeter],aqui.bornKwh[cualmeter],
				mesg,diag,horag,aqui.MODDISPLAY[cualmeter],aqui.bounce[cualmeter],aqui.beatsPerKw[cualmeter],aqui.traceflag?1:0);
		// chosenmeter,displaymode,display on/off,meterid[chosenmeter],born[meter], mesg,diag,horag, display resolution
	}
	else
		sprintf(textl,"No meter defined");
	sale:	algo=string(textl);
	sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);            // send to someones browser when asked
#ifdef DEBUGMQTT
	if(aqui.traceflag & (1<<CMDD))
		printf("Settings\n");                  // A general status condition for display. See routine for numbers.
#endif
	exit:
	algo=state="";
//	free(pArg);
//	vTaskDelete(NULL);
}



