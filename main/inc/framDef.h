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

#define BEATSTART           0
#define LIFEKWH             (BEATSTART+LONG)
#define LIFEDATE            (LIFEKWH+LONG)
#define MONTHSTART          (LIFEDATE+LONG)
#define DAYSTART            (MONTHSTART+WORD*12)
#define HOURSTART           (DAYSTART+WORD*366)
#define MINASTART           (HOURSTART+8784)
#define MAXASTART           (MINASTART+WORD)
#define CYCLECOUNT          (MAXASTART+WORD)
#define CYCLEDATE           (CYCLECOUNT+WORD*12)
#define VALORPAGO           (CYCLEDATE+LONG*12)
#define FECHACORTADO        (VALORPAGO+LONG)
#define MAXPOWER            (FECHACORTADO+LONG)
#define MSPOWER             (MAXPOWER+LONG)
#define TIMEPOWER           (MSPOWER+LONG)
#define DATAEND             (TIMEPOWER+LONG)  // as of Apr 26/2017 size is 9648

// Tarifacion
// 100 words(2bytes) de BeatPerKwh
// 366dias * 24 Horas de Tarifacion por Hora
//

#define BPH                 (DATAEND*MAXDEVSS+100+SCRATCH) // start of our tariff + 100 bytes to spare AVOID CONFLICT+SCRATCH
#define TARIFADIA           (BPH+100*WORD) // 100 Beats Per Hour tipos
#define FINTARIFA           (TARIFADIA+8784) // 366 dias * 24 horas

#endif /* framDef_h */
