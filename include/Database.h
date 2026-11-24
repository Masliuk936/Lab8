#pragma once
#include <string>
#include <sqlite_orm/sqlite_orm.h>
#include "Building.h"
#include "MeterReading.h"

inline auto createStorage(const std::string& filename)
{
    using namespace sqlite_orm;

    return make_storage(
        filename,
        make_table(
            "Building",
            make_column(
                "id",
                &Building::id,
                primary_key().autoincrement()
            ),
            make_column(
                "address",
                &Building::address,
                not_null()
            ),
            make_column(
                "district",
                &Building::district,
                not_null()
            ),
            make_column(
                "buildingType",
                &Building::buildingType,
                not_null()
            )
        ),
        make_table(
            "MeterReading",
            make_column(
                "id",
                &MeterReading::id,
                primary_key().autoincrement()
            ),
            make_column(
                "buildingId",
                &MeterReading::buildingId
            ),
            make_column(
                "readingDate",
                &MeterReading::readingDate,
                not_null()
            ),
            make_column(
                "meterType",
                &MeterReading::meterType,
                not_null()
            ),
            make_column(
                "consumedValue",
                &MeterReading::consumedValue
            ),
            foreign_key(
                &MeterReading::buildingId
            ).references(
                &Building::id
            )
        )
    );
}