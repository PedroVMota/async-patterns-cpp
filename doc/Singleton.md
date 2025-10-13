# Singleton Pattern Implementation

## Overview

The Singleton pattern ensures that a class has only one instance throughout the application lifecycle and provides a global access point to that instance. This implementation uses the CRTP (Curiously Recurring Template Pattern) to provide a reusable, thread-safe singleton base class.

## Design Pattern

### CRTP (Curiously Recurring Template Pattern)

The Singleton class is a template that takes the derived class as a template parameter:

```cpp
template <typename T>
class Singleton { ... };

class MyClass : public Singleton<MyClass> { ... };
```

This pattern allows the base class to create instances of the derived class while maintaining type safety.

## Thread Safety

### Initialization Mechanism

The implementation uses `std::call_once` with `std::once_flag` to ensure thread-safe initialization:

```cpp
static T& getInstance() {
    std::call_once(initFlag, &Singleton::initSingleton);
    return *instance;
}
```

**Key Features:**
- **Thread-Safe**: Multiple threads can safely call `getInstance()` simultaneously
- **Lazy Initialization**: The instance is created only when first accessed
- **No Race Conditions**: `std::call_once` guarantees the initialization function runs exactly once
- **Memory Efficient**: No overhead when instance is not used

### Why `std::call_once`?

Compared to other thread-safety mechanisms:

1. **Better than Double-Checked Locking**: Avoids subtle memory ordering issues
2. **Better than Static Local Variables**: More explicit control over initialization
3. **Better than Eager Initialization**: No unnecessary resource allocation
4. **Standard Library**: Portable and well-tested

## Memory Management

The singleton uses `std::unique_ptr<T>` for automatic memory management:

```cpp
static std::unique_ptr<T> instance;
```

**Benefits:**
- Automatic cleanup when program exits
- No memory leaks
- Exception-safe
- Clear ownership semantics

## Class Structure

### Protected Constructor

```cpp
protected:
    Singleton() = default;
    virtual ~Singleton() = default;
```

The constructor is protected to:
- Prevent direct instantiation
- Allow derived class construction
- Enable polymorphic behavior if needed

### Deleted Copy/Move Operations

```cpp
Singleton(const Singleton&) = delete;
Singleton& operator=(const Singleton&) = delete;
Singleton(Singleton&&) = delete;
Singleton& operator=(Singleton&&) = delete;
```

These deletions ensure:
- No accidental copying
- No moving of singleton instances
- Compiler errors if misused
- Clear intent to users

## Usage Patterns

### Basic Singleton Class

```cpp
#include "Singleton.h"

class DatabaseConnection : public Singleton<DatabaseConnection> {
    friend class Singleton<DatabaseConnection>;

private:
    DatabaseConnection() {
        // Initialize database connection
    }

    std::string connectionString;

public:
    void connect(const std::string& connStr) {
        connectionString = connStr;
        // Connect to database
    }

    void query(const std::string& sql) {
        // Execute query
    }
};
```

### Accessing the Singleton

```cpp
// Get instance and use it
DatabaseConnection& db = DatabaseConnection::getInstance();
db.connect("localhost:5432");
db.query("SELECT * FROM users");

// Or use directly
DatabaseConnection::getInstance().query("SELECT * FROM products");
```

### Multiple Singleton Types

Each class that inherits from `Singleton<T>` gets its own independent instance:

```cpp
class Logger : public Singleton<Logger> { ... };
class Config : public Singleton<Config> { ... };

Logger& log = Logger::getInstance();    // Independent instance
Config& cfg = Config::getInstance();    // Different independent instance
```

## Implementation Details

### Static Member Initialization

```cpp
template <typename T>
std::unique_ptr<T> Singleton<T>::instance = nullptr;

template <typename T>
std::once_flag Singleton<T>::initFlag;
```

- Each instantiation of `Singleton<T>` gets its own static members
- Initialized to `nullptr` and default-constructed flag
- Separate storage for each derived type

### Instance Creation

```cpp
static void initSingleton() {
    instance.reset(new T());
}
```

- Called exactly once by `std::call_once`
- Uses `new` to create the derived class instance
- Stored in `unique_ptr` for automatic cleanup
- Private constructor of `T` is accessible because `Singleton<T>` is a friend

## Best Practices

### 1. Make Constructor Private

```cpp
class MyClass : public Singleton<MyClass> {
    friend class Singleton<MyClass>;  // Required!

private:
    MyClass() { }  // Must be private
};
```

### 2. Avoid Complex Initialization

Keep the constructor simple. Use separate initialization methods if needed:

```cpp
class Service : public Singleton<Service> {
    friend class Singleton<Service>;

private:
    Service() { }  // Simple constructor

public:
    void initialize(const Config& config) {
        // Complex initialization here
    }
};

// Usage
Service::getInstance().initialize(myConfig);
```

### 3. Thread-Safe Member Access

The singleton creation is thread-safe, but member access is not automatically protected:

```cpp
class Counter : public Singleton<Counter> {
    friend class Singleton<Counter>;

private:
    Counter() : count(0) { }
    std::atomic<int> count;  // Use atomic for thread-safe access

public:
    void increment() { count++; }
    int getCount() const { return count; }
};
```

### 4. Avoid Singleton Dependencies

Be careful with singletons that depend on other singletons:

```cpp
// Potentially problematic
class Logger : public Singleton<Logger> { ... };
class Database : public Singleton<Database> {
private:
    Database() {
        // Careful: Logger might not be initialized yet
        Logger::getInstance().log("Database created");
    }
};
```

## Common Pitfalls

### 1. Forgetting Friend Declaration

```cpp
// WRONG - won't compile
class MyClass : public Singleton<MyClass> {
private:
    MyClass() { }  // Singleton can't access this!
};

// CORRECT
class MyClass : public Singleton<MyClass> {
    friend class Singleton<MyClass>;  // Now Singleton can create instance
private:
    MyClass() { }
};
```

### 2. Public Constructor

```cpp
// WRONG - defeats the purpose
class MyClass : public Singleton<MyClass> {
public:
    MyClass() { }  // Anyone can create instances!
};
```

### 3. Trying to Copy/Move

```cpp
// These will cause compiler errors (as designed)
MyClass instance = MyClass::getInstance();  // Error: deleted copy constructor
MyClass* ptr = new MyClass();                // Error: private constructor
```

## Performance Considerations

### Initialization Cost

- **First Access**: Slight overhead from `std::call_once` (~few nanoseconds)
- **Subsequent Accesses**: Just a pointer dereference (very fast)
- **Memory**: One instance per singleton type + smart pointer overhead

### When to Use Singletons

**Good Use Cases:**
- Logging systems
- Configuration managers
- Resource pools
- Hardware interface abstractions
- Application-wide caches

**Avoid When:**
- State should be scoped to specific contexts
- Testing requires multiple instances
- Dependency injection is more appropriate
- The class represents a value object

## Testing Considerations

Singletons can make testing difficult because:
1. State persists between tests
2. Cannot easily mock or substitute implementations
3. Hidden dependencies

**Mitigation Strategies:**

```cpp
// 1. Provide reset capability (for testing only)
class MyClass : public Singleton<MyClass> {
    friend class Singleton<MyClass>;

    #ifdef TESTING
    friend class MyClassTest;
    static void resetForTesting() {
        instance.reset();
        initFlag = std::once_flag();
    }
    #endif
};

// 2. Use dependency injection where possible
class Service {
    Logger& logger;
public:
    Service(Logger& log) : logger(log) { }  // Can inject mock logger
};
```

## Comparison with Other Singleton Implementations

### Meyer's Singleton (Static Local Variable)

```cpp
static T& getInstance() {
    static T instance;  // Thread-safe in C++11+
    return instance;
}
```

**Pros:**
- Simpler implementation
- Automatic initialization

**Cons:**
- Less control over initialization timing
- Harder to handle initialization errors
- Destructor order issues with multiple singletons

### Our Implementation (CRTP + call_once)

**Pros:**
- Explicit control over initialization
- Clear error handling
- Predictable behavior
- Can add reset capabilities
- Reusable template

**Cons:**
- More code
- Requires friend declaration

## Technical Specifications

- **C++ Standard**: C++17 or later
- **Header Dependencies**: `<memory>`, `<mutex>`
- **Thread Safety**: Full
- **Memory Management**: Automatic via `std::unique_ptr`
- **Initialization**: Lazy (on first access)
- **Overhead**: Minimal (one pointer + one flag per type)
