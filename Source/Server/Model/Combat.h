#pragma once

namespace Server::Model {
	class Account;
	class Character;

	class Combat final {
	public:
		Combat() = default;

		float GetBaseAttackCooldown() const {
			return base_attack_cooldown_;
		}

		void SetBaseAttackCooldown(float cooldown) {
			base_attack_cooldown_ = cooldown;
		}

		const System::Tick& GetLastBaseAttackTick() const {
			return last_base_attack_tick_;
		}

		void SetLastBaseAttackTick(const System::Tick& tick) {
			last_base_attack_tick_ = tick;
		}

	private:
		float base_attack_cooldown_ = 0.5; // Base cooldown in seconds
		System::Tick last_base_attack_tick_;
	};
}

