#ifndef defines_h
#define defines_h
#define DEBUGMQQT

#define MG_LISTEN_ADDR					"80"
#define MG_TASK_STACK_SIZE 				4096 /* bytes */
#define MGOS_TASK_PRIORITY 				1
#define AP_SSID 						"taller"
#define AP_PASS 						"csttpstt"
#define AP_CHAN 						9
#define BLINKT 							100
#define BLINKPR 						100
#define DSPIN 							18

#define CONFIG_SECTOR        			0xc000/ SPI_FLASH_SEC_SIZE
#define CONFIG_ADDRESS       			0xc000

#define BUFFSIZE 						4096
#define TEXT_BUFFSIZE 					4096
#define EXAMPLE_SERVER_IP   			"185.176.43.60"
#define EXAMPLE_SERVER_PORT 			"80"
#define EXAMPLE_FILENAME 				"http://feediot.co.nf/Meter32Git.bin"


#define TIMER_INTR_SEL 					TIMER_INTR_LEVEL  /*!< Timer level interrupt */
#define TIMER_GROUP    					TIMER_GROUP_0     /*!< Test on timer group 0 */
#define TIMER_DIVIDER   				64               /*!< Hardware timer clock divider */
#define TIMER_SCALE    					(TIMER_BASE_CLK / TIMER_DIVIDER)  /*!< used to calculate counter value */
#define TIMER_FINE_ADJ   				(1.4*(TIMER_BASE_CLK / TIMER_DIVIDER)/1000000) /*!< used to compensate alarm value */
#define TIMER_INTERVAL0_SEC   			(0.0000040)   /*!< test interval for timer 0 */
#define TIMER_INTERVAL1_SEC   			(5.78)   /*!< test interval for timer 1 */
#define TEST_WITHOUT_RELOAD   			0   /*!< example of auto-reload mode */
#define TEST_WITH_RELOAD   				1      /*!< example without auto-reload mode */

//#define RELEASE
#define MQTTPORT                   		18747
//#define MQTTSERVER		  			    "m11.cloudmqtt.com"

// pins
#define SDAW                			21      // SDA
#define SCLW                			22      // SCL for Wire service

#define METER1              			4       // 1 Meter 5 - 34
#define METER2              			16      // 2 Meter 14 -35
#define METER3              			17     // 3 Meter 4 -32
#define METER4              			15       // 4 Meter 15 -33

#define RELAY1              			13      // Relay 1
#define RELAY2              			12      // Relay 2
#define RELAY3              			14      // Relay 3
#define RELAY4              			27      // Relay 4

#define FMOSI							23
#define FMISO							19
#define FCLK							18
#define FCS								5

#define WIFILED							12
#define MQTTLED							14
// Varios
#define MAXEMAILS           			3
#define CENTINEL            			0x12112299  //our chip id
#define MAXLOSSPER          			10			//For saving beats BeatsKwH/this value. Ex 800 /10 =80 beats a save to fram
#define TIMECHECK           			1000

#define ALERT_TYPE          			0
#define ERROR_TYPE          			1
#define MAXCHARS            			40
#define VERSION             			"3.0.0" // May 7/2017 Version 1
#define NOALERT            				0
#define NOMQTT              			1
#define INITLOG             			2
#define GENERAL             			255
#define MQTTCONN            			3
#define FIRMWARE            			4

// alert errors
#define MAXALERTS           			15

#define MAXBITACORABYTES    			100000
#define MAXHTTP             			3500

#define DISPMNGR		     			100

#define YB                 				15
#define WB                 				4
#define RSSIVAL             			20

#define NOERROR             			0
#define ERROROB             			1
#define ERRORFULL           			2
#define ERRORAUTH           			3
#define ERRORCMD            			4

#define HIDESSID            			false // used to received internal commands Strategy
#define MAXCMDS             			31
#define MAXDEVS             			4
#define MINELAPSEDAMPS      			100

#define RTCTIME             			60000  //every minute

#define PRINT_MSG						printf
#define u8								uint8_t
#define u16								uint16_t
#define u32								uint32_t

#define NKEYS							26

#define DEFAULT_VREF   					1100
#define SAMPLES  						64         //Multisampling
#define ADCCHAN							ADC_CHANNEL_7;     //GPIO35 if ADC1
#define ADC_COUNTS  					(1<<12)
//#define CALIB							58.0

#define MINBEAT							105

#endif
