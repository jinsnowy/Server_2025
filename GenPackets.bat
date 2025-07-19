@echo off

SET CURRENT_PATH=%CD%
SET INCLUDE_PATH=../Library/source-build/static-md/include
SET PROTO_SRC_PATH=./Proto
SET PROTO_GRPC_SRC_PATH=./Proto
SET OUTPUT_PATH=./Source/Protobuf/Private/Protos
SET OUTPUT_GRPC_PATH=./Source/Protobuf/Private/Protos

SET UE_PROJECT_PATH=C:\Users\hyojiny1\Documents\Unreal Projects\ChroniclesOfAetheria\Source\ChroniclesOfAetheria\Protocol

if not exist "%OUTPUT_PATH%" (
    mkdir "%OUTPUT_PATH%"
    echo Directory %OUTPUT_PATH%" created successfully.
)
if not exist "%OUTPUT_GRPC_PATH%" (
    mkdir "%OUTPUT_GRPC_PATH%"
    echo Directory %OUTPUT_GRPC_PATH%" created successfully.
)

echo COMPILING PROTO FILES TO %OUTPUT_PATH%/...


SET PROTOC=%CD%\Bin\protoc.exe
SET PROTOC_GEN_GRPC=%CD%\Bin\grpc_cpp_plugin.exe

echo COMPILING PROTO FILES ...

%PROTOC% --proto_path=%INCLUDE_PATH%;%PROTO_SRC_PATH% --cpp_out=%OUTPUT_PATH% ./Proto/*.proto

echo COPY PROTO FILES %OUTPUT_PATH% TO "%UE_PROJECT_PATH%

xcopy "%OUTPUT_PATH%" "%UE_PROJECT_PATH%" /Y /D

echo COMPILING GRPC FILES ...

%PROTOC% --proto_path=%INCLUDE_PATH%;%PROTO_GRPC_SRC_PATH% --grpc_out=%OUTPUT_PATH% --plugin=protoc-gen-grpc=%PROTOC_GEN_GRPC% ./Proto/*grpc.proto



echo FINISHED

PAUSE