/*
  Library:        16x2 LCD library for 8/4 bit modes
  Author:         Daniel Koch
  Initial Date:   17/10/2024
  Last Updated:   17/10/2024
  Description:    This library provides support for 16x2 LCD displays using STM32 MCUs with HAL libraries.
                  It facilitates basic printing of text and numbers on 16x2 LCDs, functioning in both 8-bit
                  and 4-bit parallel modes.

  Notes:          The structure of this library is inspired by the widely-used Arduino LiquidCrystal library
                  and developed with reference to the official Datasheet for the 16x2 LCD module.

  License:        MIT License (See below)

 * MIT License
 *
 * Copyright (c) 2024 Daniel Koch
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions 
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED 
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 */


#include "lcd.h"
#include "lcd_config.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>


/* List of COMMANDS */
#define LCD_CLEARDISPLAY      0x01
#define LCD_DISPLAYCONTROL    0x08
#define LCD_FUNCTIONSET       0x20

/* List of commands Bitfields */
// Display control
#define LCD_DISPLAY_B         0x01
#define LCD_DISPLAY_C         0x02
#define LCD_DISPLAY_D         0x04
// Shift control
#define LCD_SHIFT_RL          0x04
#define LCD_SHIFT_SC          0x08
// Function set control
#define LCD_FUNCTION_F        0x04
#define LCD_FUNCTION_N        0x08
#define LCD_FUNCTION_DL       0x10

/* LCD Library Variables */
static bool mode8Bit = true;
bool cmd8Bit = 0x28;
static uint8_t DisplayControl = 0x0F;

/* -------- Internal functions starts here -------- */

/**
 * @brief Set falling/rising edge
 */
static void lcd_enablePulse(void)
{
  HAL_GPIO_WritePin(E_Pin.Port, E_Pin.Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(E_Pin.Port, E_Pin.Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
}

/**
 * @brief RS control
 */
static void lcd_rs(bool state)
{
  HAL_GPIO_WritePin(RS_Pin.Port, RS_Pin.Pin, (GPIO_PinState)state);
}

/**
 * @brief Write parallel signal to lcd
 */
#ifdef LCD_8_BIT
static void lcd_write(uint8_t byte)
{
    //LSB data
    HAL_GPIO_WritePin(D0_Pin.Port, D0_Pin.Pin, (GPIO_PinState)(byte&0x01));
    HAL_GPIO_WritePin(D1_Pin.Port, D1_Pin.Pin, (GPIO_PinState)(byte&0x02));
    HAL_GPIO_WritePin(D2_Pin.Port, D2_Pin.Pin, (GPIO_PinState)(byte&0x04));
    HAL_GPIO_WritePin(D3_Pin.Port, D3_Pin.Pin, (GPIO_PinState)(byte&0x08));
    //MSB data
    HAL_GPIO_WritePin(D4_Pin.Port, D4_Pin.Pin, (GPIO_PinState)(byte&0x10));
    HAL_GPIO_WritePin(D5_Pin.Port, D5_Pin.Pin, (GPIO_PinState)(byte&0x20));
    HAL_GPIO_WritePin(D6_Pin.Port, D6_Pin.Pin, (GPIO_PinState)(byte&0x40));
    HAL_GPIO_WritePin(D7_Pin.Port, D7_Pin.Pin, (GPIO_PinState)(byte&0x80));
    lcd_enablePulse();
}

#else
static void lcd_write(uint8_t byte)
{
	uint8_t lowByte = byte & 0xF;
	uint8_t	highByte = (byte>>4) & 0xF;
  //MSB data
    HAL_GPIO_WritePin(D4_Pin.Port, D4_Pin.Pin, (GPIO_PinState)(highByte&0x1));
    HAL_GPIO_WritePin(D5_Pin.Port, D5_Pin.Pin, (GPIO_PinState)(highByte&0x2));
    HAL_GPIO_WritePin(D6_Pin.Port, D6_Pin.Pin, (GPIO_PinState)(highByte&0x4));
    HAL_GPIO_WritePin(D7_Pin.Port, D7_Pin.Pin, (GPIO_PinState)(highByte&0x8));
    lcd_enablePulse();
    //LSB data
    HAL_GPIO_WritePin(D4_Pin.Port, D4_Pin.Pin, (GPIO_PinState)(lowByte&0x1));
    HAL_GPIO_WritePin(D5_Pin.Port, D5_Pin.Pin, (GPIO_PinState)(lowByte&0x2));
    HAL_GPIO_WritePin(D6_Pin.Port, D6_Pin.Pin, (GPIO_PinState)(lowByte&0x4));
    HAL_GPIO_WritePin(D7_Pin.Port, D7_Pin.Pin, (GPIO_PinState)(lowByte&0x8));
    lcd_enablePulse();
}
#endif

/**
 * @brief Write command to lcd
 */
static void lcd_writeCommand(uint8_t cmd)
{
  lcd_rs(false);
  lcd_write(cmd);
}

/**
 * @brief Write data to lcd
 */
static void lcd_writeData(uint8_t data)
{
  lcd_rs(true);
  lcd_write(data);
}

#ifdef LCD_8_BIT

extern void lcd_init()
{

  mode8Bit = true;
  cmd8Bit  = 0x38;

  HAL_Delay(20);
  lcd_writeCommand(0x30);
  HAL_Delay(5);
  lcd_writeCommand(0x30);
  HAL_Delay(1);
  lcd_writeCommand(0x30);
  HAL_Delay(1);
  //Function set: 8 bit mode, 2 lines, Data length to 8 bits
  lcd_writeCommand(LCD_FUNCTIONSET | LCD_FUNCTION_N | LCD_FUNCTION_DL);
  //Display control (Display ON, Cursor ON, blink cursor)
  lcd_writeCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_B | LCD_DISPLAY_C | LCD_DISPLAY_D);
  //Clear LCD and return home
  lcd_writeCommand(LCD_CLEARDISPLAY);
  HAL_Delay(2);
}
#else

/**
 * @brief 4-bits write only needed for initilaisation
 */
static void lcd_write4Bit(uint8_t data)
{
  uint8_t lowByte = data & 0xF;
  lcd_rs(false);
  HAL_GPIO_WritePin(D4_Pin.Port, D4_Pin.Pin, (GPIO_PinState)(lowByte&0x1));
  HAL_GPIO_WritePin(D5_Pin.Port, D5_Pin.Pin, (GPIO_PinState)(lowByte&0x2));
  HAL_GPIO_WritePin(D6_Pin.Port, D6_Pin.Pin, (GPIO_PinState)(lowByte&0x4));
  HAL_GPIO_WritePin(D7_Pin.Port, D7_Pin.Pin, (GPIO_PinState)(lowByte&0x8));
  lcd_enablePulse();
}

/* -------- Public functions starts here -------- */

extern void lcd_init()
{
  mode8Bit = false;
  cmd8Bit = 0x28;

  //Initialise LCD
  HAL_Delay(20);
  lcd_write4Bit(0x3);
  HAL_Delay(5);
  lcd_write4Bit(0x3);
  HAL_Delay(1);
  lcd_write4Bit(0x3);
  HAL_Delay(1);
  lcd_write4Bit(0x2);  
  HAL_Delay(1);
  //Function set: 8 bit mode, 2 lines, Data length to 4 bits
  lcd_writeCommand(LCD_FUNCTIONSET | LCD_FUNCTION_N);
  //Display control (Display ON, Cursor ON, blink cursor)
  lcd_writeCommand(LCD_DISPLAYCONTROL | LCD_DISPLAY_B | LCD_DISPLAY_C | LCD_DISPLAY_D);
  //Clear LCD and return home
  lcd_writeCommand(LCD_CLEARDISPLAY);
  HAL_Delay(100);
}
#endif

extern void lcd_setCursor(uint8_t row, uint8_t col)
{
  uint8_t maskData;
  maskData = (col)&0x0F;
  if(row==0)
  {
    maskData = (0x80);
    lcd_writeCommand(maskData);
  }
  else
  {
    maskData = (0xc0);
    lcd_writeCommand(maskData);
  }
}

extern void lcd_line1(void)
{
  lcd_setCursor(0,0);
}

extern void lcd_line2(void)
{
  lcd_setCursor(1,0);
}

extern void lcd_deleteLine1(void)
{
  lcd_writeCommand(0x80);
  for(int i = 0; i < 16; i++)
  {
    lcd_printf(" ");
  }
  lcd_writeCommand(0x80);

}

extern void lcd_deleteLine2(void)
{
  lcd_writeCommand(0xc0);
  for(int i = 0; i < 16; i++)
  {
    lcd_printf(" ");
  }
  lcd_writeCommand(0xc0);
}

extern void lcd_cursorShow(bool state)
{
  if(state)
  {
    DisplayControl |= (0x03);
    lcd_writeCommand(DisplayControl);
  }
  else
  {
    DisplayControl &= ~(0x03);
    lcd_writeCommand(DisplayControl);
  }
}

extern void lcd_clear(void)
{
  lcd_writeCommand(LCD_CLEARDISPLAY);
  HAL_Delay(3);
}

extern void lcd_shiftRight(uint8_t offset)
{
  for(uint8_t i=0; i<offset;i++)
  {
    lcd_writeCommand(0x1c);
  }
}

extern void lcd_shiftLeft(uint8_t offset)
{
  for(uint8_t i=0; i<offset;i++)
  {
    lcd_writeCommand(0x18);
  }
}

extern void lcd_display(bool state)
{
  if(state)
  {
    DisplayControl |= (0x04);
    lcd_writeCommand(DisplayControl);
  }
  else
  {
    DisplayControl &= ~(0x04);
    lcd_writeCommand(DisplayControl);
  }
}

extern void lcd_printf(const char* str, ...)
{
    char arr[32]; 							 // Buffer to hold the formatted string
    va_list args;

    va_start(args, str);            		 // Initialize the argument list
    vsnprintf(arr, sizeof(arr), str, args);  // Format the string with the arguments
    va_end(args);                   		 // Clean up the argument list

    // Print the buffer to the LCD (max 16 characters)
	for (uint8_t i = 0; i < strlen(arr) && i < 16; i++)
	{
		   lcd_writeData((uint8_t)arr[i]);
	}
}



