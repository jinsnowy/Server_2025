package routes

import (
	"coa-web-app/controllers"

	"github.com/gin-gonic/gin"
)

func SetupRouter() *gin.Engine {
	r := gin.Default()

	r.GET("/auth/access_token", controllers.GetAccessToken)
	r.GET("/auth/google", controllers.GoogleLogin)
	r.GET("/auth/google/callback", controllers.GoogleLoginCallback)
	r.POST("/auth/session/status", controllers.GetSessionStatus)

	r.POST("/login/create_account", controllers.CreateAccountWithUserNameAndPassword)
	r.POST("/login", controllers.LoginWithUserNameAndPassword)
	r.POST("/login/delete_account", controllers.DeleteAccountByUserName)

	return r
}
