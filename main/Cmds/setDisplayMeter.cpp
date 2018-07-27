
/*
 * set_statusSend.cpp

 *
 *  Created on: Apr 16, 2017
 *      Author: RSN
 */
using namespace std;
#include "setDisplayMeter.h"

extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);

void set_displayMeter(void * pArg){
	char textl[100];
	arg *argument=(arg*)pArg;
	string algo;

	printf("DisplayMeter\n");
	if(!set_commonCmd(argument,false))
		return;


	algo=getParameter(argument,"password");
	if(algo!="zipo")
	{
		algo="Not authorized";
		sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);            // send to someones browser when asked
		goto exit;
	}

	algo=getParameter(argument,"meter");
	if(algo!="")
	{
		chosenMeter=atoi(algo.c_str());
		algo=getParameter(argument,"display");
		if(algo!="")
			displayMode=(displayModeType)atoi(algo.c_str());
		sprintf(textl,"Meter %d Display Mode %d",chosenMeter,displayMode);
	}
	else
		sprintf(textl,"No meter given");
	algo=string(textl);

	sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);            // send to someones browser when asked
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<CMDD))
		printf("DisplayMeter\n");                  // A general status condition for display. See routine for numbers.
#endif
	exit:
	algo="";
//	free(pArg);
//	vTaskDelete(NULL);
}



