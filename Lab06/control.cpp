/* Systems & Automation - FEUP 2019/20 - Armando Sousa & Paulo Costa */
#ifdef _WIN32 
  #include "pcfiles\utils.h"
#else
  #include "Arduino.h"
#endif


#include "control.h"

uint8_t server[] = {192,168,2,12};   // IP address of the computer running the SERVER



// Variable List (for inspiration only)
//   botao_1, botao_2, button, emergency, sensor_left, sensor_right;    // input image vars
//   led_r, led_g, led_b, green, yellow, red, motor_left, motor_right;  // output image vars



// Example descriptive but short names for state,  *.TIS  and  an example shorneter for timer[0]
#define EX_STa  FSMs[0].state
#define EX_TISa FSMs[0].TIS
#define EX_Ta   timer[0]

#define EX_STb  FSMs[1].state
#define EX_TISb FSMs[1].TIS
#define EX_Tb   timer[1]

#define EX_STc  FSMs[2].state
#define EX_TISc FSMs[2].TIS
#define EX_Tc   timer[2]

#define CNTmax 2 //numero de ciclos por lubrificaçao 

void control_setup(void)
{
  timer[0].p=30; // preset for 3 seconds -- change as needed
}

uint8_t CNT;			//variaveis globais ja estao inicializadas a 0.
uint8_t old_s_down;	 
uint8_t old_down;	 
uint8_t old_up;	 
uint8_t old_light;	 

void control_loop(void)
{

  // Write your code here --- also see above control_setup()
  uint8_t Re_s_down = s_down && !old_s_down;
  uint8_t Re_down = down && !old_down;
  uint8_t Re_up = up && !old_up;
  uint8_t Re_light = light && !old_light;
  


  
  	//luz
	if ((EX_STb == 0) && (light))  
		{ EX_STb = 1; }	else
	if ((EX_STb == 0) && (EX_STa > 0))  
		{ EX_STb = 2; }	else

	if ((EX_STb == 2) && (light))  
		{ EX_STb = 1; }	else
	if ((EX_STb == 1) && (EX_TISb>600) && (EX_TISa>300) && (EX_STa==0))  
		{ EX_STb = 0; }	else
	if ((EX_STb == 2) && (EX_TISa>300) && (EX_STa==0))  
		{ EX_STb = 0; }	else
	if ((EX_STb == 1) && (Re_light))  
		{ EX_STb = 3; }	else
	if ((EX_STb == 3) )  
		{ EX_STb = 1; }	
	  
  
  	//oil
	if ((EX_STc == 0) && (Re_s_down))  
		{ CNT++;
		EX_STc = 0; }	else
	if ((EX_STc == 0) && (CNT == CNTmax) && (EX_STa == 40))  
		{ EX_STc = 1; }	else
	if ((EX_STc == 1) &&  (EX_STa == 0))  
		{ CNT=0;
		EX_STc = 0; }	
	
	
  	//motor												  // esta fsm foi colocada aqui para evitar erros em outras fsm, nomeadamente a da luz
  											
	if ((EX_STa == 0)   &&   ((Re_down) && (!s_down) ))   //	re_down ->> quando está no estado 40 e se clique down
		{ EX_STa = 20; } else							  //	ele para e so no proximo clique no down ele va para o estado 20
	if ((EX_STa==0) && ((Re_up) && (!s_up)))			  //	o re_up ->> usado de forma analoga a re_down
		{ EX_STa = 40; } else
	if ((EX_STa==20) && ((s_near)))
		{ EX_STa = 21; } else
	if ((EX_STa==20) && ((s_touch) || (up)|| (s_down)))
		{ EX_STa = 0; } else
	if ((EX_STa==21) && ((up) || (s_down)))
		{ EX_STa = 0; } else
	if ((EX_STa==40) && ((s_up) || (down)))
		{ EX_STa = 0; } 
	
	
	
  	if (CNT>=CNTmax)		//se por algum motivo o s_down se ativar de forma anormal, isto evita que o contador excede 2 (neste caso), caso em que este não ia servir de nada
  		{CNT=CNTmax;}			//pelo que o portão nunca iria lubrificar
  
	// Calculate Outputs
	m_on      = (EX_STa == 20) || (EX_STa == 21) || (EX_STa==40);
	m_up      = (EX_STa == 40);
	light_on  = (EX_STb == 1) || (EX_STb == 2) || (EX_STb == 3);
	oil       = (EX_STc == 1);


	old_s_down=s_down;
	old_down=down;
	old_up=up;
	old_light=light;
	
  // End of example simple state machine (comment out, delete or modify code above)

  // Usefull Debug - report of relevant vars
  Serial.print("up, lig, dn=");  Serial.print(up); Serial.print(light); Serial.print(down);
  Serial.print("   S_UP, S_TOU, S_NEAR, S_DOWN="); Serial.print(s_up);  Serial.print(s_touch); Serial.print(s_near);  Serial.print(s_down); 

  Serial.print("   ||| M_ON, M_UP=");     Serial.print(m_on); Serial.print(m_up);
  Serial.print("   |LAMP, OIL=");     Serial.print(light_on); Serial.print(oil);


  //Serial.print(" ||| t.p/q"); Serial.print(timer[0].p); Serial.print("/"); Serial.print(timer[0].q);
  Serial.print(" |STATE_a="); Serial.print(FSMs[0].state);  
  Serial.print(" |STATE_b="); Serial.print(FSMs[1].state);  
  Serial.print(" |STATE_c="); Serial.print(FSMs[2].state);  
  Serial.print(" |tis_a="); Serial.print(FSMs[0].TIS);
  Serial.print(" |tis_b="); Serial.print(FSMs[1].TIS);
  Serial.print(" |tis_c="); Serial.print(FSMs[2].TIS);
  Serial.print(" |||CNT="); Serial.print(CNT);  
  //Serial.print("    |||RESD="); Serial.print(Re_s_down);
  
  Serial.println();

}
