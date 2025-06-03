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

type UserLoginToken struct {
	UserId string `bson:"user_id"`
	Token  string `bson:"token"`
}

type ExternalAccount struct {
	Provider string `bson:"provider"`
	UserId   string `bson:"email"`
}

type UserSessionRequest struct {
	SessionId string `json:"session_id"`
}

type UserSessionResponse struct {
	Status string `json:"status"`
	UserId string `json:"user_id,omitempty"`
	Token  string `json:"access_token,omitempty"`
}

type UserLoginRequest struct {
	UserName string `json:"username"`
	Password string `json:"password"`
}

type UserLoginResponse struct {
	UserId string `json:"user_id"`
	Token  string `json:"access_token"`
}

type UserCreateRequest struct {
	UserName string `json:"username"`
	Password string `json:"password"`
}

type UserCreateResponse struct {
	UserName string `json:"username"`
	Success  bool   `json:"success"`
}

func CreateLoginToken() (string, error) {
	b := make([]byte, 32) // 32 bytes
	_, err := rand.Read(b)
	if err != nil {
		return "", err
	}

	// encode to base64 to make it URL-safe
	return base64.URLEncoding.EncodeToString(b), nil
}
