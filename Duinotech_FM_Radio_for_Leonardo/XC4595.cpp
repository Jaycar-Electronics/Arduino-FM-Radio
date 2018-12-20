#include "XC4595.h"

unsigned int XC4595_reg[16];          //registers stored here between reads and writes

extern int XC4595_read(){             //reads registers into buffers, returns 0=fail
  unsigned long t;
  t=millis()+1000;                    //timeout
  Wire.requestFrom(16, 32); 
  while(Wire.available()<32){if(t<millis()){return 0;}}
  for(byte i=10;i<26;i++){            //registers are read 10-15 then 0-9
    byte a=Wire.read();
    byte b=Wire.read();
    XC4595_reg[i&15]=(a<<8)|b;        //combine bytes to ints
  }
  return 1;
}

extern int XC4595_write(){             //writes from buffers
  Wire.beginTransmission(16);
  for(byte i=2;i<8;i++){         //only 2-7 should be written
    byte a=(XC4595_reg[i]>>8)&255;
    byte b=(XC4595_reg[i])&255;
    Wire.write(a);
    Wire.write(b);    
  }
  return Wire.endTransmission();
}

extern int XC4595_readfreq(){               //this assumes setup has occurred in the correct mode
  int f;
  XC4595_read();
  f=XC4595_reg[11]&0x07ff;
  return (f+875);
}

extern void XC4595_setfreq(int f){         //this assumes setup has occurred in the correct mode
  f=f-875;
  if(f<0){return;}
  if(f>1023){return;}
  XC4595_read();
  XC4595_reg[3]=0x8000|f;                 //set tune bit
  XC4595_write();
  delay(60);
  XC4595_read();
  XC4595_reg[3]=f;                        //clear tune bit
  XC4595_write();  
}

extern void XC4595_setvol(int v){   //0-15 valid
  if(v<0){return;}
  if(v>15){return;}
  XC4595_read();
  XC4595_reg[5]=(XC4595_reg[5]&0xFFF0)|v;       // 4 lsb of reg 5
  XC4595_write();
}

extern int XC4595_RSSI(){
  int f;
  XC4595_read();
  f=XC4595_reg[10]&0xff;            //stored in 8 lsb of reg 10
  return (f);
}

extern int XC4595_stereo(){
  int f;
  XC4595_read();
  f=(XC4595_reg[10]&0x0100)>>8;     //bit 8 of reg 10
  return(f);  
}

extern void XC4595_init(){             //is a 3.3V device, so we should avoid pulling pins high
  pinMode(XC4595_SDA,OUTPUT);
  digitalWrite(XC4595_SDA,LOW);
  pinMode(XC4595_RST,OUTPUT);
  digitalWrite(XC4595_RST,LOW);
  delay(2);
  digitalWrite(XC4595_RST,HIGH);      //needs to be pulled up, has 10k pulldown, using a 4.7 kOhm from 5V works
  delay(2);
  Wire.begin();
  XC4595_read();
  XC4595_reg[7]=0x8100;   //start unit
  XC4595_write();
  delay(500);             //oscillator startup
  XC4595_read();
  XC4595_reg[2]=0x4601;
  XC4595_reg[4]=0x1800;
  XC4595_reg[5]=0x0010;   //vol =0, spacing =100kHz, 2 MSN are seek RSSI threshold
  XC4595_write();
  delay(110);             //stabilise
}

