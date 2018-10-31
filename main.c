/*  -------------------------------------------
    Lab 5: Demonstration of simple ADC. 2018 Version 1

    1. The DAC inputs are
       - Single: ADC0_SE8 (PTB0)
       - Differential: ADC0_DP0 (PTE20) and ADC0_PM0 (PTE21)
    2. Measures voltage when button pressed
       - Red LED shows single measurement
       - Green LED shows differential measurement

    The system uses code from 
       adc.c   - provides 4 functions for using the ADC
       SysTick.c  - provides 2 functions for using SysTick
       gpio.c - provide code to initialise and use GPIO button and LED
------------------------------------------- */
#include <MKL25Z4.H>
#include <stdbool.h>
#include <stdint.h>

#include "..\include\gpio_defs.h"
#include "..\include\adc_defs.h"
#include "..\include\SysTick.h"


/*----------------------------------------------------------------------------
    Task 1: checkButton

  This tasks polls the button and signals when it has been pressed
*----------------------------------------------------------------------------*/
int buttonState ; // current state of the button
int bounceCounter ; // counter for debounce
bool pressed ; // signal if button pressed

void init_ButtonState() {
    buttonState = BUTTONUP ;
    pressed = false ; 
}

void task1ButtonPress() {
    if (bounceCounter > 0) bounceCounter-- ;
    switch (buttonState) {
        case BUTTONUP:
            if (isPressed()) {
                buttonState = BUTTONDOWN ;
                pressed = true ; 
            }
          break ;
        case BUTTONDOWN:
            if (!isPressed()) {
                buttonState = BUTTONBOUNCE ;
                bounceCounter = BOUNCEDELAY ;
            }
            break ;
        case BUTTONBOUNCE:
            if (isPressed()) {
                buttonState = BUTTONDOWN ;
            }
            else if (bounceCounter == 0) {
                buttonState = BUTTONUP ;
            }
            break ;
    }                
}

/*  -----------------------------------------
     Task 2: MeasureVoltage
       res - raw result
       measured_voltage - scaled
    -----------------------------------------   */
#define MSINGLE 1 
#define MDIFFERENTIAL 2 

// declare volatile to ensure changes seen in debugger
volatile float measured_voltage ;  // scaled value
volatile float diff_measured_voltage ;  // scaled value
int measureState ;

void Init_MeasureState(void) {
    measureState = MSINGLE ;
    greenLEDOnOff(LED_OFF) ;
    redLEDOnOff(LED_ON) ;  
}

void task2MeasureVoltage(void) {
    switch (measureState) {
        case MSINGLE:
            if (pressed) {
                pressed = false ;     // acknowledge event
        
                // take a simple-ended voltage reading
                MeasureVoltage() ;    // updates sres variable
                // scale to an actual voltage, assuming VREF accurate
                measured_voltage = (VREF * sres) / ADCRANGE ;
                
                // next measurement differential
                measureState = MDIFFERENTIAL ;
                greenLEDOnOff(LED_ON) ;
                redLEDOnOff(LED_OFF) ;              
            }
            break ;
        case MDIFFERENTIAL:
            if (pressed) {
                pressed = false ;     // acknowledge event

                // take a double-ended voltage reading
                MeasureVoltageDiff() ;    // updates dres variable
                // scale to an actual voltage, assuming VREF accurate
                diff_measured_voltage = 2 * (VREF * dres) / ADCRANGE ;
                
                // next measurement differential
                measureState = MSINGLE ;
                greenLEDOnOff(LED_OFF) ;
                redLEDOnOff(LED_ON) ;              
            }
            break ;
// 
}
}
int state=BLUE_LED_ON;
float time_on, v_min=0.08, v_max=3.01;
int counter=0;

void task3blueLED(void){
switch (state) {
case BLUE_LED_ON: 
MeasureVoltage();
measured_voltage=(VREF*sres)/ ADCRANGE;
time_on=(measured_voltage-v_min)/((v_max-v_min)*3600)+200;
if (counter >= time_on) {
state=BLUE_LED_OFF;
blueLEDOnOff (LED_OFF);
}
break;
case BLUE_LED_OFF:
if(counter >-4000) {
counter=0;
state = BLUE_LED_ON;
blueLEDOnOff (LED_ON);
}
break;
}
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
volatile uint8_t calibrationFailed ; // zero expected
int main (void) {
    // Enable clock to ports B, D and E
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK ;

    init_LED() ; // initialise LED
    init_ButtonGPIO() ; // initialise GPIO input
    init_ButtonState() ; // initialise button state variables
    Init_ADC() ; // Initialise ADC
    calibrationFailed = ADC_Cal(ADC0) ; // calibrate the ADC 
    while (calibrationFailed) ; // block progress if calibration failed
    Init_ADC() ; // Reinitialise ADC
    Init_MeasureState() ;  // Initialise measure state 
    Init_SysTick(1000) ; // initialse SysTick every 1ms
    waitSysTickCounter(10) ;

    while (1) {        
        task1ButtonPress() ;
        task2MeasureVoltage() ;
        // delay
      waitSysTickCounter(10) ;  // cycle every 10 ms
    }
}

