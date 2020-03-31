#include "tcp_client.h"

int tcp_sock_fd;
int addr_len, bytes_read, write_byte_count;
//char recv_data[TCP_BUF_SIZE];
//char send_data[TCP_BUF_SIZE];
struct sockaddr_in server_addr, client_addr;
int tcp_verbose;
uint8_t tcp_connected_flag;
uint8_t read_byte, read_byte_count;

/*void perror_and_clean(const char* s)
{
  #ifdef _WIN32 
  	WSACleanup();
  #endif
  
  perror(s);
}
*/
void tcp_client_init(void)
{
  #ifdef _WIN32 
	// Initialize Winsock
	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
	    printf("WSAStartup failed: %d\n", res);
	    //return 1;
	    exit(1);
	}
  #endif
}

uint8_t tcp_client_connect(char* addr, u_short port)
{
	tcp_connected_flag = 0;
	
	if ((tcp_sock_fd = socket(AF_INET,  SOCK_STREAM, 0)) == -1) {
	  printf("Socket failed\n");
      #ifdef _WIN32 
      WSACleanup();
      #endif
  
	  exit(1);
	}
	
  	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(addr);
	memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

	//if (bind(recv_sock,(struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1){
	if (connect(tcp_sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
		if (tcp_verbose) printf("Connect failed\n");
		return 0;
	}

    //make socket non blocking
	//fcntl(tcp_sock_fd, F_SETFL, O_NONBLOCK);
	//fcntl(tcp_sock_fd, F_SETFL, fcntl(tcp_sock_fd, F_GETFL) | O_NONBLOCK)
	/*
	u_long iMode = 0;
	int ioctl_result = ioctlsocket(tcp_sock_fd, FIONBIO, &iMode);
	
    if (ioctl_result != NO_ERROR) {
        if (tcp_verbose) printf("ioctlsocket failed with error: %ld\n", ioctl_result);
		return 0;
	}*/
	
	tcp_connected_flag = 1;
	
	//addr_len = sizeof(struct sockaddr);
    
	return 1;
}


uint8_t tcp_client_connected(void)
{
    return tcp_connected_flag; 	
}


void tcp_client_write(uint8_t* send_data, size_t data_size)
{
	write_byte_count = send(tcp_sock_fd, (char*)send_data, data_size, 0);	

	if (tcp_verbose) {
		if (write_byte_count == SOCKET_ERROR){
    		printf("\nSent Failed\n");
		} else {
    		printf("(%d)", write_byte_count);
		}
	}

}


void tcp_client_disconnect(void)
{
	#if NETCLI
  		shutdown(tcp_sock_fd, SD_BOTH);
  		closesocket(tcp_sock_fd);	
		
		#ifdef _WIN32 
	  		WSACleanup();
  		#endif

  		tcp_client_init(); 	
  	#endif
}


int tcp_client_read_timeout(char *buf, int len, long sec, long usec)
{
    fd_set fds;
    int n;
    struct timeval tv;

    // set up the file descriptor set
    FD_ZERO(&fds);
    FD_SET(tcp_sock_fd, &fds);

    // set up the struct timeval for the timeout
    tv.tv_sec = sec;
    tv.tv_usec = usec;

    // wait until timeout or data received
    n = select(tcp_sock_fd + 1, &fds, NULL, NULL, &tv);
    if (n == 0) return -2; // timeout!
    if (n == -1) return -1; // error

    // data must be here, so do a normal recv()
    return recv(tcp_sock_fd, buf, len, 0);
}

/*
int recvtimeout(int s, char *buf, int len, long sec, long usec)
{
    fd_set fds;
    int n;
    struct timeval tv;

    // set up the file descriptor set
    FD_ZERO(&fds);
    FD_SET(s, &fds);

    // set up the struct timeval for the timeout
    tv.tv_sec = sec;
    tv.tv_usec = usec;

    // wait until timeout or data received
    n = select(s + 1, &fds, NULL, NULL, &tv);
    if (n == 0) return -2; // timeout!
    if (n == -1) return -1; // error

    // data must be here, so do a normal recv()
    return recv(s, buf, len, 0);
}

//if( recv(socket_desc, server_reply , 2000 , 0) < 0)
void tcp_client_read_timeout(long usec)
{
	int i;
    if (recvfromTimeOutUDP(recv_sock, 0, usec) == 0 ){
		if (tcp_verbose) {
			printf(".");
			fflush(stdout);
		}
		no_net++;
    } else {
    	no_net = 0;
		bytes_read = recvfrom(recv_sock, recv_data, 1024, 0, (struct sockaddr *)&client_addr, &addr_len);
 	    for(i = 0; i < io_num_channels; i++){
 	    	in.values[i] = ((uint16_t *)recv_data)[i];
 	    }

		if (tcp_verbose) {
		    printf("\n(%s , %d) said : ",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	    	printf("%s", recv_data);
	    	fflush(stdout);
		}
    }
	
}


*/
