/*
 * setGeneral.h
 *
 *  Created on: Apr 16, 2017
 *      Author: RSN
 */

#ifndef MAIN_SETGENERAL_H_
#define MAIN_SETGENERAL_H_

#define GLOBAL  // set global variable for compilation

#include "defines.h"
#include "includes.h"
#include "projTypes.h"
#include "forward.h"
#include <string>
#include "globals.h"

extern bool set_commonCmd(arg* pArg,bool check);
extern string getParameter(arg* argument,string cual);
extern void write_to_flash();
#endif /* MAIN_SETGENERAL_H_ */
