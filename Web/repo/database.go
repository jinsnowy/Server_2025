package repo

import (
	"coa-web-app/config"
	"context"
	"log"

	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
)

var DbClient *mongo.Client
var AccountDb *mongo.Database

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
