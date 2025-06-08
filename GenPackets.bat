@echo off

SET CURRENT_PATH=%CD%
SET INCLUDE_PATH=../Library/include
SET PROTO_SRC_PATH=./Proto
SET OUTPUT_PATH=./Source/Protobuf/Private/Protos

SET UE_PROJECT_PATH=C:\Users\yoon\Documents\Unreal Projects\ChroniclesOfAetheria\Source\ChroniclesOfAetheria\Protocol

echo COMPILING PROTO FILES TO %OUTPUT_PATH%/...

%CD%\Bin\protoc.exe --proto_path=%INCLUDE_PATH%;%PROTO_SRC_PATH% --cpp_out=%OUTPUT_PATH% ./Proto/*.proto

echo COPY PROTO FILES %OUTPUT_PATH% TO "%UE_PROJECT_PATH%

@rem xcopy "%OUTPUT_PATH%" "%UE_PROJECT_PATH%" /Y /D

echo FINISHED

PAUSE