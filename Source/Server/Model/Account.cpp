#include "stdafx.h"
#include "Account.h"
#include "Core/Sql/Stmt.h"

namespace Server::Model {

	bool Account::UpsertToDb(Sql::Agent& agent) {
		auto stmt = agent.CreateStmt();
		stmt.BindInParam(Sql::WCharArray(128, user_id_.c_str()));
		stmt.BindInParam(Sql::WCharArray(128, username_.c_str()));
		stmt.BindInParam(last_login_time_);
		stmt.BindOutParam(&account_id_);
		if (stmt.Execute(L"usp_UpsertAccount") == false) {
			LOG_ERROR("Failed to upsert account: user_id: {}, username: {}, last_login_time: {}",
				user_id_, username_, last_login_time_.ToString());
			return false;
		}
		return true;
	}

	std::string Account::ToString() const {
		return FORMAT("Account(user_id: {}, username: {}, account_id: {}, last_login_time: {})",
			user_id_, username_, account_id_, last_login_time_.ToString());
	}
}

