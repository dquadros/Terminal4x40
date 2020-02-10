/*
 * Term4x40 - Terminal simples com display de 4 linhas de 40 colunas
 * 
 * Display: CCM-4040 LCD 4x40, controlado por 2 HD44780
 * Teclado: Teclado de PC com interface PS/2
 * Serial:  115200bps, 8N1
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

#include "Disp0440.h"
#include "TecPS2.h"

// Conexões do display
const int pinoD7 = 4;
const int pinoD6 = 5;
const int pinoD5 = 6;
const int pinoD4 = 7;
const int pinoE1 = 8;
const int pinoE2 = 9;
const int pinoRS = 10;

// Conexões do teclado
const int pinTecData = 3;
const int pinTecClock = 2;

// Objeto para acesso ao display
Disp0440 disp(pinoRS, pinoE1, pinoE2, pinoD4, pinoD5, pinoD6, pinoD7);

// Iniciação
void setup() {
  pinMode (LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  TecPS2::init (pinTecData, pinTecClock);
  disp.init();
  disp.writeMsg (0, 0, "DQSoft Terminal v1.00");
  delay (2000);
  disp.clear ();
}

// Laço Principal
void loop() {
  // Trata caracteres recebidos
  if (Serial.available()) {
    disp.write(Serial.read());
  }
  // Trata teclado
  byte tec = TecPS2::readKey();
  if (tec != 0xFF) {
    Serial.write(tec);
  }
}
