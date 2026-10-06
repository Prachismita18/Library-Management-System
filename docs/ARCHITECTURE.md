# 🏗️ System Architecture & Design

This document details the high-level architecture, module decomposition, class hierarchy, and data flow of the **Library Management System in C++17**.

---

## 📐 1. Architectural Pattern

The project follows a modified **N-Tier Architecture** (Model-View-Controller / DAO pattern adapted for terminal applications):

```mermaid
graph TD
    UI["Presentation Layer (UI.cpp)"] --> AuthManager["Authentication Layer (AuthenticationManager.cpp)"]
    UI --> LibraryCore["Business Logic Layer (Library.cpp)"]
    
    AuthManager --> DBMgr["Data Access Layer (DatabaseManager.cpp)"]
    LibraryCore --> DBMgr
    
    DBMgr --> SQLite[("SQLite3 Database (library.db)")]

    LibraryCore --> BookModel["Models: Book, User, Member, Admin, IssueRecord, Fine"]
```

### Layer Breakdown
1. **Presentation Layer (`UI.cpp`)**:
   - Interacts with the user via terminal ANSI output.
   - Manages menu loops, user prompts, input sanitization, and table rendering.
   - Delegates all business operations to `Library` and `AuthenticationManager`.

2. **Authentication & Security Layer (`AuthenticationManager.cpp`)**:
   - Manages user login verification and member registration.
   - Uses standard bitwise **SHA-256** for secure password hashing.
   - Instantiates polymorphic `User` pointers (`Admin` / `Member`).

3. **Business Logic Layer (`Library.cpp`)**:
   - Contains core library workflows: issuing, returning, fine computation, overdue updates, catalog filtering.
   - Enforces business rules (e.g. member borrowing limits, suspended status checks, duplicate loan prevention).

4. **Data Access Layer (`DatabaseManager.cpp`)**:
   - RAII wrapper around native `sqlite3*` C pointers.
   - Handles SQL statement execution, parameter binding, transactions (`BEGIN`, `COMMIT`, `ROLLBACK`), and automatic schema creation.

5. **Persistence Storage (`library.db`)**:
   - Relational database storing persistent state across application restarts.

---

## 🧬 2. Object-Oriented Class Hierarchy

```mermaid
classDiagram
    class User {
        #int id
        #string username
        #string passwordHash
        #string fullName
        #string email
        #string phone
        #UserRole role
        +displayInfo()*
        +getRoleString() string
        +isAdmin() bool
        +isMember() bool
    }

    class Admin {
        -string adminLevel
        +displayInfo()
        +getAdminLevel() string
    }

    class Member {
        -string membershipType
        -int maxBooksAllowed
        -string status
        +displayInfo()
        +isActive() bool
        +getMaxBooksAllowed() int
    }

    User <|-- Admin
    User <|-- Member

    class Book {
        -int id
        -string title
        -string author
        -string isbn
        -int categoryId
        -int availableCopies
        -int totalCopies
        -double price
        -string rackNumber
        +isAvailable() bool
    }

    class IssueRecord {
        -int issueId
        -int bookId
        -int memberId
        -string issueDate
        -string dueDate
        -string returnDate
        -string status
        -double fineAmount
        +isOverdue() bool
        +calculateOverdueDays() int
    }

    class Fine {
        -int fineId
        -int issueId
        -int memberId
        -double amount
        -string paidStatus
        +calculateFineAmount(overdueDays) double
    }

    class DatabaseManager {
        -sqlite3* db
        +open() bool
        +initializeSchema() bool
        +beginTransaction() bool
        +commitTransaction() bool
    }

    class Library {
        -DatabaseManager& dbManager
        +getDashboardStats() DashboardStats
        +issueBook(bookId, memberId) bool
        +returnBook(issueId) bool
        +searchBooks(keyword) vector~Book~
    }

    Library --> DatabaseManager
    Library ..> Book
    Library ..> IssueRecord
    Library ..> Fine
    Library ..> Member
```

---

## 🔄 3. Key Sequence Diagrams

### Book Issue Sequence
```mermaid
sequenceDiagram
    autonumber
    actor Admin
    participant UI as UI Layer
    participant Lib as Library Core
    participant DB as DatabaseManager
    participant SQLite as SQLite3 Engine

    Admin->>UI: Select "Issue Book" (bookId, memberId)
    UI->>Lib: issueBook(bookId, memberId)
    Lib->>DB: Fetch book & member status
    DB->>SQLite: SELECT * FROM books WHERE id = bookId
    SQLite-->>DB: Book record
    DB->>SQLite: SELECT * FROM members WHERE user_id = memberId
    SQLite-->>DB: Member record
    
    alt Copies Available & Member Limit Not Exceeded
        Lib->>DB: beginTransaction()
        Lib->>DB: INSERT INTO issues (book_id, member_id, due_date)
        DB->>SQLite: Execute INSERT
        Lib->>DB: UPDATE books SET available_copies = available_copies - 1
        DB->>SQLite: Execute UPDATE
        Lib->>DB: commitTransaction()
        Lib-->>UI: Return success
        UI-->>Admin: Display Success Banner
    else Validation Failed
        Lib-->>UI: Return error (e.g. No copies / Limit reached)
        UI-->>Admin: Display Error Message
    end
```

---

## 🛡️ 4. Safety & Robustness Features

1. **RAII (Resource Acquisition Is Initialization)**:
   - All SQLite connections and statements (`sqlite3_stmt*`) are bound to stack lifetimes or wrapped in RAII helper functions to prevent memory leaks and dangling resource handles.

2. **SQL Injection Protection**:
   - Raw user input is **never** concatenated directly into SQL queries.
   - All dynamic parameters use parameterized queries with `sqlite3_bind_text`, `sqlite3_bind_int`, and `sqlite3_bind_double`.

3. **Input Validation**:
   - `Utils::readInt` and `Utils::readDouble` catch parsing exceptions (`std::invalid_argument`, `std::out_of_range`) and handle bounds checking safely without crashing terminal stream buffers.

4. **Transaction Integrity**:
   - Multi-table operations (such as returning a book and updating stock/creating fines) are wrapped in `BEGIN TRANSACTION` and `COMMIT`. If any step fails, `ROLLBACK` is executed immediately.
