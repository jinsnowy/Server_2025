#include "stdafx.h"

#ifdef _WIN32
#include <windows.h>
#define DLL_HANDLE HMODULE
#define LOAD_DLL(path) LoadLibrary(path)
#define UNLOAD_DLL(handle) FreeLibrary(handle)
#define GET_DLL_ERROR() GetLastError()
#else
#include <dlfcn.h>
#define DLL_HANDLE void*
#define LOAD_DLL(path) dlopen(path, RTLD_LAZY)
#define UNLOAD_DLL(handle) dlclose(handle)
#define GET_DLL_ERROR() dlerror()
#endif

// 크로스 플랫폼 DLL/SO 로드 함수
DLL_HANDLE LoadDynamicLib(const char* libPath) {
    DLL_HANDLE handle = LOAD_DLL(libPath);
    if (handle == nullptr) {
        std::cout << "동적 라이브러리 로드 실패: " << GET_DLL_ERROR() << std::endl;
        return nullptr;
    }
    std::cout << "동적 라이브러리 로드 성공" << std::endl;
    return handle;
}

// 크로스 플랫폼 DLL/SO 언로드 함수 
void UnloadDynamicLib(DLL_HANDLE handle) {
    if (handle != nullptr) {
        UNLOAD_DLL(handle);
        std::cout << "동적 라이브러리 언로드 완료" << std::endl;
    }
}

// DLL에서 함수 포인터 가져오기
template<typename T>
T GetFunction(DLL_HANDLE handle, const char* funcName) {
#ifdef _WIN32
    return reinterpret_cast<T>(GetProcAddress(handle, funcName));
#else
    return reinterpret_cast<T>(dlsym(handle, funcName));
#endif
}

int session_id_counter_ = 0;

class HelloSession : public Network::Session {
public:
    HelloSession(std::shared_ptr<Network::Connection> connection)
        :
        Network::Session(connection) {
        session_id_ = session_id_counter_++;
    }

    void OnConnect() override {
        std::cout << "HelloSession::OnConnect" << session_id_ << std::endl;
    }

    void OnDisconnect() override {
        std::cout << "HelloSession::OnDisconnect" << session_id_ << std::endl;
    }

private:
    int session_id_;
};

std::vector<std::shared_ptr<HelloSession>> hello_sessions_;

int main() {

    System::Scheduler::Launch(4);

    Network::SessionFactory session_factory;
    session_factory.SetSessionClass<HelloSession>();
    session_factory.OnConnect([](std::shared_ptr<Network::Session> session) {
        session->Send("Hello, World!");
        hello_sessions_.push_back(std::static_pointer_cast<HelloSession>(session));
        return true;
    });

    Network::Listener listener(System::Scheduler::RoundRobin().GetIoContext(), session_factory);
    listener.Bind("0.0.0.0", 8080);
    listener.Start();

    return 0;
}