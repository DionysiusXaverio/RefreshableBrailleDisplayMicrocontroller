/*
	Description:    This file is used to set the correct pin layout on your STM32 controller.
	Notes:          The provided pin layout was used on an STM32F446.
*/

#ifndef LCD_CONFIG_H_
#define LCD_CONFIG_H_

typedef struct
{
	GPIO_TypeDef* Port;
	uint16_t Pin;
}lcd_pinLayout;

// to activate 8 bit mode uncomment the line below

// #define LCD_8_BIT

#ifdef LCD_8_BIT

lcd_pinLayout RS_Pin = {GPIOB, GPIO_PIN_15};
lcd_pinLayout E_Pin  = {GPIOB, GPIO_PIN_14};

lcd_pinLayout D0_Pin = {GPIOA, GPIO_PIN_9};
lcd_pinLayout D1_Pin = {GPIOC, GPIO_PIN_7};
lcd_pinLayout D2_Pin = {GPIOB, GPIO_PIN_6};
lcd_pinLayout D3_Pin = {GPIOA, GPIO_PIN_7};

lcd_pinLayout D4_Pin = {GPIOB, GPIO_PIN_10};
lcd_pinLayout D5_Pin = {GPIOB, GPIO_PIN_4};
lcd_pinLayout D6_Pin = {GPIOB, GPIO_PIN_5};
lcd_pinLayout D7_Pin = {GPIOB, GPIO_PIN_3};


#else

lcd_pinLayout D4_Pin = {GPIOB, GPIO_PIN_10};
lcd_pinLayout D5_Pin = {GPIOB, GPIO_PIN_4};
lcd_pinLayout D6_Pin = {GPIOB, GPIO_PIN_5};
lcd_pinLayout D7_Pin = {GPIOB, GPIO_PIN_3};

lcd_pinLayout RS_Pin = {GPIOB, GPIO_PIN_15};
lcd_pinLayout E_Pin  = {GPIOB, GPIO_PIN_14};

#endif

#endif /* LCD_CONFIG_H_ */
