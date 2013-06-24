/****************************************************************************************
                         Copyright (C) Adrian Lutz
                              All Rights Reserved
 ****************************************************************************************

  DESCRIPTION:        Control LED's on Olimex P207 eval board.


 ****************************************************************************************
 ****************************************************************************************/
#ifndef LED_H_
#define LED_H_

/****************************************************************************************
 Includes
*****************************************************************************************/
#include "led.h"

/****************************************************************************************
 Defines
*****************************************************************************************/


/****************************************************************************************
 Enums
*****************************************************************************************/


/****************************************************************************************
 Structs and typedefs
*****************************************************************************************/
typedef enum OmxEvalLed
{
  LED_1 = 0,
  LED_2,
  LED_3,
  LED_4
} OmxEvalLed;

/****************************************************************************************
 Macros
*****************************************************************************************/


/****************************************************************************************
 Public function declarations
*****************************************************************************************/
/**
 * Initializes the LED's on the Olimex eval board.
 */
void omxEval_led_init(void);

/**
 * Switch selected LED on.
 *
 * @param led the LED to be switched on.
 */
void omxEval_led_on(OmxEvalLed led);

/**
 * Switch selected LED off.
 *
 * @param led the LED to be switched off.
 */
void omxEval_led_off(OmxEvalLed led);

/**
 * Toggle selected LED.
 *
 * @param led the LED to be toggled.
 */
void omxEval_led_toggle(OmxEvalLed led);

/**
 * Test LED code.
 */
void omxEval_led_test(void);

#endif /* LED_H_ */
