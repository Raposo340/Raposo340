/*
 * timer_tools.h
 *
 */

#ifndef UTILS_H_
#define UTILS_H_

#ifdef _WIN32 
    #include <sys/time.h>
    #include <stdint.h>
    #include <iostream>

	typedef struct {
		struct timeval start_value;
		int delta;
	} ms_timer;
	
	void     start_ms_timer(ms_timer* timer, int delta_time_ms);
	int      get_ms_timer(ms_timer* timer);
	void     setup_timers();
	uint32_t millis(void);

#endif




//  Desguise the serial.print command under PC


#ifdef _WIN32 
    #include <iostream>
    #include <string>
    using namespace std;
#endif

class Serial_Class
{
  public:
   void print  (string s)  { cout << s.c_str()    << flush ; }
   void println(string s)  { cout << s.c_str()    << endl << flush; }
   void print  (int x)     { cout << to_string(x) << flush; }
   void println(int x)     { cout << to_string(x) << endl << flush; }
   void println()          { cout << endl << flush; }
};

extern Serial_Class Serial; //  Desguise the serial.print command under PC


#endif /* UTILS_H_ */
