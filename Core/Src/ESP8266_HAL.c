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

char buffer[20];

/*****************************************************************************************************************************************/

void ESP_Init (char *SSID, char *PASSWD)
{
	char data[80];

	Ringbuf_init();

	Uart_sendstring("AT+RST\r\n", wifi_uart);
	Uart_sendstring("RESETTING.", pc_uart);
	for (int i=0; i<5; i++)
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
//				Reset_Braille();
				// Handle parameters here
//				do {
//					if (strlen(param_start + char_count) == 1) {
//						Handle_Braille(param_start[char_count]);
//
//						char_count += 1;
//					} else if (strlen(param_start + char_count) == 2) {
//						Handle_Braille_Mult(param_start[char_count], param_start[char_count+1]);
//
//						char_count += 2;
//					}
//					HAL_Delay(1000);
////					else if (strlen(param_start) > 2) {
////
////
////					}
//				}

//				End_Braille();
//				while (param_start + char_count - 1 > 2);
				// Optional debug
				Uart_sendstring("GET parameters: ", pc_uart);
				Uart_sendstring(param_start, pc_uart);
				Uart_sendstring("\n", pc_uart);

//				Reset_Braille();
				Handle_Braille(param_start[0]);
			}
		}

		// Send back a barebones HTTP response
		sprintf(response,
			"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nOK\r\n");

		Server_Send(response, Link_ID);
	}
}

void Reset_Braille () {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 1);
	HAL_Delay(500);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	HAL_Delay(500);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, 1);


}

void Handle_Braille (char letter) {
	  if(letter == 'a' || letter == 'A')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'b' || letter == 'B')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'c' || letter == 'C')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'd' ||  letter == 'D')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'e' || letter == 'E')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'f' || letter == 'F')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'g' || letter == 'G')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'h' || letter == 'H')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'i' || letter == 'I')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
	  }
	  else if(letter == 'j' || letter == 'J')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
	  }
	  else if(letter == 'k' || letter == 'K')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'l' || letter == 'L')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'm' || letter == 'M')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'n' || letter == 'N')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'o' || letter == 'O')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'p' || letter == 'P')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'q' || letter == 'Q')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'r' || letter == 'R')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 's' || letter == 'S')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
	  }
	  else if(letter == 't' || letter == 'T')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
	  }
	  else if(letter == 'u' || letter == 'U')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'v' || letter == 'V')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'w' || letter == 'W')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
	  }
	  else if(letter == 'x' || letter == 'X')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'y' || letter == 'Y')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == 'z' || letter == 'Z')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
	  }
	  else if(letter == '.')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
	  }
	  else if(letter == ',')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
	  }
	  else if(letter == '_')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
	  }
	  else if(letter == '#')
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
	  }
}

