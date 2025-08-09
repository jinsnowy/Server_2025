#pragma once

#include "Server/DataTable/DataTable.h"
#include "Server/DataTable/FileReader.h"
#include "Core/Json/Json.h"
#include "Core/System/Path.h"
#include "Core/Math/LinAlgebra.h"

namespace Server::DataTable
{
	struct SpawnerDataRecord {
		struct Item {
			int32_t spawnerId = 0;
			Math::Vec3 location;
			float radius;
			std::string spawnableActorAssetPath;
			int32_t initSpawnCount;
			int32_t maxSpawnCount;
			int32_t spawnDurationSeconds;

			void Read(const Json::JSection& section) {
				spawnerId = section.GetValue<int32_t>("spawnerId");
				location.x() = section.GetSection("location").GetValue<float>("x");
				location.y() = section.GetSection("location").GetValue<float>("y");
				location.z() = section.GetSection("location").GetValue<float>("z");
				radius = section.GetValue<float>("radius");
				spawnableActorAssetPath = section.GetValue<std::string>("spawnableActorAssetPath");
				initSpawnCount = section.GetValue<int32_t>("initSpawnCount");
				maxSpawnCount = section.GetValue<int32_t>("maxSpawnCount");
			}
		};

		int32_t map_uid;
		std::unordered_map<int32_t, Item> items;
	};

	struct SpawnerDataRecordTable : public DataTable<int32_t, SpawnerDataRecord> {
		SpawnerDataRecordTable() = default;
		~SpawnerDataRecordTable() = default;

		static void Load(const std::string& path) {
			try
			{
				FileReader file_reader(System::Path::Join(System::Path::GetWokringDirectory(), path));
				Json::JDocument json_doc = Json::JDocument::Parse(file_reader.stream());

				auto record = std::make_unique<SpawnerDataRecord>();
				record->map_uid = json_doc.GetValue<int32_t>("mapUid");
				const auto& spawnItems = json_doc.GetSection("spawnAreaActors");
				for (const auto& item : spawnItems) {
					SpawnerDataRecord::Item spawner_item;
					spawner_item.Read(item);
					record->items.emplace(spawner_item.spawnerId, spawner_item);
				}

				const int32_t map_uid = record->map_uid;
				SpawnerDataRecordTable::GetInstance().Add(map_uid, std::move(record));
			}
			catch (const std::exception& e)
			{
				LOG_ERROR("Failed to load SpawnerDataRecord from {}: {}", path, e.what());
				throw e;
			}
		}

		static void Clear() {
			DataTable<int32_t, SpawnerDataRecord>::GetInstance().Clear();
		}

		static size_t Count() {
			return DataTable<int32_t, SpawnerDataRecord>::GetInstance().Count();
		}
	};


}


