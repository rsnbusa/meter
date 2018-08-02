/*
 * cmds.h
 *
 *  Created on: Apr 16, 2017
 *      Author: RSN
 */

#ifndef MAIN_CMDS_H_
#define MAIN_CMDS_H_


extern void set_displayMeter(void *pArg);
extern void set_generalap(void *pArg);
extern void kbd(void *pArg);
extern void displayManager(void *pArg);
extern void read_flash();
extern void write_to_fram(u8 meter,bool adding);
extern  void load_from_fram(u8 meter);
extern void recover_fram();
extern void erase_config();
extern void drawString(int x, int y, string que, int fsize, int align,displayType showit,overType erase);
extern void setLogo(string cual);

#endif /* MAIN_CMDS_H_ */
