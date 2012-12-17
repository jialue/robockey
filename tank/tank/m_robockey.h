/*
 * robockeyTest.c
 *
 * Created: 11/13/2012 11:32:58 AM
 *  Author: jialue
 */ 


#ifndef m_robockey__
#define m_robockey__

#include "m_general.h"
#include "m_usb.h"
#include "m_bus.h"
#include "m_rf.h"
#include "m_imu.h"
#include "m_wii.h"

// Define constants
#define CHANNEL	1
#define RXADDRESS	0x11
#define PACKET_LENGTH_RF	12
#define PACKET_LENGTH_USB	15
#define commandTest 0xA0
#define PLAY 0xA1
#define GOALA 0xA2
#define GOALB 0xA3
#define PAUSE 0xA4
#define HALFTIME 0xA6
#define GAMEOVER 0xA7
#define DEADBAND 0.08
#define DEADBAND2 100 // 
#define PI 3.1416

// Declare global variables



// Declare user-defined subroutines
void robockey_init(void);
int set_intersection(int* arr1, int* arr2, int m, int n, int* result);
int set_difference(int* arr1, int* arr2, int m, int n, int* result);
int cal_norm(int* x, int* y, int result[][3]);
int world_frame(int vect_norm[][3], float* result);
int id_stars(int vect_norm[][3], int* result);
int localization(unsigned int* rawData, float* calibration, float* result);
int move(float* robot, float* goal);
int qualify_test(int* command, unsigned int* blobs, float* location, int* orientation_flag, int* west_flag, int* east_flag, float* calibration);
int find(int* adc1, int* adc2, int* count);
#endif