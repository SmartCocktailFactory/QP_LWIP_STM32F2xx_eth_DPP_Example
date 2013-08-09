//
// Author: James P Lynch, 2007; adapted for Olimex P207 board by Adrian Lutz
// *********************************************************************************************************

#ifndef Lcd_h
#define Lcd_h

#define PHILIPS_PCF8833
#ifdef PHILIPS_PCF8833
// *************************************************************************************
// LCD Include File for Philips PCF8833 STN RGB- 132x132x3 Driver
//
// Taken from Philips data sheet Feb 14, 2003
// *************************************************************************************
// Philips PCF8833 LCD controller command codes
#define NOP 0x00 // nop
#define SWRESET 0x01 // software reset
#define BSTROFF 0x02 // booster voltage OFF
#define BSTRON 0x03 // booster voltage ON
#define RDDIDIF 0x04 // read display identification
#define RDDST 0x09 // read display status
#define SLEEPIN 0x10 // sleep in
#define SLEEPOUT 0x11 // sleep out
#define PTLON 0x12 // partial display mode
#define NORON 0x13 // display normal mode
#define INVOFF 0x20 // inversion OFF
#define INVON 0x21 // inversion ON
#define DALO 0x22 // all pixel OFF
#define DAL 0x23 // all pixel ON
#define SETCON 0x25 // write contrast
#define DISPOFF 0x28 // display OFF
#define DISPON 0x29 // display ON
#define CASET 0x2A // column address set
#define PASET 0x2B // page address set
#define RAMWR 0x2C // memory write
#define RGBSET 0x2D // colour set
#define PTLAR 0x30 // partial area
#define VSCRDEF 0x33 // vertical scrolling definition
#define TEOFF 0x34 // test mode
#define TEON 0x35 // test mode
#define MADCTL 0x36 // memory access control
#define SEP 0x37 // vertical scrolling start address
#define IDMOFF 0x38 // idle mode OFF
#define IDMON 0x39 // idle mode ON
#define COLMOD 0x3A // interface pixel format
#define SETVOP 0xB0 // set Vop
#define BRS 0xB4 // bottom row swap
#define TRS 0xB6 // top row swap
#define DISCTR 0xB9 // display control
#define DOR 0xBA // data order
#define TCDFE 0xBD // enable/disable DF temperature compensation
#define TCVOPE 0xBF // enable/disable Vop temp comp
#define EC 0xC0 // internal or external oscillator
#define SETMUL 0xC2 // set multiplication factor
#define TCVOPAB 0xC3 // set TCVOP slopes A and B
#define TCVOPCD 0xC4 // set TCVOP slopes c and d
#define TCDF 0xC5 // set divider frequency
#define DF8COLOR 0xC6 // set divider frequency 8-color mode
#define SETBS 0xC7 // set bias system
#define RDTEMP 0xC8 // temperature read back
#define NLI 0xC9 // n-line inversion
#define RDID1 0xDA // read ID1
#define RDID2 0xDB // read ID2
#define RDID3 0xDC // read ID3

#else  /* Epson controller */

// *****************************************************************************
// lcd.h
//
// include file for Epson S1D15G00 LCD Controller
//
//
// Author: James P Lynch August 30, 2007
// *****************************************************************************
#define DISON 0xAF // Display on
#define DISOFF 0xAE // Display off
#define DISNOR 0xA6 // Normal display
#define DISINV 0xA7 // Inverse display
#define COMSCN 0xBB // Common scan direction
#define DISCTL 0xCA // Display control
#define SLPIN 0x95 // Sleep in
#define SLPOUT 0x94 // Sleep out
#define PASET 0x75 // Page address set
#define CASET 0x15 // Column address set
#define DATCTL 0xBC // Data scan direction, etc.
#define RGBSET8 0xCE // 256-color position set
#define RAMWR 0x5C // Writing to memory
#define RAMRD 0x5D // Reading from memory
#define PTLIN 0xA8 // Partial display in
#define PTLOUT 0xA9 // Partial display out
#define RMWIN 0xE0 // Read and modify write
#define RMWOUT 0xEE // End
#define ASCSET 0xAA // Area scroll set
#define SCSTART 0xAB // Scroll start set
#define OSCON 0xD1 // Internal oscillation on
#define OSCOFF 0xD2 // Internal oscillation off
#define PWRCTR 0x20 // Power control
#define VOLCTR 0x81 // Electronic volume control
#define VOLUP 0xD6 // Increment electronic control by 1
#define VOLDOWN 0xD7 // Decrement electronic control by 1
#define TMPGRD 0x82 // Temperature gradient set
#define EPCTIN 0xCD // Control EEPROM
#define EPCOUT 0xCC // Cancel EEPROM control
#define EPMWR 0xFC // Write into EEPROM
#define EPMRD 0xFD // Read from EEPROM
#define EPSRRD1 0x7C // Read register 1
#define EPSRRD2 0x7D // Read register 2
#define NOP 0x25 // NOP instruction

#endif

// backlight control
#define BKLGHT_LCD_ON 1
#define BKLGHT_LCD_OFF 2
// Booleans
#define NOFILL 0
#define FILL 1
// 12-bit color definitions
#define WHITE 0xFFF
#define BLACK 0x000
#define RED 0xF00
#define GREEN 0x0F0
#define BLUE 0x00F
#define CYAN 0x0FF
#define MAGENTA 0xF0F
#define YELLOW 0xFF0
#define BROWN 0xB22
#define ORANGE 0xFA0
#define PINK 0xF6A
// Font sizes
#define SMALL 0
#define MEDIUM 1
#define LARGE 2

// mask definitions
#define BIT0 0x00000001
#define BIT1 0x00000002
#define BIT2 0x00000004
#define BIT3 0x00000008
#define BIT4 0x00000010
#define BIT5 0x00000020
#define BIT6 0x00000040
#define BIT7 0x00000080
#define BIT8 0x00000100
#define BIT9 0x00000200
#define BIT10 0x00000400
#define BIT11 0x00000800
#define BIT12 0x00001000
#define BIT13 0x00002000
#define BIT14 0x00004000
#define BIT15 0x00008000
#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT18 0x00040000
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000
#define BIT23 0x00800000
#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000
#define BIT27 0x08000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000

void Backlight(unsigned char state);
void WriteSpiCommand(unsigned int data);
void WriteSpiData(unsigned int data);
void InitLcd(void);
void LCDWrite130x130bmp(void);
void LCDClearScreen(void);
void LCDSetXY(int x, int y);
void LCDSetPixel(int x, int y, int color);
void LCDSetLine(int x1, int y1, int x2, int y2, int color);
void LCDSetRect(int x0, int y0, int x1, int y1, unsigned char fill, int color);
void LCDSetCircle(int x0, int y0, int radius, int color);
void LCDPutChar(char c, int x, int y, int size, int fcolor, int bcolor);
void LCDPutString (char *lcd_string, const char *font_style, unsigned char x, unsigned char y,
unsigned char fcolor, unsigned char bcolor);
void TestLcd(void);


#endif
