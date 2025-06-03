package repo

import (
	"context"
	"log"
	"os"
	"time"

	"github.com/redis/go-redis/v9"
	"github.com/spf13/viper"
)

var redisClient *redis.Client

func SetKeyValue(key string, value string, duration time.Duration) error {
	return redisClient.Set(context.Background(), key, value, duration).Err()
}

func GetValue(key string) (string, error) {
	return redisClient.Get(context.Background(), key).Result()
}

func DeleteKey(key string) error {
	return redisClient.Del(context.Background(), key).Err()
}

func InitRedisClient() error {
	redisAddress := viper.GetString("redis.address")
	redisPassword := viper.GetString("redis.password")
	redisDb := viper.GetInt("redis.db")

	redisAddressEnv := os.Getenv("REDIS_ADDRESS")
	if redisAddressEnv != "" {
		redisAddress = redisAddressEnv
		log.Printf("Using REDIS_ADDRESS from environment: %s", redisAddress)
	}

	redisClient = redis.NewClient(&redis.Options{
		Addr:     redisAddress,
		Password: redisPassword,
		DB:       redisDb,
	})

	err := redisClient.Ping(context.Background()).Err()
	if err != nil {
		log.Fatalf("Failed to connect to Redis, %s", err)
		return err
	}

	log.Print("Connected to Redis")
	return nil
}
