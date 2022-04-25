/*
  This file is part of the MKR GSM library.
  Copyright (C) 2017  Arduino AG (http://www.arduino.cc/)

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _MODEM_INCLUDED_H
#define _MODEM_INCLUDED_H

#include <stdarg.h>
#include <stdio.h>

#include <Arduino.h>

// Etiquetas de configuracion
#define MODEM_GSM_PIN_RX 14
#define MODEM_GSM_PIN_TX 2
#define MODEM_GSM_PIN_PWR_KEY 12
#define MODEM_GSM_PIN_RESET -1
#define MODEM_GSM_PIN_DTR -1
#define MODEM_UART_BAUDRATE 115200
#define GSM_SOCKET_BUFFER_SIZE 1024

class ModemUrcHandler
{
public:
  virtual void handleUrc(const String &urc) = 0;
};

class ModemClass
{
public:
  ModemClass(HardwareSerial &uart, unsigned long baud, int resetPin, int dtrPin, int pwdKeyPin = -1); // añadido pwdkeyPin

  int begin(bool restart = true, unsigned long baud = MODEM_UART_BAUDRATE, uint32_t config = SERIAL_8N1, int8_t rxPin = MODEM_GSM_PIN_RX, int8_t txPin = MODEM_GSM_PIN_TX); // añadido parámetros de uart
  void end();

  void debug();
  void debug(Print &p);
  void noDebug();

  int autosense(unsigned int timeout = 10000);

  int noop();
  int reset();
  int turnOff();

  int lowPowerMode();
  int noLowPowerMode();

  size_t write(uint8_t c);
  size_t write(const uint8_t *, size_t);

  void send(const char *command);
  void send(const String &command) { send(command.c_str()); }
  void sendf(const char *fmt, ...);

  int waitForResponse(unsigned long timeout = 100, String *responseDataStorage = NULL);
  int waitForPrompt(unsigned long timeout = 500);
  int waitForPrompt2(unsigned long timeout = 500);
  int waitForEndReadFile(unsigned long timeout, uint8_t *buffer);
  int ready();
  void poll();
  void setResponseDataStorage(String *responseDataStorage);

  void addUrcHandler(ModemUrcHandler *handler);
  void removeUrcHandler(ModemUrcHandler *handler);

  void setBaudRate(unsigned long baud);

private:
  HardwareSerial *_uart;
  unsigned long _baud;
  int _resetPin;
  int _dtrPin;
  int _pwdKey; // añadido pwdkey
  bool _lowPowerMode;
  unsigned long _lastResponseOrUrcMillis;

  enum
  {
    AT_COMMAND_IDLE,
    AT_RECEIVING_RESPONSE
  } _atCommandState;
  int _ready;
  String _buffer;
  String *_responseDataStorage;

#define MAX_URC_HANDLERS 10 // 7 sockets + GPRS + GSMLocation + GSMVoiceCall
  static ModemUrcHandler *_urcHandlers[MAX_URC_HANDLERS];
  static Print *_debugPrint;
};

extern ModemClass MODEM;

#endif
