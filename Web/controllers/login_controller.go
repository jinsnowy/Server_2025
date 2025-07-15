package controllers

import (
	"coa-web-app/models"
	"coa-web-app/repo/user_repository"
	"coa-web-app/utils"
	"log"
	"net/http"

	"github.com/gin-gonic/gin"
	"golang.org/x/crypto/bcrypt"
)

// Helper function to generate a bcrypt hash from a raw password
func GenerateHashPassword(rawPassword string) (string, error) {
	bytes, err := bcrypt.GenerateFromPassword([]byte(rawPassword), bcrypt.DefaultCost)
	return string(bytes), err
}

func CompareHashAndPassword(hashedPassword, password string) error {
	return bcrypt.CompareHashAndPassword([]byte(hashedPassword), []byte(password))
}

func CreateAccountWithUserNameAndPassword(c *gin.Context) {
	var userCreateRequest models.UserCreateRequest
	if err := c.ShouldBindJSON(&userCreateRequest); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "invalid request body"})
		return
	}

	if userCreateRequest.UserName == "" || userCreateRequest.Password == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "username and password cannot be empty"})
		return
	}

	if (len(userCreateRequest.UserName) < 3 || len(userCreateRequest.UserName) > 20) ||
		(len(userCreateRequest.Password) < 6 || len(userCreateRequest.Password) > 20) {
		c.JSON(http.StatusBadRequest, gin.H{"error": "username must be 3-20 characters and password must be 6-20 characters"})
		return
	}

	paswordHashed, errHash := GenerateHashPassword(userCreateRequest.Password)
	if errHash != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to hash password"})
		return
	}

	user := &models.User{
		UserId:        utils.GenerateGuid(),
		Username:      userCreateRequest.UserName,
		Password:      paswordHashed,
		LastLoginTime: utils.GetTimeNow(),
	}

	exists, err := user_repository.GetUserByUserName(user.Username)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to check if user exists"})
		return
	}

	if exists != nil {
		c.JSON(http.StatusOK, models.UserCreateResponse{
			UserName: userCreateRequest.UserName,
			Success:  false,
		})
		return
	}

	if err := user_repository.InsertUser(user); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to create user"})
		return
	}

	c.JSON(http.StatusOK, models.UserCreateResponse{
		UserName: userCreateRequest.UserName,
		Success:  true,
	})
}

func LoginWithUserNameAndPassword(c *gin.Context) {
	var userLoginRequest models.UserLoginRequest
	if err := c.ShouldBindJSON(&userLoginRequest); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "invalid request body"})
		return
	}

	user, err := user_repository.GetUserByUserName(userLoginRequest.UserName)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to get user"})
		return
	}

	if user == nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "invalid username or password"})
		return
	}

	if userLoginRequest.Password == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "password cannot be empty"})
		return
	}

	if CompareHashAndPassword(user.Password, userLoginRequest.Password) != nil {
		log.Println("Password comparison failed for user:", user.Username)
		c.JSON(http.StatusUnauthorized, gin.H{"error": "invalid username or password"})
		return
	}

	if user.Username != userLoginRequest.UserName {
		log.Println("Username mismatch for user:", user.Username)
		c.JSON(http.StatusUnauthorized, gin.H{"error": "invalid username or password"})
		return
	}

	access_token, createErr := models.CreateAccessToken()
	if createErr != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to create access token"})
		return
	}

	upsertErr := user_repository.InsertOrUpdateAccessToken(user.UserId, access_token)
	if upsertErr != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to upsert access token"})
		return
	}

	log.Printf("login with access token: %s", access_token)

	c.JSON(http.StatusOK, models.UserLoginResponse{
		UserId:      user.UserId,
		AccessToken: access_token,
	})
}

func DeleteAccountByUserName(c *gin.Context) {
	userName := c.DefaultQuery("username", "")
	if userName == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "username is required"})
		return
	}

	user_repository.DeleteUserByUserName(userName)

	c.JSON(http.StatusOK, gin.H{"message": "user deleted successfully"})
}
