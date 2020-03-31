#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>


#define IO_SIZE 32

extern uint8_t  inputs[IO_SIZE];
extern uint8_t outputs[IO_SIZE];

extern uint8_t s_up, s_down, s_near, s_touch, up, down, light;    // input image vars
extern uint8_t m_on, m_up, light_on, oil;  // output image vars


void control_setup(void);
void control_loop(void);


/*
 * Stuff regarding FSMs and TIS
 */
typedef struct {
  uint8_t  state ,       // Current State
           prev_state;   // Previous State
  uint32_t state_millis; // "Milis" of entry in cur state
  uint16_t TIS;          // Time In State - in units of 0.1 sec
} fsm_t;

void init_FSMs(fsm_t fsm_array[], uint8_t fsm_size);
void refresh_FSMs(fsm_t fsm_array[], uint8_t fsm_size);

#define FSM_SIZE 10          // FSM array goes from 0 to 9 
extern fsm_t FSMs[FSM_SIZE]; // Global array of FSMs

void init_FSMs(fsm_t FSMarray[], uint8_t fsm_size);


/*
 * Stuff regarding timer_t
 */
typedef struct {
  unsigned long start_time;  // from millis
  unsigned long p;           // preset in tenths of second
  uint8_t q;                 // output - ellapsed or not
} timer_t;


void refresh_timer(timer_t& T);
void start_timer(timer_t& T);
void refresh_timers(timer_t timer_array[], uint8_t timer_array_size);

#define TIMER_SIZE 10 
extern timer_t timer[TIMER_SIZE];

void init_timers(timer_t timer_array[], uint8_t timer_array_size);

#if _WIN32
    #include "pcfiles\utils.h"
#endif


#endif // CONTROL_H
