#include "stdafx.h"
#include "PlayerTick.h"
#include "Core/System/SingletonActor.h"
#include "../Session/WorldSession.h"

namespace Server::PlayerTick {	
	class PlayerTick : public System::SingletonActor<PlayerTick> {
	public:
		PlayerTick();

		void AddSession(const std::shared_ptr<WorldSession>& session);
		void RemoveSession(const std::shared_ptr<WorldSession>& session);

		const System::Tick& base_tick() const {
			return base_tick_;
		}

	private:
		System::Tick base_tick_;
		System::PeriodicTimer::Handle tick_timer_handle_;
		std::unordered_map<int64_t, std::shared_ptr<WorldSession>> world_session_map_;
		std::vector<std::shared_ptr<WorldSession>> world_sessions_;

		void OnTick();
	};

	PlayerTick::PlayerTick() {
		base_tick_ = System::Tick::Current();
		tick_timer_handle_ = System::PeriodicTimer::Schedule(System::Duration::FromMilliseconds(kPlayerTickInterval), [](System::PeriodicTimer::Handle&) {
			Ctrl(PlayerTick::GetInstance()).Post([](PlayerTick& tick) {
				tick.OnTick();
			});
		}, true);
	}

	void PlayerTick::AddSession(const std::shared_ptr<WorldSession>& session) {
		if (session) {
			if (world_session_map_.emplace(session->session_id(), session).second) {
				world_sessions_.push_back(session);
			}
		}
	}

	void PlayerTick::RemoveSession(const std::shared_ptr<WorldSession>& session) {
		if (session) {
			world_session_map_.erase(session->session_id());
			auto it = std::remove(world_sessions_.begin(), world_sessions_.end(), session);
			if (it != world_sessions_.end()) {
				world_sessions_.erase(it, world_sessions_.end());
			}
		}
	}

	void PlayerTick::OnTick() {
	}
	
	void SetServerTickInterVal(int32_t interval) {
		kPlayerTickInterval = std::max(0, interval);
	}

	float GetServerTick() {
		return static_cast<float>((System::Tick::Current() - PlayerTick::GetInstance().base_tick()).AsMicroSecs()) / 1'000'000.f;
	}

	void BeginTick(const std::shared_ptr<WorldSession>& world_session) {
		Ctrl(PlayerTick::GetInstance()).Post([world_session](PlayerTick& tick) {
			tick.AddSession(world_session);
		});
	}
	
	void EndTick(const std::shared_ptr<WorldSession>& world_session) {
		Ctrl(PlayerTick::GetInstance()).Post([world_session](PlayerTick& tick) {
			tick.RemoveSession(world_session);
		});
	}
}