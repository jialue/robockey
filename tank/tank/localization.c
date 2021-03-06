/*
 * localize.c
 *
 * Created: 11/14/2012 2:23:30 PM
 *  Author: jialue
 */ 


#include <math.h>

#define PI 3.1416
/**********************************************************************************
The following functions return 0 if it fails or no result and return 1 if succeed.
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

int cal_norm(int* x, int* y, int result[][3])//*************************************************
{
	int combos[6][2] = {{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}};
	int vect[6][2];	// vect[][0] ~ vect[][5] are x_from, y_from, x_to, y_to, point_from, to_from
	int result_flag= 0;
	for(int i = 0; i < 6; i++)
	{
		int from, to;
		// 2 points 'from' and 'to'
		from = combos[i][0];
		to = combos[i][1];
		
		// vectors
		vect[i][0] = x[to] - x[from];	// vect = {AB_(x,y), A, B}
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
	result[0] = 511.5;
	result[1] = 14.5 * scale + 383.5;
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
	int temp[4], temp1[2], temp2[2];
	int max_index, min_index;
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
	
	int intersect[2];
	int diff[4];
	
	
	if(set_intersection(temp1, temp2, 2, 2, intersect))	// check if there is a common point
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

int localization(unsigned int* rawData, float* result)
{
	int A[2], B[2], C[2], D[2];
	int x[4], y[4];
	int CA[2], stars[4];
	float theta, offset[2], center[2] = {511.5, 383.5};	// the world center in pixel
	float world[2];
	int vect_norm[6][3];
	
	x[0] = rawData[0];
	x[1] = rawData[3];
	x[2] = rawData[6];
	x[3] = rawData[9];
	y[0] = rawData[1];
	y[1] = rawData[4];
	y[2] = rawData[7];
	y[3] = rawData[10];
	

	// check 1023 before ID the stars******************************************
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
		
		world_frame(vect_norm, world);
		CA[0] = A[0] - C[0];
		CA[1] = A[1] - C[1];
		theta = PI/2 - atan2(CA[1], CA[0]);
		offset[0] = world[0] - A[0] * cos(theta) + A[1] * sin(theta);
		offset[1] = world[1] - A[0] * sin(theta) - A[1] * cos(theta);
		
		result[0] = cos(theta) * center[0] - sin(theta) * center[1] + offset[0];
		result[1] = sin(theta) * center[0] + cos(theta) * center[1] + offset[1];
		result[0] = 511 + 511 - result[0];
		result[2] = PI - theta;	// angle from X_camera to X_world in world frame.
		
		return 1;
	}
	return 0;
}