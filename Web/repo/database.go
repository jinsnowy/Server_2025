package repo

import (
	"coa-web-app/config"
	"context"
	"log"
	"os"

	"github.com/redis/go-redis/v9"
	"github.com/spf13/viper"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
)

var DbClient *mongo.Client
var AccountDb *mongo.Database
var RedisClient *redis.Client

func InitDbConn() error {
	dbUri := config.GetDbConfig().ConnStr

	clientOptions := options.Client().ApplyURI(dbUri)
	client, err := mongo.Connect(context.Background(), clientOptions)
	if err != nil {
		log.Fatalf("Failed to connect to MongoDB, %s", err)
		return err
	}

	err = client.Ping(context.Background(), nil)
	if err != nil {
		log.Fatalf("Failed to ping MongoDB, %s", err)
		return err
	}

	log.Print("Connected to MongoDB")

	DbClient = client
	AccountDb = client.Database(config.GetDbConfig().AccountDbName)

	return nil
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

	RedisClient = redis.NewClient(&redis.Options{
		Addr:     redisAddress,
		Password: redisPassword,
		DB:       redisDb,
	})

	err := RedisClient.Ping(context.Background()).Err()
	if err != nil {
		log.Fatalf("Failed to connect to Redis, %s", err)
		return err
	}

	log.Print("Connected to Redis")
	return nil
}
