#pragma once
#include <string>

struct MeterReading
{
    int id = 0;
    int buildingId = 0;
    std::string readingDate;
    std::string meterType;
    double consumedValue = 0.0;
};