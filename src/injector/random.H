
#ifndef random_H
#define random_H 

#include <math.h>
#include <iostream>
#include <list>
#include <string>
#include <random>
#include "injector.H"
namespace Foam
{
    void randomDistHandler(std::string distributionFunctionName, scalar* parameter, const scalar& mean, const scalar& variance_, const int& numObject);
    void guassianDistribution(scalar* parameter, const scalar& mean, const scalar& variance_, const int& numObject);
    void exponentialDistribution(scalar* parameter, const scalar& mean, const scalar& variance_, const int& numObject);
}   
#endif
