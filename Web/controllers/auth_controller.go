package controllers

import (
	"coa-web-app/config"
	"coa-web-app/models"
	"coa-web-app/repo"
	"coa-web-app/utils"
	"crypto/rand"
	"encoding/base64"
	"encoding/json"
	"net/http"

	"github.com/gin-gonic/gin"
	"golang.org/x/oauth2"
)

func GoogleLoginCallback(c *gin.Context) {
	code := c.DefaultQuery("code", "")
	state := c.DefaultQuery("state", "")
	google := config.GetAuthProviderConfig().Google

	if code == "" || state == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "missing code or state"})
		return
	}

	_, get_token_error := repo.GetValue(state)
	if get_token_error != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to get token"})
		return
	}

	// exchange the code for a token
	token, err := google.OAuth2Config.Exchange(c, code)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to exchange token"})
		return
	}

	// use the token to fetch user information
	client := google.OAuth2Config.Client(c, token)
	resp, err := client.Get(google.UserInfoUrl)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to fetch user info"})
		return
	}
	defer resp.Body.Close()

	// parse user data
	var user map[string]interface{}
	if err := json.NewDecoder(resp.Body).Decode(&user); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to parse user info"})
		return
	}

	// get user id if exists from external accounts
	prevUser, err := repo.GetUserByExternalAccount(google.Provider, user["email"].(string))
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to get user"})
		return
	}

	exipry := config.GetAuthProviderConfig().TempSessionExpiry

	if prevUser != nil {
		err = repo.SetUserLogined(prevUser.UserId)
		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to update user"})
			return
		}

		err = repo.SetKeyValue(state, prevUser.UserId, exipry)
		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to set token"})
			return
		}

		c.JSON(http.StatusOK, gin.H{"user_id": prevUser.UserId})
		return
	}

	// create a new user
	newUserId := utils.GenerateGuid()
	newUser := models.User{
		UserId:   newUserId,
		Username: "",
		ExternalAccounts: []models.ExternalAccount{
			{
				Provider: google.Provider,
				UserId:   user["email"].(string),
			},
		},
		LastLoginTime: utils.GetTimeNow(),
	}

	err = repo.InsertUser(&newUser)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to create user"})
		return
	}

	err = repo.SetKeyValue(state, newUserId, exipry)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to set token"})
		return
	}

	loginToken, createErr := models.CreateLoginToken()
	if createErr != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to create login token"})
		return
	}

	upsertErr := repo.InsertOrUpdateLoginToken(newUserId, loginToken)
	if upsertErr != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to upsert login token"})
	}

	c.JSON(http.StatusOK, models.UserLoginResponse{
		UserId: newUserId,
		Token:  loginToken,
	})
}

func generateStateString() (string, error) {
	b := make([]byte, 32) // 32 bytes
	_, err := rand.Read(b)
	if err != nil {
		return "", err
	}
	return base64.URLEncoding.EncodeToString(b), nil
}

func GoogleLogin(c *gin.Context) {
	// get session_id from Authorization header
	session_id := c.DefaultQuery("session_id", "")
	if session_id == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "missing session_id"})
		return
	}

	_, err := repo.GetValue(session_id)
	if err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "failed to get token"})
		return
	}

	google := config.GetAuthProviderConfig().Google
	authURL := google.OAuth2Config.AuthCodeURL(session_id, oauth2.AccessTypeOnline)

	c.Redirect(http.StatusFound, authURL)
}

func GetSessionStatus(c *gin.Context) {

	var sessionRequest models.UserSessionRequest
	if err := c.ShouldBindJSON(&sessionRequest); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "invalid request body"})
		return
	}

	value, err := repo.GetValue(sessionRequest.SessionId)
	if err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "failed to get token"})
		return
	}

	if value == "" {
		c.JSON(http.StatusOK, models.UserSessionResponse{
			Status: "pending",
		})
		return
	}

	user, err := repo.GetUserByUserId(value)
	if user == nil || err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to get user"})
		return
	}

	loginToken, createErr := models.CreateLoginToken()
	if createErr != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to create login token"})
		return
	}

	upsertErr := repo.InsertOrUpdateLoginToken(user.UserId, loginToken)
	if upsertErr != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to upsert login token"})
	}

	c.JSON(http.StatusOK, models.UserSessionResponse{
		Status: "done",
		UserId: value,
		Token:  loginToken,
	})
}

func GetAccessToken(c *gin.Context) {
	// generate a random state string

	var access_token string
	var err error
	for {
		access_token, err = generateStateString()
		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to generate state"})
			return
		}

		_, err = repo.GetValue(access_token)
		if err != nil {
			break
		}
	}

	exipry := config.GetAuthProviderConfig().TempSessionExpiry

	err = repo.SetKeyValue(access_token, "", exipry)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "failed to set state"})
		return
	}

	c.JSON(http.StatusOK, gin.H{"access_token": access_token, "expiry": exipry.Seconds()})
}
