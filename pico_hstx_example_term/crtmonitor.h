//(C)2024 Simple-Ciruit
//This program uses Bresenham's line algorithm from the Wikipedia example code
//for line plots

//crtmonitor is a dumb terminal project for educational purposes.
//It displays ASCII text from serial port-1 using the pico_hstx_example
//D0 is UART out, D1 is UART in, D2 open Baud=4800, D2 grounded Baud=500000
//Up to four Space and comma delimited numbers are plotted on a 256x256 graph.
//D3 open displays text and plots, D3 grounded plots only
//#n1 n2 n3 n4 sets the full scale where n1 through n4 are real numbers
//Only numbers containing +-. and 0 to 9 are parsed (no scientific notaton)
//
//ctrl-L clears the screens
//ctrl-T sets the plot mode to chart recorder (X-T)
//ctrl-Y sets the plot mode X-Y
//ctrl-J is a line feed
//ctrl-M is a carriage return
//ctrl-H is a back space
//ctrl-Z sets pin D22 low, connect to the run pin to force a reset


//Display Constants, adjust to suit

#define NUM_TEXT_LINES 40     //40 lines of text display
#define NUM_TEXT_CHARS 60    //60 characters per line
#define H_LEFT 12            //12 text left most starting pixel  6 x 12 = two characters
#define TOP_LINE 2           //2 text starts 2 lines from top 
#define PLOT_LEFT 376        //376 plot left most starting pixel >= 12+6*60 but < 383

//Display Constants, do not change

#define TEXT_LINE_LEN 5120                            //for left most part of display     5120
#define START_ADDR TEXT_LINE_LEN*TOP_LINE             //text starts 2-char lines from top 10240
#define CHAR_END_ADDR START_ADDR + TEXT_LINE_LEN*NUM_TEXT_LINES  //38-char of text lines from start   204800
#define END_ADDR CHAR_END_ADDR+TEXT_LINE_LEN         //one blank line used for clearing text 209920

#define LINE_LEN 640                                 //640
#define LINE_6 6*LINE_LEN                            //3840
#define TEXT_START START_ADDR+H_LEFT                //10320
#define CURS1 4481   //cursor start is eighth line of 5x7 character plus one
#define CURS2 4482
#define CURS3 4483
#define CURS4 4484
#define CURS5 4485
#define H_CHARS 6*NUM_TEXT_CHARS //each character is 6-pixels wide   222
#define H_CHAR_RIGHT H_LEFT+H_CHARS //right character pixel limit    302
#define SCREEN_WIDTH 480                                        //   480
#define H_RIGHT H_LEFT+SCREEN_WIDTH                             //   640
#define H_NEWLINE H_LEFT-6                                      //   74
#define H_PLOT_START PLOT_LEFT+START_ADDR                       //   10544
 
bool core1_separate_stack = true;
volatile word bc[128][8];    //ascii character pixel matrix

void line_to(int x0, int y0, int x1, int y1, volatile byte rgb){
 int dx = abs(x1-x0);
 int sx = (x0 < x1) ? 1 : -1;
 int dy = -abs(y1-y0);
 int sy = (y0 < y1) ? 1 : -1;
 int error = dx+dy;
 int e2;
 
 while(true){
   framebuf[H_PLOT_START+(y0*LINE_LEN)+x0] = rgb;
   if ((x0 == x1) && (y0 == y1)) break;
   e2 = 2 * error;
   if (e2 >= dy){
    error = error + dy;
    x0 = x0 + sx;
   }
   if (e2 <= dx){
    error = error + dx;
    y0 = y0 + sy;
   }
 }  
}


void draw_grid(void){
 int i,j;
    for (i=H_PLOT_START; i < (H_PLOT_START+(256*LINE_LEN)); i=i+LINE_LEN) {
    for (j=0; j<256;j++) framebuf[i+j] =  0x08;
   } 
 for (i = 0; i<257; i = i+32) line_to(0,i,256,i,0x04);
 for (i = 0; i<257; i = i+32) line_to(i,0,i,256,0x04);
   
}

void point_set(int x0, int y0, volatile byte rgb){
  framebuf[H_PLOT_START+(y0*LINE_LEN)+x0] = rgb;
}

void line_vert(int x0,volatile byte rgb){
  int i,j;

  j = H_PLOT_START+x0;
  for (i=0;i<=256;i++){
    framebuf[j] = rgb; 
    j = j+LINE_LEN;
  }
}

void line_to_fast(int x0, int y0, int y1, volatile byte rgb){
 int dy = -abs(y1-y0);
 int sy = (y0 < y1) ? 1 : -1;
 int error = 1+dy;
 int e2;
 x0 = H_PLOT_START+x0;
 int x1 = x0+1;
 
 while(true){
   framebuf[(y0*LINE_LEN)+x0] = rgb;
   if ((x0 == x1) && (y0 == y1)) break;
   e2 = 2 * error;
   if (e2 >= dy){
    error = error + dy;
    x0 = x1;
   }
   if (e2 <= 1){
    error = error + 1;
    y0 = y0 + sy;
   }
 }  
}


void clr_screen(void){
 int i,j;
 for (i=TEXT_START;i<CHAR_END_ADDR;i=i+LINE_LEN){
  for (j=0;j<H_CHARS;j++) framebuf[i+j]=0x08;
 }
}


void setup() {
#include "matrix.h"
  pinMode(2,INPUT_PULLUP);  //select baud switch
  pinMode(3,INPUT_PULLUP);  //turn text off/on switch
  pinMode(4,OUTPUT);        //timing test out
  pinMode(5,OUTPUT);        //timing test out

  Serial1.setFIFOSize(16384);
  if (digitalRead(2)==0) {
    Serial1.begin(500000); // on on
  }
  if (digitalRead(2)==1) {
    Serial1.begin(4800);   //off off
  }
}

void loop(){
  volatile unsigned int i,k,m,ii,jj,v;
  volatile int j,n,xp,yp,x1,y1,x2,y2,h;
  volatile unsigned int shft;
  volatile char c,c2;

//for plot function
  static char s[81];
  static char x[81];
  volatile float xp1,xp2,xp3,xp4;
  volatile int xar1[260];
  volatile int xar2[260];
  volatile int xar3[260];
  volatile int xar4[260];
  volatile float sf1,sf2,sf3,sf4;
  volatile int sfflg;
  volatile int nc = 0;
  volatile byte xhatch=0;
  volatile int plotmode=0;
  
  sf1 = 128.0/10.0;
  sf2 = 128.0/10.0;
  sf3 = 128.0/10.0;
  sf4 = 128.0/10.0;
  sfflg = 0;

  for (m=0; m<260;m++){
    xar1[m]=128;
    xar2[m]=128;
    xar3[m]=128;
    xar4[m]=128;
  }
  
   for (i=0; i < 307200 ; i++) framebuf[i] = 0x02; 
   for (ii=START_ADDR; ii < END_ADDR; ii=ii+LINE_LEN) {
    for (jj=H_LEFT; jj<H_CHAR_RIGHT;jj++) framebuf[ii+jj] =  0x08;
   }  
  
   draw_grid();
   v = START_ADDR;
   h = H_LEFT;
   s[0]=0;
   s[80]=0;
   i=0;
   
while(true){
  
     framebuf[v+h+CURS1] = framebuf[v+h+CURS1] ^ 0xFF;
     framebuf[v+h+CURS2] = framebuf[v+h+CURS2] ^ 0xFF;
     framebuf[v+h+CURS3] = framebuf[v+h+CURS3] ^ 0xFF;
     framebuf[v+h+CURS4] = framebuf[v+h+CURS4] ^ 0xFF;
     framebuf[v+h+CURS5] = framebuf[v+h+CURS5] ^ 0xFF;
     
     gpio_put(4,LOW);      
     while (Serial1.available() == 0) if (digitalRead(3)==LOW) break;
     gpio_put(4,HIGH); 

     framebuf[v+h+CURS1] = framebuf[v+h+CURS1] ^ 0xFF;
     framebuf[v+h+CURS2] = framebuf[v+h+CURS2] ^ 0xFF;
     framebuf[v+h+CURS3] = framebuf[v+h+CURS3] ^ 0xFF;
     framebuf[v+h+CURS4] = framebuf[v+h+CURS4] ^ 0xFF;
     framebuf[v+h+CURS5] = framebuf[v+h+CURS5] ^ 0xFF;
     
     c = Serial1.read() & 0x7f;
       
     if (c=='#') {sfflg=1; i = 0;  s[i]=0;}
     if (((c>='0')&&(c<='9'))||(c=='.')||(c=='+')||(c=='-')||(c==' ')||(c==',')){
      s[i]=c;
      s[i+1]=0;
      i++;
      if (i>78) i=78;
    } 
       
   if ((c>=0x20)&&(h<H_CHAR_RIGHT)&&(digitalRead(3)==HIGH)) {
    shft = 0x10;  
    for (k=0; k<7; k++){               //Write matrix character to screen
     n = LINE_6 - (k * LINE_LEN);
     m = bc[c][k];
     shft = 0x010;
     if ((m & shft)==0) framebuf[v+h+n] = 0x08;  else framebuf[v+h+n] = 0xff;
     shft = shft>>1;
     n++;
     if ((m & shft)==0) framebuf[v+h+n] = 0x08;  else framebuf[v+h+n] = 0xff;
     shft = shft>>1;
     n++;
     if ((m & shft)==0) framebuf[v+h+n] = 0x08;  else framebuf[v+h+n] = 0xff;
     shft = shft>>1;
     n++;
     if ((m & shft)==0) framebuf[v+h+n] = 0x08;  else framebuf[v+h+n] = 0xff;
     shft = shft>>1;
     n++;
     if ((m & shft)==0) framebuf[v+h+n] = 0x08;  else framebuf[v+h+n] = 0xff;
     n++;
     framebuf[v+h+n] = 0x08;
     }
    } else {
     switch(c) {
      case 0x08:    //back space
        h = h-12;
        break;
      case 0x0a:    //line feed
        v = v + TEXT_LINE_LEN;
        h = h-6;
        break;
      case 0x0c:    //form feed
        clr_screen();
        draw_grid();
        if (plotmode!=0) plotmode=1;
        s[0]=0;
        i=0;
        v = START_ADDR;
        h = H_NEWLINE;
        break;  
      case 0x0d:    //carriage return
        h = H_NEWLINE;
        s[i]=0;
        break;  
      case 0x1a:    //cntr-z Pico Reset
        pinMode(22,OUTPUT);
        digitalWrite(22,0);
        break;  
      case 0x1b:    //esc
        h = h - 6;
        break;  
      case 0x14:    //cntr-t set x-t mode
        h = h - 6;
        plotmode=0;
        draw_grid();
        s[0]=0;
        i=0;
        break;  
      case 0x19:    //cntr-y set x-y mode
        h = h - 6;
        plotmode=1;
        draw_grid();
        s[0]=0;
        i=0;
        break;  
      default:
        h = h - 6;
        break;  
      }
      if (h<H_NEWLINE) h = H_NEWLINE;
     }
     
    if (h < H_CHAR_RIGHT) h = h + 6;
        
    if (v>=CHAR_END_ADDR){
      v = v-TEXT_LINE_LEN;
      if (digitalRead(3)==HIGH){
       for (jj=TEXT_START;jj<CHAR_END_ADDR;jj=jj+LINE_LEN){ 
        memmove(&framebuf[jj],&framebuf[jj+TEXT_LINE_LEN],H_CHARS);
       }
      }
    }

if (c==0x0d){
  i=0;
  xp1=0.0;
  xp2=0.0;
  xp3=0.0;
  xp4=0.0;

 if (s[0]!=0){
  nc = 0;
 while(true){
   while ((s[i]==' ')||(s[i]==',')) i++;
   if (s[i]==0) break;
   j=0;
   while ((s[i]!=0)&&(s[i]!=' ')&&(s[i]!=',')) x[j++]=s[i++];
   x[j]=0;
   xp1=atof(x);
   if (sfflg!=0){
    if (xp1==0.0) xp1 = 10;
    sf1 = 128.0/xp1;
   }
   nc++;
   if (s[i]==0) break;
 
   while ((s[i]==' ')||(s[i]==',')) i++;
   if (s[i]==0) break;
   j=0;
   while ((s[i]!=0)&&(s[i]!=' ')&&(s[i]!=',')) x[j++]=s[i++];
   x[j]=0;
   xp2=atof(x); 
   if (sfflg!=0){
    if (xp2==0.0) xp2 = 10;
    sf2 = 128.0/xp2;
   }
   nc++;
   if (s[i]==0) break;
   
   while ((s[i]==' ')||(s[i]==',')) i++;
   if (s[i]==0) break;
   j=0;
   while ((s[i]!=0)&&(s[i]!=' ')&&(s[i]!=',')) x[j++]=s[i++];
   x[j]=0;
   xp3=atof(x);  
   if (sfflg!=0){
    if (xp3==0.0) xp3 = 10;
    sf3 = 128.0/xp3;
   }
   nc++;
   if (s[i]==0) break;
   
   while ((s[i]==' ')||(s[i]==',')) i++;
   if (s[i]==0) break;
   j=0;
   while ((s[i]!=0)&&(s[i]!=' ')&&(s[i]!=',')) x[j++]=s[i++];
   x[j]=0;
   xp4=atof(x);  
   if (sfflg!=0){
    if (xp4==0.0) xp4 = 10;
    sf4 = 128.0/xp4;
   }
   nc++;
   break;
 }  

  sfflg=0;
  gpio_put(4,HIGH);

  if (nc!=0){
        j=128-int(xp1*sf1);
        if (j<0) j = 0;
        if (j>256) j = 256;
       xar1[257]=j;

        j=128-int(xp2*sf2);
        if (j<0) j = 0;
        if (j>256) j = 256;
        xar2[257]=j;

        j=128-int(xp3*sf3);
        if (j<0) j = 0;
        if (j>256) j = 256;
        xar3[257]=j;

        j=128-int(xp4*sf4);
        if (j<0) j = 0;
        if (j>256) j = 256;
        xar4[257]=j;

  for (m=0; m<257;m++){ 
    xar1[m]=xar1[m+1];
    xar2[m]=xar2[m+1];
    xar3[m]=xar3[m+1];
    xar4[m]=xar4[m+1];
  }
  
 if (plotmode==0){
   
    gpio_put(5,HIGH);
    for (jj=H_PLOT_START;jj<(H_PLOT_START+(256*LINE_LEN));jj=jj+LINE_LEN){
      memmove(&framebuf[jj],&framebuf[jj+1],256);
    }
    gpio_put(5,LOW);  
    if ((xhatch++%32)==0) {
      line_vert(256,0x04);
    } else {
      line_vert(256,0x08);      
      framebuf[H_PLOT_START+256] = 0x04;
      framebuf[H_PLOT_START+(20480)+256] = 0x04;
      framebuf[H_PLOT_START+(2*20480)+256] = 0x04;
      framebuf[H_PLOT_START+(3*20480)+256] = 0x04;
      framebuf[H_PLOT_START+(4*20480)+256] = 0x04;
      framebuf[H_PLOT_START+(5*20480)+256] = 0x04;
      framebuf[H_PLOT_START+(6*20480)+256] = 0x04;
      framebuf[H_PLOT_START+(7*20480)+256] = 0x04;
      framebuf[H_PLOT_START+(8*20480)+256] = 0x04;
    }

    line_to_fast(255,xar1[255],xar1[256],0x1c); //lime
    line_to_fast(255,xar2[255],xar2[256],0x93); //violet
    line_to_fast(255,xar3[255],xar3[256],0xff); //white
    line_to_fast(255,xar4[255],xar4[256],0xc0); //red
    
    } else if (plotmode++ > 2){
      line_to(256-xar1[255],xar2[255],256-xar1[256],xar2[256],0x1c);
      plotmode=3;
    }    
   }
  }
  gpio_put(4,LOW);
  i=0;
 }   
 }
}
