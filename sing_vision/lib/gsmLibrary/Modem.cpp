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

#include "Modem.h"

#define MODEM_MIN_RESPONSE_OR_URC_WAIT_TIME_MS 40

ModemUrcHandler *ModemClass::_urcHandlers[MAX_URC_HANDLERS] = {NULL};
Print *ModemClass::_debugPrint = NULL;

ModemClass::ModemClass(HardwareSerial &uart, unsigned long baud, int resetPin, int dtrPin, int pwdKeyPin) : _uart(&uart),
                                                                                                            _baud(baud),
                                                                                                            _resetPin(resetPin),
                                                                                                            _dtrPin(dtrPin),
                                                                                                            _pwdKey(pwdKeyPin),
                                                                                                            _lowPowerMode(false),
                                                                                                            _lastResponseOrUrcMillis(0),
                                                                                                            _atCommandState(AT_COMMAND_IDLE),
                                                                                                            _ready(1),
                                                                                                            _responseDataStorage(NULL)
{
  _buffer.reserve(128);
}

int ModemClass::begin(bool restart, unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin)
{
  _uart->begin(_baud > 115200 ? 115200 : _baud, SERIAL_8N1, rxPin, txPin); // revisar configuración de la uart

  if (_pwdKey > -1)
  {
    pinMode(_pwdKey, OUTPUT);
    digitalWrite(_pwdKey, HIGH);
    delay(100);
    digitalWrite(_pwdKey, LOW);
    delayMicroseconds(65);
    digitalWrite(_pwdKey, HIGH);

    if (_debugPrint)
    {
      _debugPrint->write("PWR_KEY_SET");
    }

    delay(4000);
  }

  if (_resetPin > -1 && restart)
  {
    pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, HIGH);
    delay(100);
    digitalWrite(_resetPin, LOW);
  }
  else
  {
    if (!autosense())
    {
      return 0;
    }

    if (!reset())
    {
      return 0;
    }
  }

  if (!autosense())
  {
    return 0;
  }

  if (_baud > 115200)
  {
    sendf("AT+IPR=%ld", _baud);
    if (waitForResponse() != 1)
    {
      return 0;
    }

    _uart->end();
    delay(100);
    _uart->begin(_baud);

    if (!autosense())
    {
      return 0;
    }
  }

  if (_dtrPin > -1)
  {
    pinMode(_dtrPin, OUTPUT);
    noLowPowerMode();

    send("AT+UPSV=3");
    if (waitForResponse() != 1)
    {
      return 0;
    }
  }

  return 1;
}

int ModemClass::turnOff()
{
  if (_pwdKey > -1)
  {
    digitalWrite(_pwdKey, LOW);
    delay(1005);
    digitalWrite(_pwdKey, HIGH);
    return 0;
  }
  return -1;
}

void ModemClass::end()
{
  _uart->end(); 
  if (_resetPin > -1)
  {
    digitalWrite(_resetPin, HIGH);
  }
  if (_dtrPin > -1)
  {
    digitalWrite(_dtrPin, LOW);
  }
}

void ModemClass::debug()
{
  debug(Serial);
}

void ModemClass::debug(Print &p)
{
  _debugPrint = &p;
}

void ModemClass::noDebug()
{
  _debugPrint = NULL;
}

int ModemClass::autosense(unsigned int timeout)
{
  for (unsigned long start = millis(); (millis() - start) < timeout;)
  {
    if (noop() == 1)
    {
      return 1;
    }

    delay(100);
  }

  return 0;
}

int ModemClass::noop()
{
  send("AT");

  return (waitForResponse() == 1);
}

int ModemClass::reset()
{
  send("AT+CFUN=16");

  return (waitForResponse(1000) == 1);
}

int ModemClass::lowPowerMode()
{
  if (_dtrPin > -1)
  {
    _lowPowerMode = true;

    digitalWrite(_dtrPin, HIGH);

    return 1;
  }

  return 0;
}

int ModemClass::noLowPowerMode()
{
  if (_dtrPin > -1)
  {
    _lowPowerMode = false;

    digitalWrite(_dtrPin, LOW);

    return 1;
  }

  return 0;
}

size_t ModemClass::write(uint8_t c)
{
  return _uart->write(c);
}

size_t ModemClass::write(const uint8_t *buf, size_t size)
{
  return _uart->write(buf, size);
}

void ModemClass::send(const char *command)
{
  if (_lowPowerMode)
  {
    digitalWrite(_dtrPin, LOW);
    delay(5);
  }

  // compare the time of the last response or URC and ensure
  // at least 20ms have passed before sending a new command
  unsigned long delta = millis() - _lastResponseOrUrcMillis;
  if (delta < MODEM_MIN_RESPONSE_OR_URC_WAIT_TIME_MS)
  {
    delay(MODEM_MIN_RESPONSE_OR_URC_WAIT_TIME_MS - delta);
  }

  _uart->println(command);
  _uart->flush();
  _atCommandState = AT_COMMAND_IDLE;
  _ready = 0;
}

void ModemClass::sendf(const char *fmt, ...)
{
  char buf[BUFSIZ];

  va_list ap;
  va_start((ap), (fmt));
  vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
  va_end(ap);

  send(buf);
}

int ModemClass::waitForResponse(unsigned long timeout, String *responseDataStorage)
{
  _responseDataStorage = responseDataStorage;
  for (unsigned long start = millis(); (millis() - start) < timeout;)
  {
    int r = ready();

    if (r != 0)
    {
      _responseDataStorage = NULL;
      return r;
    }
  }

  _responseDataStorage = NULL;
  _buffer = "";
  return -1;
}

int ModemClass::waitForPrompt(unsigned long timeout)
{
  for (unsigned long start = millis(); (millis() - start) < timeout;)
  {
    ready();

    if (_buffer.endsWith(">"))
    {
      return 1;
    }
  }

  return -1;
}

int ModemClass::waitForPrompt2(unsigned long timeout)
{
  for (unsigned long start = millis(); (millis() - start) < timeout;)
  {
    ready();

    if (_buffer.endsWith("@"))
    {
      return 1;
    }
  }

  return -1;
}

int32_t ModemClass::waitForEndReadFile(unsigned long timeout, uint8_t *buffer)
{
  int32_t readBytes = 0;

  for (unsigned long start = millis(); (millis() - start) < timeout;)
  {
    while (_uart->available())
    {
      buffer[readBytes] = _uart->read();
      readBytes++;

      if (readBytes > 7)
      {
        if (strncmp((const char *)&buffer[readBytes - 7], "\"\r\nOK\r\n", strlen("\"\r\nOK\r\n")) == 0) // detectado posible final de archivo
        {
          char *index = strstr((const char *)buffer, "+URDFILE:");
          if (index != NULL)
          { // se ha encontrado la cabecera del mensaje
            index = strchr(index, ',');
            int sizeMsg = atoi((&index[1])); // recuperamos el numero de bytes del mensaje
            index = strchr(index, '"');      // nos posicionamos al principio del mensaje
            memcpy(buffer, &index[1], sizeMsg);
            return sizeMsg;
          }
        }
      }
    }
  }
  return readBytes;
}

int ModemClass::ready()
{
  poll();

  return _ready;
}

void ModemClass::poll()
{
  while (_uart->available())
  {
    char c = _uart->read();

    if (_debugPrint)
    {
      _debugPrint->write(c);
    }

    _buffer += c;

    switch (_atCommandState)
    {
    case AT_COMMAND_IDLE:
    default:
    {

      if (_buffer.startsWith("AT") && _buffer.endsWith("\r\n"))
      {
        _atCommandState = AT_RECEIVING_RESPONSE;
        _buffer = "";
      }
      else if (_buffer.endsWith("\r\n"))
      {
        _buffer.trim();

        if (_buffer.length())
        {
          _lastResponseOrUrcMillis = millis();

          for (int i = 0; i < MAX_URC_HANDLERS; i++)
          {
            if (_urcHandlers[i] != NULL)
            {
              _urcHandlers[i]->handleUrc(_buffer);
            }
          }
        }

        _buffer = "";
      }

      break;
    }

    case AT_RECEIVING_RESPONSE:
    {
      if (c == '\n')
      {
        _lastResponseOrUrcMillis = millis();

        int responseResultIndex = _buffer.lastIndexOf("OK\r\n");
        if (responseResultIndex != -1)
        {
          _ready = 1;
        }
        else
        {
          responseResultIndex = _buffer.lastIndexOf("ERROR\r\n");
          if (responseResultIndex != -1)
          {
            _ready = 2;
          }
          else
          {
            responseResultIndex = _buffer.lastIndexOf("NO CARRIER\r\n");
            if (responseResultIndex != -1)
            {
              _ready = 3;
            }
          }
        }

        if (_ready != 0)
        {
          if (_lowPowerMode)
          {
            digitalWrite(_dtrPin, HIGH);
          }

          if (_responseDataStorage != NULL)
          {
            _buffer.remove(responseResultIndex);
            _buffer.trim();

            *_responseDataStorage = _buffer;

            _responseDataStorage = NULL;
          }

          _atCommandState = AT_COMMAND_IDLE;
          _buffer = "";
          return;
        }
      }
      break;
    }
    }
  }
}

void ModemClass::setResponseDataStorage(String *responseDataStorage)
{
  _responseDataStorage = responseDataStorage;
}

void ModemClass::addUrcHandler(ModemUrcHandler *handler)
{
  for (int i = 0; i < MAX_URC_HANDLERS; i++)
  {
    if (_urcHandlers[i] == NULL)
    {
      _urcHandlers[i] = handler;
      break;
    }
  }
}

void ModemClass::removeUrcHandler(ModemUrcHandler *handler)
{
  for (int i = 0; i < MAX_URC_HANDLERS; i++)
  {
    if (_urcHandlers[i] == handler)
    {
      _urcHandlers[i] = NULL;
      break;
    }
  }
}

void ModemClass::setBaudRate(unsigned long baud)
{
  _baud = baud;
}

ModemClass MODEM(Serial1, MODEM_UART_BAUDRATE, MODEM_GSM_PIN_RESET, MODEM_GSM_PIN_DTR, MODEM_GSM_PIN_PWR_KEY);
