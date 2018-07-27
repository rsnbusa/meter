using namespace std;
#include "setEmail.h"
extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void write_to_flash();
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);

void set_addEmail(void * pArg){
	arg *argument=(arg*)pArg; //set pet name and put name in bonjour list if changed

	string state,webString;
	set_commonCmd(argument,false);
	state=getParameter(argument,"address");
	if (state!="")
	{
		if (aqui.ecount ==MAXEMAILS)
		{
			webString="Emails full";
			sendResponse( argument->pComm,argument->typeMsg,webString,webString.length(),ERRORFULL,false,false);
			goto exit;
		}
		int cuanto=state.length();
		if (state.length()>sizeof(aqui.email[0])-1)
			cuanto=sizeof(aqui.email[0])-1;
		memcpy(&aqui.email[aqui.ecount][0],state.c_str(),cuanto);
		state=getParameter(argument,"exp");
		int excep=atoi(state.c_str());
		aqui.except[aqui.ecount]=excep;
		state=getParameter(argument,"name");
		if (state !="")
		{
			cuanto=state.length();
			if (state.length()>sizeof(aqui.email[0])-1)
				cuanto=sizeof(aqui.emailName[0])-1;
			memcpy(&aqui.emailName[aqui.ecount][0],state.c_str(),cuanto);
		}
		aqui.ecount++;
		write_to_flash();
		webString="Email Added";
		sendResponse( argument->pComm,argument->typeMsg,webString,webString.length(),NOERROR,false,false);
	}
	exit:
	state=webString="";
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<CMDD))
		printf("Web addEmail\n");
#endif
//	free(pArg);
//	vTaskDelete(NULL);
}



