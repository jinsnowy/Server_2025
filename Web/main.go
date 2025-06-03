package main

import (
	"coa-web-app/config"
	"coa-web-app/repo"
	"coa-web-app/routes"
	"log"
)

func main() {
	config.InitConfig()
	err := repo.InitDbConn()
	if err != nil {
		log.Fatalf("Failed to connect to MongoDB, %s", err)
		return
	}
	err = repo.InitRedisClient()
	if err != nil {
		log.Fatalf("Failed to connect to Redis, %s", err)
		return
	}

	r := routes.SetupRouter()

	addr := ":8080"
	serverConfig := config.GetServerConfig()
	if serverConfig.UseHttps == true {
		certFile := serverConfig.CertFile
		keyFile := serverConfig.KeyFile
		log.Fatal(r.RunTLS(addr, certFile, keyFile))
	} else {
		log.Fatal(r.Run(addr))
	}
}
