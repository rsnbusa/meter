/*
 * readFlash.cpp
 *
 *  Created on: Apr 16, 2017
 *      Author: RSN
 */

#include "connectMgr.h"

extern void postLog(int code,int code1,string mensaje);

void conectManager(u8 met, u8 como)
{
    u32 corte;
    char textl[40];
    float pago;
    aqui.breakers[met]=como;
    write_to_flash();
	gpio_set_level((gpio_num_t)breakerPin[met], como);

    if(!como)
    {
        time((time_t*)&corte);
    	if(xSemaphoreTake(framSem, 1000))
    	{//portMAX_DELAY
        fram.write_corte(met,corte);
        fram.read_pago(met, (u8*)&pago);
        pago *=-1;
        fram.write_pago(met , pago);
    	xSemaphoreGive(framSem);
    	}
        printf("IMPAGO %.02f\n",pago);
        sprintf(textl,"Meter %s was Disconnected",aqui.medidor_id[met]);
        postLog(CONL,0,string(textl));
    }
    else
    {
    	if(xSemaphoreTake(framSem, 1000))
    	{//portMAX_DELAY
        fram.write_corte(met,0); // reset it
        fram.write_pago(met , 0.0);
    	xSemaphoreGive(framSem);
    	}
    }
}
