package repo

import (
	"coa-web-app/models"
	"coa-web-app/utils"
	"context"

	"go.mongodb.org/mongo-driver/mongo"
)

func InsertUser(user *models.User) error {
	collection := AccountDb.Collection("users")
	_, err := collection.InsertOne(context.Background(), user)
	if err != nil {
		return err
	}
	return nil
}

func GetUserByUserId(userId string) (*models.User, error) {
	collection := AccountDb.Collection("users")
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
	collection := AccountDb.Collection("users")
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
	collection := AccountDb.Collection("users")
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
	collection := AccountDb.Collection("users")
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

func GetLoginToken(userId string) (*models.UserLoginToken, error) {
	collection := AccountDb.Collection("login_tokens")
	filter := map[string]interface{}{
		"user_id": userId,
	}

	var loginToken models.UserLoginToken
	err := collection.FindOne(context.Background(), filter).Decode(&loginToken)
	if err != nil {
		if err == mongo.ErrNoDocuments {
			return nil, nil
		}
		return nil, err
	}

	return &loginToken, nil
}

func InsertOrUpdateLoginToken(userId string, token string) error {
	collection := AccountDb.Collection("login_tokens")

	// Check if the token already exists
	existingToken, getErr := GetLoginToken(userId)
	if getErr != nil {
		return getErr
	}

	loginToken := models.UserLoginToken{
		UserId: userId,
		Token:  token,
	}

	if existingToken != nil {
		// Update the existing token
		filter := map[string]interface{}{
			"user_id": userId,
		}
		update := map[string]interface{}{
			"$set": loginToken,
		}

		_, err := collection.UpdateOne(context.Background(), filter, update)
		if err != nil {
			return err
		}
		return nil
	} else {
		_, err := collection.InsertOne(context.Background(), loginToken)
		if err != nil {
			return err
		}
	}

	return nil
}

func DeleteUserByUserName(userName string) error {
	collection := AccountDb.Collection("users")
	filter := map[string]interface{}{
		"username": userName,
	}

	_, err := collection.DeleteOne(context.Background(), filter)
	if err != nil {
		return err
	}
	return nil
}
