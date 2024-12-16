/*
Copyright (C) 2017 Steve Benz <s8878992@hotmail.com>

Modified by Simple-Circuit 2024 for 500K baud and cntr-r Block Program 
example send. Nine Block Diagram Programs are stored in Flash

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
USA
*/
#include "ps2_Keyboard.h"
#include "ps2_AnsiTranslator.h"
#include "ps2_SimpleDiagnostics.h"

typedef ps2::SimpleDiagnostics<254> Diagnostics_;
static Diagnostics_ diagnostics;
static ps2::AnsiTranslator<Diagnostics_> keyMapping(diagnostics);
static ps2::Keyboard<3,2,1, Diagnostics_> ps2Keyboard(diagnostics);
static ps2::KeyboardLeds lastLedSent = ps2::KeyboardLeds::none;

const char pgm1[] PROGMEM  ={
"rem sin cos text out\r"
"#10 10 10 10\r"
"clr\r"
"osc w os mag sin\r"
"sub 0 sin neg\r"
"int neg w lim cos\r"
"prt sin cos\r"
"end\r"
"set dt 0.015\r"
"set w 5\r"
"set max 10000\r"
"set mag 8\r"
"set cos 8\r"
"set fmt 2\r"
};

const char pgm2[] PROGMEM  ={
"rem third order system\r"
"#100 100 256\r"
"clr\r"
"mul a2 i1 m1\r" 
"int s1 one L i1\r" 
"mul a1 i2 m4\r" 
"int i1 one L i2\r" 
"sum m1 m4 s2\r" 
"mul P i3 m3\r" 
"int i2 one L i3\r" 
"mul m2 b0 out\r" 
"sum s2 m5 s3\r" 
"mul i3 P m2\r" 
"mul m3 a0 m5\r" 
"prt out in t\r" 
"sub in s3 s1\r" 
"end\r"
"set a2 0.45\r" 
"set a1 0.05\r" 
"set a0 5.15e-6\r" 
"set P 2000\r" 
"set dt 0.1\r" 
"set max -253\r" 
"set one 1\r" 
"set in 40\r" 
"set b0 5.15e-6\r" 
"set dec 10\r" 
"set L 0.0\r" 
"set b0 5.15e-6\r" 
"set 100 100\r"
"set fmt 2\r"
};

const char pgm3[] PROGMEM  ={
"clr Blink buit-in LED using multiply block\r"
"#2\r"
"mul p -1 p\r"
"out p LED\r"
"prf p\r"
"prf p\r"
"prf p\r"
"prf p\r"
"end\r"
"set -1 -1\r"
"set max 1000\r"
"set p 1\r"
"set LED 25\r"
"set dt 1\r"  
};
const char pgm4[] PROGMEM  ={
"clr 10KHz ADC read with plot\r"
"#10\r"
"osc w 0 5 out\r"
"dac out 0\r"
"bnz lp 5\r"
"rst 1 lp\r"
"rst 0 m\r"
"rst 0 n\r"
"rst 0 tm\r"
"sub n n2 flg\r"
"brp flg 5\r"
"adc ch0 x\r"
"sta x n\r"
"sum n 1 n\r"
"end\r"
"sub m n2 flg\r"
"brp flg 5\r"
"lda m x\r"
"prt x\r"
"sum m 1 m\r"
"end\r"
"sub tm t3 flg\r"
"brp flg 3\r"
"sum tm 1 tm\r"
"end\r"
"rst 0 lp\r"
"end\r"
"set 1 1\r"
"set 2 2\r"
"set 3 3\r"
"set n2 258\r"
"set 4 4\r"
"set 5 5\r"
"set t3 20000\r"
"set dt 0.0001\r"
"set w 1000\r"
"set avg 16\r"
"set fmt 3\r"
"set max 9999\r"
};

const char pgm5[] PROGMEM  ={
"clr calculate RMS voltage for a Sine wave\r"
"#5 5\r"
"osc w os mag out\r"
"mul out out o2\r"
"sub o2 io in\r"
"int in g1 lim io\r"
"sqt io rms\r"
"prt out rms\r"
"end\r"
"set g1 1\r"
"set dt 0.001\r"
"set max 5.12\r"
"set w 37.7\r"
"set os 0\r"
"set mag 5\r"
"set dec 10\r"  
};
const char pgm6[] PROGMEM  ={
"clr Find cube root of 3 and 7 using iteration\r"
"#10 10 10\r"
"swg tw out\r"
"mul out k mg\r"
"sum mg g g2\r"
"sum out os in\r"
"sub in i3 s\r"
"eul s g2 lim i\r"
"mul i i i2\r"
"mul i i2 i3\r"
"prt in i i3\r"
"prt in i i3\r"
"end\r"
"set out 2\r"
"set os 5\r"
"set tw 0.02\r"
"set dt 0.001\r"
"set max 0.128\r"
"set g 40\r"
"set k -3\r"
"set fmt 4\r" 
};

const char pgm7[] PROGMEM  ={
"clr low pass filter\r"
"#10 10\r"
"osc g1 os mag sin\r"
"dac sin ch0\r"
"adc ch0 a\r"
"sub a out in\r"
"eul in g1 lim out\r"
"prt a out\r"
"end\r"
"set ch0 0\r"
"set g1 4.906\r"
"set lim 10\r"
"set os 0\r"
"set mag 7.07\r"
"set dt 0.01\r"
"set max 1000\r"
};

const char pgm8[] PROGMEM  ={
"clr gravity velocity position\r"
"#10 20 10 4\r"
"eul acc g lim vel\r" 
"eul vel g lim pos\r" 
"prt acc vel pos t\r" 
"end\r" 
"set acc -9.8\r"
"set pos 10\r" 
"set g 1\r" 
"set dt 0.006\r" 
"set max 1.524\r"
"set fmt 3\r" 
};

const char pgm9[] PROGMEM  ={
"clr Curve Tracer\r"
"#10 10\r"
"osc w os mag sin\r"
"adc ch0 v\r"
"adc ch1 v2\r"
"dac sin ch0\r"
"sub v2 v i\r"
"prt v i\r"
"end\r"
"set dt 0.01\r"
"set w 3\r"
"set max 10000\r"
"set ch1 1\r"
"set mag 9.8\r"
"set avg 16\r"
"set fmt 3\r"
"* adc1,dac0---1K--adc0,dut---gnd\r"
"* cal adc, cntr-y for plot\r"  
};

volatile int k;
volatile char c;
volatile int pgm = 0;
char buf[2];

void setup() {
    Serial.begin(500000); 
    ps2Keyboard.begin();
    keyMapping.setNumLock(true);
    ps2Keyboard.awaitStartup();

    // see the docs for awaitStartup - TL;DR <- when we reset the board but not the keyboard, awaitStartup
    //  records an error because it thinks the keyboard didn't power-up correctly.  When debugging, that's
    //  true - but only because it never powered down.
    diagnostics.reset();

    ps2Keyboard.sendLedStatus(ps2::KeyboardLeds::numLock);
    lastLedSent = ps2::KeyboardLeds::numLock;
}

void loop() {

    diagnostics.setLedIndicator<LED_BUILTIN>();
    ps2::KeyboardOutput scanCode = ps2Keyboard.readScanCode();
    if (scanCode == ps2::KeyboardOutput::garbled) {
        keyMapping.reset();
    }
    else if (scanCode != ps2::KeyboardOutput::none)
    {

        buf[1] = '\0';
        buf[0] = keyMapping.translatePs2Keycode(scanCode);
        if ((pgm==0)&&(buf[0]!=18)) Serial.write(buf);
        else if ((buf[0] > '0')&&(buf[0]<='9')&&(pgm=-1)){
          pgm = int(buf[0]-'0');
        }
        if (buf[0] == 18){ 
          Serial.println("... to Load Program Press 1-9");
          pgm = -1;
        }
        if (pgm > 0){  
         delay(100); 
         switch (pgm){  
          case 1:
           for (k = 0; k < strlen_P(pgm1); k++){
            c = pgm_read_byte_near(pgm1 + k);
           Serial.write(c);
            delay(1);
            if (c=='\r') {delay(100); Serial.write(10);}
           } 
           pgm = 0;
           break;
          case 2:
           for (k = 0; k < strlen_P(pgm2); k++){
            c = pgm_read_byte_near(pgm2 + k);
           Serial.write(c);
            delay(1);
            if (c=='\r') {delay(100); Serial.write(10);}
           } 
           pgm = 0;
           break;
          case 3:
           for (k = 0; k < strlen_P(pgm3); k++){
            c = pgm_read_byte_near(pgm3 + k);
           Serial.write(c);
            delay(1);
            if (c=='\r') {delay(100); Serial.write(10);}
           } 
           pgm = 0;
           break;  
          case 4:
           for (k = 0; k < strlen_P(pgm4); k++){
            c = pgm_read_byte_near(pgm4 + k);
           Serial.write(c);
            delay(1);
            if (c=='\r') {delay(100); Serial.write(10);}
           } 
           pgm = 0;
           break;           
          case 5:
           for (k = 0; k < strlen_P(pgm5); k++){
            c = pgm_read_byte_near(pgm5 + k);
           Serial.write(c);
            delay(1);
            if (c=='\r') {delay(100); Serial.write(10);}
           } 
           pgm = 0;
           break;           
          case 6:
           for (k = 0; k < strlen_P(pgm6); k++){
            c = pgm_read_byte_near(pgm6 + k);
           Serial.write(c);
            delay(1);
            if (c=='\r') {delay(100); Serial.write(10);}
           } 
           pgm = 0;
           break;           
          case 7:
           for (k = 0; k < strlen_P(pgm7); k++){
            c = pgm_read_byte_near(pgm7 + k);
           Serial.write(c);
            delay(1);
            if (c=='\r') {delay(100); Serial.write(10);}
           } 
           pgm = 0;
           break;           
          case 8:
           for (k = 0; k < strlen_P(pgm8); k++){
            c = pgm_read_byte_near(pgm8 + k);
           Serial.write(c);
            delay(1);
            if (c=='\r') {delay(100); Serial.write(10);}
           } 
           pgm = 0;
           break; 
          case 9:
           for (k = 0; k < strlen_P(pgm9); k++){
            c = pgm_read_byte_near(pgm9 + k);
           Serial.write(c);
            delay(1);
            if (c=='\r') {delay(100); Serial.write(10);}
           } 
           pgm = 0;
           break;            
          default:
           pgm=0;
           break; 
         } 
        } 
       
        buf[0]='\0';
        
        ps2::KeyboardLeds newLeds =
              (keyMapping.getCapsLock() ? ps2::KeyboardLeds::capsLock : ps2::KeyboardLeds::none)
            | (keyMapping.getNumLock() ? ps2::KeyboardLeds::numLock : ps2::KeyboardLeds::none);
        if (newLeds != lastLedSent) {
            ps2Keyboard.sendLedStatus(newLeds);
            lastLedSent = newLeds;
        }
    }
}
