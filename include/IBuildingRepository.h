#pragma once
#include <optional>
#include <string>
#include <vector>
#include "Building.h"
#include "MeterReading.h"

class IBuildingRepository
{
public:
    virtual ~IBuildingRepository() = default;

    virtual int addBuilding(const Building& building) = 0;
    virtual std::optional<Building> getBuildingById(int id) = 0;
    virtual std::vector<Building> getAllBuildings() = 0;
    virtual void updateBuilding(const Building& building) = 0;
    virtual void deleteBuilding(int id) = 0;

    virtual int addMeterReading(const MeterReading& reading) = 0;
    virtual std::vector<MeterReading> getReadingsByBuildingId(int buildingId) = 0;

    virtual std::vector<Building> findBuildingsByDistrict(const std::string& district) = 0;
    virtual std::vector<MeterReading> getReadingsSortedByValue(int buildingId) = 0;
    virtual double getTotalConsumptionByType(int buildingId, const std::string& meterType) = 0;

    virtual int addBuildingWithReadings(const Building& building, const std::vector<MeterReading>& readings) = 0;
};