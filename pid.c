#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#define ABS(x) (x > 0) ? x : -x


#define KP 1024		//e
#define KI 0		//E
#define KD 0		//e.

#define LI 2047		//I limit
#define LP 32767	//PWM limit
#define LE 2000		//error limit

#define FR 200		//read freq
#define FP 100		//pid freq

#define SM 1		//step multiplier
#define SIGNAL_MODE 1 //1=quad 0=clk/dir

#define PEA PIND	//port encoder A
#define PEB PIND	//port encoder B
#define PSA PIND	//port signal A/clk
#define PSB PIND	//port signal B/dir
#define PLR PORTD	//port led ready
#define DPWM DDRB	//port pwm
#define DLR DDRD	//port led ready

#define NEA 3		//pin nr. encoder A
#define NEB 4		//pin nr. encoder B
#define NSA 2		//pin nr. signal A/clk
#define NSB 5		//pin nr. signal B/dir
#define NPWM 3		//pin nr. pwm
#define NLR 6		//pin nr. led ready

int16_t int_sum = 0;
int16_t diff_old = 0;

int8_t enc_old;
int8_t sig_old;

int8_t pwm_res;



void delay_ms(uint16_t);
void error(int8_t);
void init(void);
void pwm_init(int8_t, int8_t);
void timer0_init(int8_t);
void encoder_init(void);
void signal_init(void);
void pwm(int16_t);
char read_encoder(void);
char read_signal(void);
int16_t pid(int16_t);


////////////////////////////////////////////////////////////////////	init

void init(){
	pwm_init(10, 0);
	//encoder_init();
	//signal_init();
	//timer0_init(0);

	DLR = (1<<NLR);
	DPWM = (1<<NPWM);
	
	return;
}

////////////////////////////////////////////////////////////////////	signal init

void signal_init(){
	if(SIGNAL_MODE){
		sig_old = ((PSA & (1<<NSA)) >> NSA) | ((PSB & (1<<NSB)) >> (NSB - 0));
	}
	else{
		sig_old = PSA & (1<<NSA);
	}
	return;
}

////////////////////////////////////////////////////////////////////	encoder init

void encoder_init(){
	enc_old = ((PEA & (1<<NEA)) >> NEA) | ((PEB & (1<<NEB)) >> (NEB - 0));
	return;
}

////////////////////////////////////////////////////////////////////	timer init

void timer0_init(int8_t i){
	switch(i){	//prescaler
		case 0:		//1
			TCCR0B = (1<<CS00);
		break;
		case 3:		//8
			TCCR0B = (1<<CS01);
		break;
		case 6:		//64
			TCCR0B = (1<<CS01)|(1<<CS00);
		break;
		case 8:		//256
			TCCR0B = (1<<CS02);
		break;
		case 10:	//1024
			TCCR0B = (1<<CS02)|(1<<CS00);
		break;
	}
	return;
}

////////////////////////////////////////////////////////////////////	pwm init

void pwm_init(int8_t i, int8_t j){	
	switch(i){	//bit
		case 8: //8 bit
			TCCR1A = (1<<WGM10)|(1<<COM1A1);
			pwm_res = 8;
		break;
		case 9: //9 bit
			TCCR1A = (1<<WGM11)|(1<<COM1A1);
			pwm_res = 7;
		break;
		case 10: //10 bit
			TCCR1A = (1<<WGM10)|(1<<WGM11)|(1<<COM1A1);
			pwm_res = 6;
		break;
	}
	
	switch(j){	//prescaler
		case 0:		//1
			TCCR1B = (1<<CS10);
		break;
		case 3:		//8
			TCCR1B = (1<<CS11);
		break;
		case 6:		//64
			TCCR1B = (1<<CS11)|(1<<CS10);
		break;
		case 8:		//256
			TCCR1B = (1<<CS12);
		break;
		case 10:	//1024
			TCCR1B = (1<<CS12)|(1<<CS10);
		break;
	}
	
	pwm(0);
	return;
}

////////////////////////////////////////////////////////////////////	signal

char read_signal(){
	if(SIGNAL_MODE){
		char A = PSA & (1<<NSA);
		char B = PSB & (1<<NSB);
			
		switch(sig_old){
			case 0:
				if(A){
					sig_old = 1;
					return(1);
				}
				else if(B){
					sig_old = 3;
					return(-1);
				}
				return(0);
			break;
			
			case 1:
				if(!A){
					sig_old = 0;
					return(-1);
				}
				else if(B){
					sig_old = 2;
					return(1);
				}
				return(0);		
			break;
			
			case 2:
				if(!A){
					sig_old = 3;
					return(1);
				}
				else if(!B){
					sig_old = 1;
					return(-1);
				}
				return(0);		
			break;
			
			case 3:
				if(A){
					sig_old = 2;
					return(-1);
				}
				else if(!B){
					sig_old = 0;
					return(1);
				}
				return(0);		
			break;
		}
		return(0);
	}
	else{
		char clk = PSA & (1<<NSA);
		char dir = (PSB & (1<<NSB)) ? 1 : -1;
	
		if(sig_old && ! clk){
			sig_old = clk;
			return(dir);
		}
	
		sig_old = clk;
		return(0);
	}
}

////////////////////////////////////////////////////////////////////	encoder

char read_encoder(){
	char A = PEA & (1<<NEA);
	char B = PEB & (1<<NEB);
		
	switch(enc_old){
		case 0:
			if(A){
				enc_old = 1;
				return(1);
			}
			else if(B){
				enc_old = 3;
				return(-1);
			}
			return(0);
		break;
		
		case 1:
			if(!A){
				enc_old = 0;
				return(-1);
			}
			else if(B){
				enc_old = 2;
				return(1);
			}
			return(0);		
		break;
		
		case 2:
			if(!A){
				enc_old = 3;
				return(1);
			}
			else if(!B){
				enc_old = 1;
				return(-1);
			}
			return(0);		
		break;
		
		case 3:
			if(A){
				enc_old = 2;
				return(-1);
			}
			else if(!B){
				enc_old = 0;
				return(1);
			}
			return(0);		
		break;
	}	
	
	error(2);
	return(0);
}

////////////////////////////////////////////////////////////////////	delay

void delay_ms(uint16_t ms)
/* delay for a minimum of <ms> */
{
	// we use a calibrated macro. This is more
	// accurate and not so much compiler dependent
	// as self made code.
	while(ms){
		_delay_ms(0.96);
		ms--;
	}
}

////////////////////////////////////////////////////////////////////	error

void error(int8_t i){
	pwm(0);
	int8_t j;
	for(j = 0; j < i; j ++){
		delay_ms(100);	
	}
	return;
}

////////////////////////////////////////////////////////////////////	pwm 

void pwm(int16_t i){
	if(i > LP){
		i = LP;
	}
	else if(i < -LP){
		i = -LP;
	}
	
	OCR1A = (i += 32768) >> pwm_res;
	return;
}

////////////////////////////////////////////////////////////////////	pid

int16_t pid(int16_t diff){
	if(ABS(int_sum + diff) <= LI){
		int_sum += diff;
	}
	
	int16_t i = diff * KP
			+	int_sum * KI
			+	(diff_old - diff) * KD;
			
	diff_old = diff;
	
	return(i);
}

////////////////////////////////////////////////////////////////////	main

int main(void){
	init();

	int tmp = 0;
	int diff = 0;

	while (1){
		if(TCNT0 >= FR){
			TCNT0 = 0;
			
			diff += read_encoder();
			diff += read_signal() * SM;
						
									
			if(tmp == FP){
				tmp = 0;
				if(ABS(diff) >= LE){
					error(5);
					diff = 0;
				}
				pwm(pid(diff));		
			}
			tmp++;
		}
	}*/
	
	return(0);
}
