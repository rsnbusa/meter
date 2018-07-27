using namespace std;
#include "setConnect.h"

extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);
extern string makeDateString(time_t t);
extern void conectManager(uint8_t met, uint8_t como);

void set_conection(void * pArg){
	char textl[100];
	arg *argument=(arg*)pArg;
	string algo,state;
	uint8_t cualmeter,como=0;

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

		state=getParameter(argument,"st");
		if(state!="")
		{
			como=atoi(state.c_str());
			conectManager(cualmeter,como);
		}
		sprintf(textl,"Meter %d %s",cualmeter,como?"Disconneted":"Connected");
	}
	else
		sprintf(textl,"No meter given");

	sale:    	    algo=string(textl);

	sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<CMDD))
		printf("Connect\n");
#endif
	exit:
	algo=state="";

//	free(pArg);
//	vTaskDelete(NULL);
}



