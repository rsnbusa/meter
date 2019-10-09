/*
 * framManager.cpp
 *
 *  Created on: Apr 17, 2017
 *      Author: RSN
 */

#include "framManager.h"
#include "framDef.h"

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
//		scratch.medidor.state=0;                                //variables written state
//		fram.write_recover(scratch);
//		scratch.medidor.state=0;                                // done state. OK
//		fram.write_recover(scratch);
		fram.write8(SCRATCHOFF,0); //Fast write first byte of Scratch record to 0=done.

		xSemaphoreGive(framSem);
		printf("Recover %s",textl);
	}
	//    mlog(GENERAL, textl);
}


void write_to_fram(u8 meter,bool adding)
{
	// FRAM Semaphore is taken by the Interrupt Manager. Safe to work.
	scratchTypespi scratch;

	if(aqui.traceflag & (1<<BEATD)) //Should not print. semaphore is taking longer
			printf("[BEATD]Save KWH Meter %d Month %d Day %d Hour %d Year %d lifekWh %d\n",meter,mesg,diag,horag,yearg,theMeters[meter].curLife+1);
	theMeters[meter].curLife++;
	theMeters[meter].curMonth++;
	theMeters[meter].curDay++;
	theMeters[meter].curHour++;
	theMeters[meter].curCycle++;
	time(&theMeters[meter].lastKwHDate); //last time we saved data

	scratch.medidor.state=1;                    //scratch written state. Must be 0 to be ok. Every 800-1000 beats so its worth it
	scratch.medidor.meter=meter;
	scratch.medidor.month=theMeters[meter].curMonth;
	scratch.medidor.life=theMeters[meter].curLife;
	scratch.medidor.day=theMeters[meter].curDay;
	scratch.medidor.hora=theMeters[meter].curHour;
	scratch.medidor.cycle=theMeters[meter].curCycle;
	scratch.medidor.mesg=mesg;
	scratch.medidor.diag=diag;
	scratch.medidor.horag=horag;
	scratch.medidor.yearg=yearg;
	fram.write_recover(scratch);            //Power Failure recover register

	fram.write_beat(meter,theMeters[meter].currentBeat);
	fram.write_lifekwh(meter,theMeters[meter].curLife);
	fram.write_month(meter,mesg,theMeters[meter].curMonth);
	fram.write_monthraw(meter,mesg,theMeters[meter].curMonthRaw);
	fram.write_day(meter,yearg,mesg,diag,theMeters[meter].curDay);
	fram.write_dayraw(meter,yearg,mesg,diag,theMeters[meter].curDayRaw);
	fram.write_hour(meter,yearg,mesg,diag,horag,theMeters[meter].curHour);
	fram.write_hourraw(meter,yearg,mesg,diag,horag,theMeters[meter].curHourRaw);
    fram.write_cycle(meter, mesg,theMeters[meter].curCycle);
//	fram.write_cycle(meter, theMeters[meter].cycleMonth,theMeters[meter].curCycle);
	fram.write_minamps(meter,theMeters[meter].minamps);
	fram.write_maxamps(meter,theMeters[meter].maxamps);
	fram.write_lifedate(meter,theMeters[meter].lastKwHDate);  //should be down after scratch record???
//	scratch.medidor.state=2;            //variables written state
//	fram.write_recover(scratch);

	fram.write8(SCRATCHOFF,0); //Fast write first byte of Scratch record to 0=done.

//	scratch.medidor.state=0;            // done state. OK
//	fram.write_recover(scratch);
}

void load_from_fram(u8 meter)
{
	if(xSemaphoreTake(framSem, portMAX_DELAY))
	{
		fram.read_lifekwh(meter,(u8*)&theMeters[meter].curLife);
		fram.read_lifedate(meter,(u8*)&theMeters[meter].lastKwHDate);
		fram.read_month(meter, mesg, (u8*)&theMeters[meter].curMonth);
		fram.read_monthraw(meter, mesg, (u8*)&theMeters[meter].curMonthRaw);
		fram.read_day(meter, yearg,mesg, diag, (u8*)&theMeters[meter].curDay);
		fram.read_dayraw(meter, yearg,mesg, diag, (u8*)&theMeters[meter].curDayRaw);
		fram.read_hour(meter, yearg,mesg, diag, horag, (u8*)&theMeters[meter].curHour);
		fram.read_hourraw(meter, yearg,mesg, diag, horag, (u8*)&theMeters[meter].curHourRaw);
		fram.read_cycle(meter, mesg, (u8*)&theMeters[meter].curCycle); //should we change this here too and use cycleMonth[meter]?????
		fram.read_beat(meter,(u8*)&theMeters[meter].currentBeat);
		theMeters[meter].oldbeat=theMeters[meter].currentBeat;
		if(aqui.beatsPerKw[meter]==0)
			aqui.beatsPerKw[meter]=800;// just in case div by 0 crash
		u16 nada=theMeters[meter].currentBeat/aqui.beatsPerKw[meter];
		printf("Beatsave Fram %d\n",theMeters[meter].beatSave);
		theMeters[meter].beatSave=theMeters[meter].currentBeat-(nada*aqui.beatsPerKw[meter]);
		theMeters[meter].beatSaveRaw=theMeters[meter].beatSave;
		fram.read_minamps(meter,(u8*)&theMeters[meter].minamps);
		fram.read_maxamps(meter,(u8*)&theMeters[meter].maxamps);
		xSemaphoreGive(framSem);

		if(aqui.traceflag & (1<<BEATD))
			printf("[BEATD]Loaded Meter %d curLife %d\n",meter,theMeters[meter].curLife);
	}
}



