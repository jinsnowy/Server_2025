package user_repository

import (
	"coa-web-app/models"
	"coa-web-app/repo"
	"coa-web-app/utils"
	"context"
	"errors"
	"log"

	"go.mongodb.org/mongo-driver/mongo"
)

func InsertUser(user *models.User) error {
	collection := repo.AccountDb.Collection("users")
	_, err := collection.InsertOne(context.Background(), user)
	if err != nil {
		return err
	}
	return nil
}

func GetUserByUserId(userId string) (*models.User, error) {
	collection := repo.AccountDb.Collection("users")
	filter := map[string]interface{}{
		"user_id": userId,
	}

	var user models.User
	err := collection.FindOne(context.Background(), filter).Decode(&user)
	if err != nil {
		if err == mongo.ErrNoDocuments {
			return nil, nil
		}
		return nil, err
	}

	return &user, nil

}
func GetUserByExternalAccount(provider string, email string) (*models.User, error) {
	collection := repo.AccountDb.Collection("users")
	filter := map[string]interface{}{
		"external_accounts": map[string]string{
			"provider": provider,
			"email":    email,
		},
	}

	var user models.User
	err := collection.FindOne(context.Background(), filter).Decode(&user)
	if err != nil {
		if err == mongo.ErrNoDocuments {
			return nil, nil
		}
		return nil, err
	}

	return &user, nil
}

func SetUserLogined(userId string) error {
	collection := repo.AccountDb.Collection("users")
	filter := map[string]interface{}{
		"user_id": userId,
	}
	update := map[string]interface{}{
		"$set": map[string]interface{}{
			"last_login": utils.GetTimeNow(),
		},
	}

	_, err := collection.UpdateOne(context.Background(), filter, update)
	if err != nil {
		return err
	}
	return nil
}

func GetUserByUserName(username string) (*models.User, error) {
	collection := repo.AccountDb.Collection("users")
	filter := map[string]interface{}{
		"username": username,
	}

	var user models.User
	err := collection.FindOne(context.Background(), filter).Decode(&user)
	if err != nil {
		if err == mongo.ErrNoDocuments {
			return nil, nil
		}
		return nil, err
	}

	return &user, nil
}

func GetAccessToken(userId string) (*models.UserAccessToken, error) {
	collection := repo.AccountDb.Collection("access_tokens")
	filter := map[string]interface{}{
		"user_id": userId,
	}

	var accessToken models.UserAccessToken
	err := collection.FindOne(context.Background(), filter).Decode(&accessToken)
	if err != nil {
		if err == mongo.ErrNoDocuments {
			return nil, nil
		}
		return nil, err
	}

	return &accessToken, nil
}

func InsertOrUpdateAccessToken(userId string, token string) error {
	collection := repo.AccountDb.Collection("access_tokens")

	// Check if the token already exists
	existingToken, getErr := GetAccessToken(userId)
	if getErr != nil {
		return getErr
	}

	accessToken := models.UserAccessToken{
		UserId:      userId,
		AccessToken: token,
	}

	log.Println("Inserting or updating access token for user: {}, token:{} ", userId, token)

	if existingToken != nil {
		// Update the existing token
		filter := map[string]interface{}{
			"user_id": userId,
		}
		// set token field to the new access token
		if existingToken.AccessToken == token {
			// No need to update if the token is the same
			return nil
		}

		// Update the access token in the database
		update := map[string]interface{}{
			"$set": map[string]interface{}{
				"token": accessToken.AccessToken,
			},
		}

		updateResult, err := collection.UpdateOne(context.Background(), filter, update)
		if err != nil {
			return err
		}
		if updateResult.MatchedCount == 0 {
			return errors.New("no matching access token found for user")
		}
		return nil
	} else {
		_, err := collection.InsertOne(context.Background(), accessToken)
		if err != nil {
			return err
		}
	}

	return nil
}

func ConsumeAccessToken(access_token string) (*models.User, error) {
	collection := repo.AccountDb.Collection("access_tokens")
	filter := map[string]interface{}{
		"token": access_token,
	}

	var accessToken models.UserAccessToken
	err := collection.FindOne(context.Background(), filter).Decode(&accessToken)
	if err != nil {
		return nil, err
	}

	// Delete the access token after consuming it
	_, err = collection.DeleteOne(context.Background(), filter)
	if err != nil {
		return nil, err
	}

	user, err := GetUserByUserId(accessToken.UserId)
	if err != nil {
		return nil, err
	}

	return user, nil
}

func DeleteUserByUserName(userName string) error {
	collection := repo.AccountDb.Collection("users")
	filter := map[string]interface{}{
		"username": userName,
	}

	_, err := collection.DeleteOne(context.Background(), filter)
	if err != nil {
		return err
	}
	return nil
}
