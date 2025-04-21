#pragma once


namespace System { 

template<typename T>
class Singleton {
public:
    static T& GetInstance() {
        static T instance;
        return instance;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;
};

}
