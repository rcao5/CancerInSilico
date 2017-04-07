#include <Rcpp.h>
#include <cmath>

#include "CellBasedModel.h"

void CellBasedModel::run()
{
	double time = 0.0;
    double recordTime = 0.0, outputTime = 0.0;

    while (time <= mParams->runTime())
    {
        Rcpp::checkUserInterrupt();

        if (time >= recordTime)
        {
            recordPopulation();
            recordTime += mParams->recordIncrement();
        }

        if (time >= outputTime)
        {
            Rprintf("time = %.2f\n", ceil(time));
            Rprintf("size = %d\n", size());

            outputTime += mParams->outputIncrement();
        }            

        oneTimeStep(time);
		time += mParams->timeIncrement();
    }

    Rprintf("final time = %.2f\n", ceil(time));
    Rprintf("final size = %d\n", size());
}