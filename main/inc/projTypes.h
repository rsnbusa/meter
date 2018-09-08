#ifndef projT_h
#define projT_h

#include "includes.h"
#include <string>
using namespace std;

typedef struct {
	 uint16_t code;
	 uint16_t code1;
	 char quedice[20];
} logq;

typedef struct {
	 uint8_t meterid,state;
	 uint32_t timestamp;
	 uint32_t currentBeat,oldbeat;
	 u16 elpin;
	 bool saveit;
	 u32 msNow, minamps,maxamps;
	 u16 curMonth,curMonthRaw,curDay,curDayRaw,beatSave,beatSaveRaw;
	 u8 curHour,cycleMonth,curHourRaw;
	 u32 curLife,curCycle,lastKwHDate;
	 u16 beatsPerkW,maxLoss;
} meterType;

typedef struct {
	char *mensaje;
	esp_mqtt_client_handle_t nc;
	int sizer;
} mqttMsg;

typedef struct {
	gpio_num_t sdaport,sclport;
	i2c_port_t i2cport;
} i2ctype;


typedef struct {
	gpio_num_t pin;
	int counter;
	SemaphoreHandle_t mimutex;
} argumento;

typedef struct {
    int type;                  /*!< event type */
    int group;                 /*!< timer group */
    int idx;                   /*!< timer number */
    uint64_t counter_val;      /*!< timer counter value */
    double time_sec;           /*!< calculated time from counter value */
} timer_event_t;

typedef enum {ONCE,TIMER,REPEAT,TIMEREPEAT} resetType;
typedef enum {NOTSENT,SENT} sendType;
typedef enum {NOREP,REPLACE} overType;
typedef enum {NODISPLAY,DISPLAYIT} displayType;
typedef enum {DISPLAYPULSES,DISPLAYKWH,DISPLAYUSER,DISPLAYALL,DISPLAYALLK,DISPLAYAMPS,DISPLAYNADA} displayModeType;
typedef enum {NORTC,LOGCLEAR,UPDATED,UPDATEFAIL} alertId;
#define u16		uint16_t
#define u8		uint8_t
#define u32		uint32_t

enum debugflags{BOOTD,WIFID,MQTTD,PUBSUBD,MONGOOSED,CMDD,WEBD,GEND,MQTTT,HEAPD,INTD,FRAMD,BEATD};
enum postlogId{BOOTL,FRAMFL,WIFIL,LOGL,ERASEL,PAYL,CONL,MQTTL,PAYLL,APIL,RESETL};

typedef struct{
    u16      state;
    u16      meter;
    u32      life;
    u16      month;
    u16      day;
    u16      cycle;
    u16      hora;
    u16      mesg,diag,horag;
    u16      yearg;
} scratchType;

typedef struct {
    resetType resendType;
    char alertName[MAXCHARS];
    sendType status;
    bool retain;
    u32 countLimit,counter;
    time_t whenLast;
} alertType;

typedef struct {
    u32 curBeat,curLife,curCycle,date;
    u16 curMonth,curDay,bpk;
    u8 curHour,state;
    char mid[MAXCHARS];
} rawStatus;

typedef struct  {
    u32 centinel;
    char ssid[5][MAXCHARS],pass[5][10],meterName[MAXCHARS];
    u8 working;
    time_t lastUpload;
    char email [MAXEMAILS][MAXCHARS];
    char emailName[MAXEMAILS][30];
    u8 except[MAXEMAILS];
    char mqtt[MAXCHARS];
    char domain[MAXCHARS];
    u16 ecount,ucount;
    u16 bootcount;
    time_t lastTime,preLastTime;
    char actualVersion[20];
    char groupName[MAXCHARS];
    u16 DISPTIME;
    u16 beatsPerKw[MAXDEVS];
    char medidor_id[MAXDEVS][MAXCHARS];
    u16 bounce[MAXDEVS];
    u16 lastResetCode;
    u16 MODDISPLAY[MAXDEVS];
    u8 activeMeters;
    u32 bornKwh[MAXDEVS];
    time_t bornDate[MAXDEVS];
    u16 diaDeCorte[MAXDEVS];
    bool corteSent[MAXDEVS];
    u8 breakers[MAXDEVS];
    u16 mqttport;
    char mqttUser[MAXCHARS];
    char mqttPass[MAXCHARS];
    u16 pollGroup;
    u16 ssl;
    u8 lastSSID;
    u16 traceflag; // to make it mod 16 for AES encryption
    u32 responses,cmdsIn;
    displayModeType dispmode;
    u8 lastMeter;
    char serial[MAXDEVS][20];
} config_flash;

//typedef struct { char key[10]; int val; } t_symstruct;

typedef void (*functrsn)(void *);

typedef struct{
    char comando[20];
    functrsn code;
}cmdRecord;

typedef struct{
    void *pMessage;
    uint8_t typeMsg;
    void *pComm;
}arg;

#endif
