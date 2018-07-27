/*
 * framManager.cpp
 *
 *  Created on: Apr 17, 2017
 *      Author: RSN
 */

#include "framManager.h"


void recover_fram()
{
	char textl[100];

	scratchTypespi scratch;
	if(xSemaphoreTake(framSem, portMAX_DELAY))
	{
		fram.read_recover(&scratch);

		if (scratch.medidor.state==0)
		{
			xSemaphoreGive(framSem);
			return;
		}

		sprintf(textl,"PF Recover. Meter %d State %d Life %x\n",scratch.medidor.meter,scratch.medidor.state,scratch.medidor.life);
		fram.write_lifekwh(scratch.medidor.meter,scratch.medidor.life);
		fram.write_month(scratch.medidor.meter,scratch.medidor.mesg,scratch.medidor.month);
		fram.write_day(scratch.medidor.meter,scratch.medidor.yearg,scratch.medidor.mesg,scratch.medidor.diag,scratch.medidor.day);
		fram.write_hour(scratch.medidor.meter,scratch.medidor.yearg,scratch.medidor.mesg,scratch.medidor.diag,scratch.medidor.horag,scratch.medidor.hora);
		fram.write_cycle(scratch.medidor.meter, scratch.medidor.mesg,scratch.medidor.cycle);
		scratch.medidor.state=2;                                //variables written state
		fram.write_recover(scratch);
		scratch.medidor.state=0;                                // done state. OK
		fram.write_recover(scratch);
		xSemaphoreGive(framSem);
		printf("Recover %s",textl);
	}
	//    mlog(GENERAL, textl);
}


void write_to_fram(u8 meter,bool adding)
{
	scratchTypespi scratch;
	if(aqui.traceflag & (1<<BEATD))
						printf("[BEATD]Save KWH Meter %d Month %d Day %d Hour %d Year %d\n",meter,mesg,diag,horag,yearg);
	if(adding)
	{
		curLife[meter]++;// ONE kWh
		curMonth[meter]++;
		curDay[meter]++;
		curHour[meter]++;
		curCycle[meter]++; // Corte a Corte counter
		curCycle[meter]++; // Corte a Corte counter ??? twice???
		// lastBeatDate[meter]=now();
		fram.write_lifedate(meter,lastBeatDate[meter]);  //should be down after scratch record???
	}
	scratch.medidor.state=1;                    //scratch written
	scratch.medidor.meter=meter;
	scratch.medidor.month=curMonth[meter];
	scratch.medidor.life=curLife[meter];
	scratch.medidor.day=curDay[meter];
	scratch.medidor.hora=curHour[meter];
	scratch.medidor.cycle=curCycle[meter];
	scratch.medidor.mesg=mesg;
	scratch.medidor.diag=diag;
	scratch.medidor.horag=horag;
	scratch.medidor.yearg=yearg;
	fram.write_recover(scratch);            //Power Failure recover register
	fram.write_beat(meter,currentBeat[meter]);
	fram.write_lifekwh(meter,curLife[meter]);
	fram.write_month(meter,mesg,curMonth[meter]);
	fram.write_day(meter,yearg,mesg,diag,curDay[meter]);
	fram.write_hour(meter,yearg,mesg,diag,horag,curHour[meter]);
	//  fram.write_cycle(meter, mesg,curCycle[meter]);
	fram.write_cycle(meter, cycleMonth[meter],curCycle[meter]);
	fram.write_minamps(meter,minamps[meter]);
	fram.write_maxamps(meter,maxamps[meter]);
	scratch.medidor.state=2;            //variables written state
	fram.write_recover(scratch);
	scratch.medidor.state=0;            // done state. OK
	fram.write_recover(scratch);
}

void load_from_fram(u8 meter)
{
	if(xSemaphoreTake(framSem, portMAX_DELAY))
	{
		fram.read_lifekwh(meter,(u8*)&curLife[meter]);
		fram.read_lifedate(meter,(u8*)&lastBeatDate[meter]);
		fram.read_month(meter, mesg, (u8*)&curMonth[meter]);
		fram.read_day(meter, yearg,mesg, diag, (u8*)&curDay[meter]);
		fram.read_hour(meter, yearg,mesg, diag, horag, (u8*)&curHour[meter]);
		fram.read_cycle(meter, mesg, (u8*)&curCycle[meter]); //should we change this here too and use cycleMonth[meter]?????
		fram.read_beat(meter,(u8*)&currentBeat[meter]);
		oldBeat[meter]=currentBeat[meter];
		if(aqui.beatsPerKw[meter]==0)
			aqui.beatsPerKw[meter]=800;// just in case div by 0 crash
		u16 nada=currentBeat[meter]/aqui.beatsPerKw[meter];
		beatSave[meter]=currentBeat[meter]-(nada*aqui.beatsPerKw[meter]);
		fram.read_minamps(meter,(u8*)&minamps[meter]);
		fram.read_maxamps(meter,(u8*)&maxamps[meter]);
		xSemaphoreGive(framSem);
	}
}



