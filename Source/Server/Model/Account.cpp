#include "stdafx.h"
#include "Account.h"
#include "Core/Sql/Stmt.h"

namespace Server::Model {

	bool Account::UpsertToDb(Sql::Agent& agent) {
		auto stmt = agent.CreateStmt();
		stmt.BindInParam(Sql::WCharArray(128, user_id_.c_str()));
		stmt.BindInParam(Sql::WCharArray(128, username_.c_str()));
		stmt.BindInParam(last_login_time_);
		if (stmt.Execute(L"usp_UpsertAccount") == false) {
			LOG_ERROR("Failed to upsert account: user_id: {}, username: {}, last_login_time: {}",
				user_id_, username_, last_login_time_.ToString());
			return false;
		}

		stmt.Reset();
		stmt.BindInParam(Sql::WCharArray(128, user_id_.c_str()));
		if (stmt.Execute(L"usp_SelectAccountByUserId") == false) {
			LOG_ERROR("Failed to get account: user_id: {}, username: {}, last_login_time: {}",
				user_id_, username_, last_login_time_.ToString());
			return false;
		}
		
		Sql::WCharArray username_buf(128);
		stmt.BindColumn(&account_id_);
		stmt.BindColumn(&username_buf);
		stmt.BindColumn(&last_login_time_);
		stmt.BindColumn(&last_logout_time_);
		stmt.BindColumn(&created_at_);
		if (stmt.FetchResult() == false) {
			LOG_ERROR("Failed to fetch account: user_id: {}, username: {}, last_login_time: {}",
				user_id_, username_, last_login_time_.ToString());
			return false;
		}
		username_ = username_buf.ToString();

		return true;
	}

	std::string Account::ToString() const {
		return FORMAT("Account(user_id: {}, username: {}, account_id: {}, last_login_time: {})",
			user_id_, username_, account_id_, last_login_time_.ToString());
	}
}

