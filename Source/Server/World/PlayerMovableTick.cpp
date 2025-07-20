#include "stdafx.h"
#include "PlayerMovableTick.h"
#include "Core/System/SingletonActor.h"
#include "../Session/WorldSession.h"

namespace Server::PlayerMovableTick {
	
	class PlayerMovableTick : public System::SingletonActor<PlayerMovableTick> {
	public:
		PlayerMovableTick();

		void AddSession(const std::shared_ptr<WorldSession>& session);
		void RemoveSession(const std::shared_ptr<WorldSession>& session);

	private:
		System::PeriodicTimer::Handle tick_timer_handle_;
		std::unordered_map<int64_t, std::shared_ptr<WorldSession>> world_session_map_;
		std::vector<std::shared_ptr<WorldSession>> world_sessions_;

		void OnTick();
	};

	PlayerMovableTick::PlayerMovableTick() {
		tick_timer_handle_ = System::PeriodicTimer::Schedule(System::Duration::FromMilliseconds(kPlayerMovableTickInterval), [](System::PeriodicTimer::Handle&) {
			Ctrl(PlayerMovableTick::GetInstance()).Post([](PlayerMovableTick& tick) {
				tick.OnTick();
			});
		}, true);
	}

	void PlayerMovableTick::AddSession(const std::shared_ptr<WorldSession>& session) {
		if (session) {
			if (world_session_map_.emplace(session->session_id(), session).second) {
				world_sessions_.push_back(session);
			}
		}
	}

	void PlayerMovableTick::RemoveSession(const std::shared_ptr<WorldSession>& session) {
		if (session) {
			world_session_map_.erase(session->session_id());
			auto it = std::remove(world_sessions_.begin(), world_sessions_.end(), session);
			if (it != world_sessions_.end()) {
				world_sessions_.erase(it, world_sessions_.end());
			}
		}
	}

	void PlayerMovableTick::OnTick() {
		const auto now = System::Tick::Current();
		for (const auto& session : world_sessions_) {
			session->OnMovableTick(now);
		}
	}
	
	void BeginTick(const std::shared_ptr<WorldSession>& world_session) {
		Ctrl(PlayerMovableTick::GetInstance()).Post([world_session](PlayerMovableTick& tick) {
			tick.AddSession(world_session);
		});
	}
	
	void EndTick(const std::shared_ptr<WorldSession>& world_session) {
		Ctrl(PlayerMovableTick::GetInstance()).Post([world_session](PlayerMovableTick& tick) {
			tick.RemoveSession(world_session);
		});
	}
}