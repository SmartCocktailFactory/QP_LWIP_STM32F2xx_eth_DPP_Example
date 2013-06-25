/****************************************************************************************
                         Copyright (C) Adrian Lutz
                              All Rights Reserved
 ****************************************************************************************

  DESCRIPTION:        Control RS-232 serial COM port on Olimex P207 eval board.


 ****************************************************************************************
 ****************************************************************************************/
#ifndef RS_232_H_
#define RS_232_H_

/****************************************************************************************
 Includes
*****************************************************************************************/


/****************************************************************************************
 Defines
*****************************************************************************************/


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
 Public function declarations
*****************************************************************************************/
/**
 * Initialize the RS-232 serial ports on the Olimex eval board.
 */
void omxEval_rs232_init(void);

/**
 * Test RS-232 code, transmit direction (using an endless loop).
 */
void omxEval_rs232_testTx(void);

/**
 * Test RS-232 code, receive direction (using an endless loop).
 */
void omxEval_rs232_testRx(void);

#endif  /* RS_232_H_ */
