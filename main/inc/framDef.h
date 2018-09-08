//
//  framDef.h
//  MeterIoT4096
//
//  Created by Robert on 2/17/17.
//  Copyright © 2017 Robert. All rights reserved.
//

#ifndef framDef_h
#define framDef_h

#define MAXDEVSUP           5
#define MAXDEVSS            4
#define WORD                2
#define LONG                4

// BeatCount (L)
// LifeKWHCount (L)
// LifeDate (l) last update
// 12 Months KWH (W)
// 366 Days KWH (W)
// 8764 (366*24) Hours KWH (byte)
// Min Amperes (W)
// Max Amperes (W)
// Date Min Amps (L)
// Date Max Amps(L)

#define SCRATCH             100  // 100 bytes of scratch space at the beginning
#define SCRATCHOFF          0       // absolute 0 start of FRAM

#define BEATSTART           (SCRATCH)
#define LIFEKWH             (BEATSTART+LONG)
#define LIFEDATE            (LIFEKWH+LONG)
#define MONTHSTART          (LIFEDATE+LONG)
#define MONTHRAW			(MONTHSTART+WORD*12)
#define DAYSTART            (MONTHRAW+WORD*12)
#define DAYRAW				(DAYSTART+WORD*12)
#define HOURSTART           (DAYRAW+WORD*366)
#define HOURRAW				(HOURSTART+366*24)
#define MINASTART           (HOURRAW+366*24)
#define MAXASTART           (MINASTART+WORD)
#define CYCLECOUNT          (MAXASTART+WORD)
#define CYCLEDATE           (CYCLECOUNT+WORD*12)
#define VALORPAGO           (CYCLEDATE+LONG*12)
#define FECHACORTADO        (VALORPAGO+LONG)
#define MAXPOWER            (FECHACORTADO+LONG)
#define MSPOWER             (MAXPOWER+LONG)
#define TIMEPOWER           (MSPOWER+LONG)
#define DATAEND             (TIMEPOWER+LONG)

#define TARIFADIA           (DATAEND*MAXDEVSS) // 1
#define FINTARIFA           (TARIFADIA+366*24*WORD) // 366 dias * 24 horas *2 this is the whole area required

#endif /* framDef_h */
