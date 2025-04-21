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

  License:        MIT License (See lcd.c file)
*/

#ifndef LCD16X2_H_
#define LCD16X2_H_

#include <stdbool.h>
#include "main.h"

/**
 * @brief Initialise LCD on 4 or 8-bits mode
 */
extern void lcd_init();

/**
 * @brief Set cursor position
 * @param[in] row : 0 or 1 for line1 or line2
 * @param[in] col : 0 - 15 (16 columns LCD)
 */
extern void lcd_setCursor(uint8_t row, uint8_t col);

/**
 * @brief Move to beginning of 1st line
 */
extern void lcd_line1(void);

/**
 * @brief Move to beginning of 2nd line
 */
extern void lcd_line2(void);

/**
 * @brief Delete 1st line
 */
extern void lcd_deleteLine1(void);

/**
 * @brief Delete 2nd line
 */
extern void lcd_deleteLine2(void);

/**
 * @brief Cursor ON/OFF
 */
extern void lcd_cursorShow(bool state);

/**
 * @brief Display clear
 */
extern void lcd_clear(void);

/**
 * @brief Display ON/OFF: hide all characters but not clear them
 */
extern void lcd_display(bool state);

/**
 * @brief Shift digits to right
 */
extern void lcd_shiftRight(uint8_t offset);

/**
 * @brief Shift digits to left
 */
extern void lcd_shiftLeft(uint8_t offset);

/**
 * @brief Works like the printf("Hello %f.2", variable) function
 */
extern void lcd_printf(const char* str, ...);

#endif /* LCD16X2_H_ */
