#include "stdafx.h"
#include "Core/Sql/Database.h"
#include "Core/Sql/Pool.h"
#include "Core/Sql/Agent.h"

namespace Sql  {
 
    Database::Database(std::string db_name)
        :
        _db_name(db_name),
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

    void Database::Destroy() {
        _conn_pool->Clear();
        _conn_pool = nullptr;
    }

    Agent Database::GetAgent() {
        return Agent(GetPool().Dequeue());
    }
}
