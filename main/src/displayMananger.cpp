/*
 * displayMananger.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: RSN
 */

#include "displayManager.h"

using namespace std;
extern  string makeDateString(time_t t);
extern void write_to_flash();
extern void loadDayBPK(u16 hoy);
extern uint32_t IRAM_ATTR millis();

void clearScreen()
		{
			display.setColor(BLACK);
			display.clear();
			display.setColor(WHITE);
		}

void sendBill(void *pArg)
{
	int *q=(int*)pArg;
	int billsendmeter=*q;

	char textll[130];

	time_t ll;
	time(&ll);
	u16 n=sprintf(textll,"%s!%d!%s!%d!%d",aqui.medidor_id[billsendmeter],theMeters[billsendmeter].curCycle,makeDateString(ll).c_str(),
			theMeters[billsendmeter].curLife,theMeters[billsendmeter].currentBeat);
	//     PRINT_MSG("%s\n",textll);
	//    mlog(GENERAL, "BILL "+String(textl));
	//    fram.write_cycle(billsendmeter, oldMesg, curCycle[billsendmeter]);
	theMeters[billsendmeter].curCycle=0;
	aqui.corteSent[billsendmeter]=true;
	//   fram.write_cycledate(billsendmeter, oldMesg, ll);

#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<CMDD))
		printf("[CMDD]Publish %s\n",spublishTopic.c_str());
#endif
	if (!mqttflag)
	{
		printf("No mqtt\n");
		return;
	}
//	mg_mqtt_publish(globalComm, sCollectionTopic.c_str(), 65, MG_MQTT_QOS(0)|MG_MQTT_RETAIN, textll,n);
}

int getrandom(int desde,int hasta)
{
	uint32_t r;

	while(1)
	{
		r=esp_random();
		if(r>=desde and r<=hasta)
			return r;
	}
}


void setCycleChange(u8 diaHoy, u8 oldDia)
{
	struct  tm timep,timenow;
	time_t now;
	u16 tDay,cDay ; // today and corteday


	time(&now);
	localtime_r(&now,&timenow);
	tDay=timenow.tm_yday;
	memcpy(&timep,&timenow,sizeof(timep));
	//	timep.tm=hour=0;
	//	timep.tm_min=0;
	//	timep.tm_sec=0;

	//oldDia day to save
	// zero curCycle ram
	// set global cycleMonth to next month or this month
	for (int a=0;a<MAXDEVS;a++)
	{
		timep.tm_mday=aqui.diaDeCorte[a];
		now=mktime(&timep);
		localtime_r(&now,&timep);
		cDay=timep.tm_yday;
		printf("mes %d dia %d corte %d tDay %d cDay %d\n",mesg,diaHoy+1,aqui.diaDeCorte[a],tDay,cDay);
		if (tDay>cDay) //corte date reached or exceeded
		{
			if(cDay==(tDay-1))
			{// just happened
				if(!aqui.corteSent[a])
				{
					printf("Hoy\n");
					int ran=getrandom(1000,60000);
					ran=1000;
					xTaskCreate(sendBill,"Bill",1024,(void*)&a, MGOS_TASK_PRIORITY,NULL );

					//launch a task with parameters: delay before sejnding, who si sneding
					//     delay(ran);
					//     send_bill(a,ran);
					//     delay(ran+100); //in case we have more than 1 meter needing to send
					theMeters[a].curCycle=0; //reset to 0 ram variable
				}
				theMeters[a].cycleMonth=mesg;
			}
			else
				if(aqui.corteSent[a])
				{ // reset flag next day
					printf("Reset Day\n");
					aqui.corteSent[a]=false;
					write_to_flash();
				}
		}
		else
			// change cycleMonth
			// to mesg+1 if is Greater than, which means month has not chenage yet
			theMeters[a].cycleMonth=mesg+1;
	}
}
void check_date_change()
{
	time_t now;
	struct tm timep;
	time(&now);
	localtime_r(&now,&timep);
	mesg=timep.tm_mon;   // Global Month
	diag=timep.tm_mday-1;    // Global Day
	yearg=timep.tm_year;     // Global Year
	horag=timep.tm_hour;     // Global Hour
	yearDay=timep.tm_yday;

	if(horag==oldHorag && diag==oldDiag && mesg==oldMesg)
		return;
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<CMDD))
		printf("[CMDD]Hour change mes %d- %d day %d- %d hora %d- %d\n",mesg,oldMesg,diag,oldDiag,horag,oldHorag);
#endif
	if(horag!=oldHorag) // hour change up or down
	{
		for (int a=0;a<MAXDEVS;a++)
		{
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<CMDD))
		printf("[CMDD]Hour change meter %d val %d\n",a,theMeters[a].curHour);
#endif

			if(xSemaphoreTake(framSem, portMAX_DELAY))
			{
				fram.write_hour(a, yearg,oldMesg,oldDiag,oldHorag, theMeters[a].curHour);//write old one before init new
				fram.write_hourraw(a, yearg,oldMesg,oldDiag,oldHorag, theMeters[a].curHourRaw);//write old one before init new
				xSemaphoreGive(framSem);
			}

			theMeters[a].curHour=0; //init it
			theMeters[a].curHourRaw=0;
			u16 oldt=diaTarifa[oldHorag];

			if (diag !=oldDiag)
				loadDayBPK(diag); // load new day values for Tariffs else use next hour

			// calculate remaining Beats to convert to next Tariff IF different
			if(oldt!=diaTarifa[horag])
			{
				u16 oldBeats=theMeters[a].beatsPerkW*diaTarifa[oldHorag]/100;
				float oldTarRemain=(float)(theMeters[a].beatSave-oldBeats);
				float perc=(float)oldTarRemain/(float)oldBeats;
				theMeters[a].beatSave=(int)(perc*(float)(theMeters[a].beatsPerkW*diaTarifa[horag]/100));
			} //else keep counting in the same fashion
			oldHorag=horag;
		}
	}

	if(diag!=oldDiag) // day change up or down. Also hour MUST HAVE CHANGED
	{
		//	setCycleChange(diag,oldDiag);

		for (int a=0;a<MAXDEVS;a++)
		{
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<CMDD))
		printf("[CMDD]Day change mes %d day %d oldday %d corte %d sent %d\n",oldMesg,diag,oldDiag,aqui.diaDeCorte[a],aqui.corteSent[a]);
#endif
			if(xSemaphoreTake(framSem, portMAX_DELAY))
			{
				fram. write_day(a,yearg, oldMesg,oldDiag, theMeters[a].curDay);
				theMeters[a].curDay=0;
				xSemaphoreGive(framSem);
			}


		}
		oldDiag=diag;
	}

	if(mesg!=oldMesg) // month change up or down. What to do with prev Year???? MONTH MUST HAVE CHANGED
	{
		for (int a=0;a<MAXDEVS;a++)
		{
			if(xSemaphoreTake(framSem, portMAX_DELAY))
			{
				fram.write_month(a, oldMesg, theMeters[a].curMonth);
				xSemaphoreGive(framSem);
				theMeters[a].curMonth=0;
			}
		}
		oldMesg=mesg;
	}
}

void drawString(int x, int y, string que, int fsize, int align,displayType showit,overType erase)
{
	if(fsize!=lastFont)
	{
		lastFont=fsize;
		switch (fsize)
		{
		case 10:
			display.setFont(ArialMT_Plain_10);
			break;
		case 12:
			display.setFont(Dialog_plain_12);
			break;
		case 16:
			display.setFont(ArialMT_Plain_16);
			break;
		case 24:
			display.setFont(ArialMT_Plain_24);
			break;
		default:
			break;
		}
	}

	if(lastalign!=align)
	{
		lastalign=align;

		switch (align) {
		case TEXT_ALIGN_LEFT:
			display.setTextAlignment(TEXT_ALIGN_LEFT);
			break;
		case TEXT_ALIGN_CENTER:
			display.setTextAlignment(TEXT_ALIGN_CENTER);
			break;
		case TEXT_ALIGN_RIGHT:
			display.setTextAlignment(TEXT_ALIGN_RIGHT);
			break;
		default:
			break;
		}
	}

	if(erase==REPLACE)
	{
		int w=display.getStringWidth((char*)que.c_str());
		int xx=0;
		switch (lastalign) {
		case TEXT_ALIGN_LEFT:
			xx=x;
			break;
		case TEXT_ALIGN_CENTER:
			xx=x-w/2;
			break;
		case TEXT_ALIGN_RIGHT:
			xx=x-w;
		default:
			break;
		}
		display.setColor(BLACK);
		display.fillRect(xx,y,w,lastFont+3);
		display.setColor(WHITE);
	}

	display.drawString(x,y,(char*)que.c_str());
	if (showit==DISPLAYIT)
		display.display();
}

void drawBars()
{
	if (!connf)
		return;
	wifi_ap_record_t wifidata;
	if (esp_wifi_sta_get_ap_info(&wifidata)==0){
		//		printf("RSSI %d\n",wifidata.rssi);
		RSSI=80+wifidata.rssi;
		if(oldRSSI==RSSI)
			return;

		oldRSSI=RSSI;
	}
	for (int a=0;a<3;a++)
	{
		if (RSSI>RSSIVAL)
			display.fillRect(barX[a],YB-barH[a],WB,barH[a]);
		else
			display.drawRect(barX[a],YB-barH[a],WB,barH[a]);
		RSSI -= RSSIVAL;
	}
	if (mqttflag)
		drawString(16, 5, string("m"), 10, TEXT_ALIGN_LEFT,NODISPLAY, NOREP);
	display.display();
}

void setLogo(string cual)
{
	if(xSemaphoreTake(I2CSem, portMAX_DELAY)) //
	{
		clearScreen();
		drawString(64, 20, cual.c_str(),24, TEXT_ALIGN_CENTER,NODISPLAY, NOREP);
		drawBars();
		xSemaphoreGive(I2CSem);
	}
}

void showKwh(u8 meter)
{
	char local[130];
	if(xSemaphoreTake(I2CSem, portMAX_DELAY)) //
	{
		clearScreen();

		drawString(64,0," KWH",24, TEXT_ALIGN_CENTER,NODISPLAY, NOREP);
		sprintf(local,"M%d",chosenMeter);
		drawString(22,0, string(local),10, TEXT_ALIGN_LEFT,NODISPLAY, NOREP);
		sprintf(local,"%d",theMeters[meter].curLife);
		drawString(64, 24, string(local), 24, TEXT_ALIGN_CENTER,NODISPLAY, NOREP);
		sprintf(local,"%s >> %d",meses[mesg],theMeters[meter].curMonth);
		drawString(64, 48, local, 16, TEXT_ALIGN_CENTER,NODISPLAY, NOREP);
		drawBars();
		xSemaphoreGive(I2CSem);
	}
}

void drawPulses(int meter)
{
	char textl[130];
	if(xSemaphoreTake(I2CSem, portMAX_DELAY)) //
	{
		clearScreen();
		drawString(64, 0, "Pulsos",24, TEXT_ALIGN_CENTER,NODISPLAY, NOREP);
		sprintf(textl,"%d",aqui.MODDISPLAY[meter]);
		drawString(102, 0, string(textl), 10, TEXT_ALIGN_LEFT, NODISPLAY, NOREP);
		sprintf(textl,"%d",meter);
		drawString(102, 16, "M"+string(textl), 10, TEXT_ALIGN_LEFT, NODISPLAY, NOREP);
		sprintf(textl,"%d",theMeters[meter].currentBeat);
		drawString(64, 28, string(textl),16, TEXT_ALIGN_CENTER,NODISPLAY, REPLACE);
		if (mqttf)
			drawString(16, 5, string("m"), 10, TEXT_ALIGN_LEFT,NODISPLAY, NOREP);
		drawBars();
		xSemaphoreGive(I2CSem);
	}

}

void drawPulsesAll()
{
	if(xSemaphoreTake(I2CSem, portMAX_DELAY)) //
	{
		clearScreen();
		xSemaphoreGive(I2CSem);
	}
}



void displayPago(u8 cual,float pago)
{
	u32 corte;
	char textl[130];
	time_t now;
	string s1;

	display.clear();
	display.setColor(WHITE);

	if(xSemaphoreTake(framSem, portMAX_DELAY))
	{
		fram.read_corte(cual, (u8*)&corte);
		xSemaphoreGive(framSem);
	}
	else
		return; // no data to display

	time(&now);
#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<CMDD))
		PRINT_MSG("Corte %d  Hoy %d\n",corte,(uint32_t)now);
#endif
	if (corte<now)
	{
		// reset
		if(xSemaphoreTake(framSem, portMAX_DELAY))
		{
			//portMAX_DELAY
			fram.write_pago(cual, 0.0);
			fram.write_corte(cual, 0);
			xSemaphoreGive(framSem);
		}
		return;
	}
	drawString(64, 0, string(aqui.medidor_id[cual]),16, TEXT_ALIGN_CENTER,DISPLAYIT, NOREP);
	sprintf(textl,"$%.02f",pago);
	drawString(64, 20, string(textl), 24, TEXT_ALIGN_CENTER,DISPLAYIT, REPLACE);
	if(xSemaphoreTake(framSem, portMAX_DELAY))
	{//portMAX_DELAY
		fram.read_corte(cual, (u8*)&corte);
		xSemaphoreGive(framSem);
	}

	s1=makeDateString(corte);
	u8 donde=s1.find(" ");
	s1[donde]=0;
	drawString(64, 48,"  "+ s1+"  ", 16, TEXT_ALIGN_CENTER,DISPLAYIT, REPLACE);
	s1="";
}

void displayCorte(u8 cual,float pago)
{
	char textl[130];
	//	u32 corte;
	clearScreen();
	drawString(64, 0, "Impago",24, TEXT_ALIGN_CENTER,DISPLAYIT, NOREP);
	drawString(64, 26, string(aqui.medidor_id[cual]), 16, TEXT_ALIGN_CENTER,DISPLAYIT, REPLACE);
	sprintf(textl,"-$%.02f",(pago*-1.0));
	drawString(64, 46, string(textl), 16, TEXT_ALIGN_CENTER,DISPLAYIT, REPLACE);
	//    read_corte_fram(cual, (u8*)&corte);
	//    nets=makeDateString(corte);
	//    u8 donde=nets.indexOf(" ");
	//    nets[donde]=0;
	//    drawString(64, 52,"  "+ nets+"  ", 10, TEXT_ALIGN_CENTER,DISPLAYIT, REPLACE);
}


void check_user_info()
{
	// global userNum is the current user to be displayed
	float pago;
	if(xSemaphoreTake(framSem, portMAX_DELAY))
	{//portMAX_DELAY
		fram.read_pago( userNum, (u8*)&pago);
		xSemaphoreGive(framSem);
	}

#ifdef DEBUGMQQT
	if(aqui.traceflag & (1<<CMDD))
		printf("[CMDD]User %d pago %.02f\n",userNum,pago);
#endif
	if(pago<0.0)
		displayCorte(userNum,pago);
	else
		if(pago>0.01)
			displayPago(userNum,pago);
	userNum++;
	if (userNum>MAXDEVS-1)
		userNum=0;
}

void displayData(u8 meter)
{
	char local[130];
	string s1,s2;
	u32 oldMeters[MAXDEVS],oldMetersK[MAXDEVS];
	u8 lsize=16;

	if (displayMode==DISPLAYUSER)
		if(millis()-usertime>10000)
		{
			usertime=millis();
			//	check_user_info();
			return;
		}

	if (displayMode!=oldMode || oldMeter!=chosenMeter )
	{ //Change Headers
		switch (displayMode)
		{
		case DISPLAYPULSES:
			drawPulses(chosenMeter);
			break;
		case DISPLAYKWH:
			showKwh(chosenMeter);
			break;
		case DISPLAYALL:
		case DISPLAYAMPS:
		case DISPLAYUSER:
		case DISPLAYALLK:
			drawPulsesAll();
			break;
		default:
			setLogo("EEQ");
			break;
		}
		oldMode=displayMode;
		oldMeter=chosenMeter;
	}
	switch (displayMode) {
	case DISPLAYPULSES:

		if(aqui.MODDISPLAY[meter]>100 || aqui.MODDISPLAY[meter]==0)
			aqui.MODDISPLAY[meter]=5;
		if(theMeters[meter].currentBeat-theMeters[meter].oldbeat>=aqui.MODDISPLAY[meter])
		{
			theMeters[meter].oldbeat=theMeters[meter].currentBeat;
			sprintf(local,"%d",theMeters[meter].currentBeat);
			if(xSemaphoreTake(I2CSem, portMAX_DELAY)) //
			{
				drawString(64, 28, string(local),16, TEXT_ALIGN_CENTER,DISPLAYIT, REPLACE);
				xSemaphoreGive(I2CSem);
			}
			s1=s2="";
		}
		break;
	case DISPLAYKWH:
		if(aqui.MODDISPLAY[meter]>100 || aqui.MODDISPLAY[meter]==0)
			aqui.MODDISPLAY[meter]=5;
		if(theMeters[meter].currentBeat-theMeters[meter].oldbeat>=aqui.MODDISPLAY[meter])
		{
			theMeters[meter].oldbeat=theMeters[meter].currentBeat;
			sprintf(local,"%d",theMeters[meter].curLife);
			s1=string(local);
			if(xSemaphoreTake(I2CSem, portMAX_DELAY)) //
			{
				drawString(64, 24,s1, 24, TEXT_ALIGN_CENTER,DISPLAYIT, REPLACE);
				sprintf(local," %4d",theMeters[meter].beatSave);
				s1=string(local);
				drawString(128, 0, s1, 10, TEXT_ALIGN_RIGHT,DISPLAYIT, REPLACE);
				sprintf(local,"%s >> %d",meses[mesg+1],theMeters[meter].curMonth);
				s1=string(local);
				drawString(64, 48, s1, 16, TEXT_ALIGN_CENTER,DISPLAYIT, REPLACE);
				s1="";
				xSemaphoreGive(I2CSem);
			}
		}
		break;

	case DISPLAYUSER:
		if(xSemaphoreTake(I2CSem, portMAX_DELAY)) //
		{
			sprintf(local,"Meter:%s ",aqui.meterName);
			drawString(2, 14, string(local),10, TEXT_ALIGN_LEFT,NODISPLAY, REPLACE);
			sprintf(local,"[Firmware %s]\n",(char*)compile_date);
			drawString(2, 26, string(local),10, TEXT_ALIGN_LEFT,NODISPLAY, REPLACE);
			sprintf(local,"BootCount %d",aqui.bootcount);
			drawString(2, 38, string(local),10, TEXT_ALIGN_LEFT,NODISPLAY, REPLACE);
			drawBars();
			display.display();
			xSemaphoreGive(I2CSem);
	}
		break;

	case DISPLAYALL:
		if(theMeters[0].currentBeat>oldMeters[0] ||theMeters[1].currentBeat>oldMeters[1] || theMeters[2].currentBeat>oldMeters[2] || theMeters[3].currentBeat>oldMeters[3] )
		{
			oldMeters[0]=theMeters[0].currentBeat;
			oldMeters[1]=theMeters[1].currentBeat;
			oldMeters[2]=theMeters[2].currentBeat;
			oldMeters[3]=theMeters[3].currentBeat;
			if(oldMeters[0]>9999 || oldMeters[1]>9999 || oldMeters[2]>9999 || oldMeters[3]>9999 )
				lsize=12;
			else
				lsize=16;
			sprintf(local,"%5dp  %5dp  ",theMeters[0].currentBeat,theMeters[1].currentBeat);
			if(xSemaphoreTake(I2CSem, portMAX_DELAY)) //
			{
				drawString(64, 16, string(local),lsize, TEXT_ALIGN_CENTER,NODISPLAY, REPLACE);
				sprintf(local,"%5dp  %5dw  ",theMeters[2].currentBeat,theMeters[3].currentBeat);
				drawString(64, 35, string(local),lsize, TEXT_ALIGN_CENTER,NODISPLAY, REPLACE);
				drawBars();
				display.display();
				xSemaphoreGive(I2CSem);
			}
		}
		break;

	case DISPLAYALLK:
		if(theMeters[0].curLife>oldMetersK[0] ||theMeters[1].curLife>oldMetersK[1] || theMeters[2].curLife>oldMetersK[2] || theMeters[3].curLife>oldMetersK[3] )
		{
			oldMetersK[0]=theMeters[0].curLife;
			oldMetersK[1]=theMeters[1].curLife;
			oldMetersK[2]=theMeters[2].curLife;
			oldMetersK[3]=theMeters[3].curLife;
			sprintf(local,"%5dK  %5dK  ",theMeters[0].curLife,theMeters[1].curLife);
			if(xSemaphoreTake(I2CSem, portMAX_DELAY)) //
			{
				drawString(64, 14, string(local),16, TEXT_ALIGN_CENTER,NODISPLAY, REPLACE);
				sprintf(local,"%5dK  %5dK  ",theMeters[2].curLife,theMeters[3].curLife);
				drawString(64, 34, string(local),16, TEXT_ALIGN_CENTER,NODISPLAY, REPLACE);
				drawBars();
				display.display();
				xSemaphoreGive(I2CSem);
			}
		}
		break;

	case DISPLAYAMPS:
		if(xSemaphoreTake(I2CSem, portMAX_DELAY)) //
					{
		if(theMeters[0].msNow>0 && theMeters[1].msNow>0 )
		{
			sprintf(local,"%3dA  %3dA  ",(int)((float)(4500/theMeters[0].msNow)*2.624),(int)((float)(4500/theMeters[1].msNow)*2.624));
			drawString(64, 14, string(local),16, TEXT_ALIGN_CENTER,NODISPLAY, REPLACE);
		}
		if(theMeters[2].msNow>0){
		sprintf(local,"%3dA           ",(int)((float)(4500/theMeters[2].msNow)*2.624));
		drawString(64, 34, string(local),16, TEXT_ALIGN_CENTER,NODISPLAY, REPLACE);
		}
		drawBars();
		display.display();
		xSemaphoreGive(I2CSem);
				}
		break;
	default:
		break;
	}
}

void timerManager(void *arg) {
	time_t t = 0;
	struct tm timeinfo ;
	char textd[20],textt[20];
	u32 nheap;

	while(true)
	{
		nheap=xPortGetFreeHeapSize();

		if(aqui.traceflag & (1<<HEAPD))
			printf("[HEAPD]Heap %d\n",nheap);

		vTaskDelay(1000/portTICK_PERIOD_MS);
		time(&t);
		localtime_r(&t, &timeinfo);

		if (displayf)
		{
			sprintf(textd,"%02d/%02d/%04d",timeinfo.tm_mday,timeinfo.tm_mon+1,1900+timeinfo.tm_year);
			sprintf(textt,"%02d:%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
			if(displayMode!=DISPLAYKWH)
				if(xSemaphoreTake(I2CSem, portMAX_DELAY))
				{
					drawString(16, 5, mqttf?string("m"):string("   "), 10, TEXT_ALIGN_LEFT,NODISPLAY, REPLACE);
					drawString(0, 51, string(textd), 10, TEXT_ALIGN_LEFT,NODISPLAY, REPLACE);
					drawString(86, 51, string(textt), 10, TEXT_ALIGN_LEFT,NODISPLAY, REPLACE);
					drawString(61, 51, aqui.working?"On  ":"Off", 10, TEXT_ALIGN_LEFT,DISPLAYIT, REPLACE);
					xSemaphoreGive(I2CSem);
				}
		}
// check for running water
		// now - lasttime >X minutes

	}
}

void timerCallback( TimerHandle_t xTimer )
{
	check_date_change();
	if(firstTimeTimer)
	{
		xTimerGenericCommand(xTimer,tmrCOMMAND_CHANGE_PERIOD,3600000 /portTICK_PERIOD_MS,NULL,0); //every hour from now on
		firstTimeTimer=false;
	}

}

void displayManager(void *arg) {

	xTaskCreate(&timerManager,"timeMgr",2048,NULL, MGOS_TASK_PRIORITY, NULL);

	//Launch timer for date/hour change
	time_t now;
	int van=20;
	while(!timef && van) //wait for SNTP Time. Could fail so count Xsecs and get the time set by RTC
	{
		van--;
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
	firstTimeTimer=true;

	time(&now);
	int faltan=3600- now % 3600+2; //second to next hour +2 secs
	if(aqui.traceflag & (1<<BOOTD))
		printf("[BOOTD]Secs to Hour %d\n",faltan);

	hourChange=xTimerCreate("OpenTimer",faltan*1000 /portTICK_PERIOD_MS,pdTRUE,( void * ) 0,&timerCallback);
	if(hourChange==NULL)
		printf("Failed to create HourChange timer\n");
	xTimerStart(hourChange,0); //Start it

	if (aqui.DISPTIME==0)
		aqui.DISPTIME=DISPMNGR;


	while (true) {
		if(aqui.pollGroup)
		{
//			if(xSemaphoreTake(I2CSem, portMAX_DELAY)) //
//			{
				displayData(chosenMeter);
//				xSemaphoreGive(I2CSem);
//			}
		}
		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}
