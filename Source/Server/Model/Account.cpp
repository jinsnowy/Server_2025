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

	bool Account::LoadFromDb(Sql::Agent& agent) {
		auto stmt = agent.CreateStmt();
		stmt.BindInParam(Sql::WCharArray(128, user_id_.c_str()));
		if (stmt.Execute(L"usp_LoadAccount") == false) {
			LOG_ERROR("Failed to load account: user_id: {}", user_id_);
			return false;
		}
		Sql::WCharArray w_username(128);
		stmt.BindColumn(&account_id_);
		stmt.BindColumn(&w_username);
		stmt.BindColumn(&last_login_time_);
		stmt.BindColumn(&last_logout_time_);
		stmt.BindColumn(&created_at_);
		if (stmt.FetchResult() == false) {
			LOG_ERROR("No account found for user_id: {}", user_id_);
			return false;
		}
		username_ = w_username.ToString();
		return true;
	}

	std::string Account::ToString() const {
		return FORMAT("Account(user_id: {}, username: {}, account_id: {}, last_login_time: {})",
			user_id_, username_, account_id_, last_login_time_.ToString());
	}
}

