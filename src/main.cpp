#include <iostream>
#include <cstdio>
#include <windows.h>
#include <sqlite3.h>
#include "Building.h"
#include "MeterReading.h"
#include "Database.h"
#include "SQLiteBuildingRepository.h"

//SQLite C API
void runRawSQLiteDemo(const char* dbFile)
{
    std::cout << "=== 1. SQLITE C API ===\n";

    sqlite3* db = nullptr;
    sqlite3_open(dbFile, &db);

    const char* createSql =
        "CREATE TABLE IF NOT EXISTS Building ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "address TEXT NOT NULL, "
        "district TEXT NOT NULL, "
        "buildingType TEXT NOT NULL);";
    sqlite3_exec(db, createSql, nullptr, nullptr, nullptr);

    const char* insertSql = "INSERT INTO Building (address, district, buildingType) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, "вул. Шевченка, 12", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, "Центральний", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, "Житловий", -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    int insertedId = static_cast<int>(sqlite3_last_insert_rowid(db));
    std::cout << "Inserted ID (Raw API): " << insertedId << "\n";

    const char* selectSql = "SELECT id, address, district, buildingType FROM Building WHERE id = ?;";
    sqlite3_prepare_v2(db, selectSql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, insertedId);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::cout << "Found: " << sqlite3_column_int(stmt, 0) << " | "
            << sqlite3_column_text(stmt, 1) << " | "
            << sqlite3_column_text(stmt, 2) << " | "
            << sqlite3_column_text(stmt, 3) << "\n";
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// sqlite_orm
void runOrmDemo(const char* dbFile)
{
    using namespace sqlite_orm;
    std::cout << "\n=== 2. SQLITE_ORM & 1:N ===\n";

    auto storage = createStorage(dbFile);
    storage.on_open = [](sqlite3* db) {
        sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
        };
    storage.open_forever();
    storage.sync_schema();

    Building b{ 0, "пр. Перемоги, 45", "Північний", "Комерційний" };
    int buildingId = storage.insert(b);
    std::cout << "Inserted ID (ORM): " << buildingId << "\n";

    auto found = storage.get<Building>(buildingId);
    std::cout << "Found: " << found.id << " | " << found.address << "\n";

    storage.insert(MeterReading{ 0, buildingId, "2026-10-01", "Електроенергія", 340.5 });
    storage.insert(MeterReading{ 0, buildingId, "2026-10-15", "Вода", 18.2 });

    auto readings = storage.get_all<MeterReading>(where(c(&MeterReading::buildingId) == buildingId));
    std::cout << "Readings for building " << buildingId << ":\n";
    for (const auto& r : readings)
    {
        std::cout << "  " << r.readingDate << " | " << r.meterType << " | " << r.consumedValue << "\n";
    }

    try
    {
        storage.insert(MeterReading{ 0, -1, "2026-10-20", "Газ", 50.0 });
    }
    catch (const std::exception& e)
    {
        std::cout << "Foreign key error (expected): " << e.what() << "\n";
    }
}

//патерн Repository та CRUD-операції
void runRepositoryDemo(const std::string& dbFile)
{
    std::cout << "\n=== 3. REPOSITORY & CRUD ===\n";

    SQLiteBuildingRepository repo(dbFile);

    int id1 = repo.addBuilding({ 0, "вул. Шевченка, 12", "Центральний", "Житловий" });
    int id2 = repo.addBuilding({ 0, "пр. Перемоги, 45", "Північний", "Комерційний" });

    repo.addMeterReading({ 0, id1, "2026-10-01", "Електроенергія", 340.5 });
    repo.addMeterReading({ 0, id1, "2026-10-15", "Вода", 18.2 });

    std::cout << "All buildings:\n";
    for (const auto& b : repo.getAllBuildings())
    {
        std::cout << "  " << b.id << " | " << b.address << " | " << b.district << " | " << b.buildingType << "\n";
    }

    std::cout << "Readings for building " << id1 << ":\n";
    for (const auto& r : repo.getReadingsByBuildingId(id1))
    {
        std::cout << "  " << r.id << " | " << r.readingDate << " | " << r.meterType << " | " << r.consumedValue << "\n";
    }

    auto building = repo.getBuildingById(id1);
    if (building)
    {
        building->address = "вул. Шевченка, 12-А";
        building->buildingType = "ОСББ";
        repo.updateBuilding(*building);
    }

    auto updated = repo.getBuildingById(id1);
    if (updated)
    {
        std::cout << "Updated building " << id1 << ": " << updated->address << " (" << updated->buildingType << ")\n";
    }

    repo.deleteBuilding(id2);
    std::cout << "Buildings count after delete: " << repo.getAllBuildings().size() << "\n";
}

// Фільтрація, сортування та підрахунок споживання
void runQueriesDemo(const std::string& dbFile)
{
    std::cout << "\n=== 4. FILTERING, SORTING & AGGREGATION ===\n";

    SQLiteBuildingRepository repo(dbFile);

    int b1 = repo.addBuilding({ 0, "вул. Соборна, 10", "Центральний", "Житловий" });
    int b2 = repo.addBuilding({ 0, "вул. Садова, 5", "Центральний", "Офісний" });
    int b3 = repo.addBuilding({ 0, "вул. Лісова, 22", "Південний", "Житловий" });

    repo.addMeterReading({ 0, b1, "2026-10-01", "Електроенергія", 210.5 });
    repo.addMeterReading({ 0, b1, "2026-11-01", "Електроенергія", 245.0 });
    repo.addMeterReading({ 0, b1, "2026-11-01", "Вода", 14.3 });
    repo.addMeterReading({ 0, b1, "2026-11-01", "Газ", 85.0 });

    std::cout << "Buildings in 'Центральний' district:\n";
    for (const auto& b : repo.findBuildingsByDistrict("Центральний"))
    {
        std::cout << "  " << b.id << " | " << b.address << " (" << b.buildingType << ")\n";
    }

    std::cout << "Readings for building " << b1 << "\n";
    for (const auto& r : repo.getReadingsSortedByValue(b1))
    {
        std::cout << "  " << r.meterType << " | " << r.consumedValue << " (" << r.readingDate << ")\n";
    }

    double totalElectro = repo.getTotalConsumptionByType(b1, "Електроенергія");
    std::cout << "Total 'Електроенергія' for building " << b1 << ": " << totalElectro << "\n";
}

// Транзакції та обробка винятків
void runTransactionsDemo(const std::string& dbFile)
{
    std::cout << "\n=== 5. TRANSACTIONS & ERROR HANDLING ===\n";

    SQLiteBuildingRepository repo(dbFile);

    Building b1{ 0, "вул. Незалежності, 8", "Західний", "Житловий" };
    std::vector<MeterReading> validReadings = {
        { 0, 0, "2026-11-01", "Електроенергія", 190.0 },
        { 0, 0, "2026-11-01", "Вода", 12.5 }
    };

    int id = repo.addBuildingWithReadings(b1, validReadings);
    std::cout << "Transaction COMMIT success. Added building ID: " << id << "\n";
    std::cout << "Buildings count: " << repo.getAllBuildings().size() << "\n";

    Building b2{ 0, "вул. Помилкова, 99", "Східний", "Комерційний" };
    std::vector<MeterReading> invalidReadings = {
        { 0, 0, "2026-11-01", "Електроенергія", 100.0 },
        { 0, 0, "2026-11-01", "Газ", -50.0 }
    };

    try
    {
        repo.addBuildingWithReadings(b2, invalidReadings);
    }
    catch (const std::exception& e)
    {
        std::cout << "Transaction ROLLBACK triggered: " << e.what() << "\n";
    }

    std::cout << "Buildings count after rollback: " << repo.getAllBuildings().size() << "\n";
}

int main()
{
    SetConsoleOutputCP(65001);

    const char* demoDb = "buildings_demo.db";
    const std::string repoDb = "buildings_repo.db";
    const std::string queryDb = "buildings_queries.db";
    const std::string txDb = "buildings_tx.db";

    std::remove(demoDb);
    std::remove(repoDb.c_str());
    std::remove(queryDb.c_str());
    std::remove(txDb.c_str());

    runRawSQLiteDemo(demoDb);
    runOrmDemo(demoDb);
    runRepositoryDemo(repoDb);
    runQueriesDemo(queryDb);
    runTransactionsDemo(txDb);

    return 0;
}