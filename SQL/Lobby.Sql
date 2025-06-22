CREATE DATABASE IF NOT EXISTS 'LobbyDb'

USE LobbyDb

CREATE TABLE IF NOT EXISTS 'Account'
(
    AccountID BIGINT NOT NULL AUTO_INCREMENT,
    UserId NVARCHAR(64) NOT NULL UNIQUE,
    Username NVARCHAR(64) NOT NULL,
)
GO

DROP PROCEDURE IF EXISTS 'usp_InsertAccount'
GO

CREATE PROCEDURE 'usp_UpsertAccount'
(
    p_UserId NVARCHAR(64) NOT NULL,
    p_Username NVARCHAR(64) NOT NULL,
    OUT p_AccountID BIGINT
)
BEGIN
    DECLARE v_AccountID BIGINT;
    DECLARE @RETURN INT DEFAULT 0;

    BEGIN TRY
        -- Check if the account already exists
        SELECT AccountID INTO v_AccountID
        FROM Account
        WHERE UserId = p_UserId;

        IF v_AccountID IS NOT NULL 
            BEGIN
                UPDATE Account
                SET Username = p_Username
                WHERE AccountID = v_AccountID
            END
        ELSE
            BEGIN
                -- If it does not exist, insert a new account
                INSERT INTO Account (UserId, Username)
                VALUES (p_UserId, p_Username)
            END
        END

    END TRY
    BEGIN CATCH
        THROW
    END CATCH

    RETURN @RETURN
END
GO

