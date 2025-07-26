
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
        SELECT ServerId, ServerType, ServerAddress, MapName, CreatedAt, LastPingTime
        FROM dbo.Server
        WHERE ServerType = @p_ServerType;

    END TRY
    BEGIN CATCH
        THROW
    END CATCH

    RETURN @RETURN
END
GO