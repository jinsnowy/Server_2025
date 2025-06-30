IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = N'LobbyDb')
BEGIN
    EXEC('CREATE DATABASE LobbyDB')
END
GO

USE LobbyDB
GO


DROP TABLE IF EXISTS dbo.Account
GO

IF OBJECT_ID(N'dbo.Account', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.Account
    (
        AccountId BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
        UserId NVARCHAR(128) NOT NULL UNIQUE,
        Username NVARCHAR(128) NOT NULL,
        LastLogin DATETIME NOT NULL
    )
END
GO

DROP PROCEDURE IF EXISTS dbo.usp_InsertAccount
GO

CREATE PROCEDURE dbo.usp_UpsertAccount
(
    @p_UserId NVARCHAR(64),
    @p_Username NVARCHAR(64),
    @p_LastLogin DATETIME,
    @p_AccountID BIGINT OUTPUT
)
AS
BEGIN
    DECLARE @v_AccountID BIGINT;
    DECLARE @RETURN INT = 0;

    BEGIN TRY
        -- Check if the account already exists
        SELECT @v_AccountID = AccountID
        FROM Account
        WHERE UserId = @p_UserId;

        IF @v_AccountID IS NOT NULL 
            BEGIN
                UPDATE Account
                SET Username = @p_Username
                ,   LastLogin = @p_LastLogin
                WHERE AccountID = @v_AccountID;
                SET @p_AccountID = @v_AccountID;
            END
        ELSE
            BEGIN
                -- If it does not exist, insert a new account
                INSERT INTO Account (UserId, Username, LastLogin)
                VALUES (@p_UserId, @p_Username, @p_LastLogin);

                SET @p_AccountID = SCOPE_IDENTITY();
            END

    END TRY
    BEGIN CATCH
        THROW
    END CATCH

    RETURN @RETURN
END
GO

