#pragma once


namespace System { 

template<typename T>
class Singleton {
public:
    static T& GetInstance() {
        static T instance;
        return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

protected:
    Singleton() = default;
    ~Singleton() = default;
};

template<typename T>
class SingletonShared  : public std::enable_shared_from_this<T> {
public:
    static T& GetInstance() {
        static SharedInstance instance;
        return *instance;
    }

    static std::shared_ptr<T> GetSharedInstance() {
		return GetInstance().shared_from_this();
	}

    SingletonShared(const SingletonShared&) = delete;
    SingletonShared& operator=(const SingletonShared&) = delete;

    SingletonShared(SingletonShared&&) = delete;
    SingletonShared& operator=(SingletonShared&&) = delete;

protected:
    SingletonShared() = default;
    ~SingletonShared() = default;

private:
    struct SharedInstance {
		std::shared_ptr<T> instance;
        SharedInstance() : instance(std::make_shared<T>()) {}

        T& operator*() {
			return *instance;
		}
	};
};

}
