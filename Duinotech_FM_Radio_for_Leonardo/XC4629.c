//basic 'library' for Jaycar XC4629 128x128 display
//copy this in your sketch folder, and reference with #include "XC4629.c"
//can use any 5 pins, Clock and data optimized slightly (may not work with non-ATMEGA boards)
//see XC4629_.... pin definitions below
//colours are 16 bit unsigned ints (binary RRRRRGGGGGGBBBBB)
//functions:
// XC4630_init() - sets up pins/registers on display, sets orientation=1
// XC4630_clear(c) - sets all pixels to colour c (can use names below)
// XC4630_rotate(n) -  sets orientation (top) 1,2,3 or 4
// typical setup code might be:
// XC4630_init();XC4630_rotate(2);XC4630_clear(BLACK); // set up display and make it landscape, black
//
// XC4630_char(x,y,c,f,b) - draw character c with lop left at (x,y), with f foreground and b background colour
// XC4630_chara(x,y,c,f,b) - draw character array c with lop left at (x,y), with f foreground and b background colour
// XC4630_box(x1,y1,x2,y2,c) - draw solid rectangular box with colour c from x1,y1 to x2,y2
// XC4630_hline(x1,y,x2,c) -  draw horizontal line from (x1,y) to (x2,y) in colour c
// XC4630_vline(x,y1,y2,c) -  draw vertical line from (x,y1) to (x,y2) in colour c
// XC4630_line(x1,y1,x2,y2,c) -  draw arbitrary line from (x1,y1) to (x2,y2) in colour c
// XC4630_point(x,y,c) - draw a single point at (x,y) in colour c
// XC4630_fcircle(x,y,r,c) - draw a filled circle centred on (x,y) with radius r, colour c
// XC4630_circle(x,y,r,c) - draw a hollow circle centred on (x,y) with radius r, colour c
// XC4630_triangle(x1,y1,x2,y2,x3,y3,c) - draw filled triangle between the three points in colour c

//todo:handle CS better to share SPI bus, allow HW SPI
#include <Arduino.h>

#define XC4629_CLK 13
#define XC4629_SDI 11
#define XC4629_RS 9
#define XC4629_RST 8
#define XC4629_CS 10

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0  
#define WHITE 0xFFFF
#define GREY 0x8410
//unit colours - multiply by 0-31
#define RED_1 0x0800
#define GREEN_1 0x0040
#define BLUE_1 0x0001

int XC4629_width,XC4629_height,XC4629_orientation,XC4629_xoffset,XC4629_yoffset;  //IC can handle 162x132, visible area is 128x128, offset changes by orientation

volatile uint8_t  *XC4629_CLKPORT;
volatile uint8_t  XC4629_CLKPIN;
volatile uint8_t  *XC4629_SDIPORT;
volatile uint8_t  XC4629_SDIPIN;

#include <avr/pgmspace.h>
const unsigned int font1216[][12] PROGMEM ={
{0,0,0,0,0,0,0,0,0,0,0,0},           // 
{0,0,0,124,13311,13311,124,0,0,0,0,0},           //!
{0,0,60,60,0,0,60,60,0,0,0,0},           //"
{512,7696,8080,1008,638,7710,8080,1008,638,30,16,0},           //#
{0,1144,3324,3276,16383,16383,3276,4044,1928,0,0,0},           //$
{12288,14392,7224,3640,1792,896,448,14560,14448,14392,28,0},           //%
{0,7936,16312,12796,8646,14306,7742,7196,13824,8704,0,0},           //&
{0,0,0,39,63,31,0,0,0,0,0,0},           //'
{0,0,1008,4092,8190,14343,8193,8193,0,0,0,0},           //(
{0,0,8193,8193,14343,8190,4092,1008,0,0,0,0},           //)
{0,3224,3768,992,4088,4088,992,3768,3224,0,0,0},           //*
{0,384,384,384,4080,4080,384,384,384,0,0,0},           //+
{0,0,0,47104,63488,30720,0,0,0,0,0,0},           //,
{0,384,384,384,384,384,384,384,384,0,0,0},           //-
{0,0,0,14336,14336,14336,0,0,0,0,0,0},           //.
{6144,7168,3584,1792,896,448,224,112,56,28,14,0},           ///
{2040,8190,7686,13059,12675,12483,12387,12339,6174,8190,2040,0},           //0
{0,0,12300,12300,12302,16383,16383,12288,12288,12288,0,0},           //1
{12316,14366,15367,15875,14083,13187,12739,12515,12407,12350,12316,0},           //2
{3084,7182,14343,12483,12483,12483,12483,12483,14823,8062,3644,0},           //3
{960,992,880,824,796,782,775,16383,16383,768,768,0},           //4
{3135,7295,14435,12387,12387,12387,12387,12387,14563,8131,3971,0},           //5
{4032,8176,14840,12508,12494,12487,12483,12483,14787,8064,3840,0},           //6
{3,3,3,12291,15363,3843,963,243,63,15,3,0},           //7
{3840,8124,14846,12519,12483,12483,12483,12519,14846,8124,3840,0},           //8
{60,126,12519,12483,12483,14531,7363,3779,2023,1022,252,0},           //9
{0,0,0,7280,7280,7280,0,0,0,0,0,0},           //:
{0,0,0,40048,64624,31856,0,0,0,0,0,0},           //;
{0,192,480,1008,1848,3612,7182,14343,12291,0,0,0},           //<
{0,1632,1632,1632,1632,1632,1632,1632,1632,1632,0,0},           //=
{0,12291,14343,7182,3612,1848,1008,480,192,0,0,0},           //>
{28,30,7,3,14211,14275,227,119,62,28,0,0},           //?
{4088,8190,6151,13299,14331,13851,14331,14331,13831,1022,504,0},           //@
{14336,16128,2016,1788,1567,1567,1788,2016,16128,14336,0,0},           //A
{16383,16383,12483,12483,12483,12483,12519,14846,8124,3840,0,0},           //B
{1008,4092,7182,14343,12291,12291,12291,14343,7182,3084,0,0},           //C
{16383,16383,12291,12291,12291,12291,14343,7182,4092,1008,0,0},           //D
{16383,16383,12483,12483,12483,12483,12483,12483,12291,12291,0,0},           //E
{16383,16383,195,195,195,195,195,195,3,3,0,0},           //F
{1008,4092,7182,14343,12291,12483,12483,12483,16327,16326,0,0},           //G
{16383,16383,192,192,192,192,192,192,16383,16383,0,0},           //H
{0,0,12291,12291,16383,16383,12291,12291,0,0,0,0},           //I
{3584,7680,14336,12288,12288,12288,12288,14336,8191,2047,0,0},           //J
{16383,16383,192,480,1008,1848,3612,7182,14343,12291,0,0},           //K
{16383,16383,12288,12288,12288,12288,12288,12288,12288,12288,0,0},           //L
{16383,16383,30,120,480,480,120,30,16383,16383,0,0},           //M
{16383,16383,14,56,240,960,1792,7168,16383,16383,0,0},           //N
{1008,4092,7182,14343,12291,12291,14343,7182,4092,1008,0,0},           //O
{16383,16383,387,387,387,387,387,455,254,124,0,0},           //P
{1008,4092,7182,14343,12291,13827,15879,7182,16380,13296,0,0},           //Q
{16383,16383,387,387,899,1923,3971,7623,14590,12412,0,0},           //R
{3132,7294,14567,12483,12483,12483,12483,14791,8078,3852,0,0},           //S
{0,3,3,3,16383,16383,3,3,3,0,0,0},           //T
{2047,8191,14336,12288,12288,12288,12288,14336,8191,2047,0,0},           //U
{7,63,504,4032,15872,15872,4032,504,63,7,0,0},           //V
{16383,16383,7168,1536,896,896,1536,7168,16383,16383,0,0},           //W
{12291,15375,3612,816,480,480,816,3612,15375,12291,0,0},           //X
{3,15,60,240,16320,16320,240,60,15,3,0,0},           //Y
{12291,15363,15875,13059,12739,12515,12339,12319,12303,12291,0,0},           //Z
{0,0,16383,16383,12291,12291,12291,12291,0,0,0,0},           //[
{14,28,56,112,224,448,896,1792,3584,7168,6144,0},           //backslash
{0,0,12291,12291,12291,12291,16383,16383,0,0,0,0},           //]
{96,112,56,28,14,7,14,28,56,112,96,0},           //^
{49152,49152,49152,49152,49152,49152,49152,49152,49152,49152,49152,0},           //_
{0,0,0,0,62,126,78,0,0,0,0,0},           //`
{7168,15936,13152,13152,13152,13152,13152,13152,16352,16320,0,0},           //a
{16383,16383,12480,12384,12384,12384,12384,14560,8128,3968,0,0},           //b
{3968,8128,14560,12384,12384,12384,12384,12384,6336,2176,0,0},           //c
{3968,8128,14560,12384,12384,12384,12512,12480,16383,16383,0,0},           //d
{3968,8128,15328,13152,13152,13152,13152,13152,5056,384,0,0},           //e
{192,192,16380,16382,199,195,195,3,0,0,0,0},           //f
{896,51136,52960,52320,52320,52320,52320,58976,32736,16352,0,0},           //g
{16383,16383,192,96,96,96,224,16320,16256,0,0,0},           //h
{0,0,12288,12384,16364,16364,12288,12288,0,0,0,0},           //i
{0,0,24576,57344,49152,49248,65516,32748,0,0,0,0},           //j
{0,16383,16383,768,1920,4032,7392,14432,12288,0,0,0},           //k
{0,0,12288,12291,16383,16383,12288,12288,0,0,0,0},           //l
{16352,16320,224,224,16320,16320,224,224,16320,16256,0,0},           //m
{0,16352,16352,96,96,96,96,224,16320,16256,0,0},           //n
{3968,8128,14560,12384,12384,12384,12384,14560,8128,3968,0,0},           //o
{65504,65504,3168,6240,6240,6240,6240,7392,4032,1920,0,0},           //p
{1920,4032,7392,6240,6240,6240,6240,3168,65504,65504,0,0},           //q
{0,16352,16352,192,96,96,96,96,224,192,0,0},           //r
{4544,13280,13152,13152,13152,13152,16224,7744,0,0,0,0},           //s
{96,96,8190,16382,12384,12384,12384,12288,0,0,0,0},           //t
{4064,8160,14336,12288,12288,12288,12288,6144,16352,16352,0,0},           //u
{96,480,1920,7680,14336,14336,7680,1920,480,96,0,0},           //v
{2016,8160,14336,7168,4064,4064,7168,14336,8160,2016,0,0},           //w
{12384,14560,7616,3968,1792,3968,7616,14560,12384,0,0,0},           //x
{0,96,33248,59264,32256,7680,1920,480,96,0,0,0},           //y
{12384,14432,15456,13920,13152,12768,12512,12384,12320,0,0,0},           //z
{0,128,448,8188,16254,28679,24579,24579,24579,0,0,0},           //{
{0,0,0,0,16383,16383,0,0,0,0,0,0},           //|
{0,24579,24579,24579,28679,16254,8188,448,128,0,0,0},           //}
{16,24,12,4,12,24,16,24,12,4,0,0},           //~
{0,0,0,0,0,0,0,0,0,0,0,0}           //
};

void XC4629_data(byte d){  
  for(byte n=128;n;n=n>>1){
    if(d&n){
      *XC4629_SDIPORT |= XC4629_SDIPIN;
    }else{
      *XC4629_SDIPORT &= ~XC4629_SDIPIN;      
    }
    *XC4629_CLKPORT |= XC4629_CLKPIN;
    *XC4629_CLKPORT &= ~XC4629_CLKPIN;    
  }
}

void XC4629_command(byte d){
  digitalWrite(XC4629_RS,LOW);
  XC4629_data(d);
  digitalWrite(XC4629_RS,HIGH);
}

void XC4629_areaset(int x1,int y1,int x2,int y2){
  if(x2<x1){int i=x1;x1=x2;x2=i;}   //sort x
  if(y2<y1){int i=y1;y1=y2;y2=i;}   //sort y
  x1=x1+XC4629_xoffset;
  x2=x2+XC4629_xoffset;
  y1=y1+XC4629_yoffset;
  y2=y2+XC4629_yoffset;  
  XC4629_command(42);               //set x bounds  
  XC4629_data(x1>>8);
  XC4629_data(x1);
  XC4629_data(x2>>8);
  XC4629_data(x2);
  XC4629_command(43);               //set y bounds
  XC4629_data(y1>>8);
  XC4629_data(y1);
  XC4629_data(y2>>8);
  XC4629_data(y2);
  XC4629_command(44);               //drawing data to follow
}

void XC4629_char2(int x,int y,char c, unsigned int f, unsigned int b){    //double size font
  c=c-32;
  if(c<0){c=0;}                     //valid chars only
  if(c>96){c=0;}
  XC4629_areaset(x,y,x+23,y+31);    //set area
  for(byte v=0;v<16;v++){
    for(byte u=0;u<12;u++){
      unsigned int d=pgm_read_word(&font1216[c][u]);
      if((1<<v)&d){XC4629_data(f>>8);XC4629_data(f);XC4629_data(f>>8);XC4629_data(f);}
      else{XC4629_data(b>>8);XC4629_data(b);XC4629_data(b>>8);XC4629_data(b);}
    }
    for(byte u=0;u<12;u++){
      unsigned int d=pgm_read_word(&font1216[c][u]);
      if((1<<v)&d){XC4629_data(f>>8);XC4629_data(f);XC4629_data(f>>8);XC4629_data(f);}
      else{XC4629_data(b>>8);XC4629_data(b);XC4629_data(b>>8);XC4629_data(b);}
    }
  }  
}

void XC4629_char(int x,int y,char c, unsigned int f, unsigned int b){
  c=c-32;
  if(c<0){c=0;}                     //valid chars only
  if(c>96){c=0;}
  XC4629_areaset(x,y,x+11,y+15);    //set area
  for(byte v=0;v<16;v++){
    for(byte u=0;u<12;u++){
      unsigned int d=pgm_read_word(&font1216[c][u]);
      if((1<<v)&d){XC4629_data(f>>8);XC4629_data(f);}
      else{XC4629_data(b>>8);XC4629_data(b);}
    }
  }
}

void XC4629_chara(int x,int y,char *c, unsigned int f, unsigned int b){
  while(*c){
    XC4629_char(x,y,*c++,f,b);
    x=x+12;
    if(x>XC4629_width-12){x=0;y=y+16;}      //wrap around (will probably look ugly)
  }  
}

void XC4629_clear(unsigned int c){
  XC4629_areaset(0,0,XC4629_width-1,XC4629_height-1);    //set area
  for(int i =0;i<16384;i++){
    XC4629_data((c>>8)&255);
    XC4629_data(c&255);    
  }
}

void XC4629_box(int x1,int y1,int x2,int y2,unsigned int c){
  if(x2<x1){int i=x1;x1=x2;x2=i;}
  if(y2<y1){int i=y1;y1=y2;y2=i;}
  XC4629_areaset(x1,y1,x2,y2);
  x2++;
  y2++;
  for(int x=x1;x<x2;x++){
    for(int y=y1;y<y2;y++){
      XC4629_data(c>>8);
      XC4629_data(c);  
    }
  }
}

void XC4629_point(int x,int y, unsigned int c){  //a single point
  XC4629_areaset(x,y,x,y);
  XC4629_data(c>>8);
  XC4629_data(c);  
}

void XC4629_hline(int x1,int y1,int x2,unsigned int c){
  if(x2<x1){int i=x1;x1=x2;x2=i;}
  XC4629_areaset(x1,y1,x2,y1);
  x2++;
  for(int x=x1;x<x2;x++){
    XC4629_data(c>>8);
    XC4629_data(c);  
  }
}

void XC4629_vline(int x1,int y1,int y2,unsigned int c){
  if(y2<y1){int i=y1;y1=y2;y2=i;}
  XC4629_areaset(x1,y1,x1,y2);
  y2++;
  for(int y=y1;y<y2;y++){
    XC4629_data(c>>8);
    XC4629_data(c);  
  }
}

void XC4629_fcircle(int xo,int yo,int r,unsigned int c){ //https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
  int e=0;
  int x=r;
  int y=0;
  while(x>=y){
    XC4629_vline(xo-y,yo+x,yo-x,c);
    XC4629_vline(xo+y,yo+x,yo-x,c);
    y=y+1;
    e=e+1+2*y;
    if(2*(e-x)+1>0){
      y=y-1;
      XC4629_vline(xo-x,yo-y,yo+y,c);
      XC4629_vline(xo+x,yo-y,yo+y,c);
      y=y+1;
      x=x-1;
      e=e+1-2*x;
    }
  }  
}

void XC4629_circle(int xo,int yo,int r,unsigned int c){ //https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
  int e=0;
  int x=r;
  int y=0;
  while(x>=y){
    XC4629_point(xo+x,yo+y,c);
    XC4629_point(xo-x,yo+y,c);
    XC4629_point(xo+x,yo-y,c);
    XC4629_point(xo-x,yo-y,c);
    XC4629_point(xo+y,yo+x,c);
    XC4629_point(xo-y,yo+x,c);
    XC4629_point(xo+y,yo-x,c);
    XC4629_point(xo-y,yo-x,c);
    y=y+1;
    e=e+1+2*y;
    if(2*(e-x)+1>0){
      x=x-1;
      e=e+1-2*x;
    }
  }
}

void XC4629_line(int x1,int y1,int x2,int y2, unsigned int c){
  int steps,stepsx,stepsy,xinc,yinc,x,y,d;
  stepsx=abs(x1-x2);
  stepsy=abs(y1-y2);
  steps=max(stepsx,stepsy)+1;   //if start and end are the same, there's still 1 point
  xinc=constrain(x2-x1,-1,1);
  yinc=constrain(y2-y1,-1,1);
  x=x1;
  y=y1;  
  if(stepsx>stepsy){
    d=stepsx/2;
    for(int i=0;i<steps;i++){
      XC4629_point(x,y,c);
      x=x+xinc;
      d=d+stepsy;
      if(d>stepsx){d=d-stepsx;y=y+yinc;}
    }
  }else{
    d=stepsy/2;
    for(int i=0;i<steps;i++){
      XC4629_point(x,y,c);
      y=y+yinc;
      d=d+stepsx;
      if(d>stepsy){d=d-stepsy;x=x+xinc;}
    } 
  }  
}

void XC4629_triangle(int x1,int y1,int x2,int y2,int x3,int y3,unsigned int c){ //custom Bresenham line algorithm
  //sort values, y1 at top
  if(y1>y2){int i=y1;y1=y2;y2=i;i=x1;x1=x2;x2=i;}
  if(y2>y3){int i=y2;y2=y3;y3=i;i=x2;x2=x3;x3=i;}
  if(y1>y2){int i=y1;y1=y2;y2=i;i=x1;x1=x2;x2=i;}
  if(y1==y3){XC4629_hline(min(x1,min(x2,x3)),y1,max(x1,max(x2,x3)),c);return;}
  if(y1!=y2){
    int dy1=y2-y1;
    int dy2=y3-y1;
    int dx1=x2-x1;
    int dx2=x3-x1;
    int xa,xb;
    xa=x1*dy1-(dx1);
    xb=x1*dy2-(dx2);
    for(int y=y1;y<y2;y++){
     xa=xa+dx1;
     xb=xb+dx2;
     XC4629_hline(xa/dy1,y,xb/dy2,c);
    }
   xb=xb+dx2;
   XC4629_hline(x2,y2,xb/dy2,c);        
  }
  if(y2!=y3){
    int dy1=y2-y3;
    int dy2=y1-y3;
    int dx1=x2-x3;
    int dx2=x1-x3;
    int xa,xb;
    xa=x3*dy1+(dx1);
    xb=x3*dy2+(dx2);
    for(int y=y3;y>y2;y--){
     xa=xa-dx1;
     xb=xb-dx2;
     XC4629_hline(xa/dy1,y,xb/dy2,c);
    }        
   if(y1==y2){
    xb=xb+dx2;
    XC4629_hline(x2,y2,xb/dy2,c);        
   }
  }
}

void XC4629_rotate(int n){
  switch(n){
    case 1:
    XC4629_command(0x36);           //Memory Access Control 
    XC4629_data(0xC8);              //1=top is 12 o'clock
    XC4629_width=128;
    XC4629_height=128;
    XC4629_xoffset=2;
    XC4629_yoffset=3;
    XC4629_orientation=1;
    break;
    case 2:
    XC4629_command(0x36);           //Memory Access Control 
    XC4629_data(0xA8);              //2=top is 3 o'clock
    XC4629_width=128;
    XC4629_height=128;
    XC4629_xoffset=3;
    XC4629_yoffset=2;
    XC4629_orientation=2;
    break;
    case 3:
    XC4629_command(0x36);           //Memory Access Control 
    XC4629_data(0x08);              //3=top is 6 o'clock
    XC4629_width=128;
    XC4629_height=128;
    XC4629_xoffset=2;
    XC4629_yoffset=1;
    XC4629_orientation=3;
    break;
    case 4:
    XC4629_command(0x36);           //Memory Access Control 
    XC4629_data(0x68);              //4=top is 9 o'clock
    XC4629_width=128;
    XC4629_height=128;
    XC4629_xoffset=1;
    XC4629_yoffset=2;
    XC4629_orientation=4;
    break;
  }
}

void XC4629_long(int x,int y,long n,unsigned int f, unsigned int b){
  if(n<0){n=-n;XC4629_char(x,y,'-',f,b);x=x+12;}
  long k=1000000000;
  byte flag=0;
  for(long k=1000000000;k>0;k=k/10){
    if(n>(k-1)||(k==1)||(flag)){
      flag=1;
      long a=(n/k);
      XC4629_char(x,y,a+'0',f,b);
      n=n-a*k;
      x=x+12;
    }
  }  
}

void XC4629_init(){
  XC4629_CLKPORT = portOutputRegister(digitalPinToPort(XC4629_CLK));
  XC4629_CLKPIN = digitalPinToBitMask(XC4629_CLK);
  XC4629_SDIPORT = portOutputRegister(digitalPinToPort(XC4629_SDI));
  XC4629_SDIPIN = digitalPinToBitMask(XC4629_SDI);
  XC4629_width=128;     //width=height, so they won't change with rotation
  XC4629_height=128;  
  XC4629_orientation=1;
  XC4629_xoffset=2;
  XC4629_yoffset=3;
  pinMode(XC4629_CLK,OUTPUT);
  pinMode(XC4629_SDI,OUTPUT);
  pinMode(XC4629_RS,OUTPUT);
  pinMode(XC4629_RST,OUTPUT);
  pinMode(XC4629_CS,OUTPUT);
  digitalWrite(XC4629_CS,LOW);
  digitalWrite(XC4629_RS,HIGH);   //default to data
  digitalWrite(XC4629_CLK,LOW);
  digitalWrite(XC4629_SDI,LOW);
  digitalWrite(XC4629_RST,LOW);
  delay(1);
  digitalWrite(XC4629_RST,HIGH);
  delay(120);
  XC4629_command(1);
  delay(120);
  XC4629_command(17);
  delay(120);
  XC4629_command(38);
  XC4629_data(4);
  XC4629_command(177);
  XC4629_data(11);
  XC4629_data(20);
  XC4629_command(192);
  XC4629_data(8);
  XC4629_data(0);
  XC4629_command(193);
  XC4629_data(5);
  XC4629_command(197);
  XC4629_data(65);
  XC4629_data(48);
  XC4629_command(199);
  XC4629_data(193);
  XC4629_command(236);
  XC4629_data(27);
  XC4629_command(58);
  XC4629_data(85);
  XC4629_command(183);
  XC4629_data(0);
  XC4629_command(242);
  XC4629_data(0);
  XC4629_command(19);
  XC4629_command(41);
  XC4629_command(54);
  XC4629_data(0xC8);      //start with display in position 1
}


