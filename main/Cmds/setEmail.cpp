using namespace std;
#include "setEmail.h"
extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void write_to_flash();
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);

void set_addEmail(void * pArg){
	arg *argument=(arg*)pArg; //set pet name and put name in bonjour list if changed


}



