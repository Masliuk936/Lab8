#pragma once
#include <string>
#include "IBuildingRepository.h"
#include "Database.h"

class SQLiteBuildingRepository : public IBuildingRepository
{
private:
    using StorageType = decltype(createStorage(""));
    StorageType storage;

public:
    explicit SQLiteBuildingRepository(const std::string& dbPath);

    int addBuilding(const Building& building) override;
    std::optional<Building> getBuildingById(int id) override;
    std::vector<Building> getAllBuildings() override;
    void updateBuilding(const Building& building) override;
    void deleteBuilding(int id) override;

    int addMeterReading(const MeterReading& reading) override;
    std::vector<MeterReading> getReadingsByBuildingId(int buildingId) override;

    std::vector<Building> findBuildingsByDistrict(const std::string& district) override;
    std::vector<MeterReading> getReadingsSortedByValue(int buildingId) override;
    double getTotalConsumptionByType(int buildingId, const std::string& meterType) override;

    int addBuildingWithReadings(const Building& building, const std::vector<MeterReading>& readings) override;
};