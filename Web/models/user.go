package models

import (
	"crypto/rand"
	"encoding/base64"
)

type User struct {
	UserId           string            `bson:"user_id"`
	Username         string            `bson:"username"`
	Password         string            `bson:"password,omitempty"`
	ExternalAccounts []ExternalAccount `bson:"external_accounts"`
	LastLoginTime    string            `bson:"last_login,omitempty"`
}

type UserAccessToken struct {
	UserId      string `bson:"user_id"`
	AccessToken string `bson:"token"`
}

type ExternalAccount struct {
	UserId   string `bson:"user_id"`
	Provider string `bson:"provider"`
	Email    string `bson:"email"`
}

type UserSessionRequest struct {
	SessionId string `json:"session_id"`
}

type UserSessionResponse struct {
	Status      string `json:"status"`
	UserId      string `json:"user_id,omitempty"`
	AccessToken string `json:"access_token,omitempty"`
}

type UserLoginRequest struct {
	UserName string `json:"username"`
	Password string `json:"password"`
}

type UserLoginResponse struct {
	UserId      string `json:"user_id"`
	AccessToken string `json:"access_token"`
}

type UserCreateRequest struct {
	UserName string `json:"username"`
	Password string `json:"password"`
}

type UserCreateResponse struct {
	UserName string `json:"username"`
	Success  bool   `json:"success"`
}

func CreateAccessToken() (string, error) {
	b := make([]byte, 32) // 32 bytes
	_, err := rand.Read(b)
	if err != nil {
		return "", err
	}

	// encode to base64 to make it URL-safe
	return base64.URLEncoding.EncodeToString(b), nil
}
