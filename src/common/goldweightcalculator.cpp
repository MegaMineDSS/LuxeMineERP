#include "goldweightcalculator.h"

GoldWeightCalculator::GoldWeightCalculator() {}

double getFinalPurityPercent(int kt)
{
    switch (kt)
    {
    case 2:  return 47.66;
    case 6:  return 52.34;
    case 9:  return 57.66;
    case 10: return 59.63;
    case 12: return 63.46;
    case 14: return 67.10;
    case 18: return 80.47;
    case 20: return 86.92;
    case 22: return 93.46;
    case 24: return 100.00;
    default: return 0.0;
    }
}
