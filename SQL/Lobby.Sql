IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = N'LobbyDb')
BEGIN
    EXEC('CREATE DATABASE LobbyDB')
END
GO

USE LobbyDB
GO

--- Account Table

DROP TABLE IF EXISTS dbo.Account
GO

IF OBJECT_ID(N'dbo.Account', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.Account
    (
        AccountId BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
        UserId NVARCHAR(128) NOT NULL UNIQUE,
        Username NVARCHAR(128) NOT NULL,
        LastLogin DATETIME NOT NULL,
        LastLogout DATETIME NULL,
        CreatedAt DATETIME NOT NULL
    )
END
GO

DROP PROCEDURE IF EXISTS dbo.usp_TestOutput
GO

CREATE PROCEDURE dbo.usp_TestOutput
(
    @p_Input TINYINT,
    @p_Input2 NVARCHAR(128),
    @p_Input3 NVARCHAR(128),
    @p_Input4 DATETIME,
    @p_Input5 DATETIME,
    @p_Output BIGINT OUTPUT
)
AS
BEGIN
    SET NOCOUNT ON;
    DECLARE @RETURN INT = 0;

    -- Simulate some processing
    SET @p_Output = @p_Input;

    -- Optionally, you can return a status code
    RETURN 0;
END
GO

DROP PROCEDURE IF EXISTS dbo.usp_UpsertAccount
GO

CREATE PROCEDURE dbo.usp_UpsertAccount
(
    @p_UserId NVARCHAR(64),
    @p_Username NVARCHAR(64),
    @p_LastLogin DATETIME
)
AS
BEGIN
    DECLARE @v_AccountID BIGINT;
    DECLARE @RETURN INT = 0;

    BEGIN TRY
        -- Check if the account already exists
        SELECT @v_AccountID = AccountId
        FROM Account
        WHERE UserId = @p_UserId;

        IF @v_AccountID IS NOT NULL 
            BEGIN
                UPDATE Account
                SET Username = @p_Username
                ,   LastLogin = @p_LastLogin
                WHERE AccountId = @v_AccountID;
            END
        ELSE
            BEGIN
                -- If it does not exist, insert a new account
                INSERT INTO Account (UserId, Username, LastLogin, LastLogout, CreatedAt)
                VALUES (@p_UserId, @p_Username, @p_LastLogin, @p_LastLogin, GETUTCDATE());
            END

    END TRY
    BEGIN CATCH
        THROW
    END CATCH

    RETURN @RETURN
END
GO


DROP PROCEDURE IF EXISTS dbo.usp_SelectAccountByUserId
GO


CREATE PROCEDURE dbo.usp_SelectAccountByUserId
(
    @p_UserId NVARCHAR(128)
)
AS
BEGIN
    DECLARE @v_AccountID BIGINT;
    DECLARE @RETURN INT = 0;

    BEGIN TRY
        -- Check if the account already exists
        SELECT AccountId, Username, LastLogin, LastLogout, CreatedAt
        FROM dbo.Account
        WHERE UserId = @p_UserId;

    END TRY
    BEGIN CATCH
        THROW
    END CATCH

    RETURN @RETURN
END

--- Server Table
DROP TABLE IF EXISTS dbo.Server
GO

IF OBJECT_ID(N'dbo.Server', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.Server
    (
        ServerId INT IDENTITY(1,1) NOT NULL PRIMARY KEY,
        ServerType TINYINT NOT NULL,
        ServerAddress NVARCHAR(256) NOT NULL,
        MapName    NVARCHAR(256) NOT NULL,
        CreatedAt DATETIME NOT NULL,
        LastPingTime DATETIME NOT NULL,
    )
END
GO

DROP PROCEDURE IF EXISTS dbo.usp_UpsertServer
GO

CREATE PROCEDURE dbo.usp_UpsertServer
(
    @p_ServerType TINYINT,
    @p_ServerAddress NVARCHAR(256),
    @p_MapName NVARCHAR(256),
    @p_CreatedAt DATETIME
)
AS
BEGIN
    DECLARE @v_ServerId INT;
    DECLARE @RETURN INT = 0;

    BEGIN TRY
        -- Check if the account already exists
        SELECT @v_ServerId = ServerId
        FROM dbo.Server
        WHERE ServerAddress = @p_ServerAddress;

        IF @v_ServerId IS NOT NULL 
            BEGIN
                UPDATE dbo.Server
                SET ServerType = @p_ServerType
                ,   MapName    = @p_MapName
                ,   LastPingTime = GETUTCDATE()
                WHERE ServerAddress = @p_ServerAddress
            END
        ELSE
            BEGIN
                INSERT INTO dbo.Server (ServerType, ServerAddress, MapName, CreatedAt, LastPingTime)
                VALUES (@p_ServerType, @p_ServerAddress, @p_MapName, @p_CreatedAt, GETUTCDATE());

                SET @v_ServerId = SCOPE_IDENTITY();
            END
    END TRY
    BEGIN CATCH
        THROW
    END CATCH

    RETURN @RETURN
END
GO

DROP PROCEDURE IF EXISTS dbo.usp_SelectServer
GO

CREATE PROCEDURE dbo.usp_SelectServer
(
    @p_ServerAddress NVARCHAR(256)
)
AS
BEGIN
    DECLARE @v_ServerId INT;
    DECLARE @RETURN INT = 0;

    BEGIN TRY

        -- Check if the account already exists
        SELECT ServerId
        FROM dbo.Server
        WHERE ServerAddress = @p_ServerAddress;

    END TRY
    BEGIN CATCH
        THROW
    END CATCH

    RETURN @RETURN
END
GO