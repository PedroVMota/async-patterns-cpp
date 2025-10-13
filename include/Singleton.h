#ifndef SINGLETON_H
#define SINGLETON_H

#include <memory>
#include <mutex>

template <typename T>
class Singleton {
protected:
    Singleton() = default;
    virtual ~Singleton() = default;

    // Delete copy constructor and assignment operator
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    // Delete move constructor and move assignment operator
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

public:
    static T& getInstance() {
        std::call_once(initFlag, &Singleton::initSingleton);
        return *instance;
    }

private:
    static void initSingleton() {
        instance.reset(new T());
    }

    static std::unique_ptr<T> instance;
    static std::once_flag initFlag;
};

// Static member initialization
template <typename T>
std::unique_ptr<T> Singleton<T>::instance = nullptr;

template <typename T>
std::once_flag Singleton<T>::initFlag;

#endif // SINGLETON_H
