/*
 * ESP8266_HAL.h
 *
 */

#ifndef INC_ESP8266_HAL_H_
#define INC_ESP8266_HAL_H_


void ESP_Init (char *SSID, char *PASSWD);

void Server_Start (void);

void Handle_Braille (char letter);

//void Handle_Braille_Mult(char letter, char letter);

//void Reset_Braille ();

//void End_Braille ();


#endif /* INC_ESP8266_HAL_H_ */
