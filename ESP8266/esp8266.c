#include "esp8266.h"
/**
 * Hàm này dùng để giao tiếp với esp
 * @param  USARTx   Bộ USART
 * @param  pinspack Chân USART
 * @param  baudrate Tốc độ bound
 * @return          1
 */
int setEsp(USART_TypeDef* USARTx, TM_USART_PinsPack_t pinspack,
           uint32_t baudrate)
{
  TM_USART_Init(USARTx, pinspack, baudrate);
  printf(" AT+RST\r\n");
  Delayms(1500);
  return 1;
}

int connectWifi(char* sidd, char* passWifi)
{
  printf("AT+CWMODE=1\r\n");
  Delayms(500);
  printf("AT+CWJAP=\"%s\",\"%s\"\r\n", sidd, passWifi);
  Delayms(1000);
  printf("AT+CIPMUX=1\r\n");
  Delayms(500);
  return 1;
}
/**
 * Hàm dùng để gửi lệnh kết nối với các port, chân đã thiết lập
 * @param  ip     Địa chỉ IP của trang thingspeaks
 * @param  port   Port esp
 * @param  apiKey channel key trên thingspeak
 * @param  fieldx Trường cần gửi
 * @param  data   Dữ liệu cần gửi
 * @return        1
 */
int sendEsp(char* ip, uint32_t port, char apiKey[], char fieldx[],
            uint8_t data)
{
  char server[] = "GET /update?key=";
  strcat(server, apiKey);
  // strcat(server,"&");
  strcat(server, fieldx);
  // strcat(server,"=");
  printf("AT+CIPSTART=3,\"TCP\",\"%s\",%d\r\n", ip, port);
  Delayms(500);
  printf("AT+CIPSEND=3,%d\r\n", strlen(server) + 6);
  Delayms(500);
  printf("%s%d\r\n", server, data);
  Delayms(300);
  printf("AT+CIPSEND=3,%d\r\n", strlen(server) + 6);
  Delayms(300);
  printf("AT+CIPCLOSE\r\n");
  return 1;
}

int connect(void)
{
  connectWifi(SIDD, PASS);
  return 1;
}

/**
 * Gửi 4 dữ liệu đo được lên Thingspeak
 * @param  NRF Struct thời tiết
 * @return     1
 */
int senddata(NRF_ThoiTiet NRF)
{

  sendEsp(IP, PORT, APIKEY, "field1=", NRF.NhietDo);
  Delayms(3000);
  sendEsp(IP, PORT, APIKEY, "field2=", NRF.DoAm);
  Delayms(3000);
  sendEspPre(IP, PORT, APIKEY, "field3=", NRF.ApSuat);
  Delayms(3000);
  sendEsp(IP, PORT, APIKEY, "field4=", NRF.Mua);
  Delayms(3000);
  return 1;
}


int sendEspPre(char* ip, uint32_t port, char apiKey[], char fieldx[],
               long data)
{

  char server[] = "GET /update?key=";
  strcat(server, apiKey);
  // strcat(server,"&");
  strcat(server, fieldx);
  // strcat(server,"=");
  printf("AT+CIPSTART=3,\"TCP\",\"%s\",%d\r\n", ip, port);
  Delayms(500);
  printf("AT+CIPSEND=3,%d\r\n", strlen(server) + 8);
  Delayms(500);
  printf("%s%ld\r\n", server, data);
  Delayms(300);
  printf("AT+CIPSEND=3,%d\r\n", strlen(server) + 8);
  Delayms(300);
  printf("AT+CIPCLOSE\r\n");
  return 1;
}
PUTCHAR_PROTOTYPE {
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART1, (uint8_t)ch);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {
  }

  return ch;
}
