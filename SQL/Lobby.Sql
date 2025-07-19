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

DROP PROCEDURE IF EXISTS dbo.usp_UpsertAccount
GO

CREATE PROCEDURE dbo.usp_UpsertAccount
(
    @p_UserId NVARCHAR(64),
    @p_Username NVARCHAR(64),
    @p_LastLogin DATETIME,
    @p_AccountId BIGINT OUTPUT
)
AS
BEGIN
    DECLARE @v_AccountID BIGINT;
    DECLARE @RETURN INT = 0;

    SET NOCOUNT ON;

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

                SET @p_AccountId = @v_AccountID;
            END
        ELSE
            BEGIN
                -- If it does not exist, insert a new account
                INSERT INTO Account (UserId, Username, LastLogin, LastLogout, CreatedAt)
                VALUES (@p_UserId, @p_Username, @p_LastLogin, @p_LastLogin, GETUTCDATE());

                SET @p_AccountId = SCOPE_IDENTITY();
            END

    END TRY
    BEGIN CATCH
        THROW
    END CATCH

    SET NOCOUNT OFF;

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
GO

-- Character Table
DROP TABLE IF EXISTS dbo.Character
GO

IF OBJECT_ID(N'dbo.Character', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.Character
    (
        CharacterId BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,
        AccountId BIGINT NOT NULL,
        ServerId INT NOT NULL,
        Name NVARCHAR(128) NOT NULL UNIQUE,
        Level INT NOT NULL,
        Exp BIGINT NOT NULL,
        LastPlayed DATETIME NOT NULL,
        CreatedAt DATETIME NOT NULL,
    )
END
GO

DROP PROCEDURE IF EXISTS dbo.usp_CheckCharacterExistsByName
GO

CREATE PROCEDURE dbo.usp_CheckCharacterExistsByName
(
    @p_Name NVARCHAR(128)
)
AS
BEGIN
    DECLARE @RETURN INT = 0;

    BEGIN TRY
       
        IF EXISTS (SELECT 1 FROM dbo.Character WHERE Name = @p_Name)
        BEGIN
            SET @RETURN = 1;
        END

    END TRY
    BEGIN CATCH
        THROW
    END CATCH

    RETURN @RETURN
END
GO

DROP PROCEDURE IF EXISTS dbo.usp_UpsertCharacter
GO

CREATE PROCEDURE dbo.usp_UpsertCharacter
(
    @p_AccountId BIGINT,
    @p_ServerId INT,
    @p_Name NVARCHAR(128),
    @p_Level INT,
    @p_Exp BIGINT,
    @p_LastPlayed DATETIME,
    @p_CharacterId BIGINT OUTPUT
)
AS
BEGIN
    DECLARE @v_CharacterId BIGINT;
    DECLARE @RETURN INT = 0;

    SET NOCOUNT ON;

    BEGIN TRY
        -- Check if the character already exists
        SELECT @v_CharacterId = CharacterId
        FROM dbo.Character
        WHERE AccountId = @p_AccountId AND Name = @p_Name;

        IF @v_CharacterId IS NOT NULL 
            BEGIN
                UPDATE dbo.Character
                SET Level = @p_Level
                ,   LastPlayed = @p_LastPlayed
                ,   Exp = @p_Exp
                ,   ServerId = @p_ServerId
                WHERE CharacterId = @v_CharacterId;

                SET @p_CharacterId = @v_CharacterId;
            END
        ELSE
            BEGIN
                INSERT INTO dbo.Character (AccountId, ServerId, Name, Level, Exp, LastPlayed, CreatedAt)
                VALUES (@p_AccountId, @p_ServerId, @p_Name, @p_Level, @p_Exp, @p_LastPlayed, GETUTCDATE());

                SET @p_CharacterId = SCOPE_IDENTITY();
            END

    END TRY
    BEGIN CATCH
        THROW
    END CATCH

    SET NOCOUNT OFF;

    RETURN @RETURN
END
GO

DROP PROCEDURE IF EXISTS dbo.usp_SelectCharacterByAccountIdAndServerId
GO

CREATE PROCEDURE dbo.usp_SelectCharacterByAccountIdAndServerId  
(
    @p_AccountId BIGINT,
    @p_ServerId INT
)
AS
BEGIN
    DECLARE @RETURN INT = 0;

    BEGIN TRY
        -- Select all characters for the given account
        SELECT CharacterId, AccountId, ServerId, Name, Level, Exp, LastPlayed, CreatedAt
        FROM dbo.Character
        WHERE AccountId = @p_AccountId AND ServerId = @p_ServerId;

    END TRY
    BEGIN CATCH
        THROW
    END CATCH

    RETURN @RETURN
END
GO

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
    @p_CreatedAt DATETIME,
    @p_ServerId INT OUTPUT
)
AS
BEGIN
    DECLARE @v_ServerId INT;
    DECLARE @RETURN INT = 0;

    SET NOCOUNT ON;

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

                SET @p_ServerId = @v_ServerId;
            END
        ELSE
            BEGIN
                INSERT INTO dbo.Server (ServerType, ServerAddress, MapName, CreatedAt, LastPingTime)
                VALUES (@p_ServerType, @p_ServerAddress, @p_MapName, @p_CreatedAt, GETUTCDATE());

                SET @p_ServerId = SCOPE_IDENTITY();
            END
    END TRY
    BEGIN CATCH
        THROW
    END CATCH

    SET NOCOUNT OFF;

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


DROP PROCEDURE IF EXISTS dbo.usp_SelectServerByServerType
GO

CREATE PROCEDURE dbo.usp_SelectServerByServerType
(
    @p_ServerType TINYINT
)
AS
BEGIN
    DECLARE @v_ServerId INT;
    DECLARE @RETURN INT = 0;

    BEGIN TRY

        -- Check if the account already exists
        SELECT ServerId
        FROM dbo.Server
        WHERE ServerType = @p_ServerType;

    END TRY
    BEGIN CATCH
        THROW
    END CATCH

    RETURN @RETURN
END
GO