#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include <sys/types.h>
#ifdef _WIN32 
  //#include <winsock.h>
  #include <winsock2.h>
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

//#include <netdb.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <fcntl.h>

#define TCP_BUF_SIZE 2048

extern int tcp_sock_fd;
extern int addr_len, bytes_read, write_byte_count;
//extern char recv_data[TCP_BUF_SIZE];
//extern char send_data[TCP_BUF_SIZE];
extern struct sockaddr_in server_addr, client_addr;

extern int tcp_verbose;
extern uint8_t tcp_connected_flag;


//void perror_and_clean(const char* s);

void tcp_client_init(void);
uint8_t tcp_client_connect(char* addr, u_short port);
uint8_t tcp_client_connected(void);

void tcp_client_write(uint8_t* send_data, size_t data_size);

//uint8_t tcp_client_read_available(void);
//int tcp_client_read(void);
int tcp_client_read_timeout(char *buf, int len, long sec, long usec);


void tcp_client_disconnect(void);


#endif /* TCP_CLIENT_H_ */
