/*
 * utils.c
 *
 */

#include "utils.h"
#include "..\control.h"

ms_timer millis_ms_timer;

void start_ms_timer(ms_timer* timer, int ms)
{
  gettimeofday(&(timer->start_value), NULL);
  timer->delta = ms;
}

int get_ms_timer(ms_timer* timer)
{
  double dt;
  struct timeval act;
  gettimeofday(&act, NULL);
  dt = (act.tv_sec * 1000 + act.tv_usec / 1000) - (timer->start_value.tv_sec * 1000 + timer->start_value.tv_usec / 1000.0);
  if (dt > timer->delta)
	  return 1;
  else
	  return 0;
}


int get_ms_timer_time(ms_timer* timer)
{
  struct timeval act;
  gettimeofday(&act, NULL);
  return (act.tv_sec * 1000 + act.tv_usec / 1000) - (timer->start_value.tv_sec * 1000 + timer->start_value.tv_usec / 1000.0);
}


void setup_timers()
{
	start_ms_timer(&millis_ms_timer,-1); 
}

uint32_t millis(void)
{
	return get_ms_timer_time(&millis_ms_timer);
}


/*  Timer Stuff of timer_t and start_timer() */

void refresh_timer(timer_t& T);

void start_timer(timer_t& T)
{
  T.start_time = millis();
  refresh_timer(T);
}

void init_timers(timer_t timer_array[], uint8_t timer_array_size)
{
  uint8_t i;
  for (i=0; i<timer_array_size; ++i) 
  {
  	start_timer(timer_array[i]);
  }
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



