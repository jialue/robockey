/*
 * robockeyTest.c
 *
 * Created: 11/13/2012 11:32:58 AM
 *  Author: jialue
 */ 


#include <avr/io.h>
#include "m_robockey.h"
#include <math.h>




/**********************************************************************************
INITIALIZATION
***********************************************************************************/
void timer1_init(float pwm_freq);
void timer3_init(int sample_rate);
void timer0_init(int sample_rate);
void adc_init();
void mode_init();

// Public functions
// Initialize peripheral devices
void robockey_init(void)
{
	m_clockdivide(0);	// 16 MHz
	sei();				// Enable global interrupt
	
	timer1_init(1);	// kHz
	timer3_init(50);	// Hz
	//timer0_init(1);	// kHz
	adc_init();
	mode_init();
	
	m_rf_open(CHANNEL,RXADDRESS,PACKET_LENGTH_RF);	// Enable wireless
	m_usb_init();									// Enable usb communication
	m_bus_init();									// Enable mbus
	m_wii_open();									// Enable wii camera
}

// Private Functions
// Initialize timer3
void timer3_init(int sample_rate)	// using TIMER3
{
	// Localization sample rate, using timer 3
	set(TIMSK3,OCIE3A);	// Enable interrupt
	set(TCCR3B,WGM32);	// (mode 4) Up to OCR3A
	OCR3A = 62500/sample_rate;
	set(TCCR3B,CS32);	// Clock source /256
}

//void timer0_init(int sample_rate)	// using TIMER3
//{
	//// Localization sample rate, using timer 3
	//set(TIMSK0,OCIE0A);	// Enable interrupt
	//set(TCCR0A,WGM01);	// (mode 2) Up to OCR3A
	//OCR3A = 250/sample_rate;
	//set(TCCR0B,CS01);	// Clock source /64
	//set(TCCR0B,CS00);	// Clock source /64
//}

void timer1_init(float pwm_freq)
{
	set(DDRB,6);	// Configure B6 for output
	set(DDRB,7);	// Configure B7 for output

	set(TCCR1B,WGM13);	// Mode 15 fast PWM
	set(TCCR1B,WGM12);
	set(TCCR1A,WGM11);
	set(TCCR1A,WGM10);
	set(TCCR1A,COM1B1);	// Clear at OCR1B, set at rollover
	set(TCCR1A,COM1C1);	// Clear at OCR1C, set at rollover
	OCR1A = 16000/pwm_freq;
	OCR1B = 0;
	OCR1C = 0;
	//OCR1B = 8000/pwm_freq;
	set(TCCR1B,CS10);	// Pre-scaler /1
}

// Initialize ADC
void adc_init()
{
	set(DIDR0, ADC0D);
	set(DIDR0, ADC1D);
	set(ADMUX,REFS0);	// Set voltage reference to Vcc
	clear(ADMUX,REFS1);
	set(ADCSRA,ADPS0);	// Set ADC prescaler to /128
	set(ADCSRA,ADPS1);
	set(ADCSRA,ADPS2);
	set(ADCSRA, ADIE);	// enable the interrupt after conversion
	
	set(ADCSRA,ADEN);	// Enable the ADC subsystem
	set(ADCSRA,ADSC);
}

void mode_init()
{
	clear(DDRB,0);
	clear(DDRB,1);
	clear(DDRB,2);
	clear(DDRB,3);
	
	// ADC Input
	clear(DDRD,7);
	
	// Output signal for shooting
	set(DDRD,4);
	set(DDRB,4);
	set(DDRB,5);
}



/**********************************************************************************
CHECK MODE AND DO CORRESPONDING EXECUTION
***********************************************************************************/
int check_mode()
{
	//cli();
	int digit_1, digit_2, digit_3, digit_4, mode;
	
	digit_1 = check(PINB,0);
	digit_2 = check(PINB,1);
	digit_3 = check(PINB,2);
	digit_4 = check(PINB,3);
	mode = digit_1*2*2*2 + digit_2*2*2 + digit_3*2 + digit_4;
	
	return mode;
}


/**********************************************************************************
COMMAND	EXECUTION
***********************************************************************************/
int comm(int commd)
{
	if(commd)
	{
		//m_red(TOGGLE);
		switch (commd){
			case commandTest:
			m_red(ON);
			m_wait(500);
			m_red(OFF);
			m_wait(500);
			break;
			
			case PLAY:
			m_green(TOGGLE);
			break;
			
			case GOALA:
			OCR1B = 0;
			OCR1C = 0;
			break;
			
			case GOALB:
			OCR1B = 0;
			OCR1C = 0;
			break;
			
			case PAUSE:
			OCR1B = 0;
			OCR1C = 0;
			break;
			
			case HALFTIME:
			OCR1B = 0;
			OCR1C = 0;
			break;

			case GAMEOVER:
			OCR1B = 0;
			OCR1C = 0;
			break;

			default:			
			OCR1B = 0;
			OCR1C = 0;
			commd = 0;
			return 0;
			break;
		}
		return 1;
	}
	return 0;
}


/**********************************************************************************
LOCALIZATION
***********************************************************************************/

// Set Operations
int set_intersection(int* arr1, int* arr2, int m, int n, int* result)
{
	int temp = 0, result_flag = 0;
	for(int i = 0; i < m; i++)
	{
		for(int j = 0; j < n; j++)
		{
			if(arr1[i] == arr2[j])
			{
				result[0] = arr1[i];
				temp++;
				result_flag = 1;
			}
		}
	}
	return result_flag;
}

int set_difference(int* arr1, int* arr2, int m, int n, int* result)
{
	int temp = 0, result_flag;
	for(int i = 0; i < m; i++)
	{
		int flag = 0;
		for(int j = 0; j < n; j++)
		{
			if(arr1[i] == arr2[j]) flag = 1;
		}
		if(flag == 0)
		{
			result[temp++] = arr1[i];
			result_flag = 1;
		}
	}
	return result_flag;
}

int cal_norm(int* x, int* y, int result[][3])
{
	int combos[6][2] = {{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}};
	int vect[6][2];	// vect[i] = {x, y}, the value of the ith vectors
	int result_flag= 0;
	for(int i = 0; i < 6; i++)
	{
		int from, to;
		// 2 points 'from' and 'to'
		from = combos[i][0];
		to = combos[i][1];
		
		// vectors
		vect[i][0] = x[to] - x[from];
		vect[i][1] = y[to] - y[from];
		
		result[i][0] = vect[i][0]*vect[i][0] + vect[i][1]*vect[i][1];
		result[i][1] = from;
		result[i][2] = to;
		
		result_flag = 1;
	}
	return result_flag;
}

int world_frame(int vect_norm[][3], float* result)	// scale for world frame, in pixel.
{
	float m = 0, scale = 0;
	for(int i = 0; i < 6; i++)
	{
		m = fmax(vect_norm[i][0], m);
	}
	scale =  sqrt(m)/29.0;
	result[0] = 511.5;	// the unit is originally pixel during the calculation
	result[1] = 14.5*scale + 383.5;
	result[2] = scale;
	//result[2] = -10.563 * scale + 511.5;
	//result[3] = 2.483 * scale + 383.5;
	//result[4] = 511;
	//result[5] = -14.5 * scale +383.5;
	//result[6] = 11.655 * scale + 511;
	//result[7] = 8.741 * scale + 383.5;
	//result[0] = 511.5;
	//result[1] = 14.5 * scale + 383.5;
	//float world_B[2] = {-10.563 * scale + 511.5, 2.483 * scale + 383.5};
	//float world_C[2] = {511, -14.5 * scale +383.5};
	//float world_D[2] = {11.655 * scale + 511, 8.741 * scale + 383.5};
	if(scale) return 1;
	return 0;
}

int id_stars(int vect_norm[][3], int* result)
{
	int temp[4], temp1[2], temp2[2], intersect[2], diff[4], max_index, min_index;
	float max = vect_norm[0][0], min = vect_norm[0][0];
	for(int i = 0; i < 6; i++)
	{
		if(vect_norm[i][0] >= max)
		{
			max = vect_norm[i][0];
			max_index = i;
		}
		
		if(vect_norm[i][0] <= min)
		{
			min = vect_norm[i][0];
			min_index = i;
		}
	}
	
	temp1[0] = vect_norm[max_index][1];
	temp1[1] = vect_norm[max_index][2];
	temp2[0] = vect_norm[min_index][1];
	temp2[1] = vect_norm[min_index][2];
	
	temp[0] = vect_norm[max_index][1];
	temp[1] = vect_norm[max_index][2];
	temp[2] = vect_norm[min_index][1];
	temp[3] = vect_norm[min_index][2];
	
	// id the stars if no star is lost
	if(set_intersection(temp1, temp2, 2, 2, intersect))
	{
		result[0] = intersect[0];	// north star
		
		set_difference(temp, result, 2, 1, diff);
		result[1] = diff[0];		// south star
		
		set_difference(&temp[2], result, 2, 1, diff);
		result[2] = diff[0];		// east star
		
		set_difference(temp, result, 4, 3, diff);
		result[3] = diff[0];		// west star
		return 1;
	}
	return 0;
}

int localization(unsigned int* rawData, float* calibration, float* result)
{
	int A[2], B[2], C[2], D[2];
	int x[4], y[4];
	int CA[2], stars[4];
	float theta, offset[2], center[2] = {511.5, 383.5};	// the center in pixel under camera frame
	float world[3];
	int vect_norm[6][3];
	
	x[0] = rawData[0];
	x[1] = rawData[3];
	x[2] = rawData[6];
	x[3] = rawData[9];
	y[0] = rawData[1];
	y[1] = rawData[4];
	y[2] = rawData[7];
	y[3] = rawData[10];
	
	if(y[0] == 1023 || y[1] == 1023 || y[2] == 1023 || y[3] == 1023) return 0;
	
	cal_norm(x, y, vect_norm);
	
	if(id_stars(vect_norm, stars))
	{
		A[0] = x[stars[0]];
		A[1] = y[stars[0]];
		C[0] = x[stars[1]];
		C[1] = y[stars[1]];
		D[0] = x[stars[2]];
		D[1] = y[stars[2]];
		B[0] = x[stars[3]];
		B[1] = y[stars[3]];
		
		int valid_stars[8] = {A[0], A[1], B[0], B[1], C[0], C[1], D[0], D[1]};
		int valid_star[2] = {0, 0};
		int valid_index;
		
		// calculate the angle between 2 y axis in the world frame 
		world_frame(vect_norm, world);
		
		// we need 2 stars to indicate the orientation, here we just take the north and the south. If CA does not exist, return 0
		if(A[1] != 1023 && C[1] != 1023)
		{
			CA[0] = A[0] - C[0];
			CA[1] = A[1] - C[1];
			theta = PI/2 - atan2(CA[1], CA[0]);	// -pi/2 if change to x-axis
		}
		else return 0;

//************************************************************		
		// if any star exist, use it to calculate the location, NOT COMPLETED!!!!!!!!!!!!!!!!!!!!
		//for(int i = 0; i < 4; i++)
		//{
			//if(valid_stars[2 * i + 1] != 1023) 
			//{
				//valid_star[0] = valid_stars[2 * i];
				//valid_star[1] = valid_stars[2 * i + 1];
				//valid_index = i;
				//break;
			//}
		//}
		//offset[0] = world[2 * valid_index] - valid_stars[2 * valid_index] * cos(theta) + valid_stars[2 * valid_index + 1] * sin(theta);
		//offset[1] = world[2 * valid_index + 1] - valid_stars[2 * valid_index] * sin(theta) - valid_stars[2 * valid_index + 1] * cos(theta);
//************************************************************
		
		// calculate the offset between camera frame and world frame.
		offset[0] = world[0] - A[0] * cos(theta) + A[1] * sin(theta);
		offset[1] = world[1] - A[0] * sin(theta) - A[1] * cos(theta);
		

		
		// change the center to the world frame
		result[0] = cos(theta) * center[0] - sin(theta) * center[1] + offset[0];
		result[1] = sin(theta) * center[0] + cos(theta) * center[1] + offset[1];
		result[0] = (511.5 + 511.5 - result[0] - 511.5) / world[2] - calibration[0];	// change the unit to centimeter
		result[1] = (result[1] - 383.5) / world[2] - calibration[1];
		result[2] = PI - theta;	// angle from Y_camera to X_world in world frame.
		
		while(result[2] > PI) result[2] -= 2*PI;	// make the abs(angle) within PI
		while(result[2] < -PI) result[2] += 2*PI;
		
		return 1;
	}
	return 0;
} 


/**********************************************************************************
PUCK FINDING
***********************************************************************************/
int find(int* adc1, int* adc2, int* adc3, int* adc4, int* adc5, int* adc6, int* count)	// return 1 when the robot get the puck, or 0 when it is still searching for puck.
{
	if(!check(PIND, 7)) return 1;
	else
	{
		int ocr1a, ocr1b, ocr1c, diff, d1, d2, d_gain;
		float speed_gain;
		ocr1a = OCR1A;
		d_gain = 10; // 8
		speed_gain = 0.75; // 0.75
		
		//if(*count)
		//{
			//for(int i = 0; i < *count; i++)
			//{
				//ocr1b = 0.8*ocr1a;
				//ocr1c = 0.2*ocr1a;
				//OCR1B = ocr1b;
				//OCR1C = ocr1c;
			//}
			//*count = 0;
		//}
		
		if(adc1[0] > 50 || adc2[0] > 50)	// 50
		{
			diff = adc1[0] - adc2[0];			
			d1 = adc1[0] - adc1[1];
			d2 = adc2[0] - adc2[1];
			
			if(diff > DEADBAND2)		// turn left
			{
				set(PORTB, 4);
				ocr1b = 0.45*ocr1a;	// 0.65
				
				set(PORTB, 5);
				ocr1c = 0.6*ocr1a; //0.75
				OCR1B = ocr1b;
				OCR1C = ocr1c;
				if(d1 < 0)
				{
					ocr1c =  ocr1c - d_gain*d1;
					OCR1C = ocr1c;
					ocr1b =  ocr1b + d_gain*d1;
					OCR1B = ocr1b;
				}
			}
			else if(diff < -DEADBAND2)	// turn right
			{
				set(PORTB, 4);
				ocr1b = 0.6*ocr1a;
				
				set(PORTB, 5);
				ocr1c = 0.45*ocr1a;
				OCR1B = ocr1b;
				OCR1C = ocr1c;
				if(d2 < 0)
				{
					ocr1b =  ocr1b - d_gain*d2;
					OCR1B = ocr1b;
					ocr1c =  ocr1c + d_gain*d2;
					OCR1C = ocr1c;
				}
			}
			
			else
			{
				set(PORTB, 4);
				set(PORTB, 5);
				ocr1b = speed_gain*ocr1a;
				ocr1c = speed_gain*ocr1a;
				OCR1B = ocr1b;
				OCR1C = ocr1c;
			}
		}
		else
		{
			//if(adc3[0] > 360)
			//{
				//ocr1b = 1*ocr1a;
				//ocr1c = 0*ocr1a;
				//OCR1B = ocr1b;
				//OCR1C = ocr1c;
			//}
			//else if(adc4[0] > 320)
			//{
				//ocr1b = 1*ocr1a;
				//ocr1c = 0*ocr1a;
				//OCR1B = ocr1b;
				//OCR1C = ocr1c;
			//}
			//else if(adc5[0] > 340)
			//{
				//ocr1b = 0*ocr1a;
				//ocr1c = 1*ocr1a;
				//OCR1B = ocr1b;
				//OCR1C = ocr1c;
			//}
			//else if(adc6[0] > 350)
			//{
				//ocr1b = 0*ocr1a;
				//ocr1c = 1*ocr1a;
				//OCR1B = ocr1b;
				//OCR1C = ocr1c;
			//}
			//else
			//{
				//ocr1b = 0;
				//ocr1c = 0;
				//OCR1B = ocr1b;
				//OCR1C = ocr1c;
			//}
			
			clear(PORTB, 4);
			ocr1b = 0.25*ocr1a;
			set(PORTB, 5);
			ocr1c = 0.25*ocr1a;
			OCR1B = ocr1b;
			OCR1C = ocr1c;
			//*count++;
		}
	}
	return 0;
}


/**********************************************************************************
MOVE
***********************************************************************************/
int move(float* robot, float* goal)	// move to a certain place from the current location
{
	float error, dist[2], p_gain;
	int ocr1a, ocr1b, ocr1c;
	ocr1a = OCR1A;
	
	p_gain = 1;
	goal[2] = atan2(goal[1] - robot[1], goal[0] - robot[0]);
	error = robot[2] - goal[2];
	dist[0] = fabs(robot[0] - goal[0]);
	dist[1] = fabs(robot[1] - goal[1]);
	while(error > PI) error -= 2*PI;
	while(error < -PI) error += 2*PI;
	
	
	if(fabs(error) > DEADBAND && fabs(error) < PI/3)
	{
		if(error < 0)	// turn left
		{
			//ocr1b = 0.7*ocr1a;	// 0.5 for RUNNING BACK, 
			//ocr1c = (0.7 + error*p_gain)*ocr1a;	// 0.65 for RUNNING BACK,
			set(PORTB, 4);
			set(PORTB, 5);
			ocr1b = 0.45*ocr1a;
			ocr1c = 0.65*ocr1a;	// 0.6
			OCR1B = ocr1b;
			OCR1C = ocr1c;
		}
		else	// turn right
		{
			//ocr1b = (0.7 + error*p_gain)*ocr1a;	// 0.5 for RUNNING BACK,
			//ocr1c = 0.7*ocr1a;	// 0.65 for RUNNING BACK,
			set(PORTB, 4);
			set(PORTB, 5);
			ocr1b = 0.65*ocr1a;	// 0.65 for RUNNING BACK,
			ocr1c = 0.45*ocr1a;	// 0.5 for RUNNING BACK,
			OCR1B = ocr1b;
			OCR1C = ocr1c;
		}
	}
	//else if(dist[0] < 20 && dist[1] < 20)	// this is not good for a real match
	//{
		//OCR1B = 0;
		//OCR1C = 0;
		//return 0;
	//}
	else if(fabs(error) > PI/3)
	{
		if(error < 0)
		{
			set(PORTB, 4);
			set(PORTB, 5);
			ocr1b = 0.25*ocr1a;	// 0.3
			ocr1c = 0.5*ocr1a;
			OCR1B = ocr1b;
			OCR1C = ocr1c;
		}
		else
		{
			set(PORTB, 4);
			set(PORTB, 5);
			ocr1b = 0.5*ocr1a;	// 0.65 for RUNNING BACK,
			ocr1c = 0.25*ocr1a;	// 0.5 for RUNNING BACK,
			OCR1B = ocr1b;
			OCR1C = ocr1c;
		}
	}
	else
	{
		set(PORTB, 4);
		set(PORTB, 5);
		OCR1B = 0.75*OCR1A;	// 0.75 for RUNNING BACK
		OCR1C = 0.75*OCR1A;	// 0.75 for RUNNING BACK
	}
	return 1;
}



/**********************************************************************************
QUALIFYING
***********************************************************************************/
// This is only for the qualifying
int qualify_test(int* command, unsigned int* blobs, float* location, int* orientation_flag, int* west_flag, int* east_flag, float* calibration)
{
	if(*command == PLAY)
	{
		int temp;
		temp = localization(blobs, calibration, location);
		float goal[3];
		if(temp)
		{
			if(*orientation_flag == 0)
			{
				*orientation_flag = 1;
				if(location[0] >= 0) *east_flag = 1;
				else *west_flag = 1;
			}
		}
		
		if(*west_flag)
		{
			goal[0] = 115;
			goal[1] = 0;
		}
		else if(*east_flag)
		{
			goal[0] = -115;
			goal[1] = 0;
		}
		
		if(*orientation_flag)
		{
			int temp;
			temp = move(location, goal);
			if(!temp)
			{
				*command = PAUSE;
				*orientation_flag = 0;
				*west_flag = 0;
				*east_flag = 0;
			}
			
		}
	}
	else if(*command == GOALA)
	{
		*west_flag = 0;
		*east_flag = 1;
		*orientation_flag = 1;
		*command = PLAY;
	}
	else if(*command == GOALB)
	{
		*west_flag = 1;
		*east_flag = 0;
		*orientation_flag = 1;
		*command = PLAY;
	}
	else if(*command == HALFTIME)
	{
		OCR1B = 0;
		OCR1C = 0;
	}
	else if(*command == GAMEOVER)
	{
		OCR1B = 0;
		OCR1C = 0;
	}
	else if(*command == PAUSE)
	{
		OCR1B = 0;
		OCR1C = 0;
	}	
}

/**********************************************************************************
SHOOTING
***********************************************************************************/
//int shoot()
//{
	//// 10cm
	//if(!check(PIND, 7))
	//{
		//set(PORTD, 4);
		//m_red(ON);
		//m_wait(1600);	// 1600/16ms = 0.1s
		//clear(PORTD, 4);
		//return 1;
	//}
	//return 0;	
//}
//
