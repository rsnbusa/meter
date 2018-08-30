using namespace std;
#include "loadTariffs.h"
#include "framDef.h"

extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void sendResponse(void* comm,int msgTipo,string que,int len,int errorcode,bool withHeaders, bool retain);
extern uint32_t IRAM_ATTR millis();
extern void postLog(int code,int code1,string mensaje);
extern void	loadDayBPK(uint16_t diag);

int download_callback(request_t *req, char *data, int len)
{
	int ret;

	if(xSemaphoreTake(framSem, portMAX_DELAY))
	{
		ret=fram.writeMany(addHTTP,(uint8_t*) data, len);
		xSemaphoreGive(framSem);
		if(ret!=0)
			printf("Tariff save error %d\n",ret);
		addHTTP+=len;
		llevoHTTP+=len;
		return 0;
	}
	else
	{
		printf("Error reserve Tariff\n");
		return -1;
	}
}

void set_tariff(void * pArg){
	char textl[100];
	arg *argument=(arg*)pArg;
	string algo;
	request_t *req;
	uint32_t mils,mile;

	int status;
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<FRAMD))
		printf("[FRAMD]Tariff loading\n");
#endif
	addHTTP=TARIFADIA;
	llevoHTTP=0;

	if(!set_commonCmd(argument,false))
		return;

	algo=getParameter(argument,"password");
	if(algo!="zipo")
	{
		algo="Not authorized";
		sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);            // send to someones browser when asked
		goto exit;
	}
	//printf("Load tariffs authorized. Start %d\n",addHTTP);
	mils=millis();
//	req = req_new("http://feediot.co.nf/tarifasDia.txt");
	req = req_new("http://feediot.co.nf/tarifasPer.txt");
	req_setopt(req, REQ_SET_METHOD, "POST");
	req_setopt(req, REQ_FUNC_DOWNLOAD_CB,download_callback);
//	printf("Calling request\n");
	status = req_perform(req);

	mile=millis();
	req_clean(req);
	sprintf(textl,"Tariffs ended len %d status %d time %d",llevoHTTP,status,mile-mils);
	algo=string(textl);
	postLog(PAYLL,0,algo);
	loadDayBPK(yearDay);
	sendResponse( argument->pComm,argument->typeMsg, algo,algo.length(),NOERROR,false,false);
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<FRAMD))
		printf("[FRAMD]Load Tariffs\n");
#endif
	exit:
	algo="";
//	free(pArg);
//	vTaskDelete(NULL);
}



