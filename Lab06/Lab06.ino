/* Sistemas e Automação - FEUP 2019/20 - Armando Sousa e Paulo Costa */

#include "Arduino.h"
#include <avr/boot.h>

#include "modbusTCP.h"

#include <SPI.h>
#include <Ethernet.h>

#include "control.h"

#define UniqueIDsize 9

uint8_t UniqueID[UniqueIDsize];

void fill_unique_ID(void)
{
  for (uint8_t i = 0; i < UniqueIDsize; i++) {
    UniqueID[i] = boot_signature_byte_get(0x0E + i + (UniqueIDsize == 9 && i > 5 ? 1 : 0));
  }
}


uint32_t crc32c(uint8_t* data, uint8_t num_bytes)
{
  uint32_t b, mask;
  uint32_t crc = 0xFFFFFFFF;
  uint8_t i;
  int8_t shift;

  for(i = 0 ; i < num_bytes ; i++ ) {
    b = data[i];
    crc = crc ^ b;

    for(shift = 7; shift >= 0; shift--) {
      mask = -(crc & 1);
      crc = (crc >> 1) ^ (0xEDB88320 & mask);
    }
  }
  return ~crc;
}


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  //physical mac address
extern byte server[];// = {192, 168, 1, 172};

EthernetClient client;

uint32_t previousMicros, interval;

void serial_print_ip(byte ip[])
{
  Serial.print(ip[0]);
  Serial.print('.');

  Serial.print(ip[1]);
  Serial.print('.');
  
  Serial.print(ip[2]);
  Serial.print('.');
  
  Serial.print(ip[3]);
}


void tcp_connect(void)
{
  Serial.print("Connecting to: ");
  serial_print_ip(server);
  Serial.print(" : 5502");
  Serial.println();

  client.connect(server, 5502);  // Silently try once
  delay(3000);
  
  while (!client.connect(server, 5502)) {
    Serial.println("Connection failed! Retrying...");
    delay(3000);
  }
  
  Serial.println("Connected!");
}


modbusTCP_t modbus;

uint8_t  inputs[IO_SIZE];
uint8_t outputs[IO_SIZE];


void setup(void) 
{
  // Setup Local Pins (LED_RGB)
  const int pino_led_r  = 2;    // Output
  const int pino_led_g  = 3;    // Output
  const int pino_led_b  = 5;    // Output
  const int pino_botao_1  = 7;  // Input
  const int pino_botao_2  = 6;  // Input

  pinMode(pino_led_r,   OUTPUT);     
  pinMode(pino_led_g,   OUTPUT);     
  pinMode(pino_led_b,   OUTPUT);     
  pinMode(pino_botao_1, INPUT_PULLUP); 
  pinMode(pino_botao_2, INPUT_PULLUP); 

  // Power On Self Test (POST)
  digitalWrite(pino_led_r,   HIGH); delay(500);                        
  digitalWrite(pino_led_g,   HIGH); delay(500);                        
  digitalWrite(pino_led_b,   HIGH); delay(500);                        

  digitalWrite(pino_led_r,   LOW);  
  digitalWrite(pino_led_g,   LOW);  
  digitalWrite(pino_led_b,   LOW);  


  uint32_t crc;
  uint8_t i;
   
  Serial.begin(115200);
  Serial.println("Sistemas e Automacao - FEUP - 2019/20");
  
  fill_unique_ID();
  for (i = 0; i < UniqueIDsize; i++) {
    if (UniqueID[i] < 0x10) Serial.print("0");
    Serial.print(UniqueID[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  crc = crc32c(UniqueID, UniqueIDsize);
  mac[2] = (crc >> 24) & 0xFF;
  mac[3] = (crc >> 16) & 0xFF;
  mac[4] = (crc >>  8) & 0xFF;
  mac[5] = (crc >>  0) & 0xFF;

  Serial.print(crc, HEX);
  Serial.println();

  #if NETCLI

  
      #define DHCP 1
      
      #ifdef DHCP
        Serial.println("Getting DHCP IP address...");
        while(Ethernet.begin(mac) == 0) {
          Serial.println("Failed to configure Ethernet using DHCP");
          delay(2000);
        }
      #else
        byte static_ip[] = { 192, 168, 112, 48  };    
        byte dns_ip[]    = { 193, 136, 28,  10  };    
        byte gateway_ip[]= { 192, 168, 112, 254 };    
        byte subnet[]    = { 255, 255, 255, 0   };    
        Ethernet.begin(mac, static_ip, dns_ip, gateway_ip, subnet);  // Ethernet.begin(mac, ip, dns, gateway, subnet);
      #endif
      
       
      Serial.print("Ethernet Shield IP address: ");
      Serial.println(Ethernet.localIP());
      Serial.println();
    
      tcp_connect();

  #endif 
  //NETCLI

  previousMicros =  micros();
  interval = 100UL * 1000UL;

  init_FSMs(FSMs, FSM_SIZE);

  control_setup(); // Cascade call
}


/*  Timer Stuff of timer_t and start_timer() */

void start_timer(timer_t& T)
{
  T.start_time = millis();
  refresh_timer(T);
}

void refresh_timer(timer_t& T)
{
  if(millis() - T.start_time > T.p * 100)  
    T.q = 1;
  else 
    T.q = 0;
}

void refresh_timers(timer_t timer_array[], uint8_t timer_array_size)
{
  uint8_t i;
  for (i=0; i<timer_array_size; ++i) refresh_timer(timer_array[i]);
}



/*  FSMs and timers with TIS */
void init_FSMs(fsm_t fsm_array[], uint8_t fsm_size)
{
  uint8_t i;
  for (i=0; i<fsm_size; ++i) 
  {
    fsm_array[i].state=0;
    fsm_array[i].prev_state=-1;
    fsm_array[i].state_millis=millis();
    fsm_array[i].TIS=0;
  }
}



void refresh_FSMs(fsm_t fsm_array[], uint8_t fsm_size)
{
  uint8_t i;
  for (i=0; i<fsm_size; ++i) {
    if (fsm_array[i].state != fsm_array[i].prev_state)  {
      fsm_array[i].state_millis=millis();
      fsm_array[i].TIS=0;
      fsm_array[i].prev_state=fsm_array[i].state;
    } else {
      fsm_array[i].TIS = (millis() - fsm_array[i].state_millis)/100; // Units of 0.1 sec
    } 
  }
}




// variables for controller and the Arduino Loop and 

uint32_t loop_count;
uint8_t s_up, s_down, s_near, s_touch, up, down, light;    // input image vars
uint8_t m_on, m_up, light_on, oil;  // output image vars
// FSMs and timer_t stuff
fsm_t FSMs[FSM_SIZE]; // Global array of FSMs
timer_t timer[TIMER_SIZE];




void loop()
{
  byte i;

  #if NETCLI
    if (client.available()) {
        byte c = client.read();
        //Serial.print(c);
        //Serial.print(' ');
        modbus.read_state_machine(c);
      }  

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("Disconnecting.");
    client.stop();
    // try to connect again:
    tcp_connect();
  }
  #endif
  // NETCLI

  uint32_t currentMicros = micros();
  uint32_t elapsed = currentMicros - previousMicros;
  
  if (elapsed >= interval) {
    previousMicros = currentMicros;

    modbus.set_coil(0, loop_count & 2); // Debug Stuff

    if (loop_count & 1) {

      // Systematic read ALL Inputs form Modbus communications
      for (i = 0; i < IO_SIZE; i++) {
        inputs[i] = modbus.get_input(i);
      }

      // Read from addresses to specific variables; correct to positive logic
  // check for serial input
  if (Serial.available()   &&   (loop_count % 4) == 3)  { //if something in serial buffer
    uint8_t key = Serial.read(); //gets byte from buffer
    if (key == 'q') up ^= 1;
    if (key == 'a') down ^= 1;
    if (key == 'l') light ^= 1;
    if (key == 'u') s_up ^= 1;
    if (key == 'd') s_down ^= 1;
    if (key == 'n') s_near ^= 1;
    if (key == 't') s_touch ^= 1;
  } 
  
      // Timers and FSM stuff
      refresh_FSMs(FSMs, FSM_SIZE);  
      refresh_timers(timer, TIMER_SIZE);
      
      // User control function
      control_loop();

      // Write vars to specific addresses

      // Systematic write ALL outputs to Modbus communications
      for (i = 0; i< IO_SIZE; i++) {
        modbus.set_coil(i, outputs[i]);
      }

      modbus.write_multiple_coils(); 
      client.write(modbus.out_buf, modbus.out_buf_count);
    } else {
      modbus.read_multiple_inputs();
      client.write(modbus.out_buf, modbus.out_buf_count);
    }
    
    /*for (i = 0; i < modbus.out_buf_count; i++) {
      if (modbus.out_buf[i] < 0x10) Serial.print("0");
      Serial.print(modbus.out_buf[i], HEX);
      Serial.print(" ");
    }*/

    /*
    // Debug Stuff
    Serial.print("I=");
    for (i = 0; i < IO_SIZE; i++) {
      if (i%8==0) Serial.print(" ");
      Serial.print(inputs[i]&1);
    }
    Serial.print("\tO=");
    for (i = 0; i < IO_SIZE; i++) {
      if (i%8==0) Serial.print(" ");
      Serial.print(outputs[i]&1);
    }
    */
    //Serial.print("Count ");
    //Serial.print(modbus.ReceivedMessagesCount);
    //Serial.print(" Inputs ");
    //Serial.println(modbus.Inputs, BIN);
    //Serial.println();
    loop_count++;
  } 
}
