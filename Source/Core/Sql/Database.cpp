#include "stdafx.h"
#include "Core/Sql/Database.h"
#include "Core/Sql/Pool.h"
#include "Core/Sql/Agent.h"

namespace Sql  {
    class Database final : public System::Singleton<Database> {
    public:
        Database();

        ~Database();

        bool Initialize(const std::wstring& db_dsn, uint32_t db_pool_size);

        Pool& GetPool() {
			return *_conn_pool;
		}

    private:
        std::unique_ptr<Pool> _conn_pool;
    };

    Database::Database()
        :
        _conn_pool(std::make_unique<Pool>())
    {
    }

    Database::~Database() {
    }

    bool Database::Initialize(const std::wstring& db_dsn, uint32_t db_pool_size) {
        LOG_INFO(L"[DATABASE] db_pool_size: {}, dsn: {}", db_pool_size, db_dsn);

        if (_conn_pool->Connect(db_dsn.c_str(), db_pool_size) == false) {
            return false;
        }

        return true;
    }


    void Initialize(const std::wstring& connectionString, uint32_t connectionCount) {
        if (Database::GetInstance().Initialize(connectionString, connectionCount) == false) {
			throw std::runtime_error("Failed to initialize database");
		}
    }

    void Destroy() {
        GetPool().Clear();
    }

    Pool& GetPool() {
        return Database::GetInstance().GetPool();
    }

    Agent GetAgent() {
        return Agent(Database::GetInstance().GetPool().Dequeue());
    }


}
