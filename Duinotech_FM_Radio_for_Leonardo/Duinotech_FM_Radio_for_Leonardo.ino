#include "XC4595.h"
#include "XC4629.c"

//customize these colours for your decor
#define FGCOLOUR WHITE
#define BGCOLOUR (BLUE_1*20)
#define SHCOLOUR GREY

//control keys, connect between pin and ground
#define KEY1 6
#define KEY2 7

int freq=880;   //initial value, set to your favourite!
int vol=5;

void setup() {
  pinMode(KEY1,INPUT_PULLUP);
  pinMode(KEY2,INPUT_PULLUP);
  XC4629_init();                  //setup display
  XC4629_rotate(1);
  XC4629_clear(BGCOLOUR);  
  XC4595_init();                  //set up FM radio module
  //XC4595_readfreq();
  XC4595_setfreq(freq);
  XC4595_setvol(vol);
  updatefreq();                   //update frequency display     
  updatevolume();                 //update volume display
  XC4629_chara(88,36,"MHz",FGCOLOUR,BGCOLOUR);      //static parts of display
  XC4629_chara(4,60,"VOL:",FGCOLOUR,BGCOLOUR);
  XC4629_chara(4,84,"SIG:",FGCOLOUR,BGCOLOUR);
  updatedisplay();                        //update changing parts of display
}

void loop() {
  static unsigned long t=millis();
  if(!digitalRead(KEY1)){key1down();}       //monitor button while held down
  if(!digitalRead(KEY2)){key2down();}       //monitor button while held down
  if(millis()>t+5000){
    updatedisplay();                        //update changing parts of display every five seconds
    t=millis();
  }
}

void key1down(){                            //while key 1 pressed- long is freq up, short is volume up
  unsigned long t=millis();
  unsigned long k;
  while(!digitalRead(KEY1)){
    k=millis()-t;    
    if(k>400){          //long hold
      t=t+200;          //wait another 200ms till next step
      freq=freq+1;
      if(freq>1080){freq=1080;}
      XC4595_setfreq(freq);
      updatefreq();      
    }
  }
  if((k>10)&&(k<200)){         //short debounced press
    vol=vol+1;
    if(vol>15){vol=15;}
    XC4595_setvol(vol);
    updatevolume();
  }
}

void key2down(){                            //while key 2 pressed- long is freq down, short is volume down
  unsigned long t=millis();
  unsigned long k;
  while(!digitalRead(KEY2)){
    k=millis()-t;    
    if(k>400){          //long hold
      t=t+200;          //wait another 200ms till next step
      freq=freq-1;
      if(freq<875){freq=875;}
      XC4595_setfreq(freq);
      updatefreq();      
    }
  }
  if((k>10)&&(k<200)){         //short debounced press
    vol=vol-1;
    if(vol<0){vol=0;}
    XC4595_setvol(vol);
    updatevolume();
  }  
}

void updatevolume(){          //change displayed volume
  for(int i=0;i<15;i++){
    if(vol>i){
      XC4629_box(52+i*5,60,55+i*5,75,FGCOLOUR);
    }else{
      XC4629_box(52+i*5,60,55+i*5,75,SHCOLOUR);      
    }
  }
}

void updatefreq(){            //change displayed frequency  
  if(freq<1000){
    XC4629_char2(4,4,' ',FGCOLOUR,BGCOLOUR);            //leading zero blanking
  }else{
    XC4629_char2(4,4,(freq/1000)%10+'0',FGCOLOUR,BGCOLOUR);    
  }
  XC4629_char2(28,4,(freq/100)%10+'0',FGCOLOUR,BGCOLOUR);    
  XC4629_char2(52,4,(freq/10)%10+'0',FGCOLOUR,BGCOLOUR);    
  XC4629_char2(76,4,'.',FGCOLOUR,BGCOLOUR);    
  XC4629_char2(100,4,(freq)%10+'0',FGCOLOUR,BGCOLOUR);      
}

void updatedisplay(){                     //update other parts of display
  int rssi=XC4595_RSSI();
  rssi=constrain(rssi/3,0,15);            //normally maxes out at 75, so scale down, but 30 is pretty good, clip top of scale
  unsigned int rssicolour=RED;
  if(rssi>5){rssicolour=YELLOW;}
  if(rssi>8){rssicolour=GREEN;}
  for(int i=0;i<15;i++){
    if(rssi>i){
      XC4629_box(52+i*5,84,55+i*5,99,rssicolour);   //bar graph
    }else{
      XC4629_box(52+i*5,84,55+i*5,99,SHCOLOUR);      
    }
  }  
  int stereo=XC4595_stereo();
  if(stereo){
    XC4629_chara(28,108,"STEREO",FGCOLOUR,BGCOLOUR);      //stereo or mono
  }else{
    XC4629_chara(28,108," MONO ",SHCOLOUR,BGCOLOUR);
  }
}

