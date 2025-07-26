#pragma once

namespace Server::Model {
	class Account {
	public:
		Account() = default;
		Account(const std::string& user_id, const std::string& username)
			: user_id_(user_id), username_(username) {
		}

		const std::string& user_id() const {
			return user_id_;
		}

		const std::string& username() const {
			return username_;
		}

		const System::Time& last_login_time() const {
			return last_login_time_;
		}

		const System::Time& last_logout_time() const {
			return last_logout_time_;
		}

		const System::Time& created_at() const {
			return created_at_;
		}

		int64_t account_id() const {
			return account_id_;
		}

		void set_user_id(const std::string& user_id) {
			user_id_ = user_id;
		}

		void set_username(const std::string& username) {
			username_ = username;
		}

		void set_last_login_time(const System::Time& last_login_time) {
			last_login_time_ = last_login_time;
		}

		void set_account_id(int64_t account_id) {
			account_id_ = account_id;
		}
		
		void set_access_token(const std::string& access_token) {
			access_token_ = access_token;
		}

		std::string ToString() const;

		bool UpsertToDb(Sql::Agent& agent);
		bool LoadFromDb(Sql::Agent& agent);
		void SetLogoutToDb(Sql::Agent& agent, const System::Time& logout_time);
		void SetLoginToDb(Sql::Agent& agent, const System::Time& login_time);

		static std::optional<Account> LoadByAccessToken(Sql::Agent& agent, const std::string& access_token);

	private:
		int64_t account_id_ = 0;
		std::string user_id_;
		std::string username_;
		std::string access_token_;
		System::Time last_login_time_;
		System::Time last_logout_time_;
		System::Time created_at_;
	};

	struct AccountTokenInfo {
		std::string user_id;
		std::string username;
		std::string access_token;
	};
}

