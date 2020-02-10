/*
 * Controle de display de 4 linhas por 40 colunas
 * 
 * MIT License
 *
 * Copyright (c) 2020, Daniel Quadros
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#ifndef Disp0440_h
#define Disp0440_h

#include <inttypes.h>

class Disp0440 {

  public:
    Disp0440(byte rs, byte en1, byte en2,
          byte d0, byte d1, byte d2, byte d3);

    void init(void);
    void clear(void);
    void setCursor(byte line, byte col); 
    void write(byte car);
    void writeMsg(byte line, byte col, char *msg);

  private:
    void setRS(byte valor);
    void setE(byte en, byte valor);
    void setData(byte nib);
    void writeByte(byte en, byte rs, byte data);
    void writeCmd(byte en, byte cmd);
    void writeChar(byte en, byte chr);
    void rollUp(void);
    void init(byte en);

    const byte CMD = LOW;
    const byte DATA = HIGH;
    
    const byte CMD_CLS = 0x01;
    const byte CMD_DISPON_CURON = 0x0E;
    const byte CMD_DISPON_NOCUR = 0x0C;
    const byte CMD_POSCUR = 0x80;
    const byte CMD_FUNCTIONSET = 0x20;
    const byte LCD_4BITMODE = 0x00;
    const byte LCD_2LINE = 0x08;
    const byte LCD_5x8DOTS = 0x00;

    byte pinRS, pinEN1, pinEN2;
    byte pinD0, pinD1, pinD2, pinD3;

    byte linAtl, colAtl, enAtl;
    byte screen[160];
};


#endif
