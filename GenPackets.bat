protoc.exe -I=./Proto/ --cpp_out=./Source/Protocol/Private ./Proto/Enum.proto
protoc.exe -I=./Proto/ --cpp_out=./Source/Protocol/Private ./Proto/Struct.proto
protoc.exe -I=./Proto/ --cpp_out=./Source/Protocol/Private ./Proto/User.proto
IF ERRORLEVEL 1 PAUSE