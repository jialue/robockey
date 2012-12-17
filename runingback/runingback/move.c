/*
 * CFile1.c
 *
 * Created: 11/20/2012 7:47:35 PM
 *  Author: jialue
 */ 
int move(float* robot, float* goal)
{
	float error;
	int ocr1a, ocr1b, ocr1c;
	ocr1a = OCR1A;
	error = robot[2] - goal[2];
	if(error > DEADBAND)
	{
		if(error > 0)	// turn left
		{
			ocr1b = 0.4*ocr1a;
			ocr1c = 0.6*ocr1a;
			OCR1B = ocr1b;
			OCR1C = ocr1c;
		}
		else	// turn right
		{
			ocr1b = 0.6*ocr1a;
			ocr1c = 0.4*ocr1a;
			OCR1B = ocr1b;
			OCR1C = ocr1c;
		}
		return 1;
	}
	else
	{
		OCR1B = OCR1A;
		OCR1C = OCR1A;
		return 0;
	}
	return 0;
}