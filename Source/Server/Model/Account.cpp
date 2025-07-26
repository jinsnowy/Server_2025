#include "stdafx.h"
#include "Account.h"
#include "Core/Sql/Stmt.h"

namespace Server::Model {

	bool Account::UpsertToDb(Sql::Agent& agent) {
		auto stmt = agent.CreateStmt();
		stmt.BindInParam(Sql::WCharArray(128, user_id_.c_str()));
		stmt.BindInParam(Sql::WCharArray(128, username_.c_str()));
		stmt.BindInParam(Sql::WCharArray(256, access_token_.c_str()));
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
		if (stmt.Execute(L"dbo.usp_SelectAccountByUserId") == false) {
			LOG_ERROR("Failed to load account: user_id: {}", user_id_);
			return false;
		}
		Sql::WCharArray w_username(128);
		Sql::WCharArray w_access_token(256);
		stmt.BindColumn(&account_id_);
		stmt.BindColumn(&w_username);
		stmt.BindColumn(&w_access_token);
		stmt.BindColumn(&last_login_time_);
		stmt.BindColumn(&last_logout_time_);
		stmt.BindColumn(&created_at_);
		if (stmt.FetchResult() == false) {
			LOG_ERROR("No account found for user_id: {}", user_id_);
			return false;
		}
		username_ = w_username.ToString();
		access_token_ = w_access_token.ToString();
		return true;
	}

	void Account::SetLoginToDb(Sql::Agent& agent, const System::Time& login_time) {
		auto stmt = agent.CreateStmt();
		stmt.BindInParam(account_id_);
		stmt.BindInParam(login_time);
		if (stmt.Execute(L"usp_UpdateAccountLogin") == false) {
			LOG_ERROR("Failed to set account login: user_id: {}, login_time: {}", user_id_, login_time.ToString());
		} else {
			last_login_time_ = login_time;
		}
	}

	void Account::SetLogoutToDb(Sql::Agent& agent, const System::Time& logout_time) {
		auto stmt = agent.CreateStmt();
		stmt.BindInParam(account_id_);
		stmt.BindInParam(logout_time);
		if (stmt.Execute(L"usp_UpdateAccountLogout") == false) {
			LOG_ERROR("Failed to set account logout: user_id: {}, logout_time: {}", user_id_, logout_time.ToString());
		} else {
			last_logout_time_ = logout_time;
		}
	}

	std::optional<Account> Account::LoadByAccessToken(Sql::Agent& agent, const std::string& access_token) {
		auto stmt = agent.CreateStmt();
		stmt.BindInParam(Sql::WCharArray(256, access_token.c_str()));
		if (stmt.Execute(L"usp_SelectAccountByAccessToken") == false) {
			LOG_ERROR("Failed to load account by access token: {}", access_token);
			return std::nullopt;
		}
		// 
		//	SELECT AccountId, Username, AccessToken, LastLogin, LastLogout, CreatedAt
		//	FROM dbo.Account
		//	WHERE AccessToken = @p_AccessToken;

		Account account;
		Sql::WCharArray w_user_id(128);
		Sql::WCharArray w_username(128);
		Sql::WCharArray w_access_token(256);
		stmt.BindColumn(&account.account_id_);
		stmt.BindColumn(&w_user_id);
		stmt.BindColumn(&w_username);
		stmt.BindColumn(&w_access_token);
		stmt.BindColumn(&account.last_login_time_);
		stmt.BindColumn(&account.last_logout_time_);
		stmt.BindColumn(&account.created_at_);
		if (stmt.FetchResult() == false) {
			LOG_ERROR("No account found for access token: {}", access_token);
			return std::nullopt;
		}

		return account;
	}

	std::string Account::ToString() const {
		return FORMAT("Account(user_id: {}, username: {}, account_id: {}, last_login_time: {})",
			user_id_, username_, account_id_, last_login_time_.ToString());
	}
}

