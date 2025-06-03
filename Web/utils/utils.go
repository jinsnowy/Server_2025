package utils

import (
	"encoding/base64"
	"time"

	"github.com/google/uuid"
)

func EncodeBase64(s string) string {
	return base64.URLEncoding.EncodeToString([]byte(s))
}

func DecodeBase64(s string) (string, error) {
	bytes, err := base64.URLEncoding.DecodeString(s)
	if err != nil {
		return "", err
	}
	return string(bytes), nil
}

func GenerateGuid() string {
	return uuid.New().String()
}

func GetTimeNow() string {
	// format as RFC3339 (e.g., 2023-10-01T12:00:00Z)
	return time.Now().UTC().Format(time.RFC3339)
}
