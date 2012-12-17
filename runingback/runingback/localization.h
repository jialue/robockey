/*
 * IncFile1.h
 *
 * Created: 11/16/2012 10:41:30 PM
 *  Author: jialue
 */ 


#ifndef localization__
#define localization__


#define PI 3.1416

int set_intersection(int* arr1, int* arr2, int m, int n, int* result);
int set_difference(int* arr1, int* arr2, int m, int n, int* result);
int cal_norm(int* x, int* y, int result[][3]);
int world_frame(int vect_norm[][3], float* result);
int id_stars(int vect_norm[][3], int* result);
int localization(unsigned int* rawData, float* result);

#endif /* INCFILE1_H_ */