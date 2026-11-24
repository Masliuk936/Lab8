#include "SQLiteBuildingRepository.h"
#include <stdexcept>

SQLiteBuildingRepository::SQLiteBuildingRepository(const std::string& dbPath)
    : storage(createStorage(dbPath))
{
    storage.on_open = [](sqlite3* db) {
        sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
        };
    storage.open_forever();
    storage.sync_schema();
}

int SQLiteBuildingRepository::addBuilding(const Building& building)
{
    return storage.insert(building);
}

std::optional<Building> SQLiteBuildingRepository::getBuildingById(int id)
{
    return storage.get_optional<Building>(id);
}

std::vector<Building> SQLiteBuildingRepository::getAllBuildings()
{
    return storage.get_all<Building>();
}

void SQLiteBuildingRepository::updateBuilding(const Building& building)
{
    storage.update(building);
}

void SQLiteBuildingRepository::deleteBuilding(int id)
{
    using namespace sqlite_orm;
    storage.remove_all<MeterReading>(where(c(&MeterReading::buildingId) == id));
    storage.remove<Building>(id);
}

int SQLiteBuildingRepository::addMeterReading(const MeterReading& reading)
{
    return storage.insert(reading);
}

std::vector<MeterReading> SQLiteBuildingRepository::getReadingsByBuildingId(int buildingId)
{
    using namespace sqlite_orm;
    return storage.get_all<MeterReading>(where(c(&MeterReading::buildingId) == buildingId));
}

std::vector<Building> SQLiteBuildingRepository::findBuildingsByDistrict(const std::string& district)
{
    using namespace sqlite_orm;
    return storage.get_all<Building>(where(c(&Building::district) == district));
}

std::vector<MeterReading> SQLiteBuildingRepository::getReadingsSortedByValue(int buildingId)
{
    using namespace sqlite_orm;
    return storage.get_all<MeterReading>(
        where(c(&MeterReading::buildingId) == buildingId),
        order_by(&MeterReading::consumedValue).desc()
    );
}

double SQLiteBuildingRepository::getTotalConsumptionByType(int buildingId, const std::string& meterType)
{
    using namespace sqlite_orm;
    auto list = storage.get_all<MeterReading>(
        where(c(&MeterReading::buildingId) == buildingId and c(&MeterReading::meterType) == meterType)
    );

    double total = 0.0;
    for (const auto& item : list)
    {
        total += item.consumedValue;
    }
    return total;
}

int SQLiteBuildingRepository::addBuildingWithReadings(
    const Building& building,
    const std::vector<MeterReading>& readings)
{
    int createdBuildingId = 0;

    storage.transaction([&]() -> bool {
        createdBuildingId = storage.insert(building);

        for (auto r : readings)
        {
            if (r.consumedValue < 0)
            {
                throw std::invalid_argument("Показання лічильника не може бути від'ємним");
            }
            r.buildingId = createdBuildingId;
            storage.insert(r);
        }

        return true;
        });

    return createdBuildingId;
}