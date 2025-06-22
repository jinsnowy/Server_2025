package token_repository

import (
	"coa-web-app/repo"
	"context"
	"time"
)

func SetKeyValue(key string, value string, duration time.Duration) error {
	return repo.RedisClient.Set(context.Background(), key, value, duration).Err()
}

func GetValue(key string) (string, error) {
	return repo.RedisClient.Get(context.Background(), key).Result()
}

func DeleteKey(key string) error {
	return repo.RedisClient.Del(context.Background(), key).Err()
}
