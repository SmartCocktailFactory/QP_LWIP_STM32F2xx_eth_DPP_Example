/****************************************************************************************
                         Copyright (C) Adrian Lutz
                              All Rights Reserved
 ****************************************************************************************

  DESCRIPTION:        Control LED's on Olimex P207 eval board.

 ****************************************************************************************

 ****************************************************************************************
 Includes
*****************************************************************************************/
#include "led.h"
#include <STM32F2xx_StdPeriph_Driver/inc/stm32f2xx_gpio.h>

/****************************************************************************************
 Defines
*****************************************************************************************/
#define LED1_PIN                         GPIO_Pin_6
#define LED2_PIN                         GPIO_Pin_7
#define LED3_PIN                         GPIO_Pin_8
#define LED4_PIN                         GPIO_Pin_9


/****************************************************************************************
 Enums
*****************************************************************************************/


/****************************************************************************************
 Structs and typedefs
*****************************************************************************************/


/****************************************************************************************
 Macros
*****************************************************************************************/


/****************************************************************************************
 Forward declarations
*****************************************************************************************/


/****************************************************************************************
 Local data
*****************************************************************************************/
static const uint16_t GPIO_PIN[4] = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN};


/****************************************************************************************
 Public function implementations
*****************************************************************************************/
void omxEval_led_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /* Enable the GPIOF Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);


  /* Configure the GPIOF pins */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOF, &GPIO_InitStructure);
}

void omxEval_led_on(OmxEvalLed led)
{
  GPIOF->BSRRL = GPIO_PIN[led];
}

void omxEval_led_off(OmxEvalLed led)
{
  GPIOF->BSRRH = GPIO_PIN[led];
}

void omxEval_led_toggle(OmxEvalLed led)
{
  GPIOF->ODR ^= GPIO_PIN[led];
}

void omxEval_led_test(void)
{
  int i = 0;
  omxEval_led_init();


  for(i = 0; i<4; i++) {
    omxEval_led_on(i);
    omxEval_led_off(i);
    omxEval_led_toggle(i);
    omxEval_led_toggle(i);
  }
}


/****************************************************************************************
 Private function implementations
*****************************************************************************************/
