// AutoHeadlight.cpp - read an LDR and turn headlights on when needed. 

#include <avr/io.h>
#include <util/delay.h>
#include "digital.h"    // digital io func header in sketchLib
#include "analog.h"     // analog func header in sketchLib

// defines
#define OFF false
#define ON  true

#define HYSTERESIS      (12)
#define LIGHTS_ON_VAL   (930)
#define NAV_ILL_ON_VAL  (330)

// use hysteresis to avoid rapid on/off toggling as light level changes
// the light value swings are higher in the lower range, so use a higher hysteresis 
// value for the nav illumination decision
#define LIGHTS_OFF_VAL  (LIGHTS_ON_VAL + HYSTERESIS)
#define NAV_ILL_OFF_VAL (NAV_ILL_ON_VAL + 3*HYSTERESIS)

// func prototypes
void     delay(uint16_t ms);
uint16_t expSmooth64(uint16_t newVal, uint16_t currVal, uint8_t alpha64);

int main(void)
{
    uint8_t lightRelay = 1;   // output connected to relay for headlights (also board LED)
    uint8_t navOutput  = 4;   // ouptut connected to relay for nav illumination

    pinMode(2, INPUT);     // set pin 2 (A1) as input
    digitalWrite(2, LOW);  // turn off internal pull-up for A1

    analogInit(); // init the a2d funcs

    // initialize the digital outputs
    pinMode(lightRelay, OUTPUT);
    pinMode(navOutput, OUTPUT);

    boolean  lights = OFF;  // init the headlight output state
    boolean  navOut = OFF;  // init the nav illumination output state
    uint16_t ldr = LIGHTS_ON_VAL;

    // set both outputs to the initial values
    digitalWrite(lightRelay, lights);
    digitalWrite(navOutput,  navOut);

    while(1)
    {
        // LDR is connected to pin A1, 10-bit range (0-1023)
        ldr = expSmooth64(analogRead(1), ldr, 3);  // alpha = 3/64

        if (lights == ON)
        {
            if (ldr > LIGHTS_OFF_VAL) // LDR val is above OFF limit
            {
                lights = OFF;  // turn the lights off
            }
        }
        else // lights == OFF
        {
            if (ldr < LIGHTS_ON_VAL)  // LDR val is below ON limit
            {
                lights = ON;   // turn the lights on
            }
        }
        digitalWrite(lightRelay, lights);

        if (navOut == ON)
        {
            if (ldr > NAV_ILL_OFF_VAL) // LDR val is above OFF limit
            {
                navOut = OFF;  // disconnect the nav illumination signal (nav screen to full-bright)
            }
        }
        else // navOut == OFF
        {
            if (ldr < NAV_ILL_ON_VAL)  // LDR val is below ON limit
            {
                navOut = ON;   // connect the nav illumination signal (dim the nav screen)
            }
        }
        digitalWrite(navOutput, navOut);

        delay(250);  // wait 1/4 second before repeating
    }
}

// use a fixed point exp filter to smooth, filter constant alpha is alpha64/64
uint16_t expSmooth64(uint16_t newVal, uint16_t currVal, uint8_t alpha64)
{
    uint32_t filterVal = ((uint32_t)currVal * (64 - alpha64)) + (alpha64 * newVal);
    return (uint16_t)(filterVal >> 6);
}

// avoid pulling in float code by using fixed value in _delay_ms()
void delay(uint16_t ms)
{
    while (ms--)
    {
        _delay_ms(1);
    }
}
