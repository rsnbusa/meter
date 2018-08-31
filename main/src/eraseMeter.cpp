/*
 * eraseMeter.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: RSN
 */

#include "eraseMeter.h"
extern void write_to_flash();
extern void postLog(int code, string mensaje);

void erase_config() //do the dirty work
{
	memset(&aqui,0,sizeof(aqui));
	aqui.centinel=CENTINEL;
	aqui.DISPTIME=DISPMNGR;
	aqui.ssl=0;
	memcpy(aqui.mqtt,MQTTSERVER,sizeof(MQTTSERVER));//fixed mosquito server
	aqui.mqtt[sizeof(MQTTSERVER)]=0;
	aqui.mqttport=MQTTPORT;
	printf("Mqtt Erase %s\n",aqui.mqtt);
	memcpy(aqui.domain,"feediot.co.nf",13);// mosquito server feediot.co.nf
	aqui.domain[13]=0;
	aqui.beatsPerKw[0]=aqui.beatsPerKw[1]=aqui.beatsPerKw[2]=800;//old way
	aqui.bounce[0]=aqui.bounce[1]=aqui.bounce[2]=100;
	aqui.bounce[3]=450;//water meter 450 pulses perliter
	//    fram.write_tarif_bpw(0, 800); // since everything is going to be 0, BPW[0]=800 HUMMMMMM????? SHould load Tariffs after this
	write_to_flash();
	//	if(  xSemaphoreTake(logSem, portMAX_DELAY))
	//	{
	//		fclose(bitacora);
	//	    bitacora = fopen("/spiflash/log.txt", "w"); //truncate
	//	    if(bitacora)
	//	    {
	//	    	fclose(bitacora); //Close and reopen r+
	//		    bitacora = fopen("/spiflash/log.txt", "r+");
	//		    if(!bitacora)
	//		    	printf("Could not reopen logfile\n");
	//		    else
	//			    postLog(0,"Log cleared");
	//	    }
	//	    xSemaphoreGive(logSem);
	//	}
	printf("Centinel %x\n",aqui.centinel);
}



