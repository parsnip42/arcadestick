/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

/*
  This example shows how to send HID (keyboard/mouse/etc) data via BLE
  Note that not all devices support BLE keyboard! BLE Keyboard != Bluetooth Keyboard
*/

#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined(ARDUINO_ARCH_SAMD)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

struct Button
{
    int pin;
    byte hidCode;
};

const Button buttons[] =
{
    { 18, 0x50 },
    { 19, 0x4f },
    { 20, 0x51 },
    { 21, 0x52 }
};

const size_t buttonCount = sizeof(buttons) / sizeof(*buttons);

uint32_t lastMask = 0;

int led = 13;

void setup(void)
{
    pinMode(led, OUTPUT);
    digitalWrite(led, HIGH);

    delay(3000);

    ble.begin();
    
    ble.echo(false);
    ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Arcade Stick"));
    ble.sendCommandCheckOK(F("AT+BleHIDEn=On"));

    ble.reset();

    for (int i = 0; i < buttonCount; ++i)
    {
        int buttonPin = buttons[i].pin;

        pinMode(buttonPin, INPUT);
        digitalWrite(buttonPin, HIGH);
    }

    digitalWrite(led, LOW);
}

void loop(void)
{
    uint32_t buttonMask = 0;
    byte keys[6] = { 0 };
    int keyCount = 0;
    
    for (int i = 0; i < buttonCount; ++i)
    {
        const Button& button = buttons[i];
        
        int state = !digitalRead(button.pin);
        
        buttonMask |= (state & 1);
        buttonMask <<= 1;

        if (state && keyCount < 6)
        {
            keys[keyCount++] = button.hidCode;
        }
    }

    if (lastMask != buttonMask)
    {
        if (buttonMask)
        {
            char atStr[64];
            
            snprintf(atStr, sizeof(atStr), "AT+BLEKEYBOARDCODE=00-00-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x",
                     keys[0], keys[1], keys[2], keys[3], keys[4], keys[5]);
            
            ble.println(atStr);
        }
        else
        {
            ble.println(F("AT+BLEKEYBOARDCODE=00-00"));
        }

        lastMask = buttonMask;

        //digitalWrite(led, buttonMask != 0);
    }
}

