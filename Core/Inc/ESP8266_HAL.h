/*
 * ESP8266_HAL.h
 *
 */

#ifndef INC_ESP8266_HAL_H_
#define INC_ESP8266_HAL_H_


void ESP_Init (char *SSID, char *PASSWD);

void Server_Start (void);

void Handle_Braille (char letter);
void Handle_Braille_Second (char letter);
void Handle_Braille_Mult(char letter1, char letter2);

void Reset_Braille ();

void setpin(int pin, int state);
void setvalue(int value, int state, int* braille_arr);

#endif /* INC_ESP8266_HAL_H_ */
