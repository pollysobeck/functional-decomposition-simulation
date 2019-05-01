//Author: Polly Sobeck
//Program: CS475 Project-3
//Date: 4/30/2019
//Description: This program simulates the growth of grain,
//number of deer, and number of tigers over the course of 
//6 years with varied levels of precipitation and temperature.

// Base Structure and Randf helper methods were 
// extracted from OSU CS475 Project-3

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <stdio.h>
#include <cmath>

//State of the system
int	NowYear;		// 2019 - 2024
int	NowMonth;		// 0 - 11

float NowPrecip;		// inches of rain per month
float NowTemp;		// temperature this month
float NowHeight;		// grain height in inches
int NowNumDeer;		// number of deer in the current population
int NowNumTiger;

//Constant Parameters
const float GRAIN_GROWS_PER_MONTH =		8.0;
const float ONE_DEER_EATS_PER_MONTH =		0.5;

const float AVG_PRECIP_PER_MONTH =		6.0;	// average
const float AMP_PRECIP_PER_MONTH =		6.0;	// plus or minus
const float RANDOM_PRECIP =			3.0;	// plus or minus noise

const float AVG_TEMP =				50.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise
const float RANDOM_DEER =           		3.0;

const float MIDTEMP =				40.0;
const float MIDPRECIP =				10.0;

const float TIGER_PRECIP_LIMIT = 		10.0;

//Function Prototypes
float Ranf( unsigned int*, float, float);
int Ranf( unsigned int*, int, int);
void GrainDeer(unsigned int);
void Grain();
void Watcher(unsigned int);
void Tiger();
float SQR(float);
void CalculateTempAndPrecip(unsigned int);

int main( int argc, char *argv[ ] )
{
    unsigned int seed = 0;
    float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

    float temp = AVG_TEMP - AMP_TEMP * cos( ang );
    NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
    NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
    if( NowPrecip < 0. )
        NowPrecip = 0.;


    printf("NowYear,NowMonth,NowPrecip,NowTemp,NowHeight,NowNumDeer,NowNumTiger\n");

    // starting date and time:
    NowMonth =    0;
    NowYear  = 2019;

    // starting state (feel free to change this if you want):
    NowNumDeer = 2;
    NowHeight =  1.;
    NowNumTiger = 1;

    omp_set_num_threads( 4 );	// same as # of sections
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            GrainDeer(seed);
        }

        #pragma omp section
        {
            Grain( );
        }

        #pragma omp section
        {
            Watcher(seed);
        }

        #pragma omp section
        {
            Tiger( );	// your own
        }
    }  
}

void GrainDeer(unsigned int seed) {
    while( NowYear < 2025 )
    {
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:
	int nextNumDeer = ceil(NowHeight / 0.5) + Ranf( &seed, -RANDOM_DEER, RANDOM_DEER );

        // DoneComputing barrier:
        #pragma omp barrier
        if (nextNumDeer > 0) {
            NowNumDeer = nextNumDeer;
        } else {
            NowNumDeer = 0;
        }

        // DoneAssigning barrier:
        #pragma omp barrier

        // DonePrinting barrier:
        #pragma omp barrier
    }
}

void Grain() {
    while( NowYear < 2025 )
    {
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:
        float tempFactor = exp(   -SQR(  ( NowTemp - MIDTEMP ) / 10.  )   );
        float precipFactor = exp(   -SQR(  ( NowPrecip - MIDPRECIP ) / 10.  )   );

        float heightInc = (tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH);
        float heightDec = ((float)NowNumDeer * ONE_DEER_EATS_PER_MONTH);

        // DoneComputing barrier:
        #pragma omp barrier
        NowHeight = NowHeight + heightInc - heightDec;

        if (NowHeight < 0) {
            NowHeight = 0;
        }

        // DoneAssigning barrier:
        #pragma omp barrier

        // DonePrinting barrier:
        #pragma omp barrier
    }
}

void Watcher(unsigned int seed) {
    while( NowYear < 2025 )
    {
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:

        // DoneComputing barrier:
        #pragma omp barrier

        // DoneAssigning barrier:
        #pragma omp barrier
        printf("%d,%d,%.2lf,%.2lf,%.2lf,%d,%d\n", NowYear, NowMonth, NowPrecip, NowTemp, NowHeight, NowNumDeer, NowNumTiger);

        NowMonth++;
        if (NowMonth == 12) {
            NowMonth = 0;
            NowYear++;
        } 
        //CalculateTempAndPrecip(seed);
	float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );
	
	float temp = AVG_TEMP - AMP_TEMP * cos( ang );
	NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

	float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
	NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
	if( NowPrecip < 0. )
        	NowPrecip = 0.;

        // DonePrinting barrier:
        #pragma omp barrier
    }
}

void Tiger() {
    while( NowYear < 2025 )
    {
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:
	int nextTiger = 0;
	int nextDeer = 0;	

	// Tigers will eat two deer and have two babies per month
	// if the precipitation levels are below the tiger limit
	// and if there are more deer than tigers
	if (NowPrecip < TIGER_PRECIP_LIMIT && NowNumDeer > NowNumTiger) {
		nextTiger = NowNumTiger + 2;
		nextDeer = NowNumDeer - 2;

	// Otherwise if there are more tigers than deer, they will
	// not have enough to eat and tiger population will decrease 
	// by one tiger
	} else if (NowNumDeer < NowNumTiger) {
		nextTiger = NowNumTiger - 1;
	}

        // DoneComputing barrier:
        #pragma omp barrier
	if (nextDeer > 0) {
		NowNumDeer = nextDeer;
	} else {
		NowNumDeer = 0;
	}

	if (nextTiger > 0) {
		NowNumTiger = nextTiger;
	} else {
		NowNumTiger = 0;
	}

        // DoneAssigning barrier:
        #pragma omp barrier

        // DonePrinting barrier:
        #pragma omp barrier
    }
}

float Ranf( unsigned int *seedp,  float low, float high )
{
        float r = (float) rand_r( seedp );              // 0 - RAND_MAX
        return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}


int Ranf( unsigned int *seedp, int ilow, int ihigh )
{
        float low = (float)ilow;
        float high = (float)ihigh + 0.9999f;

        return (int)(  Ranf(seedp, low,high) );
}

float SQR( float x )
{
        return x*x;
}

