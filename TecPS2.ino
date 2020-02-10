/*
 * Interface com teclado PS/2
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

#include "TecPS2.h"

/*
 * Resumo dos códigos recebidos pelo teclado (por default é o "Scan Code Set 2)
 * 
 *  E1 no nosso contexto, usado somente pela tecla Pause
 *     E1 14 77 E1 F0 14 F0 77
 *  E0 tecla extendida
 *     F0 xx soltou  tecla extendida
 *     xx    apertou tecla extendida
 *  F0 xx soltou  tecla normal
 *  xx apertou tecla normal
 * 
 *  Nós estamos interessados somente nas teclas modificadoras Shift, Control, Caps
 *  e nas teclas que geram código ASCII (0 a 7F)
 * 
 */

namespace TecPS2 {

  // Tabelas de decodificação do teclado
  // (o teclado utilizado tem o layout americano)
  const byte tabNormal [127] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x60, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00,  'q',  '1', 0x00, 0x00, 0x00,  'z',  's',  'a',  'w',  '2', 0x00, 
    0x00,  'c',  'x',  'd',  'e',  '4',  '3', 0x00, 0x00,  ' ',  'v',  'f',  't',  'r',  '5', 0x00, 
    0x00,  'n',  'b',  'h',  'g',  'y',  '6', 0x00, 0x00, 0x00,  'm',  'j',  'u',  '7',  '8', 0x00,
    0x00,  ',',  'k',  'i',  'o',  '0',  '9', 0x00, 0x00,  '.',  '/',  'l',  ';',  'p',  '-', 0x00, 
    0x00, 0x00, '\'', 0x00,  '[',  '=', 0x00, 0x00, 0x00, 0x00, 0x0D,  ']', 0x00, '\\', 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00,  '1', 0x00,  '4',  '7', 0x00, 0x00, 0x00, 
     '0',  '.',  '2',  '5',  '6',  '8', 0x1B, 0x00, 0x00,  '+',  '3',  '-',  '*',  '9', 0x00
  };
  
  const byte tabShift [127] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x7E, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00,  'Q',  '!', 0x00, 0x00, 0x00,  'Z',  'S',  'A',  'W',  '@', 0x00, 
    0x00,  'C',  'X',  'D',  'E',  '$',  '#', 0x00, 0x00,  ' ',  'V',  'F',  'T',  'R',  '%', 0x00, 
    0x00,  'N',  'B',  'H',  'G',  'Y',  '^', 0x00, 0x00, 0x00,  'M',  'J',  'U',  '&',  '*', 0x00,
    0x00,  '<',  'K',  'I',  'O',  ')',  '(', 0x00, 0x00,  '>',  '?',  'L',  ':',  'P',  '_', 0x00, 
    0x00, 0x00, '\"', 0x00,  '{',  '+', 0x00, 0x00, 0x00, 0x00, 0x0D,  '}', 0x00,  '|', 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00,  '1', 0x00,  '4',  '7', 0x00, 0x00, 0x00, 
     '0',  '.',  '2',  '5',  '6',  '8', 0x1B, 0x00, 0x00,  '+',  '3',  '-',  '*',  '9', 0x00
  };
  
  // Conexões de hardware
  int pinoData;
  
  // Estado do tratamento
  typedef enum { 
    NORMAL, SOLTOU, EXTENDIDA, SOLTOU_EXTENDIDA, 
    PAUSE_1, PAUSE_2, PAUSE_3, PAUSE_4, PAUSE_5, PAUSE_6, PAUSE_7 
  } ESTADO_TEC;
  ESTADO_TEC estadoTec = NORMAL;
  
  // Controle das teclas modificadoras
  const byte MSK_SHFT_R = 0x01;
  const byte MSK_SHFT_L = 0x02;
  const byte MSK_CTRL_R = 0x04;
  const byte MSK_CTRL_L = 0x08;
  const byte MSK_CAPS   = 0x10;
  byte modTec = 0; 
  
  // fila para os códigos recebidos e decodificados
  const byte TAMFILA = 16;
  byte poe = 0;
  byte tira = 0;
  byte fila[TAMFILA];

  void kbdint(void);

  // Iniciacao
  void init (int pinData, int pinClock) {
    // iniciar os pinos
    pinMode (pinData, INPUT_PULLUP);
    pinMode (pinClock, INPUT_PULLUP);
    pinoData = pinData;
    // assumir a interrupção do sinal de clock
    attachInterrupt(digitalPinToInterrupt(pinClock), kbdint, FALLING);
  }

  // Le uma tecla
  // Retorna 0xFF se fila vazia
  byte readKey (void) {
    byte key = 0xFF;
    if (poe != tira) {
      key = fila[tira];
      byte prox = tira+1;
      if (prox == TAMFILA) {
        tira = 0;
      } else {
        tira = prox;
      }
    }
    return key;
  }
    
  // Trata a interrupção do teclado
  void kbdint(void) {
    static byte cod = 0;
    static byte nbit = 0;
  
    // le o bit de dados
    int dado = digitalRead(pinoData);
  
    // despreza start, paridade e stop
    if ((nbit > 0) && (nbit < 9)) {
      cod = cod >> 1;
      if (dado == HIGH) {
        cod |= 0x80;
      }
    }
    nbit++;
    if (nbit == 11) {
      byte decod = 0;
      switch (estadoTec) {
          case NORMAL:
              if (cod == 0x58) {
                  modTec ^= MSK_CAPS;
              } else if (cod == 0x12) {
                  modTec |= MSK_SHFT_L;
              } else if (cod == 0x14) {
                  modTec |= MSK_CTRL_L;
              } else if (cod == 0x59) {
                  modTec |= MSK_SHFT_R;
              } else if (cod < 127) {
                  if (modTec & (MSK_SHFT_R | MSK_SHFT_L)) {
                      decod = pgm_read_byte(&tabShift[cod]);
                  } else {
                      decod = pgm_read_byte(&tabNormal[cod]);
                  }
              } else if (cod == 0xE0) {
                  estadoTec = EXTENDIDA;
              } else if (cod == 0xF0) {
                  estadoTec = SOLTOU;
              } else if (cod == 0xE1) {
                  estadoTec = PAUSE_1;
              }
              break;
          case SOLTOU:
              if (cod == 0x12) {
                  modTec &= ~MSK_SHFT_L;
              } else if (cod == 0x14) {
                  modTec &= ~MSK_CTRL_L;
              } else if (cod == 0x59) {
                  modTec &= ~MSK_SHFT_R;
              }
              estadoTec = NORMAL;
              break;
          case EXTENDIDA:
              if (cod == 0xF0) {
                estadoTec = SOLTOU_EXTENDIDA;
              } else {
                if (cod == 0x14) {
                    modTec |= MSK_CTRL_R;
                } else if (cod == 0x71) {
                    decod = 0x7F; // DEL
                } else if (cod == 0x4A) {
                    decod = '/'; // barra no keypad
                } else if (cod == 0x5A) {
                    decod = 0x0D; // Enter no keypad
                }
                estadoTec = NORMAL;
              }
              break;
          case SOLTOU_EXTENDIDA:
              if (cod == 0x14) {
                  modTec &= ~MSK_CTRL_R;
              }
              estadoTec = NORMAL;
              break;
          case PAUSE_1:
              estadoTec = PAUSE_2;
              break;
          case PAUSE_2:
              estadoTec = PAUSE_3;
              break;
          case PAUSE_3:
              estadoTec = PAUSE_4;
              break;
          case PAUSE_4:
              estadoTec = PAUSE_5;
              break;
          case PAUSE_5:
              estadoTec = PAUSE_6;
              break;
          case PAUSE_6:
              estadoTec = PAUSE_7;
              break;
          case PAUSE_7:
              estadoTec = NORMAL;
              break;
      }
      if (decod > 0) {
        if (decod < 128) {
          if ((modTec & (MSK_CTRL_R | MSK_CTRL_L)) && (decod > 0x3F)) {
            // Control ligado
            decod &= 0x1F;
          } else if (modTec & MSK_CAPS)  {
            // Caps ligado
            if ((decod >= 'A') && (decod <= 'Z')) {
              decod += 32;
            } else if ((decod >= 'a') && (decod <= 'z')) {
              decod -= 32;
            }
          }
          // coloca na fila
          fila[poe] = decod;
          byte prox = poe+1;
          if (prox == TAMFILA) {
            prox = 0;
          }
          if (prox != tira) {
            poe = prox;
          }
        }
      }
      nbit = cod = 0;
    }
  }
}
