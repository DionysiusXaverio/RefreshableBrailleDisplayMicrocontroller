/*
 * ESP8266_HAL.c
 *
 *  Created on: Apr 14, 2020
 *      Author: Controllerstech
 */


#include "UartRingbuffer_multi.h"
#include "ESP8266_HAL.h"
#include "stdio.h"
#include "string.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

#define wifi_uart &huart1
#define pc_uart &huart2

#define var_timing 1000

char buffer[20];

/*****************************************************************************************************************************************/

int braille_arr[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

void ESP_Init (char *SSID, char *PASSWD)
{
	char data[80];

	Ringbuf_init();

	Uart_sendstring("AT+RST\r\n", wifi_uart);
	Uart_sendstring("RESETTING.", pc_uart);
	for (int i=0; i<7; i++)
	{
		Uart_sendstring(".", pc_uart);
		HAL_Delay(1000);
	}

	/********* AT **********/
	Uart_sendstring("AT\r\n", wifi_uart);
	while(!(Wait_for("AT\r\r\n\r\nOK\r\n", wifi_uart)));
	Uart_sendstring("AT---->OK\n\n", pc_uart);


	/********* AT+CWMODE=1 **********/
	Uart_sendstring("AT+CWMODE=1\r\n", wifi_uart);
	while (!(Wait_for("AT+CWMODE=1\r\r\n\r\nOK\r\n", wifi_uart)));
	Uart_sendstring("CW MODE---->1\n\n", pc_uart);


	/********* AT+CWJAP="SSID","PASSWD" **********/
	Uart_sendstring("connecting... to the provided AP\n", pc_uart);
	sprintf (data, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASSWD);
	Uart_sendstring(data, wifi_uart);
	sprintf (data, "AT+CWJAP=\"%s\",\"%s\"\r\r\n\r\nOK\r\n", SSID, PASSWD);
	while (!(Wait_for(data, wifi_uart)));
//	while (!(Copy_upto("\"",buffer, wifi_uart)));
//	while (!(Wait_for("OK\r\n", wifi_uart)));
	sprintf (data, "Connected to,\"%s\"\n\n", SSID);
	Uart_sendstring(data,pc_uart);

	/********* AT+CIFSR **********/
	Uart_sendstring("AT+CIFSR\r\n", wifi_uart);
	while (!(Wait_for("+CIFSR:STAIP,\"", wifi_uart)));
	while (!(Copy_upto("\"",buffer, wifi_uart)));
//	while (!(Wait_for("OK\r\n", wifi_uart)));
	int len = strlen (buffer);
	buffer[len-1] = '\0';
	sprintf (data, "IP ADDR: %s\n\n", buffer);
	Uart_sendstring(data, pc_uart);


	Uart_sendstring("AT+CIPMUX=1\r\n", wifi_uart);
	while (!(Wait_for("AT+CIPMUX=1\r\r\n\r\nOK\r\n", wifi_uart)));
	Uart_sendstring("CIPMUX---->OK\n\n", pc_uart);

	Uart_sendstring("AT+CIPSERVER=1,80\r\n", wifi_uart);
	while (!(Wait_for("OK\r\n", wifi_uart)));
	Uart_sendstring("CIPSERVER---->OK\n\n", pc_uart);

	Uart_sendstring("Now Connect to the IP ADRESS\n\n", pc_uart);

}

int Server_Send (char *str, int Link_ID)
{
	int len = strlen (str);
	char data[80];
	sprintf (data, "AT+CIPSEND=%d,%d\r\n", Link_ID, len);
	Uart_sendstring(data, wifi_uart);
	while (!(Wait_for(">", wifi_uart)));
	Uart_sendstring (str, wifi_uart);
	while (!(Wait_for("SEND OK", wifi_uart)));
	sprintf (data, "AT+CIPCLOSE=5\r\n");
	Uart_sendstring(data, wifi_uart);
	while (!(Wait_for("OK\r\n", wifi_uart)));
	return 1;
}

void Server_Handle (char *str, int Link_ID)
{

}

void Server_Start (void)
{

	char response[256];
	char buffer[512];
	int Link_ID;

	if (Wait_for("+IPD,", wifi_uart) == 1)
	{
//		uint32_t start_time = HAL_GetTick();
//
//		// Log start time
//		char log_msg[64];
//		sprintf(log_msg, "GET string received at %lu ms\r\n", start_time);
//		Uart_sendstring(log_msg, pc_uart);

		// Get Link ID
		char link_id_char;
		while (!IsDataAvailable(wifi_uart));
		link_id_char = Uart_read(wifi_uart);
		Link_ID = link_id_char - '0';

		// Skip until ':' (start of data)
		while (Uart_read(wifi_uart) != ':');

		// Read request line (GET /?... HTTP/1.1)
		int i = 0;
		int char_count = 0;
		while (1)
		{
			while (!IsDataAvailable(wifi_uart));
			char c = Uart_read(wifi_uart);
			if (c == '\n' || i >= sizeof(buffer) - 1) break;
			buffer[i++] = c;
		}
		buffer[i] = '\0';

		// Parse GET parameters
		char *param_start = strstr(buffer, "GET /?");
		if (param_start)
		{
			param_start += strlen("GET /?");
			char *param_end = strchr(param_start, ' ');
			if (param_end)
			{
				*param_end = '\0'; // Null-terminate param string

				// Optional debug
				Uart_sendstring("GET parameters: ", pc_uart);
				Uart_sendstring(param_start, pc_uart);
				Uart_sendstring("\n", pc_uart);

				sprintf(response,
					"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nOK\r\n");

				Server_Send(response, Link_ID);
//				Reset_Braille();
//				Handle_Braille_Second(param_start[0]);

//				Reset_Braille();
//
				uint32_t end_time = HAL_GetTick();
//				sprintf(log_msg, "Finished processing at %lu ms\r\n", end_time);
//				Uart_sendstring(log_msg, pc_uart);
				do {
						Handle_Braille_Second(param_start[char_count]);

						char_count += 1;
						HAL_Delay(1000);
//						Reset_Braille();
				} while (strlen(param_start) - char_count > 0);
				Handle_Braille_Second('_');

//				Reset_Braille();
////
//				do {
//					if (strlen(param_start + char_count) == 1) {
//						Handle_Braille_Mult(param_start[char_count], '/');
//
//						char_count += 1;
//					}
//					else if (strlen(param_start + char_count) == 2) {
//						Handle_Braille_Mult(param_start[char_count], param_start[char_count+1]);
//
//						char_count += 2;
////						HAL_Delay(var_timing);
//
//						Handle_Braille_Mult('/', '/');
//					}
//					else if (strlen(param_start) > 2) {
//						Handle_Braille_Mult(param_start[char_count], param_start[char_count+1]);
//
//						char_count += 2;
////						Reset_Braille();
//					}
//				} while (strlen(param_start) - char_count > 0);
			}
		}


	}
}

void Reset_Braille () {
  setvalue(0, 0, braille_arr);
  setvalue(1, 0, braille_arr);
  setvalue(2, 0, braille_arr);
  setvalue(3, 0, braille_arr);
  setvalue(4, 0, braille_arr);
  setvalue(5, 0, braille_arr);

  setvalue(6, 0, braille_arr);
  setvalue(7, 0, braille_arr);
  setvalue(8, 0, braille_arr);
  setvalue(9, 0, braille_arr);
  setvalue(10, 0, braille_arr);
  setvalue(11, 0, braille_arr);
}

void Handle_Braille_Mult(char letter1, char letter2)
{
	Handle_Braille(letter1);
	Handle_Braille_Second(letter2);
}

void setvalue(int value, int state, int* braille_arr){
    if(braille_arr[value] == state){
        return;
    }
    setpin(13,value/6);
    setpin(8,(value % 6) % 2);
    setpin(9,((value % 6) / 2) % 2);
    setpin(10,((value % 6) / 4) % 2);
    if(state == 0){
        setpin(11,0);
        setpin(12,1);
        braille_arr[value] = 0;
    }
    if(state == 1){
        setpin(11,1);
        setpin(12,0);
        braille_arr[value] = 1;
    }

    HAL_Delay(200);

    setpin(11,0);
    setpin(12,0);
}

void setpin(int pin, int state) {
    if (state) {
        GPIOB->BSRR = (1 << pin);  // Set pin (turn ON)
    } else {
        GPIOB->BSRR = (1 << (pin + 16)); // Reset pin (turn OFF)
    }
}


void Handle_Braille (char letter) {
	  if(letter == 'a' || letter == 'A')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 0, braille_arr);

	  }
	  else if(letter == 'b' || letter == 'B')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'c' || letter == 'C')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'd' ||  letter == 'D')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'e' || letter == 'E')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'f' || letter == 'F')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'g' || letter == 'G')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'h' || letter == 'H')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'i' || letter == 'I')
	  {
		  setvalue(0, 0, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'j' || letter == 'J')
	  {
		  setvalue(0, 0, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'k' || letter == 'K')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'l' || letter == 'L')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'm' || letter == 'M')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'n' || letter == 'N')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'o' || letter == 'O')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'p' || letter == 'P')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'q' || letter == 'Q')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'r' || letter == 'R')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 's' || letter == 'S')
	  {
		  setvalue(0, 0, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 't' || letter == 'T')
	  {
		  setvalue(0, 0, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == 'u' || letter == 'U')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 1, braille_arr);
	  }
	  else if(letter == 'v' || letter == 'V')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 1, braille_arr);
	  }
	  else if(letter == 'w' || letter == 'W')
	  {
		  setvalue(0, 0, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 1, braille_arr);
	  }
	  else if(letter == 'x' || letter == 'X')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 1, braille_arr);
	  }
	  else if(letter == 'y' || letter == 'Y')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 1, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 1, braille_arr);
	  }
	  else if(letter == 'z' || letter == 'Z')
	  {
		  setvalue(0, 1, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 1, braille_arr);
	  }
	  else if(letter == '.')
	  {
		  setvalue(0, 0, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 1, braille_arr);
	  }
	  else if(letter == ',')
	  {
		  setvalue(0, 0, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == '_')
	  {
		  setvalue(0, 0, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 0, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 0, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == '!')
	  {
		  setvalue(0, 0, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 1, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 0, braille_arr);
	  }
	  else if(letter == '?')
	  {
		  setvalue(0, 0, braille_arr);
		  setvalue(1, 0, braille_arr);
		  setvalue(2, 1, braille_arr);
		  setvalue(3, 0, braille_arr);
		  setvalue(4, 1, braille_arr);
		  setvalue(5, 1, braille_arr);
	  }
}

void Handle_Braille_Second (char letter) {
	  if(letter == 'a' || letter == 'A')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 0, braille_arr);

	  }
	  else if(letter == 'b' || letter == 'B')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'c' || letter == 'C')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'd' ||  letter == 'D')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'e' || letter == 'E')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'f' || letter == 'F')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'g' || letter == 'G')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'h' || letter == 'H')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'i' || letter == 'I')
	  {
		  setvalue(6, 0, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'j' || letter == 'J')
	  {
		  setvalue(6, 0, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'k' || letter == 'K')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'l' || letter == 'L')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'm' || letter == 'M')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'n' || letter == 'N')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'o' || letter == 'O')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'p' || letter == 'P')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'q' || letter == 'Q')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'r' || letter == 'R')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 's' || letter == 'S')
	  {
		  setvalue(6, 0, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 't' || letter == 'T')
	  {
		  setvalue(6, 0, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == 'u' || letter == 'U')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 1, braille_arr);
	  }
	  else if(letter == 'v' || letter == 'V')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 1, braille_arr);
	  }
	  else if(letter == 'w' || letter == 'W')
	  {
		  setvalue(6, 0, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 1, braille_arr);
	  }
	  else if(letter == 'x' || letter == 'X')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 1, braille_arr);
	  }
	  else if(letter == 'y' || letter == 'Y')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 1, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 1, braille_arr);
	  }
	  else if(letter == 'z' || letter == 'Z')
	  {
		  setvalue(6, 1, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 1, braille_arr);
	  }
	  else if(letter == '.')
	  {
		  setvalue(6, 0, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 1, braille_arr);
	  }
	  else if(letter == ',')
	  {
		  setvalue(6, 0, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == '_')
	  {
		  setvalue(6, 0, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 0, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 0, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == '!')
	  {
		  setvalue(6, 0, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 1, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 0, braille_arr);
	  }
	  else if(letter == '!')
	  {
		  setvalue(6, 0, braille_arr);
		  setvalue(7, 0, braille_arr);
		  setvalue(8, 1, braille_arr);
		  setvalue(9, 0, braille_arr);
		  setvalue(10, 1, braille_arr);
		  setvalue(11, 1, braille_arr);
	  }
}

