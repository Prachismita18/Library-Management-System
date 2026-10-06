# 🧬 Object-Oriented Programming (OOP) Concepts in C++17

This document provides a detailed breakdown of the **Object-Oriented Programming (OOP)** principles and modern C++ design patterns implemented throughout the **Library Management System**.

---

## 🏛️ 1. Classes and Objects

A **Class** is a user-defined blueprint containing attributes (member variables) and behaviors (member functions). An **Object** is an instance of a class.

### Code Implementation
In `include/Book.hpp` and `src/Book.cpp`:
```cpp
class Book {
private:
    int id;
    std::string title;
    std::string author;
    std::string isbn;
    int availableCopies;
    int totalCopies;
    double price;

public:
    Book(int id, const std::string& title, ...);
    bool isAvailable() const;
    int getAvailableCopies() const;
};
```
- `Book` serves as a blueprint.
- Instantiations like `Book newBook(...)` create discrete objects storing individual book inventory states.

---

## 🔒 2. Encapsulation and Data Hiding

**Encapsulation** bundles data and functions operating on that data into a single unit while restricting direct external access to internal state (**Data Hiding**).

### Code Implementation
- Private members (`private:`) protect internal data from unauthorized modifications.
- Public getter and setter methods enforce validation rules:
```cpp
// In Book.cpp
void Book::setAvailableCopies(int copies) {
    if (copies >= 0 && copies <= totalCopies) {
        availableCopies = copies;
    }
}
```

---

## 🌳 3. Inheritance

**Inheritance** enables a derived class to inherit properties and behaviors from a base class, promoting code reuse and establishing an "IS-A" relationship.

### Code Implementation
In `include/User.hpp`, `include/Admin.hpp`, and `include/Member.hpp`:

```
                 +-------------------+
                 |    User (Base)    |
                 +-------------------+
                           |
            +--------------+--------------+
            |                             |
  +-------------------+         +-------------------+
  |  Admin (Derived)  |         | Member (Derived)  |
  +-------------------+         +-------------------+
```

```cpp
// Base Class
class User {
protected:
    int id;
    std::string username;
    std::string fullName;
    UserRole role;
    ...
};

// Derived Classes
class Admin : public User {
private:
    std::string adminLevel;
public:
    Admin(...);
};

class Member : public User {
private:
    std::string membershipType;
    int maxBooksAllowed;
public:
    Member(...);
};
```
- `Admin` IS-A `User`.
- `Member` IS-A `User`.

---

## 🎭 4. Polymorphism and Virtual Functions

**Polymorphism** ("many forms") allows objects of different classes to be treated as objects of a common base class. **Dynamic Polymorphism** is achieved using `virtual` functions and dynamic binding via virtual table pointers (vptr/vtable).

### Code Implementation
In `User.hpp`:
```cpp
class User {
public:
    virtual void displayInfo() const;
    virtual ~User() = default; // Virtual Destructor for safe dynamic deletion
};
```
In `Admin.cpp` and `Member.cpp`:
```cpp
void Admin::displayInfo() const override {
    std::cout << "=== ADMIN PROFILE ===\n";
    User::displayInfo();
    std::cout << "Admin Level: " << adminLevel << "\n";
}

void Member::displayInfo() const override {
    std::cout << "=== MEMBER PROFILE ===\n";
    User::displayInfo();
    std::cout << "Max Books: " << maxBooksAllowed << "\n";
}
```
**Dynamic Execution**:
```cpp
std::shared_ptr<User> userPtr = authManager.login(username, password);
userPtr->displayInfo(); // Dynamically executes Admin::displayInfo or Member::displayInfo based on actual runtime object type!
```

---

## 🎨 5. Abstraction

**Abstraction** hides complex background implementation details and exposes only essential interfaces to the user.

### Code Implementation
`UI.cpp` calls simple business methods:
```cpp
library.issueBook(bookId, memberId);
```
Behind this simple function call, `Library` and `DatabaseManager` handle:
- SQLite connection initialization
- Multi-table validation checks
- SQL statement preparation and parameter binding
- Database transaction handling (`BEGIN`, `COMMIT`, `ROLLBACK`)

All these lower-level storage and SQL complexities are completely abstracted away from the UI layer.

---

## 📦 6. RAII (Resource Acquisition Is Initialization)

**RAII** ties resource management (file descriptors, sockets, database handles) to object lifetime.

### Code Implementation
In `DatabaseManager`:
```cpp
DatabaseManager::DatabaseManager(const std::string& path) : db(nullptr) {
    // Resource acquired in constructor
}

DatabaseManager::~DatabaseManager() {
    close(); // Resource released automatically in destructor when object leaves scope
}
```

---

## 🧠 7. Modern C++ Memory Management & Smart Pointers

Instead of raw C-style pointers (`User*`) susceptible to memory leaks, the system leverages modern C++17 memory features:

1. **`std::shared_ptr<User>`**: Reference-counted smart pointer managing dynamically allocated `Admin` and `Member` objects.
2. **`std::optional<Book>`**: Type-safe wrapper indicating the presence or absence of a return value without using null pointers.
3. **`std::unique_ptr`**: Used for scoped resources.

---

## 🛠️ 8. Standard Template Library (STL)

The application utilizes standard template containers and algorithms:
- `std::vector<Book>`, `std::vector<IssueRecord>`, `std::vector<Fine>` for dynamic collections.
- `std::string` and stringstream operations for formatting.
- `std::chrono` and `std::tm` for date arithmetic.
- `<algorithm>` functions like `std::transform` and `std::max`.
