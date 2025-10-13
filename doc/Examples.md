# Examples

Practical examples demonstrating how to use the C++17 Utilities Library.

## Table of Contents

1. [Singleton Examples](#singleton-examples)
2. [TaskRunner Examples](#taskrunner-examples)
3. [Combined Usage Examples](#combined-usage-examples)

---

## Singleton Examples

### Example 1: Logger Singleton

A simple logging system that maintains a single log file.

```cpp
#include "Singleton.h"
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

class Logger : public Singleton<Logger> {
    friend class Singleton<Logger>;

private:
    Logger() {
        logFile.open("application.log", std::ios::app);
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    std::ofstream logFile;
    std::mutex logMutex;

public:
    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        logFile << message << std::endl;
        logFile.flush();
    }

    void info(const std::string& message) {
        log("[INFO] " + message);
    }

    void error(const std::string& message) {
        log("[ERROR] " + message);
    }
};

// Usage
int main() {
    Logger::getInstance().info("Application started");
    Logger::getInstance().error("Something went wrong");
    Logger::getInstance().info("Application stopped");

    return 0;
}
```

### Example 2: Configuration Manager

Global configuration accessible from anywhere in the application.

```cpp
#include "Singleton.h"
#include <string>
#include <map>
#include <mutex>

class Config : public Singleton<Config> {
    friend class Singleton<Config>;

private:
    Config() {
        // Load default configuration
        settings["app_name"] = "MyApp";
        settings["version"] = "1.0.0";
        settings["debug"] = "false";
    }

    std::map<std::string, std::string> settings;
    mutable std::mutex configMutex;

public:
    void set(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(configMutex);
        settings[key] = value;
    }

    std::string get(const std::string& key) const {
        std::lock_guard<std::mutex> lock(configMutex);
        auto it = settings.find(key);
        return (it != settings.end()) ? it->second : "";
    }

    bool getBool(const std::string& key) const {
        return get(key) == "true";
    }

    void loadFromFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(configMutex);
        // Load settings from file
        // ... implementation ...
    }
};

// Usage
int main() {
    Config& config = Config::getInstance();

    config.set("debug", "true");
    config.set("max_connections", "100");

    if (config.getBool("debug")) {
        std::cout << "Debug mode enabled" << std::endl;
    }

    std::cout << "Max connections: " << config.get("max_connections") << std::endl;

    return 0;
}
```

### Example 3: Database Connection Pool

Single connection pool shared across the application.

```cpp
#include "Singleton.h"
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>

// Mock database connection
struct DBConnection {
    int id;
    void query(const std::string& sql) {
        // Execute query
    }
};

class ConnectionPool : public Singleton<ConnectionPool> {
    friend class Singleton<ConnectionPool>;

private:
    ConnectionPool() {
        // Create initial pool of connections
        for (int i = 0; i < 10; ++i) {
            availableConnections.push_back(std::make_unique<DBConnection>(DBConnection{i}));
        }
    }

    std::vector<std::unique_ptr<DBConnection>> availableConnections;
    std::vector<DBConnection*> usedConnections;
    std::mutex poolMutex;
    std::condition_variable cv;

public:
    DBConnection* acquire() {
        std::unique_lock<std::mutex> lock(poolMutex);

        // Wait for available connection
        cv.wait(lock, [this] { return !availableConnections.empty(); });

        auto conn = std::move(availableConnections.back());
        availableConnections.pop_back();

        DBConnection* ptr = conn.get();
        usedConnections.push_back(ptr);

        return ptr;
    }

    void release(DBConnection* conn) {
        std::lock_guard<std::mutex> lock(poolMutex);

        // Move back to available
        auto it = std::find(usedConnections.begin(), usedConnections.end(), conn);
        if (it != usedConnections.end()) {
            usedConnections.erase(it);
            availableConnections.push_back(std::unique_ptr<DBConnection>(conn));
            cv.notify_one();
        }
    }

    size_t availableCount() const {
        std::lock_guard<std::mutex> lock(poolMutex);
        return availableConnections.size();
    }
};

// Usage
int main() {
    ConnectionPool& pool = ConnectionPool::getInstance();

    DBConnection* conn = pool.acquire();
    conn->query("SELECT * FROM users");
    pool.release(conn);

    std::cout << "Available connections: " << pool.availableCount() << std::endl;

    return 0;
}
```

### Example 4: Application State Manager

Manage global application state.

```cpp
#include "Singleton.h"
#include <atomic>
#include <string>

class AppState : public Singleton<AppState> {
    friend class Singleton<AppState>;

private:
    AppState() : running(false), userId(0) {}

    std::atomic<bool> running;
    std::atomic<int> userId;
    std::string username;  // Note: not atomic, requires external sync

public:
    void start() { running = true; }
    void stop() { running = false; }
    bool isRunning() const { return running; }

    void setUserId(int id) { userId = id; }
    int getUserId() const { return userId; }

    // For non-atomic members, provide synchronized access
    void setUsername(const std::string& name) {
        // In real code, protect with mutex
        username = name;
    }

    std::string getUsername() const {
        return username;
    }
};

// Usage
int main() {
    AppState& state = AppState::getInstance();

    state.start();
    state.setUserId(12345);
    state.setUsername("john_doe");

    if (state.isRunning()) {
        std::cout << "User " << state.getUsername()
                  << " (ID: " << state.getUserId() << ") is active" << std::endl;
    }

    state.stop();

    return 0;
}
```

---

## TaskRunner Examples

### Example 1: Parallel File Processing

Process multiple files concurrently.

```cpp
#include "TaskRunner.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

void processFile(const std::string& filename) {
    std::ifstream file(filename);
    // Process file contents
    std::cout << "Processing " << filename << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Finished " << filename << std::endl;
}

int main() {
    TaskRunner runner;

    std::vector<std::string> files = {
        "file1.txt", "file2.txt", "file3.txt",
        "file4.txt", "file5.txt"
    };

    // Process all files in parallel
    for (const auto& file : files) {
        runner.executeTask([file]() {
            processFile(file);
        });
    }

    std::cout << "All tasks submitted, waiting for completion..." << std::endl;
    runner.waitForCompletion();
    std::cout << "All files processed!" << std::endl;

    return 0;
}
```

### Example 2: Periodic Health Check

Monitor system health every second.

```cpp
#include "TaskRunner.h"
#include <iostream>
#include <atomic>

std::atomic<int> systemLoad{0};

void checkSystemHealth() {
    // Simulate health check
    systemLoad = rand() % 100;
    std::cout << "System load: " << systemLoad << "%" << std::endl;

    if (systemLoad > 80) {
        std::cout << "WARNING: High system load!" << std::endl;
    }
}

int main() {
    TaskRunner runner;

    std::cout << "Starting health monitor (will run for 10 seconds)..." << std::endl;

    // Check health every second, 10 times
    runner.executeRepeatedTask(
        checkSystemHealth,
        std::chrono::seconds(1),
        10
    );

    runner.waitForCompletion();
    std::cout << "Health monitoring completed" << std::endl;

    return 0;
}
```

### Example 3: Background Data Sync

Continuously sync data in the background.

```cpp
#include "TaskRunner.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

std::atomic<int> dataCounter{0};

void syncData() {
    dataCounter++;
    std::cout << "Syncing data... (sync #" << dataCounter << ")" << std::endl;
    // Simulate network request
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

int main() {
    TaskRunner runner;

    std::cout << "Starting background sync..." << std::endl;

    // Sync every 500ms indefinitely
    runner.executeRepeatedTask(
        syncData,
        std::chrono::milliseconds(500),
        0  // 0 = infinite
    );

    // Simulate main application work
    std::cout << "Main application running..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Stop background sync
    std::cout << "Stopping background sync..." << std::endl;
    runner.stopAll();
    runner.waitForCompletion();

    std::cout << "Total syncs: " << dataCounter << std::endl;

    return 0;
}
```

### Example 4: Task Pipeline

Execute tasks in stages with dependencies.

```cpp
#include "TaskRunner.h"
#include <iostream>
#include <atomic>
#include <vector>

std::atomic<bool> stage1Complete{false};
std::atomic<bool> stage2Complete{false};
std::vector<int> processedData;

void stage1() {
    std::cout << "Stage 1: Loading data..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stage1Complete = true;
    std::cout << "Stage 1: Complete" << std::endl;
}

void stage2() {
    std::cout << "Stage 2: Processing data..." << std::endl;
    while (!stage1Complete) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stage2Complete = true;
    std::cout << "Stage 2: Complete" << std::endl;
}

void stage3() {
    std::cout << "Stage 3: Saving results..." << std::endl;
    while (!stage2Complete) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Stage 3: Complete" << std::endl;
}

int main() {
    TaskRunner runner;

    std::cout << "Starting pipeline..." << std::endl;

    // Submit all stages (they'll wait for dependencies)
    runner.executeTask(stage3);
    runner.executeTask(stage2);
    runner.executeTask(stage1);

    runner.waitForCompletion();
    std::cout << "Pipeline complete!" << std::endl;

    return 0;
}
```

### Example 5: Rate-Limited API Calls

Make API calls at a controlled rate.

```cpp
#include "TaskRunner.h"
#include <iostream>
#include <atomic>
#include <vector>
#include <string>

std::atomic<int> apiCallCount{0};
std::vector<std::string> endpoints = {
    "/api/users", "/api/products", "/api/orders",
    "/api/analytics", "/api/reports"
};

void makeApiCall(const std::string& endpoint) {
    apiCallCount++;
    std::cout << "API Call #" << apiCallCount << " to " << endpoint << std::endl;
    // Simulate API request
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

int main() {
    TaskRunner runner;

    std::cout << "Making rate-limited API calls..." << std::endl;

    // Make one API call every 200ms
    int currentEndpoint = 0;
    runner.executeRepeatedTask(
        [&currentEndpoint]() {
            if (currentEndpoint < endpoints.size()) {
                makeApiCall(endpoints[currentEndpoint]);
                currentEndpoint++;
            }
        },
        std::chrono::milliseconds(200),
        endpoints.size()
    );

    runner.waitForCompletion();
    std::cout << "All API calls completed" << std::endl;

    return 0;
}
```

### Example 6: Auto-Save Feature

Automatically save application state every few seconds.

```cpp
#include "TaskRunner.h"
#include <iostream>
#include <string>
#include <fstream>
#include <mutex>

class Document {
private:
    std::string content;
    bool modified;
    mutable std::mutex docMutex;

public:
    Document() : modified(false) {}

    void append(const std::string& text) {
        std::lock_guard<std::mutex> lock(docMutex);
        content += text;
        modified = true;
    }

    void save() {
        std::lock_guard<std::mutex> lock(docMutex);
        if (modified) {
            std::cout << "Saving document... (" << content.length() << " bytes)" << std::endl;
            std::ofstream file("document.txt");
            file << content;
            file.close();
            modified = false;
            std::cout << "Document saved" << std::endl;
        }
    }
};

int main() {
    Document doc;
    TaskRunner runner;

    // Auto-save every 2 seconds
    runner.executeRepeatedTask(
        [&doc]() { doc.save(); },
        std::chrono::seconds(2),
        0  // Infinite
    );

    // Simulate user typing
    doc.append("Hello ");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    doc.append("World! ");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    doc.append("More text here.");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Stop auto-save
    runner.stopAll();
    doc.save();  // Final save
    runner.waitForCompletion();

    return 0;
}
```

---

## Combined Usage Examples

### Example 7: Web Server with Singleton and TaskRunner

Combine both utilities for a simple web server simulation.

```cpp
#include "Singleton.h"
#include "TaskRunner.h"
#include <iostream>
#include <string>
#include <atomic>
#include <map>
#include <mutex>

// Logger singleton
class Logger : public Singleton<Logger> {
    friend class Singleton<Logger>;
private:
    Logger() {}
    std::mutex logMutex;
public:
    void log(const std::string& msg) {
        std::lock_guard<std::mutex> lock(logMutex);
        std::cout << "[LOG] " << msg << std::endl;
    }
};

// Stats singleton
class ServerStats : public Singleton<ServerStats> {
    friend class Singleton<ServerStats>;
private:
    ServerStats() : requestCount(0), errorCount(0) {}
    std::atomic<int> requestCount;
    std::atomic<int> errorCount;
public:
    void incrementRequests() { requestCount++; }
    void incrementErrors() { errorCount++; }
    int getRequests() const { return requestCount; }
    int getErrors() const { return errorCount; }
};

// Simulate handling a request
void handleRequest(int requestId) {
    Logger::getInstance().log("Handling request #" + std::to_string(requestId));
    ServerStats::getInstance().incrementRequests();

    // Simulate work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Simulate occasional error
    if (requestId % 7 == 0) {
        ServerStats::getInstance().incrementErrors();
        Logger::getInstance().log("Error in request #" + std::to_string(requestId));
    }
}

// Print stats periodically
void printStats() {
    auto& stats = ServerStats::getInstance();
    Logger::getInstance().log(
        "Stats - Requests: " + std::to_string(stats.getRequests()) +
        ", Errors: " + std::to_string(stats.getErrors())
    );
}

int main() {
    TaskRunner runner;

    Logger::getInstance().log("Server starting...");

    // Print stats every second
    runner.executeRepeatedTask(
        printStats,
        std::chrono::seconds(1),
        0  // Infinite
    );

    // Simulate incoming requests
    for (int i = 1; i <= 20; ++i) {
        runner.executeTask([i]() {
            handleRequest(i);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Wait for all requests to complete
    std::this_thread::sleep_for(std::chrono::seconds(3));

    Logger::getInstance().log("Server shutting down...");
    runner.stopAll();
    runner.waitForCompletion();

    // Final stats
    printStats();

    return 0;
}
```

### Example 8: Resource Manager with Lazy Initialization

Manage resources with singleton pattern and async cleanup.

```cpp
#include "Singleton.h"
#include "TaskRunner.h"
#include <iostream>
#include <vector>
#include <memory>
#include <mutex>

struct Resource {
    int id;
    std::string name;
    bool active;
};

class ResourceManager : public Singleton<ResourceManager> {
    friend class Singleton<ResourceManager>;

private:
    ResourceManager() : nextId(1) {
        std::cout << "ResourceManager initialized" << std::endl;
    }

    ~ResourceManager() {
        std::cout << "ResourceManager shutting down..." << std::endl;
        cleanup();
    }

    std::vector<std::unique_ptr<Resource>> resources;
    std::mutex resourceMutex;
    int nextId;

public:
    int createResource(const std::string& name) {
        std::lock_guard<std::mutex> lock(resourceMutex);
        int id = nextId++;
        resources.push_back(std::make_unique<Resource>(Resource{id, name, true}));
        std::cout << "Created resource: " << name << " (ID: " << id << ")" << std::endl;
        return id;
    }

    void deactivateResource(int id) {
        std::lock_guard<std::mutex> lock(resourceMutex);
        for (auto& res : resources) {
            if (res->id == id) {
                res->active = false;
                std::cout << "Deactivated resource ID: " << id << std::endl;
                break;
            }
        }
    }

    void cleanup() {
        std::lock_guard<std::mutex> lock(resourceMutex);
        std::cout << "Cleaning up " << resources.size() << " resources..." << std::endl;
        resources.clear();
    }

    size_t activeCount() const {
        std::lock_guard<std::mutex> lock(resourceMutex);
        return std::count_if(resources.begin(), resources.end(),
            [](const auto& res) { return res->active; });
    }
};

int main() {
    TaskRunner runner;
    std::vector<int> resourceIds;

    // Create resources in parallel
    std::cout << "Creating resources..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        runner.executeTask([i, &resourceIds]() {
            int id = ResourceManager::getInstance().createResource("Resource_" + std::to_string(i));
            resourceIds.push_back(id);
        });
    }

    runner.waitForCompletion();

    std::cout << "Active resources: " << ResourceManager::getInstance().activeCount() << std::endl;

    // Deactivate some resources asynchronously
    std::cout << "\nDeactivating some resources..." << std::endl;
    for (size_t i = 0; i < resourceIds.size(); i += 2) {
        runner.executeTask([id = resourceIds[i]]() {
            ResourceManager::getInstance().deactivateResource(id);
        });
    }

    runner.waitForCompletion();

    std::cout << "Active resources: " << ResourceManager::getInstance().activeCount() << std::endl;

    return 0;
}
```

### Example 9: Event System

Event dispatcher using both patterns.

```cpp
#include "Singleton.h"
#include "TaskRunner.h"
#include <iostream>
#include <functional>
#include <vector>
#include <mutex>
#include <string>

class EventDispatcher : public Singleton<EventDispatcher> {
    friend class Singleton<EventDispatcher>;

private:
    EventDispatcher() {}

    using EventHandler = std::function<void(const std::string&)>;
    std::map<std::string, std::vector<EventHandler>> handlers;
    std::mutex handlerMutex;
    TaskRunner taskRunner;

public:
    void subscribe(const std::string& eventType, EventHandler handler) {
        std::lock_guard<std::mutex> lock(handlerMutex);
        handlers[eventType].push_back(handler);
        std::cout << "Subscribed to event: " << eventType << std::endl;
    }

    void emit(const std::string& eventType, const std::string& data) {
        std::lock_guard<std::mutex> lock(handlerMutex);
        auto it = handlers.find(eventType);
        if (it != handlers.end()) {
            for (const auto& handler : it->second) {
                // Execute each handler asynchronously
                taskRunner.executeTask([handler, data]() {
                    handler(data);
                });
            }
        }
    }

    void waitForEvents() {
        taskRunner.waitForCompletion();
    }
};

int main() {
    auto& dispatcher = EventDispatcher::getInstance();

    // Subscribe to events
    dispatcher.subscribe("user_login", [](const std::string& user) {
        std::cout << "Handler 1: User logged in: " << user << std::endl;
    });

    dispatcher.subscribe("user_login", [](const std::string& user) {
        std::cout << "Handler 2: Logging user activity for: " << user << std::endl;
    });

    dispatcher.subscribe("data_updated", [](const std::string& data) {
        std::cout << "Data updated: " << data << std::endl;
    });

    // Emit events
    std::cout << "\nEmitting events..." << std::endl;
    dispatcher.emit("user_login", "john_doe");
    dispatcher.emit("data_updated", "user_profile");
    dispatcher.emit("user_login", "jane_smith");

    // Wait for all event handlers
    dispatcher.waitForEvents();

    return 0;
}
```

---

## Compilation Commands

### Singleton Examples (Header-Only)

```bash
g++ -std=c++17 -I./include example.cpp -o example -pthread
```

### TaskRunner Examples

```bash
# Build library first
make

# Compile example
g++ -std=c++17 -I./include example.cpp -L./lib -ltaskrunner -o example -pthread
```

### Combined Examples

```bash
# Build library first
make

# Compile combined example
g++ -std=c++17 -I./include combined_example.cpp -L./lib -ltaskrunner -o combined_example -pthread
```

---

## Tips for Using Examples

1. **Error Handling**: Production code should add proper error handling
2. **Resource Management**: Examples use simplified resource management
3. **Thread Safety**: Always protect shared state with mutexes or atomics
4. **Testing**: Test concurrent code thoroughly with various timing scenarios
5. **Performance**: Profile before optimizing for your specific use case
