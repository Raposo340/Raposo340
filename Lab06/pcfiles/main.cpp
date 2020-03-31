#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#include "tcp_client.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "..\\control.h"
#if NETCLI
  #include "modbusTCP.h"
#endif
#include "utils.h"

#include <conio.h>

void control_loop(void);

char tcp_address[]={"127.000.000.001\0\0\0\0\0\0"};
int port = 5502;

#if NETCLI
	modbusTCP_t modbus;
#endif

uint8_t  inputs[IO_SIZE];
uint8_t outputs[IO_SIZE];

// Global variables for controller and the Arduino Loop and FSMs and timer_t
uint8_t s_up, s_down, s_near, s_touch, up, down, light;    // input image vars
uint8_t m_on, m_up, light_on, oil;  // output image vars
uint32_t loop_count;
uint8_t cnt;


fsm_t FSMs[FSM_SIZE];      // Global array of fsm_t
timer_t timer[TIMER_SIZE]; // Global array of timer_t

Serial_Class Serial;  // Desguise the serial.print command under PC


void tcp_connect(void)
{
	#if NETCLI
		printf("Connecting to: %s:%d\n", tcp_address, port);

    	while (!tcp_client_connect(tcp_address, port)) {
        	printf("Connection failed! Retrying...\n");
        	sleep(2);
    	}
	    printf("\nTCP client on port %d connected\n", port);
    #else
      printf("Standalone version... no client/server...");
    #endif        

    fflush(stdout);
}


uint32_t previousMillis, interval;
int key, oldkey;
extern uint8_t server[];
#define ESC_KEY 27

int main(int argc, char *argv[])
{
	
    if (argc > 1) strcpy(tcp_address, argv[1]);
    else  sprintf(tcp_address,"%d.%d.%d.%d\0\0", server[0], server[1], server[2], server[3]);

	#if NETCLI
    	tcp_verbose = 0;
    	tcp_client_init();
    	tcp_connect();
    #endif

    interval = 100;
    
    setup_timers(); // give support to millis function on a PC
    init_FSMs(FSMs, FSM_SIZE);
	control_setup();     // Cascade Call to setup

    while(1) {

		#if NETCLI
	        uint8_t b;
    	    int i;
    	    int c = tcp_client_read_timeout((char*)&b, 1, 0, 100);
	        if (c == -1 || c == 0) {
	            // if the server's disconnected, stop the client:
	            Sleep(1);  // miliseconds
	            printf("\nDisconnecting.\n");
	            tcp_client_disconnect();
	            Sleep(500);
	
	            // try to connect again:
	            tcp_connect();
	        } else if (c == -2) {  // Timeout
	            //printf("*");
	        } else  {
	            //printf("%c", (char)b);
	            //printf(" ");
	            modbus.read_state_machine(b);
	        }
		#endif

        uint32_t currentMillis = GetTickCount();
        uint32_t elapsed = currentMillis - previousMillis;
        if (elapsed >= interval) {
            previousMillis = currentMillis;

            if (loop_count & 1) {

				#if NETCLI
	    	    	// Systematic read ALL Inputs form Modbus communications
	        		for (i = 0; i < IO_SIZE; i++) {
		            	inputs[i] = modbus.get_input(i);
		            	// printf("%d", inputs[i]);  // for debug
		        	}
        		#endif    

				} 

				if (_kbhit() && (loop_count % 4) == 3) {
					key = _getch();
					if (key == 'q') up ^= 1;
					if (key == 'a') down ^= 1;
					if (key == 'l') light ^= 1;
					if (key == 'u') s_up ^= 1;
					if (key == 'd') s_down ^= 1;
					if (key == 'n') s_near ^= 1;
					if (key == 't') s_touch ^= 1;
					if(key == ESC_KEY) {
						tcp_client_disconnect();
			            printf("\n \r \n \r    Exiting... \n");
	            		fflush(stdout);
						Sleep(300);
						exit(0);
					}
				}

                // Timers and FSM stuff
                refresh_FSMs(FSMs, FSM_SIZE);
                refresh_timers(timer, TIMER_SIZE);
                

                // User control function
                control_loop();

             
				#if NETCLI
	                // Systematic write ALL outputs to Modbus communications
	                for (i = 0; i< IO_SIZE; i++) {
	                    modbus.set_coil(i, outputs[i]);
	                modbus.write_multiple_coils();
    	            tcp_client_write(modbus.out_buf, modbus.out_buf_count);
	                }
                #endif

                //printf(" - b12=%d%d  led_RGB=%d%d%d", botao_1, botao_2,   led_r, led_g, led_b);
            } else {
				#if NETCLI
	                modbus.read_multiple_inputs();
    	            tcp_client_write(modbus.out_buf, modbus.out_buf_count);
    	        #endif
            }

            // Debug Stuff
            loop_count++;
            //printf("\r");
            fflush(stdout);
        }

        //printf(".");
        //fflush(stdout);
        //Sleep(10);  // miliseconds
    return 0;
}


