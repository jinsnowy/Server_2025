#include "stdafx.h"
#include "PlayerRepository.h"

#include "Server/Model/Player.h"
#include "Core/System/SingletonActor.h"
#include "Core/Container/LruCache.h"


namespace Server {
    class PlayerRepositoryImpl : public System::SingletonActor<PlayerRepositoryImpl> {
    public:
        PlayerRepositoryImpl()
            :
            player_cache_(1000) // Initialize with a capacity of 1000
        {
        }

        void Insert(std::unique_ptr<Model::Player> player) {
            if (player) {
                player_cache_.Put(player->character_id(), std::move(player));
            }
        }


        std::unique_ptr<Model::Player> Remove(int64_t character_id) {
            std::unique_ptr<Model::Player> player;
            player_cache_.Pop(character_id, player);
            return player;
        }


    private:
        Container::LruCache<int64_t, std::unique_ptr<Model::Player>> player_cache_;
    };

    namespace PlayerRepository {
        
        System::Future<std::unique_ptr<Model::Player>> Pop(int64_t character_id) {
            return Ctrl(PlayerRepositoryImpl::GetInstance()).Async([character_id](PlayerRepositoryImpl& repo) {
                return repo.Remove(character_id);
			});
        }

        void Insert(std::unique_ptr<Model::Player> player) {
            Ctrl(PlayerRepositoryImpl::GetInstance()).Post([player = std::move(player)](PlayerRepositoryImpl& repo) mutable {
                repo.Insert(std::move(player));
            });
		}

        void Remove(int64_t character_id) {
            Ctrl(PlayerRepositoryImpl::GetInstance()).Post([character_id](PlayerRepositoryImpl& repo) {
                repo.Remove(character_id);
            });
        }
    }

}

