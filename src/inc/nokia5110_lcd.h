/*
 * Nokia 5110 LCD library
 *
 * Controls LCD via SPI protocol
 *
 *  Created on: 5. jul. 2017
 *  Author: Jure Rebernik
 */

#ifndef _LCD_H_
#define _LCD_H_

#include <cr_section_macros.h>
#include <cmsis.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "chip.h"
#include <display_jure.h>





/* LCD PIN DEFINTIONS */
#define LCD_CLK_PIN			4		// Clock - DATA4 in schematic
#define LCD_DATA_PIN		5		// Data - DATA5 in schematic
#define LCD_DC_PIN			6		// Data / ~Command - DATA6 in schematic
#define LCD_CS_PIN			7		// ~ChipSelect - DATA7 in schematic
#define LCD_RST_PIN			8		// ~Reset - LCD_RST in schemati
#define LCD_BACKLIGHT_PIN	17		// Backlight ~On / Off


/* VARIABLES */





/* REGISTER LOCATIONS AND SOME COMMANDS FOR THE LCD */
#define LCD_CMD_EXT_INST	(1<<5)
#define LCD_CMD_DISP_CFG	(1<<3)
#define LCD_CMD_Y_ADDR		(1<<6)
#define LCD_CMD_X_ADDR		(1<<7)
#define LCD_CMD_TEMP_CTRL	(1<<2)
#define LCD_CMD_BIAS		(1<<4)
#define LCD_CMD_CONTRAST	(1<<7)





/* --- FUNCKIJE --- */

void LCD_Init();


/*
 * @Brief	Funkcija ki napiše text na ekran
 * @Input	*text: pointer do texta ki ga želimo prikazati
 */
void LCD_Write_Text(char *text);


/*
 * @Brief	Funkcija ki napiše custum simbol na ekran
 * @Input 	character: številka simbola ki naj bo prikazana (glej tabelo ASCII_Custum)
 */
void LCD_Write_Custom(char character);


/*
 * @Brief	Funkcija ki napiše večje črke na ekran
 * @Input	*text: pointer do texta ki ga želimo prikazati
 */
void LCD_Write_Text_11x16(char* text, uint8_t x, uint8_t y);



/*
 * @Breif 	Funkcija ki napiše na ekran spremenljivko tipa integer
 * @Input	value: števio, ki bo prikazano na ekranu
 */
void LCD_Write_Int(int value);


/*
 * @Brief	Funkcija ki vzame uint32_t številko v milivoltih/miliamperih/... in
 * 			jo pretvori v volte/ampere/... in doda piko in use da lahko to
 * 			lepo napišemo na ekran
 * @Input	value: od 0 do 999 999.
 */
void LCD_Write_Float(float value, uint8_t decimals, uint8_t x, uint8_t y, uint8_t size);


/*
 * @Brief 	Draw 84x48 size bitmap
 */
void LCD_Draw_Bitmap(const char *bitmap);


/*
 * @Brief 	Write while display RAM with '0' - Clear display
 */
void LCD_Clear_Screen();


/*
 * @Brief 	Funckija ki nastavi address od RAM-a v ekranu
 * @Inputs	x: število od 0 do 83
 * 			y: število od 0 do 5
 */
void LCD_Set_Cursor(uint8_t x, uint8_t y);


/*
 * @Brief 	Funckija ki preko SPI pošlje komando v lcd
 * @Inputs	cmd: komanda za jo poslat
 */
void LCD_Write_Command(uint8_t cmd);


/*
 * @Brief 	Funckija ki preko SPI pošlje podatke v ram od lcd
 * @Inputs	data: podatki za jih poslat
 */
void LCD_Write_Data(uint8_t data);


#endif /* _LCD_H_ */
