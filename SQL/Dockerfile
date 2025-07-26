# Dockerfile
FROM mcr.microsoft.com/mssql/server:2022-latest

# Copy your init.sql script into the container
# The /usr/src/app directory is a common convention
COPY init.sql /usr/src/app/init.sql

# Grant execute permissions to the script (optional, but good practice if it's a shell script)
# If init.sql is a SQL script, this is not strictly necessary for execution via sqlcmd
# RUN chmod +x /usr/src/app/init.sql

# Execute the script when the container starts
# This assumes sqlcmd is available in the image, which it typically is for MSSQL images
CMD /opt/mssql-tools/bin/sqlcmd -S localhost -U SA -P "StrongPass1!" -i /usr/src/app/init.sql