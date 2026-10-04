# 📚 Library Management System in C++17 & SQLite3

A high-performance, object-oriented Library Management System built with **C++17**, standard **SQLite3** relational persistence, and interactive terminal UI. Designed as an academic showcase project for college presentation and viva voce.

---

## 🌟 Key Features

### 🔐 1. Role-Based Authentication
- **Admin Portal**: Full access to catalog management, user accounts, transactions, fine collection, and system metrics.
- **Member Portal**: Search catalog, view borrowed books, check borrowing history, view outstanding fines, and pay online.
- **Security**: Passwords hashed using **SHA-256** standard bitwise implementation.

### 📖 2. Book Management
- Full CRUD operations (Add, View, Search, Update, Delete).
- Categorization and Rack/Shelf tracking.
- Stock availability control (`available_copies` vs `total_copies`).
- Search & multi-field filtering by Title, Author, ISBN, or Category.

### 👥 3. Member Management
- Member registration with validation.
- Membership tiers (`REGULAR`, `PREMIUM`, `FACULTY`) with dynamic borrowing limits (5, 10, 8 books).
- Account status tracking (`ACTIVE`, `SUSPENDED`).

### 🔄 4. Book Issuing & Return Workflow
- Issue validation: checks copy availability, borrowing limit, account status, and duplicate loans.
- Automatic Return processing with instant stock restoration.
- Automatic **due-date** assignment (default 14 days loan period).
- Automatic **overdue detection** and fine generation at **$1.00 per day late**.

### 📊 5. Real-Time Dashboard & Analytics
- Total books & available copies.
- Currently issued books & overdue counts.
- Total registered members.
- Total fines collected vs pending balance.

### 💾 6. Database Persistence
- Powered by **SQLite3** relational database (`database/library.db`).
- Auto-initialization with schema migration and seed demo data on first launch.
- Prepared SQL statements for security against SQL Injection.

---

## 📁 Directory Structure

```
Library-Management-System/
├── src/                        # C++ Source Files
│   ├── main.cpp                # Application entry point
│   ├── User.cpp                # Base User class
│   ├── Admin.cpp               # Derived Admin class
│   ├── Member.cpp              # Derived Member class
│   ├── Book.cpp                # Book domain model
│   ├── IssueRecord.cpp         # Borrowing transaction model
│   ├── Fine.cpp                # Fine record & policy model
│   ├── DatabaseManager.cpp     # SQLite3 RAII wrapper & DAO
│   ├── AuthenticationManager.cpp# Auth & hashing service
│   ├── Library.cpp             # Core business logic coordinator
│   ├── UI.cpp                  # Terminal user interface
│   └── Utils.cpp               # Utilities (SHA-256, Date, ANSI colors)
├── include/                    # C++ Header Files
│   ├── User.hpp
│   ├── Admin.hpp
│   ├── Member.hpp
│   ├── Book.hpp
│   ├── IssueRecord.hpp
│   ├── Fine.hpp
│   ├── DatabaseManager.hpp
│   ├── AuthenticationManager.hpp
│   ├── Library.hpp
│   ├── UI.hpp
│   └── Utils.hpp
├── database/                   # Database files
│   ├── schema.sql              # SQL DDL database schema
│   └── library.db              # SQLite Database (auto-generated)
├── tests/                      # Test Suite
│   └── test_main.cpp           # Automated unit & integration tests
├── docs/                       # Comprehensive Academic Documentation
│   ├── ARCHITECTURE.md         # System design & UML diagrams
│   ├── DATABASE_DESIGN.md      # ER diagram & SQL schemas
│   ├── TESTING_REPORT.md       # Test plan & execution matrix
│   ├── VIVA_QA.md              # 20+ Viva Voce Q&A guide
│   └── OOP_CONCEPTS.md         # Explanation of OOP principles used
├── Makefile                    # Linux/GCC build script
├── README.md                   # Project overview & documentation
└── .gitignore                  # Git ignore rules
```

---

## 🛠️ Prerequisites & Compilation

### Requirements
- **OS**: Linux / Ubuntu / WSL / macOS
- **Compiler**: `g++` or `clang++` supporting **C++17** (`-std=c++17`)
- **Libraries**: `libsqlite3-dev` (`-lsqlite3`)
- **Build Tool**: `make`

### Installation (Ubuntu / WSL)
```bash
sudo apt update
sudo apt install -y build-essential sqlite3 libsqlite3-dev
```

### Build & Run
To compile the project and start the application:

```bash
make
./library
```

To run the automated unit test suite:
```bash
make test
```

To clean build artifacts:
```bash
make clean
```

---

## 🔑 Demo Credentials

| Role | Username | Password | Notes |
| :--- | :--- | :--- | :--- |
| **Admin** | `admin` | `admin123` | Full administrator privileges |
| **Member** | `john_doe` | `user123` | Regular member with 1 overdue book & pending fine |
| **Member** | `jane_smith` | `user123` | Premium member with 10 book limit |

---

## 💡 C++ & OOP Concepts Demonstrated

1. **Encapsulation**: Private/protected data members with public getters/setters and strict interface boundaries.
2. **Inheritance**: `Admin` and `Member` classes inherit from the base `User` class.
3. **Polymorphism**: Virtual methods like `displayInfo()` allow unified dynamic binding via `std::shared_ptr<User>`.
4. **Abstraction**: Complex database handling and SQL queries are abstracted behind clean interfaces in `Library` and `DatabaseManager`.
5. **RAII (Resource Acquisition Is Initialization)**: `DatabaseManager` manages SQLite database handles safely with automated cleanup in destructors.
6. **Smart Pointers**: `std::shared_ptr` and `std::unique_ptr` used for automated memory management without memory leaks.
7. **Design Patterns**: DAO (Data Access Object), Factory/Singleton principles.

---

## 📜 Academic Documentation Links
- [System Architecture & UML](file:///docs/ARCHITECTURE.md)
- [Database Schema & ER Diagram](file:///docs/DATABASE_DESIGN.md)
- [Testing Report & Matrix](file:///docs/TESTING_REPORT.md)
- [Viva Voce Preparation Guide](file:///docs/VIVA_QA.md)
- [OOP Concepts Breakdown](file:///docs/OOP_CONCEPTS.md)

---
*Created for Academic Excellence in C++ Software Engineering & System Architecture.*
