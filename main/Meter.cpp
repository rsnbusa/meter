#include <stdio.h>
#include <stdint.h>
#include "defines.h"
#include "forward.h"
#include "includes.h"
#include "projTypes.h"
#include <string>
#include "globals.h"
#include "cmds.h"
#include "framSPI.h"
#include "framDef.h"
#include "esp_bt.h"

extern void postLog(int code, int code1,string que);
extern  string makeDateString(time_t t);

using namespace std;
void initMeters();

uint32_t IRAM_ATTR millis()
{
	return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void delay(uint16_t a)
{
	vTaskDelay(a /  portTICK_RATE_MS);
}


void datacallback(esp_mqtt_client_handle_t self, esp_mqtt_event_handle_t event)
{
	mqttMsg mensa;

	mensa.mensaje=(char*)event->data;
	mensa.mensaje[event->data_len]=0;
	mensa.nc=self;
	mensa.sizer=event->data_len;
	xQueueSend( mqttQ, ( void * ) &mensa,( TickType_t ) 0 );

}

void get_meter_name()
{
	char local[20];
	string appn;
	u8  mac[6];

	if (aqui.ssid[0]==0)
		esp_wifi_get_mac(WIFI_IF_AP, mac);
	else
		esp_wifi_get_mac(WIFI_IF_STA, mac);

	sprintf(local,"%02x%02x",mac[4],mac[5]);//last tow bytes of MAC to identify the connection
	string macID=string(local);
	transform(macID.begin(), macID.end(),macID.begin(), ::toupper);

	appn=string(aqui.meterName);//meter name will be used as SSID if available else [APP]IoT-XXXX

	if (appn.length()<2)
	{
		AP_NameString = string(APP)+"-" + macID;
		appn=string(AP_NameString);
	}
	else
	{
		AP_NameString = appn +"-"+ macID;
		// AP_NameString = appn ;
	}
	macID="";
	appn="";

}

// Convert a Mongoose string type to a string.
char *mgStrToStr(struct mg_str mgStr) {
	char *retStr = (char *) malloc(mgStr.len + 1);
	memcpy(retStr, mgStr.p, mgStr.len);
	retStr[mgStr.len] = 0;
	return retStr;
} // mgStrToStr

string getParameter(arg* argument, string cual)
{
	char paramr[60];
	if (argument->typeMsg ==1) //Json get parameter cual
	{
		cJSON *param= cJSON_GetObjectItem((cJSON*)argument->pMessage,cual.c_str());
		if(param)
			return string(param->valuestring);
		else
			return string("");
	}
	else //standard web server parameter
	{
		struct http_message * param=(struct http_message *)argument->pMessage;
		int a= mg_get_http_var(&param->query_string, cual.c_str(), paramr,sizeof(paramr));
		if(a>=0)
			paramr[a]=0;
		return string(paramr);
	}
	return "";
}

int findCommand(string cual)
{
	for (int a=0;a<MAXCMDS;a++)
	{
		if(cual==string(cmds[a].comando))
			return a;
	}
	return -1;
}


void webCmds(void * nc,struct http_message * params)
{
	arg *argument=(arg*)malloc(sizeof(arg));
	char *uri=mgStrToStr(params->uri);
	int cualf=findCommand(uri);
	if(cualf>=0)
	{
		argument->pMessage=(void*)params;
		argument->typeMsg=0;
		argument->pComm=nc;
		(*cmds[cualf].code)(argument);
	}
	free(uri);
	free(argument);
}

void processCmds(void * nc,cJSON * comands)
{
	cJSON *monton= cJSON_GetObjectItem(comands,"batch");
	if(monton!=NULL)
	{
		int son=cJSON_GetArraySize(monton);
		for (int a=0;a<son;a++)
		{
			cJSON *cmdIteml = cJSON_GetArrayItem(monton, a);
			cJSON *cmd= cJSON_GetObjectItem(cmdIteml,"cmd");
			if(cmd!=NULL)
			{
				int cualf=findCommand(string(cmd->valuestring));
				if(cualf>=0)
				{
					arg *argument=(arg*)malloc(sizeof(arg));
					argument->pMessage=(void*)cmdIteml;
					argument->typeMsg=1;
					argument->pComm=nc;
					(*cmds[cualf].code)(argument);
					free(argument);
				}
#ifdef DEBUGMQQT
				else
					if(aqui.traceflag & (1<<CMDD))
							printf("[CMDD]Cmd Not found\n");

#endif
			}
		}
		cJSON_Delete(comands);
	}
}

void sendResponse( void * comm,int msgTipo,string que,int len,int code,bool withHeaders, bool retain)
{
	int msg_id ;

#ifdef DEBUGMQQT

	if(aqui.traceflag & (1<<PUBSUBD))
		printf("[PUBSUBD]Type %d Sending response [%s] len=%d\n",msgTipo,que.c_str(),que.length());
#endif

	if(msgTipo==1)
	{ // MQTT Response
		 esp_mqtt_client_handle_t mcomm=( esp_mqtt_client_handle_t)comm;

		if (!mqttf)
		{
#ifdef DEBUGMQQT

			if(aqui.traceflag & (1<<MQTTD))
				printf("[MQTTD]No mqtt\n");
#endif
			return;
		}

	//	if(withHeaders)
			if(1)
		{
			for (int a=0;a<sonUid;a++)
			{
			//	spublishTopic="";
				if(montonUid[a]!="")
					spublishTopic=string(APP)+"/"+string(aqui.groupName)+"/"+string(aqui.meterName)+"/"+montonUid[a]+"/MSG";
				else
					spublishTopic=string(APP)+"/"+string(aqui.groupName)+"/"+string(aqui.meterName)+"/MSG";
#ifdef DEBUGMQQT

				if(aqui.traceflag & (1<<PUBSUBD))
					printf("[PUBSUBD]Publish %s Msg %s for %d\n",spublishTopic.c_str(),que.c_str(),(u32)mcomm);
#endif
		//					printf(" Heap Sendb %d\n",xPortGetFreeHeapSize());
				 msg_id = esp_mqtt_client_publish(mcomm, (char*)spublishTopic.c_str(), (char*)que.c_str(),que.length(), 0, 0);
						//		printf(" Heap SendA %d\n",xPortGetFreeHeapSize());
				 if(msg_id<0)
					 printf("Error publish %d\n",msg_id);
				delay(200); //wait a while for next destination
			}
		}
		else
		{
			//	spublishTopic="";
				spublishTopic=string(APP)+"/"+string(aqui.groupName)+"/"+string(aqui.meterName)+"/MSG";
#ifdef DEBUGMQQT

				if(aqui.traceflag & (1<<PUBSUBD))
#endif
					printf("[PUBSUBD]DirectPublish %s Msg %s\n",spublishTopic.c_str(),que.c_str());
				msg_id = esp_mqtt_client_publish(mcomm, (char*)spublishTopic.c_str(), (char*)que.c_str(),que.length(), 0, 0);
				que="";

		}

	}
	else
	{ //Web Response
		struct mg_connection *nc=(struct mg_connection*)comm;
#ifdef DEBUGMQQT

		if(aqui.traceflag & (1<<WEBD))
			printf("[WEBD]Web response nc %d\n",(u32)nc);
#endif

		if(len==0)
		{
			que=" ";
			len=1;
}
		mg_send_head(nc, 200, len, withHeaders?"Content-Type: text/html":"Content-Type: text/plain");
		mg_printf(nc, "%s", que.c_str());
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
}


static void IRAM_ATTR gpio_isr_handler(void * arg)
{
	meterType *meter=(meterType*)arg;
	u32 fueron;
	BaseType_t tasker;

//	if(memchr(METERS,meter->elpin,4)!=NULL)
//	{
		u32 ahorita=xTaskGetTickCountFromISR();

		if(meter->elpin==WATER)
		{
			if(ahorita-meter->lastKwHDate>1000)
				meter->lastKwHDate=ahorita;
			meter->curLife++;
			meter->timestamp=ahorita;
			meter->beatSave++;
			meter->currentBeat++;

			if((meter->currentBeat % meter->maxLoss)==0) // 1/10 of liter
							{
								if(meter->beatSave>=aqui.beatsPerKw[4])
								{ //One Liter has flowed
									meter->saveit=true;
									meter->beatSave=0;
								}
								else
									meter->saveit=false;
								xQueueSendFromISR( isrW,arg,&tasker ); //Water Manager wake up
								if (tasker)
									portYIELD_FROM_ISR();
							}
			return;
		}

		taskENTER_CRITICAL_ISR(&myMutex);
		u8 como=gpio_get_level((gpio_num_t)meter->elpin);
		taskEXIT_CRITICAL_ISR(&myMutex);

		if(!como)
		{
			fueron=ahorita-meter->timestamp;
			 if(fueron>aqui.bounce[meter->meterid])
			 {
				meter->timestamp=ahorita; //last valid isr
				meter->beatSave++;
				meter->currentBeat++;
				if((meter->currentBeat % meter->maxLoss)==0) //every GMAXLOSSPER interval
				{
					if(meter->beatSave>=(meter->beatsPerkW*diaTarifa[horag]/100))
					{ //One kWh has occurred
						meter->saveit=true;
						meter->beatSave=0;
					}
					else
						meter->saveit=false;
					xQueueSendFromISR( isrQ,arg,&tasker );
					if (tasker)
						portYIELD_FROM_ISR();
				}
				meter->msNow=fueron;
				if(fueron<meter->minamps)
					meter->minamps=fueron;
				if(fueron>meter->maxamps)
					meter->maxamps=fueron;
			 }
		}

	}
//}

void waterManager(void *pArg)
{
	meterType soyYo;

	while(1){
		if( xQueueReceive( isrW, &soyYo, portMAX_DELAY ))
		{
			if(xSemaphoreTake(framSem, portMAX_DELAY))  //reserve FRAM
			{
				fram.write_beat(soyYo.meterid,soyYo.currentBeat);
#ifdef DEBUGMQQT

				if(aqui.traceflag & (1<<BEATD))
					printf("[BEATD]Water %d pulses %d\n",soyYo.meterid,soyYo.currentBeat);
#endif
				if(soyYo.saveit) //One Liter has flowed
					write_to_fram(soyYo.meterid,true); // last saved beat count

				xSemaphoreGive(framSem); //free FRAM reserve
			}
		else
			printf("Failed reserve\n"); //Should be FATAL and stop
		}
		else
		{
			printf("Sem fail\n");
					delay(100);
		}
	}
}


void interruptTask(void* pvParameter)
{
	uint64_t mask=1;
	uint32_t aca;
	meterType soyYo;
	//set Breaker Pin Output
	//	printf("Breaker for int %d, inter Pin %d and relaypin %d is %s Mutex %p horag %d\n",yo,interruptPin,relayPin,aqui.breakers[yo]?"On":"Off",arg->mimutex,horag);
//	gpio_set_direction((gpio_num_t)relayPin, GPIO_MODE_OUTPUT);
//	gpio_set_level((gpio_num_t)relayPin, aqui.breakers[yo]); // Inverse since its a Normally Closed Relays strategy

	isrQ = xQueueCreate( 100, sizeof( meterType ) ); //Meter queue

	if(!isrQ)
	{
		printf("Can not create Meter queue. Stopping\n");
		while(1)
			delay(100);
	}

	isrW = xQueueCreate( 100, sizeof( meterType ) ); //Water Queue

	if(!isrW)
	{
		printf("Can not create Water queue. Stopping\n");
		while(1)
			delay(100);
	}

	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_NEGEDGE;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_down_en =GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en =GPIO_PULLUP_ENABLE;

	gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
	for(int a=0;a<MAXDEVS;a++)
		gpio_intr_disable(METERS[a]);
	for (int a=0;a<MAXDEVS;a++)
	{
		theMeters[a].meterid=a;
		theMeters[a].elpin=METERS[a];
		theMeters[a].state=1;
		theMeters[a].minamps=0xffffffff;
		theMeters[a].beatsPerkW=aqui.beatsPerKw[a];
		theMeters[a].maxLoss=aqui.beatsPerKw[a]/10;
		io_conf.pin_bit_mask = (mask<<METERS[a]);
		gpio_config(&io_conf);
		gpio_isr_handler_add((gpio_num_t)METERS[a], gpio_isr_handler,(void*)&theMeters[a]);
	}
	for(int a=0;a<MAXDEVS;a++)
		gpio_intr_enable(METERS[a]);

		xTaskCreate(&waterManager, "water", 2048, NULL, 5, NULL);

	while(1){
		if( xQueueReceive( isrQ, &soyYo, portMAX_DELAY ))
		{
			if(xSemaphoreTake(framSem, portMAX_DELAY))  //reserve FRAM
			{
				fram.write_beat(soyYo.meterid,soyYo.currentBeat);
#ifdef DEBUGMQQT

				if(aqui.traceflag & (1<<INTD))
					printf("[INTD]Meter %d curBeat %d BPK %d SaveC %d Tariff %d CurLife %d\n",soyYo.meterid,soyYo.currentBeat,soyYo.beatsPerkW,soyYo.maxLoss,
							diaTarifa[horag],soyYo.curLife);
#endif
				if(soyYo.saveit){ //One kWh has occurred
			//		printf("Meter[%d]=%d saveit %d hora[%d] %d\n",soyYo.meterid,soyYo.currentBeat,soyYo.saveit,horag,diaTarifa[horag]);
					write_to_fram(soyYo.meterid,true); // last saved beat count
			}

				xSemaphoreGive(framSem); //free FRAM reserve
			}
		else
			printf("Failed reserve\n"); //Should be FATAL and stop
		}
		else
		{
			printf("Sem fail\n");
					delay(100);
		}
	}
}

void read_flash()
{
	esp_err_t q ;
	size_t largo;
	q = nvs_open("config", NVS_READONLY, &nvshandle);
		if(q!=ESP_OK)
		{
			printf("Error opening NVS Read File %x\n",q);
			return;
		}

			 largo=sizeof(aqui);
				q=nvs_get_blob(nvshandle,"config",(void*)&aqui,&largo);

			if (q !=ESP_OK)
				printf("Error read %x largo %d aqui %d\n",q,largo,sizeof(aqui));
	nvs_close(nvshandle);

}

uint32_t readADC()
{
	u32 adc_reading=0;

    for (int i = 0; i < SAMPLES; i++)
    	adc_reading += adc1_get_raw((adc1_channel_t)adcchannel);

        adc_reading /= SAMPLES;
	return adc_reading;
}

void write_to_flash() //save our configuration
{
	esp_err_t q ;
	q = nvs_open("config", NVS_READWRITE, &nvshandle);
		if(q!=ESP_OK)
		{
			printf("Error opening NVS File RW %x\n",q);
			return;
		}

		delay(300);
	q=nvs_set_blob(nvshandle,"config",(void*)&aqui,sizeof(aqui));
	if (q ==ESP_OK)
		q = nvs_commit(nvshandle);
	nvs_close(nvshandle);
}

esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
 //   int msg_id=0;
    esp_err_t err;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            if(client==clientCloud){
            	esp_mqtt_client_subscribe(client, cmdTopic.c_str(), 0);
        		gpio_set_level((gpio_num_t)MQTTLED, 1);
            }
            else
        		mqttThingf=true;
#ifdef DEBUGMQQT
        	if(aqui.traceflag & (1<<MQTTD))
        		printf("[MQTTD]Connected %s(%d)\n",(char*)event->user_context,(u32)client);
        	postLog(MQTTL,(u32)client,"MqttConnect");
        	logdiscf=false; //reset flag to log disconnect
#endif
            break;
        case MQTT_EVENT_DISCONNECTED:
#ifdef DEBUGMQQT
        	if(aqui.traceflag & (1<<MQTTD))
        		printf( "[MQTTD]MQTT_EVENT_DISCONNECTED %s(%d)\n",(char*)event->user_context,(u32)client);
        	if(!logdiscf)
        		{
        			postLog(MQTTL,(u32)client,"MqttDisconnect");//just one disconnect per connect. logdiscf controls this
        			logdiscf=true;
        		}

#endif
        	if(client==clientCloud){
        		gpio_set_level((gpio_num_t)MQTTLED, 0);
        		mqttf=false;
        	}
        	else
        		mqttThingf=false;

            break;

        case MQTT_EVENT_SUBSCRIBED:
        	if(client==clientThing)
        	{
#ifdef DEBUGMQQT
            	if(aqui.traceflag & (1<<MQTTD))
            		printf( "[MQTTD]Subscribe ThingSpeak\n");
#endif
            	mqttThingf=true;
        	}
        	else{
#ifdef DEBUGMQQT
            	if(aqui.traceflag & (1<<MQTTD))
            		printf("[MQTTD]Subscribed Cloud\n");
#endif
            	mqttf=true;
        		}
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
#ifdef DEBUGMQQT
        	if(aqui.traceflag & (1<<MQTTD))
        		printf( "[MQTTD]MQTT_EVENT_UNSUBSCRIBED %s(%d)\n",(char*)event->user_context,(u32)client);
#endif
        	  if(client==clientCloud)
        		  esp_mqtt_client_subscribe(client, cmdTopic.c_str(), 0);
            break;
        case MQTT_EVENT_PUBLISHED:
#ifdef DEBUGMQQT
        	if(aqui.traceflag & (1<<MQTTD))
        		printf( "[MQTTD]MQTT_EVENT_PUBLISHED %s(%d)\n",(char*)event->user_context,(u32)client);
#endif
            break;
        case MQTT_EVENT_DATA:
    //		printf("Event Heap Msg %d\n",xPortGetFreeHeapSize());
#ifdef DEBUGMQQT
        	if(aqui.traceflag & (1<<MQTTD))
        	{
        		printf("[MQTTD]MSG for %s(%d)\n",(char*)event->user_context,(u32)client);
        		printf("[MQTTD]TOPIC=%.*s\r\n", event->topic_len, event->topic);
        		printf("[MQTTD]DATA=%.*s\r\n", event->data_len, event->data);
        	}
#endif
            if(client==clientCloud)
            	datacallback(client,event);
            break;
        case MQTT_EVENT_ERROR:
#ifdef DEBUGMQQT
        	if(aqui.traceflag & (1<<MQTTD))
        		printf("[MQTTD]MQTT_EVENT_ERROR %s(%d)\n",(char*)event->user_context,(u32)client);
        	postLog(MQTTL,(u32)client,"MqttError");

#endif

            break;
    }
    return ESP_OK;
}

void mqttmanager(void * parg)
{
	mqttMsg mensa;

	mqttQ = xQueueCreate( 20, sizeof( mensa ) );
	if(mqttQ)
	{
		while(1)
		{
			if( xQueueReceive( mqttQ, &mensa, portMAX_DELAY ))
			{
			//	printf(" Heap MqttMgrIn %d need %d\n",xPortGetFreeHeapSize(),strlen(mensa.mensaje));
#ifdef DEBUGMQQT
				if(aqui.traceflag & (1<<MQTTD))
					printf("[MQTTD]MqttMsg:%s\n",mensa.mensaje);
#endif
				root=cJSON_Parse( mensa.mensaje);
			//	printf(" Heap MqttMgrRoot %d\n",xPortGetFreeHeapSize());

				if(root==NULL)
					printf("Not valid Json\n");
				else
					processCmds(mensa.nc,root);
			//	printf(" Heap MqttMgr After %d\n",xPortGetFreeHeapSize());

			}
			delay(100);//just in case it fails to wait forever do no eat the cpu
		}
	}
	else
		delay(1000);
	vTaskDelete(NULL); //in case it reaches end
}

void mongoose_event_handler(struct mg_connection *nc, int ev, void *evData) {
	//	char * evento=mongoose_eventToString(ev);
	//	printf("Event %s\n",evento);
	switch (ev) {
	case MG_EV_HTTP_REQUEST:
	{
		struct http_message *message = (struct http_message *) evData;
#ifdef DEBUGMQQT
		if(aqui.traceflag&(1<<WEBD)){
			char *uri = mgStrToStr(message->uri);
			printf("cmd in http %s\n",uri);
		}
#endif
		//	webCmds(uri,mgStrToStr(message->query_string));
		webCmds((void*)nc,message);
		break;
		//	char *paramr=(char*)malloc(20);

	//	nc->flags |= MG_F_SEND_AND_CLOSE;
		break;
	}
	//	default:
	//		printf("Mong %d\n",ev);
	//		nc->flags |= MG_F_SEND_AND_CLOSE;
	//				break;
	}
} // End of mongoose_event_handler


void mongooseTask(void *data) {
	mongf=true;
	mg_mgr_init(&mgr, NULL);
	struct mg_connection *c = mg_bind(&mgr, ":80", mongoose_event_handler);
	if (c == NULL) {
		printf( "No connection from the mg_bind()\n");
			vTaskDelete(NULL);
		return;
	}
	mg_set_protocol_http_websocket(c);
#ifdef DEBUGMQQT
	if(aqui.traceflag&(1<<BOOTD))
		printf("[BOOTD]Started mongoose\n");
#endif

	while (1)
		mg_mgr_poll(&mgr, 10);

} // mongooseTask


void mdnstask(void *args){
	char textl[60];
	time_t now;
	struct tm timeinfo;
	esp_err_t ret;
	time(&now);
	localtime_r(&now, &timeinfo);

		esp_err_t err = mdns_init();
		if (err) {
			printf( "Failed starting MDNS: %u\n", err);
		}
		else
		{
	//		if(aqui.traceflag&(1<<CMDD))
	//			printf("[CMDD]MDNS hostname %s\n",AP_NameString.c_str());
			mdns_hostname_set(AP_NameString.c_str()) ;
			mdns_instance_name_set(AP_NameString.c_str()) ;

			  mdns_txt_item_t serviceTxtData[4] = {
			        {(char*)"WiFi",(char*)"Yes"},
			        {(char*)"ApMode",(char*)"Yes"},
			        {(char*)"OTA",(char*)"Yes"},
			        {(char*)"Boot",(char*)""}
			    };

			sprintf(textl,"%d/%d/%d %d:%d:%d",1900+timeinfo.tm_year,timeinfo.tm_mon,timeinfo.tm_mday,timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);

		//	if(aqui.traceflag&(1<<CMDD))
		//		printf("[CMDD]MDNS time %s\n",textl);
			string s=string(textl);
			ret=mdns_service_add( AP_NameString.c_str(),"_heaterIoT", "_tcp", 80,serviceTxtData, 4 );
	//		if(ret && (aqui.traceflag&(1<<CMDD)))
		//					printf("Failed add service  %d\n",ret);
			ret=mdns_service_txt_item_set("_heaterIoT", "_tcp", "Boot", s.c_str());
		//	if(ret && (aqui.traceflag&(1<<CMDD)))
			//				printf("Failed add txt %d\n",ret);
		//	query_mdns_service(mdns, "_http", 0);

			s="";
		}

	vTaskDelete(NULL);
}

void initialize_sntp(void *args)
{
	 struct timeval tvStart;
//	struct tm mitime;
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, (char*)"pool.ntp.org");
	sntp_init();
	time_t now = 0;


	int retry = 0;
	const int retry_count = 10;
	setenv("TZ", "EST5", 1); //UTC is 5 hours ahead for Quito
	tzset();

	struct tm timeinfo;// = { 0 };
//		timeinfo.tm_hour=timeinfo.tm_min=timeinfo.tm_sec=0;
//		timeinfo.tm_mday=1;
//		timeinfo.tm_mon=0;
//		timeinfo.tm_year=100;
//		// set to 1/1/2000 0:0:0
////		magicNumber = mktime(&timeinfo); // magic number for timers .Sec  at above date to substract to all set timers to know secs to fire the
//		// timer *1000 im ms

	while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
	//	ESP_LOGI(TAGG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		time(&now);
		localtime_r(&now, &timeinfo);
	}
	gettimeofday(&tvStart, NULL);
	sntpf=true;
#ifdef DEBUGMQQT
	if(aqui.traceflag&(1<<BOOTD))
		printf("[BOOTD]Internet Time %04d/%02d/%02d %02d:%02d:%02d YDays %d DoW:%d\n",1900+timeinfo.tm_year,timeinfo.tm_mon+1,timeinfo.tm_mday,
			timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec,timeinfo.tm_yday,timeinfo.tm_wday);
#endif
	mesg=timeinfo.tm_mon;
	diag=timeinfo.tm_mday-1;
	horag=timeinfo.tm_hour;
	yearg=timeinfo.tm_year+1900;
	yearDay=timeinfo.tm_yday;

	rtc.setEpoch(now);
//	gdayOfWeek=timeinfo.tm_wday;
//	printf("Magic Number: %ld Day:%d\n",magicNumber,gdayOfWeek); //Should be 946702800

	aqui.preLastTime=aqui.lastTime;
	time(&aqui.lastTime);
	write_to_flash();
	timef=1;
	postLog(BOOTL,aqui.bootcount,"Boot");

	if(!mdnsf)
		xTaskCreate(&mdnstask, "mdns", 4096, NULL, 5, NULL); //Ota Interface Controller
	//release this task
	vTaskDelete(NULL);
}

void ConfigSystem(void *pArg)
{
	uint32_t del=(uint32_t)pArg;
	while(1)
	{
		gpio_set_level((gpio_num_t)WIFILED, 1);
		delay(del);
		gpio_set_level((gpio_num_t)WIFILED, 0);
		delay(del);
	}
}


void newSSID(void *pArg)
{
	string temp;
	wifi_config_t sta_config;
	int len,cual=(int)pArg;

	len=0;
	esp_wifi_stop();

	temp=string(aqui.ssid[cual]);
	len=temp.length();

	if(xSemaphoreTake(I2CSem, portMAX_DELAY))
		{
			drawString(64,34,"               ",10,TEXT_ALIGN_CENTER,DISPLAYIT,REPLACE);
			drawString(64,34,string(aqui.ssid[curSSID]),10,TEXT_ALIGN_CENTER,DISPLAYIT,REPLACE);
			xSemaphoreGive(I2CSem);
		}
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<WIFID))
			printf("[WIFID]Try SSID =%s= %d %d\n",temp.c_str(),cual,len);
#endif
		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

	temp=string(aqui.ssid[cual]);
	len=temp.length();
	memcpy((void*)sta_config.sta.ssid,temp.c_str(),len);
	sta_config.sta.ssid[len]=0;
	temp=string(aqui.pass[cual]);
	len=temp.length();
	memcpy((void*)sta_config.sta.password,temp.c_str(),len);
	sta_config.sta.bssid_set=0;
	sta_config.sta.password[len]=0;
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
	esp_wifi_start(); //if error try again indefinitly

	vTaskDelete(NULL);

}


esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
	string local="Closed",temp;
	u8 mac[8];
	wifi_config_t config;
//	int len;
//	char textl[50];
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<WIFID))
		printf("[WIFID]Wifi Handler %d\n",event->event_id);
#endif
    mdns_handle_system_event(ctx, event);

	//delay(100);
	switch(event->event_id)
	{
	case SYSTEM_EVENT_STA_GOT_IP:
	//	len=10;
		gpio_set_level((gpio_num_t)WIFILED, 1);
		connf=true;
		localIp=event->event_info.got_ip.ip_info.ip;
		get_meter_name();

		initMeters();

#ifdef DEBUGMQQT
		if(aqui.traceflag&(1<<BOOTD))
			printf( "[BOOTD]Got IP: %d.%d.%d.%d Mqttf %d\n", IP2STR(&event->event_info.got_ip.ip_info.ip),mqttf);
#endif

		if(!mqttf)
		{
#ifdef DEBUGMQQT
			if(aqui.traceflag&(1<<CMDD))
				printf("[CMDD]Connect to mqtt\n");
#endif
			xTaskCreate(&mqttmanager,"mgr",10240,NULL,  5, NULL);		// User interface while in development. Erased in RELEASE

			clientCloud = esp_mqtt_client_init(&settings);
			 if(clientCloud)
			    esp_mqtt_client_start(clientCloud);
			 else
				 printf("Fail mqtt initCloud\n");
//				 clientThing = esp_mqtt_client_init(&settingsThing);
//				 if(clientThing)
//				    esp_mqtt_client_start(clientThing);
//				 else
//					 printf("Fail mqtt init Thing\n");
		}

		if(!mongf)
		{
			if(I2CSem)
			{
				if(xSemaphoreTake(I2CSem, portMAX_DELAY))
				{
					setLogo("MeterIoT");
					xSemaphoreGive(I2CSem);
				}
			}
			xTaskCreate(&mongooseTask, "mongooseTask", 10240, NULL, 5, NULL); //  web commands Interface controller
			xTaskCreate(&initialize_sntp, "sntp", 2048, NULL, 3, NULL); //will get date
		}
		displayMode=aqui.dispmode;

		if(!displayf)
		{
			delay(3000);
			display.displayOff();
		}
		break;
	case SYSTEM_EVENT_AP_START:  // Handle the AP start event
		tcpip_adapter_ip_info_t ip_info;
		tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info);
		printf("System not Configured. Use local AP and IP:" IPSTR "\n", IP2STR(&ip_info.ip));
		if(!mongf)
		{
			xTaskCreate(&mongooseTask, "mongooseTask", 10240, NULL, 5, NULL);
			xTaskCreate(&initialize_sntp, "sntp", 2048, NULL, 3, NULL);
			xTaskCreate(&ConfigSystem, "cfg", 1024, (void*)100, 3, NULL);
		}
		break;

	case SYSTEM_EVENT_STA_START:
#ifdef DEBUGMQQT
		if(aqui.traceflag & (1<<WIFID))
			printf("[WIFID]Connect\n");
#endif
		esp_wifi_connect();
		break;

	case SYSTEM_EVENT_STA_DISCONNECTED:
	case SYSTEM_EVENT_AP_STADISCONNECTED:
	case SYSTEM_EVENT_ETH_DISCONNECTED:
		connf=false;
		gpio_set_level((gpio_num_t)WIFILED, 0);

#ifdef DEBUGMQQT
		if(aqui.traceflag & (1<<WIFID))
			printf("[WIFID]Reconnect %d\n",curSSID);
		postLog(WIFIL,0,"Disconnected");
#endif
		curSSID++;
		if(curSSID>4)
			curSSID=0;

		temp=string(aqui.ssid[curSSID]);
#ifdef DEBUGMQQT
		if(aqui.traceflag & (1<<WIFID))
			printf("[WIFID]Temp[%d]==%s=\n",curSSID,temp.c_str());
#endif
		if(temp!="")
		{
			xTaskCreate(&newSSID,"newssid",2048,(void*)curSSID, MGOS_TASK_PRIORITY, NULL);
		}
		else
		{
			curSSID=0;
			xTaskCreate(&newSSID,"newssid",2048,(void*)curSSID, MGOS_TASK_PRIORITY, NULL);
		}
		break;

	case SYSTEM_EVENT_STA_CONNECTED:
#ifdef DEBUGMQQT
		if(aqui.traceflag & (1<<WIFID))
			printf("[WIFID]Connected SSID[%d]=%s\n",curSSID,aqui.ssid[curSSID]);
		postLog(WIFIL,0,"Connected");
#endif
		aqui.lastSSID=curSSID;
		write_to_flash();
		gpio_set_level((gpio_num_t)WIFILED, 1);

		break;

	default:
#ifdef DEBUGMQQT
		if(aqui.traceflag & (1<<WIFID))
			printf("[WIFID]default WiFi %d\n",event->event_id);
#endif
		break;
	}
	return ESP_OK;
} // wifi_event_handler

void initI2C()
{
	i2cp.sdaport=(gpio_num_t)SDAW;
	i2cp.sclport=(gpio_num_t)SCLW;
	i2cp.i2cport=I2C_NUM_0;
	miI2C.init(i2cp.i2cport,i2cp.sdaport,i2cp.sclport,700000,&I2CSem);//Will reserve a Semaphore for Control
}

void initWiFi(void *pArg)
{
	u8 mac[8];
	char textl[20];
	string temp;
	int len;
	wifi_init_config_t 				cfg=WIFI_INIT_CONFIG_DEFAULT();
	wifi_config_t 					sta_config,configap;
	tcpip_adapter_init();
	ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL));
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));



	if (aqui.ssid[curSSID][0]!=0)
	{
		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
		temp=string(aqui.ssid[curSSID]);
		len=temp.length();
		memcpy(sta_config.sta.ssid,temp.c_str(),len+1);
		temp=string(aqui.pass[curSSID]);
		len=temp.length();
		memcpy(sta_config.sta.password,temp.c_str(),len+1);
		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));
	}

	else
	{
		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
		esp_wifi_get_mac(ESP_IF_WIFI_STA, (u8*)&mac);
		sprintf(textl,"MeterIoT%02x%02x",mac[6],mac[7]);
		memcpy(configap.ap.ssid,textl,12);
		memcpy(configap.ap.password,"csttpstt\0",9);
		configap.ap.ssid[12]=0;
		configap.ap.password[9]=0;
		configap.ap.ssid_len=0;
		configap.ap.authmode=WIFI_AUTH_WPA_PSK;
		configap.ap.ssid_hidden=false;
		configap.ap.max_connection=4;
		configap.ap.beacon_interval=100;
		ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &configap));
	}

	ESP_ERROR_CHECK(esp_wifi_start());

	vTaskDelete(NULL);
}

void initMeters()
{
	string s1="meterX";
	xTaskCreate(&interruptTask,"MeterISR",10240, NULL,(configMAX_PRIORITIES - 1),NULL); //Max Priority
}

void initScreen()
{
	if(xSemaphoreTake(I2CSem, portMAX_DELAY))
	{
		display.init();
		display.flipScreenVertically();
		display.clear();
		drawString(64,10,"WiFi",24,TEXT_ALIGN_CENTER,DISPLAYIT,NOREP);
		drawString(64,34,string(aqui.ssid[0]),10,TEXT_ALIGN_CENTER,DISPLAYIT,NOREP);
		xSemaphoreGive(I2CSem);
	}
	else
		printf("Failed to InitScreen\n");
}


void initVars()
{
	char textl[40];
	string idd;

	//We do it this way so we can have a single global.h file with EXTERN variables(when not main app)
	// and be able to compile routines in an independent file

		uint16_t a=esp_random();
		sprintf(textl,"meter%04d",a);
		idd=string(textl);
#ifdef DEBUGMQQT
		if(aqui.traceflag & (1<<BOOTD))
			printf("[BOOTD]Id %s\n",textl);
#endif
		// Water max Loss
		maxw=aqui.bounce[4]/10;

		settings.host=aqui.mqtt;
		settings.port = aqui.mqttport;
		settings.client_id=strdup(idd.c_str());
		settings.username=aqui.mqttUser;
		settings.password=aqui.mqttPass;
		settings.event_handle = mqtt_event_handler;
		settings.user_context =aqui.mqtt; //name of server
		settings.transport=0?MQTT_TRANSPORT_OVER_SSL:MQTT_TRANSPORT_OVER_TCP;
		settings.buffer_size=2048;
		settings.disable_clean_session=true;

		settingsThing.host="mqtt.thingspeak.com";
		settingsThing.port=1883;
		settingsThing.event_handle = mqtt_event_handler;
		settingsThing.user_context =(void*)"mqtt.thingspeak.com"; //name of server

	strcpy(APP,"MeterIoT");
	strcpy(aqui.mqtt,"m13.cloudmqtt.com");
	strcpy(aqui.mqttUser,"wckwlvot");
	strcpy(aqui.mqttPass,"MxoMTQjeEIHE");
	aqui.mqttport=18747;

	//Set up Mqtt Variables
	spublishTopic=string(APP)+"/"+string(aqui.groupName)+"/"+string(aqui.meterName)+"/MSG";
	sCollectionTopic=string(APP)+"/"+string(aqui.groupName)+"/"+string(aqui.meterName)+"/BILLING";
	cmdTopic=string(APP)+"/"+string(aqui.groupName)+"/"+string(aqui.meterName)+"/CMD";

//	bbuffer=(uint8_t*)pvPortMallocCaps(2200, MALLOC_CAP_DMA);
	bbuffer=(uint8_t*)malloc(2200);
//	memset(&mcount,0,sizeof(mcount));

	GMAXLOSSPER=80;
	strcpy(WHOAMI,"rsimpsonbusa@gmail.com");
	strcpy(MQTTSERVER,"m11.cloudmqtt.com");

	strcpy(meses[0],"Ene");
	strcpy(meses[1],"Feb");
	strcpy(meses[2],"Mar");
	strcpy(meses[3],"Abr");
	strcpy(meses[4],"May");
	strcpy(meses[5],"Jun");
	strcpy(meses[6],"Jul");
	strcpy(meses[7],"Ago");
	strcpy(meses[8],"Sep");
	strcpy(meses[9],"Oct");
	strcpy(meses[10],"Nov");
	strcpy(meses[11],"Dic");

	daysInMonth [0] =31;
	daysInMonth [1] =28;
	daysInMonth [2] =31;
	daysInMonth [3] =30;
	daysInMonth [4] =31;
	daysInMonth [5] =30;
	daysInMonth [6] =31;
	daysInMonth [7] =31;
	daysInMonth [8] =30;
	daysInMonth [9] =31;
	daysInMonth [10] =30;
	daysInMonth [11] =31;

	// set pairs of "command name" with Function to be called
	// OJO commandos son con el backslash incluido ej: /mt_HttpStatus y no mt_HttpStatus a secas!!!!

	strcpy((char*)&cmds[0].comando,"/mt_firmware");			cmds[0].code=set_FirmUpdateCmd;			//done...needs testing in a good esp32
	strcpy((char*)&cmds[1].comando,"/mt_erase");			cmds[1].code=set_eraseConfig;			//done
	strcpy((char*)cmds[2].comando,"/mt_httpstatus");		cmds[2].code=set_HttpStatus;			//done
	strcpy((char*)cmds[3].comando,"/mt_status");			cmds[3].code=set_statusSend;			//done
	strcpy((char*)cmds[4].comando,"/mt_reset");				cmds[4].code=set_reset;					//done
	strcpy((char*)cmds[5].comando,"/mt_resetstats");		cmds[5].code=set_resetstats;			//done
	strcpy((char*)cmds[6].comando,"/mt_rates");	 			cmds[6].code=set_rates;					//done
	strcpy((char*)cmds[7].comando,"/mt_internal");			cmds[7].code=set_internal;				//done
	strcpy((char*)cmds[8].comando,"/mt_email");				cmds[8].code=set_addEmail;				//done
	strcpy((char*)cmds[9].comando,"/mt_getmonth");			cmds[9].code=set_getMonth;				//done
	strcpy((char*)cmds[10].comando,"/mt_getday");			cmds[10].code=set_getDay;				//done
	strcpy((char*)cmds[11].comando,"/mt_gethour");			cmds[11].code=set_getHour;				//done
	strcpy((char*)cmds[12].comando,"/mt_getmonthall");		cmds[12].code=set_getMonthAll;			//done
	strcpy((char*)cmds[13].comando,"/mt_getdayall");		cmds[13].code=set_getDayAll;			//done
	strcpy((char*)cmds[14].comando,"/mt_getdaysinmonth");	cmds[14].code=set_getDaysInMonth;		//done
	strcpy((char*)cmds[15].comando,"/mt_gethoursinday");	cmds[15].code=set_getHoursInDay;		//done
	strcpy((char*)cmds[16].comando,"/mt_gethoursinmonth");	cmds[16].code=set_getHoursInMonth;		//done
	strcpy((char*)cmds[17].comando,"/mt_getcycle");			cmds[17].code=set_getCycle;				//done
	strcpy((char*)cmds[18].comando,"/mt_getcycledate");		cmds[18].code=set_getCycleDate;			//done
	strcpy((char*)cmds[19].comando,"/mt_conection");		cmds[19].code=set_conection;			//done
	strcpy((char*)cmds[20].comando,"/mt_displaymeter");		cmds[20].code=set_displayMeter;			//done
	strcpy((char*)cmds[21].comando,"/mt_displaymanager");	cmds[21].code=set_displayManager;		//done
	strcpy((char*)cmds[22].comando,"/mt_frammanager");		cmds[22].code=set_framManager;			//done
	strcpy((char*)cmds[23].comando,"/mt_settings");			cmds[23].code=set_settingsStatus;		//free
	strcpy((char*)cmds[24].comando,"/mt_payment");			cmds[24].code=set_payment;				//done
	strcpy((char*)cmds[25].comando,"/mt_tariff");			cmds[25].code=set_tariff;				//done
	strcpy((char*)cmds[26].comando,"/mt_generalap");		cmds[26].code=set_generalap;			//done
	strcpy((char*)cmds[27].comando,"/mt_scan");				cmds[27].code=set_scanCmd;				//done
	strcpy((char*)cmds[28].comando,"/mt_clearlog");			cmds[28].code=set_clearLog;				//done
	strcpy((char*)cmds[29].comando,"/mt_readlog");			cmds[29].code=set_readlog;				//done
	strcpy((char*)cmds[30].comando,"/mt_session");			cmds[30].code=set_session;				//done

	METERS[0]=METER1;
	METERS[1]=METER2;
	METERS[2]=METER3;
	METERS[3]=METER4;

	breakerPin[0]=RELAY1;
	breakerPin[1]=RELAY2;
	breakerPin[2]=RELAY3;
	breakerPin[3]=RELAY4;

	barX[0]=0;
	barX[1]=6;
	barX[2]=12;

	barH[0]=5;
	barH[1]=10;
	barH[2]=15;

	eport = 2525;

	chosenMeter=aqui.lastMeter;// first meter as default
	displayMode=DISPLAYNADA; //Displaymode when boot first time
	oldMode=displayMode;
	oldMeter=chosenMeter;

	strcpy((char*)WIFIME,"MeterIoT0");
//	strcpy((char*)eserver , "mail.smtp2go.com");;

	showf=true;

	mongf=false;
    compile_date = __DATE__ ;
    // " " __TIME__;
    printf("Compile date %s\n",compile_date);

	usertime=millis();

//	memset(&opts, 0, sizeof(opts));
//	opts.user_name =s_user_name ;
//	opts.password = s_password;
//	opts.keep_alive=0;

	subf=false;

	strcpy(lookuptable[0],"BOOTD");
	strcpy(lookuptable[1],"WIFID");
	strcpy(lookuptable[2],"MQTTD");
	strcpy(lookuptable[3],"PUBSUBD");
	strcpy(lookuptable[4],"MONGOOSED");
	strcpy(lookuptable[5],"CMDD");
	strcpy(lookuptable[6],"WEBD");
	strcpy(lookuptable[7],"GEND");
	strcpy(lookuptable[8],"MQTTT");
	strcpy(lookuptable[9],"HEAPD");
	strcpy(lookuptable[10],"INTD");
	strcpy(lookuptable[11],"FRAMD");
	strcpy(lookuptable[12],"BEATD");

	logText[0]="System booted";
	logText[1]="FramFormat";
	logText[2]="WiFi";
	logText[3]="Log";
	logText[4]="EraseConfig";
	logText[5]="Payment";
	logText[6]="Connection";
	logText[7]="MQTT";
	logText[8]="PayTable";
	logText[9]="AP Set";
	logText[10]="Reset";
	logText[11]="Timer Delete";
	logText[12]="Timer Sync";
	logText[13]="Heater Off";
	logText[14]="Heater On";
	logText[15]="Reload Reboot";
	logText[16]="Manual On";
	logText[17]="Manual Off";
	logText[18]="Heap Guard";

	string debugs;

	for (int a=0;a<NKEYS/2;a++)
	{
		debugs="-"+string(lookuptable[a]);
		strcpy(lookuptable[a+NKEYS/2],debugs.c_str());
	}

	//		// ADC setup will use ADC1 fixed Default VREF
		    adc1_config_width(ADC_WIDTH_BIT_12);
		    adcchannel=(adc1_channel_t)ADCCHAN;
		    adc1_config_channel_atten(adcchannel, ADC_ATTEN_DB_11); //3300 mv is the target

	//	    //Characterize ADC
		//    adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));
		 //   esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

		    gpio_config_t io_conf;
		    uint64_t mask=1;

			io_conf.intr_type = GPIO_INTR_DISABLE;
			io_conf.mode = GPIO_MODE_OUTPUT;
			io_conf.pull_down_en =GPIO_PULLDOWN_DISABLE;
			io_conf.pull_up_en =GPIO_PULLUP_DISABLE;
			io_conf.pin_bit_mask = (mask<<WIFILED);
			gpio_config(&io_conf);
			io_conf.pin_bit_mask = (mask<<MQTTLED);
			gpio_config(&io_conf);

}


void loadDayBPK(u16 hoy)
{
	if(xSemaphoreTake(framSem, portMAX_DELAY)){
		fram.read_tarif_day(hoy, (u8*)&diaTarifa); // all 24 hours of todays Tariff Types [0..255] of TarifaBPW
		xSemaphoreGive(framSem);
	}
	else
		return;
	if(aqui.traceflag & (1<<BOOTD))
	{
		for (int a=0;a<24;a++)
			printf("[BOOTD]H[%d]=%d ",a,diaTarifa[a]);
		printf("\n");

	}
}

void init_fram()
{
	scratchTypespi scratch;
	// FRAM Setup
	fram.begin(FMOSI,FMISO,FCLK,FCS,&framSem); //will create SPI channel and Semaphore
	framWords=fram.intframWords;
	spi_flash_init();



		if(xSemaphoreTake(framSem, portMAX_DELAY))
		{
			fram.read_recover(&scratch);
			xSemaphoreGive(framSem);
		}

		if (scratch.medidor.state!=0)
		{
			//  check_log_file(); //Our log file. Need to open it before normal sequence
			printf("Recover Fram\n");
			recover_fram();
			//recf=true;
		}
		//all okey in our Fram after this point

		//load all devices counters from FRAM
		for (int a=0;a<MAXDEVS;a++)
			load_from_fram(a);

//		if(xSemaphoreTake(framSem, portMAX_DELAY))
//		{
//			fram.read_tarif_bytes(0, (u8*)&tarifaBPK, sizeof(tarifaBPK)); // read all 100 types of BPK
//			xSemaphoreGive(framSem);
//		}

		if(fram.intframWords>32768)
		{
			//	printf("Call load day \n");
			loadDayBPK(yearDay);
		}

}

void initRtc()
{
	DateTime algo;

	rtc.begin(i2cp.i2cport);		// RTC
	if(xSemaphoreTake(I2CSem, portMAX_DELAY))
	{
		algo=rtc.now();
		xSemaphoreGive(I2CSem);
	}
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<BOOTD))
		printf("[BOOTD]RTC Year %d Month %d Day %d Hora %d Min %d Sec %d Week %d\n",algo.year(),algo.month(),algo.date(),algo.hour(),algo.minute(),algo.second(),algo.dayOfWeek());
#endif
	mesg=oldMesg=algo.month()-1;                       // Global Month
	diag=oldDiag=algo.date()-1;                         // Global Day
	horag=oldHorag=algo.hour()-1-UIO;                      // Global Hour - 5 UIO
	yearg=algo.year();
	if(oldMesg>12 || oldDiag>31 || oldHorag>23) //Sanity check
		oldMesg=oldDiag=1;
	rtcf=true;

	//Now load system time for internal use

	u32 now=algo.getEpoch()-(UIO*3600);
	struct timeval tm;
	tm.tv_sec=now;
	tm.tv_usec=0;
	settimeofday(&tm,0); //Now local time is set. It could be changed from the SNTP if we get a connections else we use this date

	struct tm timeinfo;
	time_t tt;
	time(&tt);
	localtime_r(&tt, &timeinfo);
	yearDay=timeinfo.tm_yday;
	if(aqui.traceflag & (1<<BOOTD))
		printf("[BOOTD]RTC->UNIX Date %s yDay %d\n",makeDateString(tt).c_str(),yearDay);


}

int init_log()
{
	const char *base_path = "/spiflash";
	static wl_handle_t	s_wl_handle=WL_INVALID_HANDLE;
	esp_vfs_fat_mount_config_t mount_config={0,0,0};

	mount_config.max_files=1;
	mount_config.format_if_mount_failed=true;

	loggf=false;

	logQueue=xQueueCreate(10,sizeof(logq));
	// Create Queue
	if(logQueue==NULL)
		return -1;

	logSem= xSemaphoreCreateBinary();
	if(logSem)
		xSemaphoreGive(logSem);  //SUPER important else its born locked
	else
		printf("Cant allocate Log Sem\n");

	esp_err_t err = esp_vfs_fat_spiflash_mount(base_path, "storage", &mount_config, &s_wl_handle);
	if (err != ESP_OK) {
		printf( "Failed to mount FATFS %d \n", err);
		return -1;
	}
	bitacora = fopen("/spiflash/log.txt", "r+");
	if (bitacora == NULL) {
		bitacora = fopen("/spiflash/log.txt", "a");
		if(bitacora==NULL)
		{
			if(aqui.traceflag&(1<<BOOTD))
			printf("[BOOTD]Could not open file\n");
			return -1;
		}
		else
			if(aqui.traceflag&(1<<BOOTD))
				printf("[BOOTD]Opened file append\n");
	}
	loggf=true;

	return ESP_OK;
}

/*
int init_log()
{
	const char *base_path = "/spiflash";
	static wl_handle_t	s_wl_handle=WL_INVALID_HANDLE;
	esp_vfs_fat_mount_config_t mount_config;
	mount_config.max_files=2;
	mount_config.format_if_mount_failed=true;

	logQueue=xQueueCreate(10,sizeof(logq));
		// Create Queue
		if(logQueue==NULL)
			return -1;

		logSem= xSemaphoreCreateBinary();
		if(logSem)
			xSemaphoreGive(logSem);  //SUPER important else its born locked
		else
			printf("Cant allocate Log Sem\n");

	esp_err_t err = esp_vfs_fat_spiflash_mount(base_path, "storage", &mount_config, &s_wl_handle);
	if (err != ESP_OK) {
		printf( "Failed to mount FATFS (0x%x)", err);
		return -1;
	}
	bitacora = fopen("/spiflash/log.txt", "r+");
	if (bitacora == NULL) {
		bitacora = fopen("/spiflash/log.txt", "a");
		if(bitacora==NULL)
		{
			printf("Could not open file\n");
			return -1;
		}
		else
			printf("Opened file append\n");
	}


	return ESP_OK;
}

 */
void init_temp()
{
	//Temp sensors
	numsensors=DS_init(DSPIN,bit9,&sensors[0][0]);
	if(numsensors==0)
		numsensors=DS_init(DSPIN,bit9,&sensors[0][0]); //try again
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<BOOTD))
	{
		printf("[BOOTD]There are %d sensors\n",numsensors);

		for (int a=0;a<numsensors;a++)
		{
			printf("Sensor %d Id=",a);
			for (int b=0;b<8;b++)
				printf("%02x",sensors[a][b]);
			delay(750);
			printf(" Temp:%.02fC\n",DS_get_temp(&sensors[a][0]));
		}
	}
#endif

}

void app_main(void)
{
	esp_log_level_set("*", ESP_LOG_ERROR); //shut up
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
    	printf("No free pages erased!!!!\n");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

	gpio_set_direction((gpio_num_t)0, GPIO_MODE_INPUT);
	delay(3000);
	int rebootl= rtc_get_reset_reason(1); //Reset cause for CPU 1
    // load configuration
	read_flash();

	if (aqui.centinel!=CENTINEL || !gpio_get_level((gpio_num_t)0))
		//	if (aqui.centinel!=CENTINEL )
	{
		printf("Read centinel %x",aqui.centinel);
		erase_config();
	}
    printf("Esp32-Meter\n");
    myMutex = portMUX_INITIALIZER_UNLOCKED;
	initVars(); 			// used like this instead of var init to be able to have independent file per routine(s)
	initI2C();  			// for Screen and RTC
	initScreen();			// Screen
	initRtc();				// RTC until we find out how to use the ESP32 with a Battery
	init_fram();			// Fram Setup
	init_temp();			// Temperature sensors
	init_log();				// Log file management

	//Save new boot count and reset code
	aqui.bootcount++;
	aqui.lastResetCode=rebootl;
	write_to_flash();
	displayf=aqui.pollGroup;
	// Start Main Tasks

	xTaskCreate(&displayManager,"dispMgr",10240,NULL, MGOS_TASK_PRIORITY, NULL);		//Manages all display to LCD
	xTaskCreate(&kbd,"kbd",8192,NULL, MGOS_TASK_PRIORITY, NULL);					// User interface while in development. Erased in RELEASE
	xTaskCreate(&logManager,"log",8192,NULL, MGOS_TASK_PRIORITY, NULL);				// Log Manager
	xTaskCreate(&initWiFi,"log",10240,NULL, MGOS_TASK_PRIORITY, NULL);						// Log Manager

	// Start Monitoring the Meter Lines
}
