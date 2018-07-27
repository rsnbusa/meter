/*
 * kbd.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: RSN
 */
#include "kbd.h"
#include "FramSPI.h"
#include "framDef.h"

extern void show_config( u8 meter, bool full);
extern void load_from_fram(u8 meter);
extern void write_to_flash();
extern void write_to_fram(u8 meter,bool adding);
extern string makeDateString(time_t t);
extern void delay(uint16_t a);
/*
void test_write(int add, uint8_t valor,int len)
{
	uint8_t comp;
 	 for (int a=0;a<len;a++)
 	 {
 		 comp=fram.read8(add+a);
 		 if(comp!=valor)
 		 {
 			 printf("Break at %d V[%d]!=C[%d]\n",a,valor,comp);
 			 return;
 		 }
 	 }
 	 printf("Test of %d bytes is Ok\n",len);
}
 */

int keyfromstring(char *key)
{

    int i;
    for (i=0; i < NKEYS; i++) {

        if (strcmp(lookuptable[i], key) == 0)
            return i;
    }
    return 100;
}

string get_string(uart_port_t uart_num,u8 cual)
{
	uint8_t ch;
	char dijo[20];
	int son=0,len;
	memset(dijo,0,20);
	while(1)
	{
		len = uart_read_bytes(UART_NUM_0, (uint8_t*)&ch, 1,4/ portTICK_RATE_MS);
		if(len>0)
		{
			if(ch==cual)
				return string(dijo);

			else
				dijo[son++]=ch;
			if (son>sizeof(dijo)-1)
				son=sizeof(dijo)-1;
		}

		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}

void kbd(void *arg) {
	int len,ret,cualf;
	u8 valor8;
	uart_port_t uart_num = UART_NUM_0 ;
	uint32_t add;
	int erri,tlen,comp,hasta,valor;
	uint32_t beat;
	uint8_t *pattern;
	char textl[50];
	char data[50];
	string s1;
	esp_err_t q ;
    time_t t;
	uint16_t errorcode,code1;
	int total;
	char lastcmd=10;

	uart_config_t uart_config = {
			.baud_rate = 115200,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
			.rx_flow_ctrl_thresh = 122,
	};
	uart_param_config(uart_num, &uart_config);
	uart_set_pin(uart_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	esp_err_t err= uart_driver_install(uart_num, 1024 , 1024, 10, NULL, 0);
	if(err!=ESP_OK)
		printf("Error UART Install %d\n",err);
//	else
	//	printf("Uart installed %d\n",uart_num);
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = (1L<<15);
	io_conf.pull_down_en =GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en =GPIO_PULLUP_DISABLE;
	gpio_config(&io_conf);

	while(1)
	{
		len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
		if(len>0)
		{
			if(data[0]==10)
				data[0]=lastcmd;
			lastcmd=data[0];
		//	data[len]=0;
			switch(data[0])
			{
			case '6':
				printf("SetHi\n");
				gpio_set_level((gpio_num_t)15, 0);
				delay(10);
				gpio_set_level((gpio_num_t)15, 1);
				break;
			case '5'://All days in Month
				printf("Days in Month search\nMonth(0-11):");
				fflush(stdout);
				s1=get_string((uart_port_t)uart_num,10);
				len=atoi(s1.c_str());
				if(len<0 || len>11){
					printf("Invalid month\n");
					break;
				}
				if(xSemaphoreTake(framSem, portMAX_DELAY))		{
					for (int a=0;a<daysInMonth[len];a++)
					{
						fram.read_day(chosenMeter,yearg,len, a, (u8*)&valor);
						if(valor>0)
							printf("M[%d]D[%d]=%d ",len,a,valor);
					}
					xSemaphoreGive(framSem);

				}
					printf("\n");
						break;
			case '4': //All Hours in Day
				printf("Hours in Day search\nMonth(0-11):");
				fflush(stdout);
				s1=get_string((uart_port_t)uart_num,10);
				len=atoi(s1.c_str());
				if(len<0 || len>11){
					printf("Invalid month\n");
					break;
				}
				printf("Day(0-%d):",daysInMonth[len]);
				fflush(stdout);
				s1=get_string((uart_port_t)uart_num,10);
				ret=atoi(s1.c_str());
				if(ret<0 || ret>daysInMonth[len]){
					printf("Invalid Day range\n");
					break;
				}
				if(xSemaphoreTake(framSem, portMAX_DELAY))		{
					for (int a=0;a<24;a++)
					{
						fram.read_hour(chosenMeter, yearg,len, ret, a, (u8*)&valor);
						if(valor>0)
							printf("M[%d]D[%d]H[%d]=%d ",len,ret,a,valor);
					}
					xSemaphoreGive(framSem);

				}
					printf("\n");
						break;
			case '3': //Hour search
				printf("Month-Day-Hour search\nMonth(0-11):");
				fflush(stdout);
				s1=get_string((uart_port_t)uart_num,10);
				len=atoi(s1.c_str());
				if(len<0 || len>11){
					printf("Invalid month\n");
					break;
				}
				printf("Day(0-%d):",daysInMonth[len]);
				fflush(stdout);
				s1=get_string((uart_port_t)uart_num,10);
				ret=atoi(s1.c_str());
				if(ret<0 || ret>daysInMonth[len]){
					printf("Invalid Day range\n");
					break;
				}
				printf("Hour(0-23):");
				fflush(stdout);
				s1=get_string((uart_port_t)uart_num,10);
				cualf=atoi(s1.c_str());
				if(cualf<0 || cualf>23){
					printf("Invalid Hour range\n");
					break;
				}
				if(xSemaphoreTake(framSem, portMAX_DELAY))
				{
					fram.read_day(chosenMeter, yearg,len, ret, (u8*)&valor);
					xSemaphoreGive(framSem);
					if(valor>0)
						printf("Date %d/%d/%d=%d\n",yearg,len,ret,valor);
				}
						break;
			case '2':
				printf("Month-Day search\nMonth:");
				fflush(stdout);
				s1=get_string((uart_port_t)uart_num,10);
				len=atoi(s1.c_str());
				if(len<0 || len>11){
					printf("Invalid month\n");
					break;
				}
				printf("Day:");
				fflush(stdout);
				s1=get_string((uart_port_t)uart_num,10);
				ret=atoi(s1.c_str());
				if(ret<0 || ret>daysInMonth[len]){
					printf("Invalid day range\n");
					break;
				}

				if(xSemaphoreTake(framSem, portMAX_DELAY))
				{
					fram.read_day(chosenMeter, yearg,len, ret, (u8*)&valor);
					xSemaphoreGive(framSem);
					printf("Date %d/%d/%d=%d\n",yearg,len,ret,valor);
				}
						break;
			case '1':
				total=0;
				printf("Months Readings\n");
				if(xSemaphoreTake(framSem, portMAX_DELAY))		{
					for (int a=0;a<12;a++)
					{
						fram.read_month(chosenMeter, a, (u8*)&valor);
						if(valor>0)
							printf("M[%d]=%d ",a,valor);
						total+=valor;
					}
					xSemaphoreGive(framSem);
					printf("\nTotal %d\n",total);
				}
				break;
			case '0':
					printf("New SSID Pos:");
					fflush(stdout);
					s1=get_string((uart_port_t)uart_num,10);
					len=atoi(s1.c_str());
					printf("SSID Name:");
					fflush(stdout);
					s1=get_string((uart_port_t)uart_num,10);
					memset((void*)&aqui.ssid[len][0],0,sizeof(aqui.ssid[len]));
					memcpy((void*)&aqui.ssid[len][0],(void*)s1.c_str(),s1.length());//without the newline char
					printf("SSID Password:");
					fflush(stdout);
					s1=get_string((uart_port_t)uart_num,10);
					memset((void*)&aqui.pass[len][0],0,sizeof(aqui.pass[len]));
					memcpy((void*)&aqui.pass[len][0],(void*)s1.c_str(),s1.length());//without the newline char

					curSSID=aqui.lastSSID=0;
					write_to_flash();
					break;
			case 'Z':
			{
				uart_write_bytes((uart_port_t)uart_num,"Format Fram Confirm:",20);
				do{
					len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				} while (len==0);
				printf("%c",data[0]);
				fflush(stdout);

				if (data[0]=='y')
				{
					uart_write_bytes((uart_port_t)uart_num," Fill Value:",12);
					do{
						len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
					} while (len==0);
					data[len]=0;
					valor=atoi(data);
					printf("%d ",valor);
					fflush(stdout);
					uart_write_bytes((uart_port_t)uart_num,"Fast?",5);
					do{
						len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
					} while (len==0);
					printf("%c",data[0]);
					fflush(stdout);


					if(xSemaphoreTake(framSem, 1000))
					{
						if (data[0]=='y')
							ret=fram.format(valor,ota_write_data,2000);
						else
							ret=fram.formatSlow(valor);
						if(ret!=0)
							printf("Format failed ");
						xSemaphoreGive(framSem);
						printf("Done\n");
					}
					else
						printf("Resource locked\n");
				}

				memset(&currentBeat,0,sizeof(currentBeat)); //counter to zero
				for (int a=0;a<MAXDEVS;a++)
				{
					load_from_fram(a);
					oldTime[a]=0;
					minTime[a]=maxTime[a]=maxbeatTime[a]=0;
					minbeatTime[a]=99999;
					comofue[a]=0;
					maxPower[a]=0.0;
					msPower[a]=99999;
					quien[a].fcount=0;
					quien[a].beatc=0;
					quien[a].portid=a;
					quien[a].fullkwh=0;
				}

				break;
			}

			case 'L':
					fclose(bitacora);
					bitacora = fopen("/spiflash/log.txt", "w");//truncate to 0 len
					fclose(bitacora);
					printf("Log cleared\n");
					break;
			case 'l':
				printf("Log:\n");
				fseek(bitacora,0,SEEK_SET);
				while(1)
				{
					//read date
					add=fread(&t,1,4,bitacora);
					if(add==0)
						break;
					//read code
					add=fread(&errorcode,1,2,bitacora);
					if(add==0)
						break;
					add=fread(&code1,1,2,bitacora);
					if(add==0)
						break;
					add=fread(&textl,1,20,bitacora);
					if(add==0)
						{printf("Could not read text log\n");
						break;
						};

					printf("Date %s|Code %d|%s|Code1 %d|%s\n",makeDateString(t).c_str(),errorcode,logText[errorcode].c_str(),code1,textl);
				}
				break;
			case 'y':
			{
				uart_write_bytes((uart_port_t)uart_num,"Meter:",6);
				do{
					len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				} while (len==0);
				data[len]=0;

				add=atoi(data);
				printf("%d\n",add);
				beat=0;
				xSemaphoreTake(framSem, portMAX_DELAY);//portMAX_DELAY
				fram.read_beat(add,(uint8_t*)&beat);
				xSemaphoreGive(framSem);
				printf("Beat[%d]=%06x-%d current=%d\n",add,beat,beat,currentBeat[add]);
				break;
			}
			case 'q':
			case 'Q':{
				showf=!showf;
				printf("Show %s\n",showf?"ON":"OFF");
				break;
			}
			case 'v':
			case 'V':{
				printf("Trace Flags ");
				for (int a=0;a<NKEYS/2;a++)
					if (aqui.traceflag & (1<<a))
					{
						if(a<(NKEYS/2)-1)
							printf("%s-",lookuptable[a]);
						else
							printf("%s",lookuptable[a]);
					}
				printf("\nEnter TRACE FLAG:");
				fflush(stdout);
				s1=get_string((uart_port_t)uart_num,10);
				transform(s1.begin(), s1.end(),s1.begin(), ::toupper);
				if(strcmp(s1.c_str(),"NONE")==0)
				{
					aqui.traceflag=0;
					write_to_flash();
					break;
				}
				if(strcmp(s1.c_str(),"ALL")==0)
				{
					aqui.traceflag=0xFFFF;
					write_to_flash();
					break;
				}
				cualf=keyfromstring((char*)s1.c_str());
				if(cualf==100)
				{
					printf("Invalid Debug Option\n");
					break;
				}
				if(cualf<NKEYS/2 )
				{
					printf("Debug Key Pos %d %s added\n",cualf,s1.c_str());
					aqui.traceflag |= 1<<cualf;
					write_to_flash();
					break;
				}
				else
				{
					cualf=cualf-NKEYS/2;
					printf("Debug Key Pos %d %s removed\n",cualf,s1.c_str());
					aqui.traceflag ^= (1<<cualf);
					write_to_flash();
					break;
				}

				}

			case 'T':{
				printf("Temp Sensor #:");
				fflush(stdout);
				do{
					len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				} while(len==0);
				add=atoi(data);
				if (add>=numsensors)
				{
					printf("Out of range[0-%d]\n",numsensors-1);
					break;
				}
				printf("%d\nId=",add);
				for (int a=0;a<8;a++)
					printf("%02x",sensors[add][a]);
				printf(" Temp: %0.1f\n",DS_get_temp(&sensors[add][0]));
				break;}
			case 'c':{
				for (int a=0;a<MAXDEVS;a++)
					printf("C[%d]=%d ",a,currentBeat[a]);
				printf("\n");
				for (int a=0;a<MAXDEVS;a++)
					printf("C1[%d]=%d ",a,quien[a].fcount);
					printf("\n");
				break;}
			case 'z':{
				memset(&currentBeat,0,sizeof(currentBeat));
				for (int a=0;a<MAXDEVS;a++){
					quien[a].fcount=0;
					quien[a].beatc=0;
					quien[a].fullkwh=0;
				}
				//	count[0]=count[1]=count[2]=0;
				break;}
			case 'f':{
				show_config(chosenMeter, true) ;
				break;}
			case 'h':
			case 'H':{
				DateTime algo=rtc.now();
				printf("Year %d Month %d Day %d Hora %d Min %d Sec %d Week %d\n",algo.year(),algo.month(),algo.date(),algo.hour(),algo.minute(),algo.second(),algo.dayOfWeek());
				break;}
			case 'd':
			case 'D':
			{
				printf("Display Mode:");
				fflush(stdout);
				do{
					len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				} while(len==0);
				add=atoi(data);
				printf("%d\n",add);
				displayMode=(displayModeType)add;
				break;
			}
			case 'b':
			case 'B':
			{
				printf("Display Resolution:");
				fflush(stdout);
				do{
					len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				} while(len==0);
				add=atoi(data);
				printf("%d\n",add);
				aqui.MODDISPLAY[chosenMeter]=add;
				write_to_flash();
			}
			break;
			case 's':
				fclose(bitacora);
				bitacora = fopen("/spiflash/log.txt", "r+");
				if(bitacora==NULL)
					printf("Could reopen file\n");
				break;
			case 'S':
			{
				printf("Commit?");
				fflush(stdout);
				do{
					len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				} while(len==0);
				printf("%c\n",data[0]);
				if (data[0]=='y')
				{
					for (int a=0;a<MAXDEVS;a++)
						write_to_fram(a,false);
				}

					q=nvs_set_blob(nvshandle,"config",(void*)&aqui,sizeof(aqui));
					if (q ==ESP_OK)
						q = nvs_commit(nvshandle);
			}

			break;
			case 'm':
			case 'M':
			{
				printf("Choose Meter:");
				fflush(stdout);
				do{
					len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				} while(len==0);
				add=atoi(data);
				printf("%d\n",add);
				if (add>(MAXDEVS-1))
					printf("Meter out of Range[0-%d]\n",MAXDEVS-1);
				else
					chosenMeter=add;

			}
			break;
			default:
				printf("No cmd\n");
				break;

				// FRAM stuff
			case 'r':
				uart_write_bytes((uart_port_t)uart_num,"ReadFram8 Address:",18);
				do{
					len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				} while(len==0);
				add=atoi(data);
				printf("%d=",add);
				if (add>framWords)
					printf("[%d] exceeds Fram size %d\n",add,framWords);
				else
				{
					//	erri=fram.read8(add);
					xSemaphoreTake(framSem, portMAX_DELAY);//portMAX_DELAY
					erri=fram.read8(add,(uint8_t*)data);
					xSemaphoreGive(framSem);
					if(erri!=0)
						printf("Error%d\n",erri);
					else
						printf("%d\n",data[0]);
				}
				break;
			case 'R':
			//	memset(mpointer,0,sizeof(10250));
				uart_write_bytes((uart_port_t)uart_num,"ReadMany Address:",17);
				do{
					len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				} while (len==0);
				data[len]=0;

				add=atoi(data);
				printf("%d\n",add);
				if (add>framWords)
					printf("[%d] exceeds Fram size %d\n",add,framWords);
				else
				{
					uart_write_bytes((uart_port_t)uart_num,"Length:",7);
					do{
						len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
					} while (len==0);
					data[len]=0;

					tlen=atoi(data);
					printf("%d\n",tlen);
					uart_write_bytes((uart_port_t)uart_num,"Expected:",9);
					do{
						len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
					} while (len==0);
					data[len]=0;
					comp=atoi(data);
					printf("%d\n",comp);

					pattern=(uint8_t*)malloc(tlen);
					memset(pattern,comp,tlen);

					uart_write_bytes((uart_port_t)uart_num,"Dump:",5);
					do{
						len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
					} while (len==0);
					data[len]=0;
					hasta=atoi(data);
					printf("%d\n",hasta);

					//fram.readMany(add,mpointer,tlen);
					xSemaphoreTake(framSem, portMAX_DELAY);//portMAX_DELAY
					fram.readMany(add,bbuffer,tlen);
					xSemaphoreGive(framSem);//portMAX_DELAY

					for (int a=0;a<hasta;a++)
						printf("Read[%d]=%d\n",a,bbuffer[a]);
					add=memcmp(bbuffer,pattern,tlen);
					if(add==0)
						printf("Ok\n");
					else
						printf("No\n");
					free(pattern);
				}
				break;
			case 'w':
				uart_write_bytes((uart_port_t)uart_num,"WriteFram8 Valor:",17);
				do{
					len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				} while (len==0);
				data[len]=0;
				valor=atoi(data);
				len=sprintf(textl,"%d Address:",valor);
				uart_write_bytes((uart_port_t)uart_num,textl,len);

				do{
					len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				} while (len==0);
				data[len]=0;
				add=atoi(data);
				printf("%d\n",add);
				if (add>framWords)
					printf("[%d] exceeds Fram size %d\n",add,framWords);
				else
				{
					//	erri=fram.write8(add,valor);
					xSemaphoreTake(framSem, portMAX_DELAY);//portMAX_DELAY
					erri=fram.write8(add,valor);
					xSemaphoreGive(framSem);

					if(erri!=ESP_OK)
						printf("error writing %d\n",erri);
				}
				break;
			case 'W':
				uart_write_bytes((uart_port_t)uart_num,"WriteMany Valor:",16);
				do{
					len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				} while (len==0);
				data[len]=0;

				valor=atoi(data);
				printf("%d\n",valor);
				uart_write_bytes((uart_port_t)uart_num,"Address:",8);
				do{
					len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				} while (len==0);
				data[len]=0;

				add=atoi(data);
				printf("%d\n",add);
				if (add>framWords)
					printf("[%d] exceeds Fram size %d\n",add,framWords);
				else
				{
					uart_write_bytes((uart_port_t)uart_num,"Length:",7);
					do{
						len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
					} while (len==0);
					data[len]=0;

					len=atoi(data);
					memset(mpointer,valor,len);
					printf("%d\n",len);
					// 	 fram.writeMany(add,mpointer,len);
					xSemaphoreTake(framSem, portMAX_DELAY);//portMAX_DELAY
					fram.writeMany(add,mpointer,len);
					xSemaphoreGive(framSem);//portMAX_DELAY

				}
				// 	 free(mpointer);
				break;
				//			case 'M':
				//				uart_write_bytes((uart_port_t)uart_num,"Memory:",7);
				//						do{
				//									len = uart_read_bytes((uart_port_t)uart_num, (uint8_t*)data, sizeof(data),20);
				//						} while (len==0);
				//						data[len]=0;
				//						add=atoi(data);
				//						printf("[%d]=%d\n",add,mpointer[add]);
				//				break;



			}

		}
		vTaskDelay(BLINKT / portTICK_PERIOD_MS);
	}
}


