#ifndef XC4595_H
#define XC4595_H
//basic library for XC4595 FM radio module based on Si4703 IC
//info is read to/from registers via I2C, and stored in XC4595_reg[], so that unchanged bits are written back the same
//Frequency is in multiples of 100kHz- eg 95.3MHz is passed as freq value 953
//User functions:
//XC4595_read() - reads from module into register array, returns 0 on fail, 1 on success (ie can use to check if module is present and working)
//XC4595_write() - write from array to registers
//XC4595_readfreq() - read tuning frequency, returns int
//XC4595_setfreq(int) - write tuning frequency
//XC4595_setvol(int) - set volume 0-15, all others ignored
//XC4595_RSSI() - returns signal strength 0-255, typically maxes out at 75, even 30-40 seems to be strong enough
//XC4595_stereo() -returns true if a stereo signal is being received
//XC4595_init() - performs all setup - XC4595_init();XC4595_setfreq(953);XC4595_setvol(15); should be enough to have it outputting sound
//check pinouts below and uncomment board of choice. Needs HW I2C, but also needs to know which pins for correct initialisation

//for Leonardo- needs to point to HW I2C pins,RST can be any
#define XC4595_SDA 2
#define XC4595_SCL 3
#define XC4595_RST 5

//for Mega- needs to point to HW I2C pins,RST can be any
//#define XC4595_SDA 20
//#define XC4595_SCL 21
//#define XC4595_RST 5

//for Uno/Nano- needs to point to HW I2C pins,RST can be any
//#define XC4595_SDA A4
//#define XC4595_SCL A5
//#define XC4595_RST 4

#include <Arduino.h>
#include <Wire.h>

extern int XC4595_read();
extern int XC4595_write();
extern int XC4595_readfreq();
extern void XC4595_setfreq(int);
extern void XC4595_setvol(int);
extern int XC4595_RSSI();
extern int XC4595_stereo();
extern void XC4595_init();

#endif

