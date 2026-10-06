# 🎓 Viva Voce Questions & Answers Guide

This document contains 20 comprehensive Viva Voce questions and detailed technical answers to help students excel during project defense and oral examinations.

---

### Q1: What programming language standard was used and why?
**Answer**: The project is written in **C++17** (compiled with `-std=c++17`). C++17 was selected because it introduces modern standard library features such as `std::optional` (used for null-safe object retrieval in `getBookById`), `std::shared_ptr`, `std::chrono` for time manipulation, structured bindings, and enhanced type safety while maintaining maximum operational performance.

---

### Q2: How is Encapsulation achieved in your domain classes?
**Answer**: Encapsulation is achieved by keeping all class data members `private` or `protected` (e.g., `id`, `isbn`, `passwordHash` in `Book` and `User`) and providing access only through public member functions (getters and setters). This protects the internal state from illegal external mutations and maintains invariant rules.

---

### Q3: Explain how Inheritance and Polymorphism are implemented in your codebase.
**Answer**: 
- **Inheritance**: The base class `User` defines core user properties (`id`, `username`, `passwordHash`, `email`, `role`). `Admin` and `Member` publicly inherit from `User` (`class Admin : public User`), inheriting base attributes while extending functionality with subclass-specific attributes like `maxBooksAllowed` and `membershipType`.
- **Polymorphism**: The method `virtual void displayInfo() const` in `User` is overridden in both `Admin` and `Member`. When invoking `userPtr->displayInfo()` on a `std::shared_ptr<User>`, dynamic dispatch (vtable lookup) automatically executes the specific subclass version.

---

### Q4: How do you prevent SQL Injection vulnerabilities in SQLite3?
**Answer**: User inputs are **never** concatenated directly into raw SQL strings. Instead, we use parameterized SQL queries with SQLite prepared statements (`sqlite3_prepare_v2`) and bind parameters using `sqlite3_bind_text`, `sqlite3_bind_int`, and `sqlite3_bind_double`. This ensures user input is treated strictly as data literals, rendering SQL injection impossible.

---

### Q5: What is RAII and where is it applied in your application?
**Answer**: **RAII (Resource Acquisition Is Initialization)** is a C++ idiom where resource management (like database connections or dynamic memory) is tied to object lifetime. In `DatabaseManager`, opening the SQLite handle occurs in the constructor (`open()`), and resource cleanup occurs automatically in the destructor (`~DatabaseManager()`), guaranteeing zero resource leaks even if exceptions occur.

---

### Q6: How are passwords stored securely?
**Answer**: Passwords are never stored in plaintext. They are hashed using a standard **SHA-256** bitwise algorithm implemented in `Utils::sha256()`. When a user logs in, the input password is hashed and compared against the stored hash in the database.

---

### Q7: Explain the database relationship between `users` and `members`.
**Answer**: This represents a **Class Table Inheritance** design. The `users` table holds shared credentials and profile information, while the `members` table uses `user_id` as its Primary Key and Foreign Key referencing `users(id) ON DELETE CASCADE`.

---

### Q8: What design patterns are implemented in the project?
**Answer**:
1. **DAO (Data Access Object) Pattern**: `DatabaseManager` encapsulates database communication details.
2. **Controller / Coordinator Pattern**: `Library` acts as the business logic coordinator bridging UI and Persistence.
3. **RAII Pattern**: Automated database connection lifetime management.

---

### Q9: How is fine calculation performed for overdue books?
**Answer**: When a book is returned, `Utils::daysBetweenDates(dueDate, returnDate)` computes late days. If late days > 0, `Fine::calculateFineAmount(overdueDays)` multiplies late days by `DAILY_FINE_RATE` ($1.00/day). An automatic record is inserted into the `fines` table with status `PENDING`.

---

### Q10: How does the system handle database transactions?
**Answer**: Critical multi-table operations (e.g. issuing a book requires inserting an issue record AND decrementing book copies) are wrapped inside `dbManager.beginTransaction()` and `dbManager.commitTransaction()`. If any step fails, `dbManager.rollbackTransaction()` reverts changes to maintain ACID compliance.

---

### Q11: What happens if a user inputs non-numeric characters when a menu integer is requested?
**Answer**: `Utils::readInt()` reads input as a full line (`std::getline`), trims whitespace, and uses `std::stoi()` inside a `try-catch` block. If invalid input is provided, an error prompt is printed without corrupting the `std::cin` input stream.

---

### Q12: Why use `std::shared_ptr` instead of raw pointers (`User*`)?
**Answer**: `std::shared_ptr` provides automatic reference-counted memory management. When a logged-in user object goes out of scope or is logged out, memory is reclaimed automatically without needing explicit `delete` calls, preventing memory leaks and dangling pointer bugs.

---

### Q13: What is the purpose of `std::optional<Book>` in `getBookById`?
**Answer**: `std::optional<T>` represents a value that may or may not exist. If the book ID exists in SQLite, `getBookById` returns `std::optional<Book>` containing the book object; if not found, it returns `std::nullopt`, avoiding null pointer exceptions.

---

### Q14: How are categories linked to books in the database?
**Answer**: The `books` table contains a Foreign Key `category_id` referencing `categories(id) ON DELETE SET NULL`. If a category is deleted, books in that category are preserved and assigned `category_id = NULL` ('Uncategorized').

---

### Q15: How does the system enforce member borrowing limits?
**Answer**: Before issuing a book, `Library::issueBook()` checks `activeBorrowedCount(memberId)`. If current borrowed books equal or exceed `member->getMaxBooksAllowed()`, checkout is blocked.

---

### Q16: How is cross-platform terminal compatibility handled?
**Answer**: Preprocessor conditional compilation (`#ifdef _WIN32`) handles platform differences. For example, `_getch()` is used on Windows for hidden password entry, while `termios` echo disabling is used on Linux/POSIX systems.

---

### Q17: What is the complexity of searching books by title or author?
**Answer**: SQLite queries with `LIKE '%keyword%'` search indexed columns (`idx_books_title`, `idx_books_author`), providing fast query execution ($O(\log N)$ or fast table scans for small datasets).

---

### Q18: What is the role of `PRAGMA foreign_keys = ON;` in SQLite?
**Answer**: By default, SQLite disables foreign key constraint enforcement for backwards compatibility. Executing `PRAGMA foreign_keys = ON;` upon connection forces relational constraints (`ON DELETE CASCADE`, `ON DELETE SET NULL`) to be respected.

---

### Q19: How are overdue books automatically updated?
**Answer**: Before rendering dashboards or issuing books, `Library::updateOverdueStatusAll()` executes an SQL update: `UPDATE issues SET status = 'OVERDUE' WHERE status = 'ISSUED' AND due_date < date('now');`.

---

### Q20: How would you scale this system to a multi-client web architecture in the future?
**Answer**: The core business classes (`Library`, `DatabaseManager`, `Book`, `User`) are cleanly decoupled from `UI.cpp`. To transform this into a web application or REST API, `UI.cpp` could be replaced by a C++ web framework (such as Crow or Oat++) exposing HTTP endpoints returning JSON without altering business logic.
