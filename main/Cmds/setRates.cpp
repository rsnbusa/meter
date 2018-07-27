using namespace std;
#include "setRates.h"

extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);
extern void write_to_flash();

void set_rates(void * pArg){
	arg *argument=(arg*)pArg;
	string algo;

	if(!set_commonCmd(argument,false))
		return;

	algo=getParameter(argument,"password");
	if(algo!="zipo")
	{
		algo="Not authorized";
		sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);            // send to someones browser when asked
		goto exit;
	}


	algo=getParameter(argument,"display");
	int val=atoi(algo.c_str());
	if (val>0)
		aqui.DISPTIME=val;
	write_to_flash();

	exit:algo="Rates done";
	sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);            // send to someones browser when asked

#ifdef DEBUGMQTT
	if(aqui.traceflag & (1<<CMDD))
		printf("Set rates\n");
#endif
	//useless but....
	algo="";
//	free(pArg);
//	vTaskDelete(NULL);
}

