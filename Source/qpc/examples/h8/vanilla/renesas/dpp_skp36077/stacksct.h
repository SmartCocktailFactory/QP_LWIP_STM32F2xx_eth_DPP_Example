/***********************************************************************/
/*                                                                     */
/*  FILE        :stacksct.h                                            */
/*  DATE        :Sun, Dec 31, 2006                                     */
/*  DESCRIPTION :Setting of Stack area                                 */
/*  CPU TYPE    :H8/36077                                              */
/*                                                                     */
/*  This file is generated by Renesas Project Generator (Ver.4.5).     */
/*                                                                     */
/***********************************************************************/

/* make sure that start of the S (stack) section + stacksize < 0xFF80.
* The start of the S section is defined in the Link/Sections dialog box.
*/
#pragma stacksize 0x200
