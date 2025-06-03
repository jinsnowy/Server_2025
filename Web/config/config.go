package config

import (
	"log"
	"os"
	"time"

	"github.com/spf13/viper"
	"golang.org/x/oauth2"
	"golang.org/x/oauth2/google"
)

type AuthProvider struct {
	Provider     string
	OAuth2Config oauth2.Config
	UserInfoUrl  string
}

type AuthProviderConfig struct {
	TempSessionExpiry time.Duration
	Google            *AuthProvider
	Naver             *AuthProvider
	Twitter           *AuthProvider
}

type ServerConfig struct {
	UseHttps bool
	CertFile string
	KeyFile  string
}

type DbConfig struct {
	ConnStr       string
	AccountDbName string
}

var dbConfig *DbConfig
var serverConfig *ServerConfig
var authProviderConfig *AuthProviderConfig

func GetDbConfig() *DbConfig {
	return dbConfig
}

func GetServerConfig() *ServerConfig {
	return serverConfig
}

func GetAuthProviderConfig() *AuthProviderConfig {
	return authProviderConfig
}

func InitConfig() {
	configFile := os.Getenv("APP_CONFIG_FILE")
	if configFile == "" {
		log.Print("No config path provided, using default config file")
		configFile = "config"
	}

	configPath := os.Getenv("APP_CONFIG_PATH")
	if configPath == "" {
		log.Print("No config path provided, using default config path")
		configPath = "."
	}

	viper.SetConfigName(configFile)
	viper.AddConfigPath(configPath)
	viper.SetConfigType("yaml")
	viper.ReadInConfig()

	loadServerConfig()
	loadDbConfig()
	loadAuthProvider()
}

func loadServerConfig() {
	useHttps := viper.GetBool("server.use-https")
	certFile := viper.GetString("server.cert-file")
	keyFile := viper.GetString("server.key-file")

	serverConfig = &ServerConfig{
		UseHttps: useHttps,
		CertFile: certFile,
		KeyFile:  keyFile,
	}

	if useHttps {
		if _, err := os.Stat(certFile); os.IsNotExist(err) {
			log.Fatalf("Certificate file does not exist: %s", certFile)
		}
		if _, err := os.Stat(keyFile); os.IsNotExist(err) {
			log.Fatalf("Key file does not exist: %s", keyFile)
		}
	}
}

func loadDbConfig() {
	dbConfig = &DbConfig{
		ConnStr:       viper.GetString("mongodb.connstr"),
		AccountDbName: viper.GetString("mongodb.accountdb"),
	}

	// get os environment MONGODB_HOST
	connStrEnv := os.Getenv("MONGODB_CONN_STRING")
	if connStrEnv != "" {
		dbConfig.ConnStr = connStrEnv
		log.Printf("Using MONGODB_CONN_STRING from environment variable %s", connStrEnv)
	}
}

func loadAuthProvider() {
	tempSessionExpirySeconds := viper.GetInt("auth-provider.temp-session-expiry-seconds")
	authProviderConfig = &AuthProviderConfig{}
	authProviderConfig.TempSessionExpiry = time.Duration(tempSessionExpirySeconds) * time.Second

	if viper.IsSet("auth-provider.google") {
		clientId := viper.GetString("auth-provider.google.client-id")
		clientSecret := viper.GetString("auth-provider.google.client-secret")
		redirectUri := viper.GetString("auth-provider.google.redirect-uri")
		scopes := viper.GetStringSlice("auth-provider.google.scopes")
		userinfoApi := viper.GetString("auth-provider.google.userinfo-api-uri")
		oauth2Config := oauth2.Config{
			ClientID:     clientId,
			ClientSecret: clientSecret,
			RedirectURL:  redirectUri,
			Scopes:       scopes,
			Endpoint:     google.Endpoint,
		}

		authProviderConfig.Google = &AuthProvider{
			Provider:     "google",
			OAuth2Config: oauth2Config,
			UserInfoUrl:  userinfoApi,
		}
	}

	if viper.IsSet("auth-provider.x") {
		clientId := viper.GetString("auth-provider.x.client-id")
		clientSecret := viper.GetString("auth-provider.x.client-secret")
		redirectUri := viper.GetString("auth-provider.x.redirect-uri")
		scopes := viper.GetStringSlice("auth-provider.x.scopes")
		userinfoApi := viper.GetString("auth-provider.x.userinfo-api-uri")
		oauth2Config := oauth2.Config{
			ClientID:     clientId,
			ClientSecret: clientSecret,
			RedirectURL:  redirectUri,
			Scopes:       scopes,
			Endpoint:     google.Endpoint,
		}

		authProviderConfig.Google = &AuthProvider{
			Provider:     "x",
			OAuth2Config: oauth2Config,
			UserInfoUrl:  userinfoApi,
		}
	}

	if viper.IsSet("auth-provider.naver") {
		clientId := viper.GetString("auth-provider.naver.client-id")
		clientSecret := viper.GetString("auth-provider.naver.client-secret")
		redirectUri := viper.GetString("auth-provider.naver.redirect-uri")
		scopes := viper.GetStringSlice("auth-provider.naver.scopes")
		oauth2Config := oauth2.Config{
			ClientID:     clientId,
			ClientSecret: clientSecret,
			RedirectURL:  redirectUri,
			Scopes:       scopes,
			Endpoint:     google.Endpoint,
		}

		authProviderConfig.Naver = &AuthProvider{
			Provider:     "naver",
			OAuth2Config: oauth2Config,
		}
	}

}
