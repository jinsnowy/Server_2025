@echo off

SET INCLUDE_PATH=../vcpkg/vcpkg/installed/x64-windows/include
SET PROTO_SRC_PATH=./Proto
SET OUTPUT_PATH=./Source/Protobuf/Private

echo COMPILING PROTO FILES TO %OUTPUT_PATH%/...

protoc.exe --proto_path=%INCLUDE_PATH%;%PROTO_SRC_PATH% --cpp_out=%OUTPUT_PATH% ./Proto/*.proto

echo FINISHED

PAUSE