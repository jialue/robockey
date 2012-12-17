/*
 * robockey.c
 *
 * Created: 11/13/2012 11:10:14 AM
 *  Author: jialue
 */ 


#include <avr/io.h>
#include "m_robockey.h"

volatile unsigned char commandFlag = 0;
unsigned char buffer_R[PACKET_LENGTH_RF] = {0,0,0,0,0,0,0,0,0,0,0};

volatile int command = 0;
volatile int pauseFlag = 0;

unsigned int blobs[12] = {492,330,10,473,368,8,567,367,10,513,402,5};
int wii_flag = 0;
float location[3] = {0, 0, 0}, calibration[2] = {0, 0};
int orientation_flag = 0, west_flag = 0, east_flag = 0;
int adc_channel = 0;
int adc1[2], adc2[2];
int count = 0;
int found, stall_count;
int temp;
int mode;
float goal[3];
int switch_flag = 1;
float last_location[3];
int location_flag;
int main(void)
{
	robockey_init();
    while(1)
    {
	    comm(command);
		mode = check_mode();
		if(wii_flag && command == PLAY) 
		{
			location_flag = localization(blobs, calibration, location);
			wii_flag = 0;
		}			
		switch(mode)
		{
			case 0:	// normal mode(GOAL A), need command PLAY TO START			
			if(switch_flag)
			{
				goal[0] = -125;
				goal[1] = 0;
				goal[2] = 0;
				if(command == HALFTIME)
				{
					goal[0] = 125;
					switch_flag = 0;
				}
			}
			if(command == PLAY)
			{
				//if(!found) 
				//{
					//found = find(adc1, adc2, &count);
					//set(TIMSK0,OCIE0A);	// Enable timer0 interrupt
				//}				
				if(find(adc1, adc2, &count))
				{
					m_green(TOGGLE);
					m_red(OFF);
					move(location, goal);
				}
			}
			break;
			
			case 1:	// normal mode(GOAL B), need command PLAY TO START
			if(switch_flag)
			{
				goal[0] = 125;
				goal[1] = 0;
				goal[2] = 0;
				if(command == HALFTIME)
				{
					goal[0] = -125;
					switch_flag = 0;
				}
			}
			if(command == PLAY)
			{
				if(find(adc1, adc2, &count))
				{
					m_green(TOGGLE);
					m_red(OFF);
					move(location, goal);
				}
			}
			break;
			
			case 2:	// calibrate the center of the rink
			m_green(ON);
			m_red(ON);
			if(wii_flag)
			{
				float calibrationDefault[2] = {0, 0};
				float calibrationResult[3] = {0, 0, 0};
				if(localization(blobs, calibrationDefault, calibrationResult))
				{
					//m_green(ON);
					//m_red(ON);
					calibration[0] = calibrationResult[0];
					calibration[1] = calibrationResult[1];
				}
			}
			break;
			
			case 3:	// test mode(GOAL A), always play while the power is on.
			m_green(TOGGLE);
			m_red(OFF);
			if(find(adc1, adc2, &count))
			{
				if(wii_flag)
				{
					//if(!found)
					//{
						//found = find(adc1, adc2, &count);
						//set(TIMSK0,OCIE0A);	// Enable interrupt
					//}
					if(localization(blobs, calibration, location))
					{
					float goal[3] = {-115, 0, 0};
					move(location, goal);
					}
				}
			}
			break;
			
			case 4:	// test mode(GOAL B), always play while the power is on.
			m_red(TOGGLE);
			m_green(OFF);
			if(find(adc1, adc2, &count))
			{
				if(wii_flag)
				{
					if(localization(blobs, calibration, location))
					{
					float goal[3] = {115, 0, 0};
					move(location, goal);
					}
				}
			}
			break;
			
			case 5:
			m_green(ON);
			m_red(ON);
			OCR1B = OCR1A;
			OCR1C = 0.98*OCR1A;
			break;
			
			case 6:
			qualify_test(&command, blobs, location, &orientation_flag, &west_flag, &east_flag, calibration);
			break;
			
			case 7:
			OCR1B = OCR1A;
			OCR1C = 0;
			m_wait(3200);
			OCR1B = OCR1A/2;
			OCR1C = OCR1A/2;
			m_wait(32000);
			break;
			
			default:
			OCR1B = 0.5*OCR1A;
			OCR1C = 0.5*OCR1A;
			m_green(OFF);
			m_red(OFF);
		}
		m_usb_tx_int((int)(location[0]));
		m_usb_tx_char('\t');
		m_usb_tx_int((int)(location[1]));
		m_usb_tx_char('\t');
		m_usb_tx_int((int)(adc1[0]));
		m_usb_tx_char('\t');
		m_usb_tx_int((int)(adc2[0]));
		m_usb_tx_char('\t');
		m_usb_tx_int((int)(check(PIND, 7)));
		m_usb_tx_char('\t');
		m_usb_tx_char('\r');
		if(!check(ADCSRA, ADIF)) set(ADCSRA,ADSC);
    }
}

ISR(TIMER3_COMPA_vect)
{
	//m_green(TOGGLE);
	wii_flag = m_wii_read(blobs);
	//wii_flag = 1;
	// return the location to terminal
	if(location_flag)
	{
		if(fabs(last_location[0] - location[0]) < 3 && fabs(last_location[1] - location[1]) < 3 && adc1[0] < 1000 && adc2[0] < 1000 && command == PLAY)    // count when robot get stall
		{
			m_red(TOGGLE);
			stall_count++;
		}
		else stall_count = 0;
		if(stall_count >= 100)    // rotate after getting stall for several sec
		{
			OCR1B = 0;
			OCR1C = 0;
			m_red(ON);
			m_wait(5000);
			m_red(OFF);
			OCR1B = OCR1A/2;
			OCR1C = OCR1A/2;
			stall_count = 0;
			location_flag = 0;
		}
		last_location[0] = location[0];
		last_location[1] = location[1];
		last_location[2] = location[2];
	}
}

//ISR(TIMER0_COMPA_vect)
//{
//
//}

ISR(INT2_vect)
{
	// Receive wireless data
	if (m_rf_read(buffer_R, PACKET_LENGTH_RF))
	{
		command = buffer_R[0];
		commandFlag = 1;
	}
}

ISR(ADC_vect)
{
	if(adc_channel == 1)
	{
		clear(ADMUX, MUX0);	// F0
		set(ADCSRA,ADSC);	// Start conversion
	}
	else if(adc_channel == 2)
	{
		adc1[1] = adc1[0];
		adc1[0] = ADC;
		set(ADMUX, MUX0);	// F1
		set(ADCSRA,ADSC);	// Start conversion
	}
	else
	{
		adc2[1] = adc2[0];
		adc2[0] = ADC;
		adc_channel = 0;
		set(ADCSRA,ADSC);	// Start conversion
	}
	adc_channel++;
}