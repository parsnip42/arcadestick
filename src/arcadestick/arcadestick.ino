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
    byte altHidCode;
};

const Button buttons[] =
{
    { 18, 0x52, 0x52 }, // Up
    { 19, 0x51, 0x51 }, // Down
    { 20, 0x50, 0x50 }, // Left
    { 21, 0x4f, 0x4f }, // Right

    //{ 22, 0x2b }, // Tab
    { 23, 0x13, 0x13 }, // P

    { 0, 0x1e, 0x29 }, // 1 / Esc
    { 1, 0x22, 0x22 }, // 5 / Esc

    { 12, 0x28, 0x2b }, // Enter / Tab
    { 11, 0x16, 0x16 }, // S
    { 10, 0x7, 0x7 }, // D
    { 9, 0x9, 0x9 }, // F

    { 6, 0x1d, 0x1d }, // Z
    { 5, 0x1b, 0x1b }, // X
    { 3, 0x6, 0x6 }, // C
    { 2, 0x19, 0x19 } // V
};

const size_t buttonCount = sizeof(buttons) / sizeof(*buttons);

uint32_t lastMask = 0;

int led = 13;
int altPin = 22;

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

    pinMode(altPin, INPUT);
    digitalWrite(altPin, HIGH);
    
    digitalWrite(led, LOW);
}

void loop(void)
{
    uint32_t buttonMask = 0;
    byte keys[6] = { 0 };
    int keyCount = 0;

    bool altKey = !digitalRead(altPin);
    
    for (int i = 0; i < buttonCount; ++i)
    {
        const Button& button = buttons[i];
        
        int state = !digitalRead(button.pin);
        
        buttonMask |= (state & 1);
        buttonMask <<= 1;

        if (state && keyCount < 6)
        {
            keys[keyCount++] = !altKey ? button.hidCode : button.altHidCode;
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

