/*
 * Controle de display de 4 linhas por 40 colunas
 * 
 * Restrições
 * - suporta apenas conexão com 4bit de dados
 * - assume R/W ligado a terra (sempre escrita)
 * - funções mínimas para implementar o terminal
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

// Construtor
Disp0440::Disp0440(byte rs, byte en1, byte en2,
          byte d0, byte d1, byte d2, byte d3) {
  // Salva conexões
  pinRS = rs;
  pinEN1 = en1;
  pinEN2 = en2;
  pinD0 = d0;
  pinD1 = d1;
  pinD2 = d2;
  pinD3 = d3;
}

// Iniciação
void Disp0440::init(void) {
  // Todos os pinos são saida
  pinMode (pinRS, OUTPUT);
  pinMode (pinEN1, OUTPUT);
  pinMode (pinEN2, OUTPUT);
  pinMode (pinD0, OUTPUT);
  pinMode (pinD1, OUTPUT);
  pinMode (pinD2, OUTPUT);
  pinMode (pinD3, OUTPUT);
  
  // Inicia os controladores
  init(pinEN1);
  init(pinEN2);

  // Limpa a tela e coloca cursor em home
  enAtl = 0xFF; // para forçar atualização do cursor
  clear();

  // Mosta a tela
  writeCmd (pinEN1, CMD_DISPON_CURON);
  delay(1);    
  writeCmd (pinEN2, CMD_DISPON_NOCUR);
  delay(1);
}

// Limpa a tela e coloca o cursor em home
void Disp0440::clear(void) {
  memset(screen, 0x20, sizeof(screen));
  writeCmd (pinEN1, CMD_CLS);
  delay(2);    
  writeCmd (pinEN2, CMD_CLS);
  delay(2);    
  setCursor(0, 0);
}

// Posiciona o cursor
void Disp0440::setCursor(byte line, byte col) {
  byte en;
  byte addr = col;

  // Determina controlador e endereço
  switch(line) {
    case 0:
      en = pinEN1;
      break;
    case 1:
      addr +=0x40;
      en = pinEN1;
      break;
    case 2:
      en = pinEN2;
      break;
    case 3:
      addr +=0x40;
      en = pinEN2;
      break;
  }

  // Se mudou de controlador, acerta visibilidade do cursor
  if (en != enAtl) {
    if (enAtl != 0xFF) {
      writeCmd (enAtl, CMD_DISPON_NOCUR);
    }
    writeCmd (en, CMD_DISPON_CURON);
    enAtl = en;
  }

  // Posiciona o cursor
  writeCmd(en, CMD_POSCUR | addr);
  linAtl = line;
  colAtl = col;
}

// Escreve caracter
//   \r    volta a coluna 0
//   \n    passa para a linha abaixo
//   0x08  volta para coluna anterior
//   0x0C  limpa a tela
void Disp0440::write(byte car) {
  byte lin, col;

  lin = linAtl;
  col = colAtl;
  
  // Tratar os caracteres de controle
  if (car == '\r') {
    // Carriage Return
    setCursor(lin, 0);
  } else if (car == 0x0C) {
    // Form Feed
    clear();
  } else if (car == 0x08) {
    // Backspace
    if (colAtl > 0) {
      col = colAtl - 1;
    } else if (linAtl > 0) {
      col = 39;
      lin--;
    }
    setCursor(lin, col);
  } else if (car == '\n') {
    // Line Feed
    if (lin < 3) {
      setCursor(lin+1, col);
    } else {
      rollUp();
      setCursor(lin, col);
    }
  } else {
    // Caracter normal
    writeChar(enAtl, car);
    screen[linAtl*40+colAtl] = car;
    if (++col > 39) {
      col = 0;
      if (lin < 3) {
        lin++;
      } else {
        rollUp();
      }
    }
    setCursor(lin, col);
  }
}

// Escreve um string a partir de uma linha e coluna
void Disp0440::writeMsg(byte line, byte col, char *msg) {
  setCursor(line, col);
  while (*msg) {
    write(*msg++);
  }
}

// Controla o sinal RS (seleção registrador / memória)
inline void Disp0440::setRS(byte valor) {
  digitalWrite(pinRS, valor);
}

// Controla o sinal E (enable)
inline void Disp0440::setE(byte en, byte valor) {
  digitalWrite(en, valor);
}

// Controla os sinais de dados
void Disp0440::setData(byte nib) {
  digitalWrite(pinD0, (nib & 1) ? HIGH : LOW);
  digitalWrite(pinD1, (nib & 2) ? HIGH : LOW);
  digitalWrite(pinD2, (nib & 4) ? HIGH : LOW);
  digitalWrite(pinD3, (nib & 8) ? HIGH : LOW);
}

// Escreve um byte no controlador selecionado por en
void Disp0440::writeByte(byte en, byte rs, byte data) {
  setRS(rs);
  setE(en, HIGH);
  setData(data >> 4);
  delay(1);
  setE(en, LOW);
  setE(en, HIGH);
  setData(data & 0x0F);
  delay(1);
  setE(en, LOW);
}

// Escreve um comando no controlador selecionado por en
inline void Disp0440::writeCmd(byte en, byte cmd) {
  writeByte(en, CMD, cmd);
}

// Escreve um caracter no controlador selecionado por en
inline void Disp0440::writeChar(byte en, byte chr) {
  writeByte(en, DATA, chr);
}

// Rola toda a tela uma linha para cima (na raça)
void Disp0440::rollUp() {
  // Rola a imagem salva
  memmove (screen, screen+40, 3*40);
  memset (screen+3*40, ' ', 40);
  // Escreve a imagem nos controladores
  writeCmd(pinEN1, CMD_POSCUR);
  for (int i = 0; i< 40; i++) {
    writeChar (pinEN1, screen[i]);
  }
  writeCmd(pinEN1, CMD_POSCUR | 0x40);
  for (int i = 0; i< 40; i++) {
    writeChar (pinEN1, screen[40+i]);
  }
  writeCmd(pinEN2, CMD_POSCUR);
  for (int i = 0; i< 40; i++) {
    writeChar (pinEN2, screen[2*40+i]);
  }
  writeCmd(pinEN2, CMD_POSCUR | 0x40);
  for (int i = 0; i< 40; i++) {
    writeChar (pinEN2, screen[3*40+i]);
  }
}
    
// Inicia o controlador selecionado por en
void Disp0440::init(byte en) {
  delay (100);
  writeCmd (en, 0x03);
  delay(5);    
  writeCmd (en, 0x03);
  delay(1);    
  writeCmd (en, 0x03);
  delay(1);    
  writeCmd (en, 0x02);
  delay(1);    
  writeCmd(en, CMD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
  delay(1);    
} 
