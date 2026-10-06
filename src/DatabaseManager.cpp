#include "DatabaseManager.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

namespace {
    void createDirectoryIfNotExists(const std::string& path) {
#ifdef _WIN32
        _mkdir(path.c_str());
#else
        mkdir(path.c_str(), 0755);
#endif
    }
}

DatabaseManager::DatabaseManager(const std::string& dbPath)
    : db(nullptr), dbPath(dbPath), isConnected(false) {
    createDirectoryIfNotExists("database");
}

DatabaseManager::~DatabaseManager() {
    close();
}

bool DatabaseManager::open() {
    if (isConnected) return true;

    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        Utils::printError("Failed to open SQLite database: " + std::string(sqlite3_errmsg(db)));
        isConnected = false;
        return false;
    }

    // Enable Foreign Keys
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    isConnected = true;
    return true;
}

void DatabaseManager::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
    isConnected = false;
}

sqlite3* DatabaseManager::getDbHandle() const {
    return db;
}

bool DatabaseManager::executeRawSQL(const std::string& sql) {
    if (!isConnected && !open()) return false;

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string errStr = errMsg ? errMsg : "Unknown SQL error";
        sqlite3_free(errMsg);
        Utils::printError("SQL Execution Error: " + errStr);
        return false;
    }
    return true;
}

int64_t DatabaseManager::getLastInsertedRowId() const {
    return db ? sqlite3_last_insert_rowid(db) : 0;
}

bool DatabaseManager::beginTransaction() {
    return executeRawSQL("BEGIN TRANSACTION;");
}

bool DatabaseManager::commitTransaction() {
    return executeRawSQL("COMMIT;");
}

bool DatabaseManager::rollbackTransaction() {
    return executeRawSQL("ROLLBACK;");
}

bool DatabaseManager::initializeSchema(const std::string& schemaFilePath) {
    if (!isConnected && !open()) return false;

    std::ifstream schemaFile(schemaFilePath);
    if (!schemaFile.is_open()) {
        Utils::printWarning("Schema file not found at " + schemaFilePath + ". Initializing internal fallback schema.");
        std::string fallbackSchema = R"(
            PRAGMA foreign_keys = ON;

            CREATE TABLE IF NOT EXISTS categories (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT UNIQUE NOT NULL,
                description TEXT
            );

            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT UNIQUE NOT NULL,
                password_hash TEXT NOT NULL,
                full_name TEXT NOT NULL,
                email TEXT NOT NULL,
                phone TEXT NOT NULL,
                role TEXT NOT NULL CHECK(role IN ('ADMIN', 'MEMBER')),
                created_at TEXT NOT NULL DEFAULT (date('now'))
            );

            CREATE TABLE IF NOT EXISTS members (
                user_id INTEGER PRIMARY KEY,
                membership_type TEXT NOT NULL DEFAULT 'REGULAR',
                max_books_allowed INTEGER NOT NULL DEFAULT 5,
                status TEXT NOT NULL DEFAULT 'ACTIVE' CHECK(status IN ('ACTIVE', 'SUSPENDED')),
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            );

            CREATE TABLE IF NOT EXISTS books (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                title TEXT NOT NULL,
                author TEXT NOT NULL,
                isbn TEXT UNIQUE NOT NULL,
                category_id INTEGER,
                publisher TEXT NOT NULL DEFAULT 'Unknown Publisher',
                publication_year INTEGER NOT NULL DEFAULT 2023,
                total_copies INTEGER NOT NULL DEFAULT 1,
                available_copies INTEGER NOT NULL DEFAULT 1,
                price REAL NOT NULL DEFAULT 0.0,
                rack_number TEXT NOT NULL DEFAULT 'A1',
                FOREIGN KEY (category_id) REFERENCES categories(id) ON DELETE SET NULL
            );

            CREATE TABLE IF NOT EXISTS issues (
                issue_id INTEGER PRIMARY KEY AUTOINCREMENT,
                book_id INTEGER NOT NULL,
                member_id INTEGER NOT NULL,
                issue_date TEXT NOT NULL DEFAULT (date('now')),
                due_date TEXT NOT NULL,
                return_date TEXT,
                status TEXT NOT NULL DEFAULT 'ISSUED' CHECK(status IN ('ISSUED', 'RETURNED', 'OVERDUE')),
                fine_amount REAL NOT NULL DEFAULT 0.0,
                FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,
                FOREIGN KEY (member_id) REFERENCES users(id) ON DELETE CASCADE
            );

            CREATE TABLE IF NOT EXISTS fines (
                fine_id INTEGER PRIMARY KEY AUTOINCREMENT,
                issue_id INTEGER NOT NULL,
                member_id INTEGER NOT NULL,
                amount REAL NOT NULL,
                paid_status TEXT NOT NULL DEFAULT 'PENDING' CHECK(paid_status IN ('PENDING', 'PAID')),
                payment_date TEXT,
                reason TEXT NOT NULL DEFAULT 'Overdue Book Return',
                FOREIGN KEY (issue_id) REFERENCES issues(issue_id) ON DELETE CASCADE,
                FOREIGN KEY (member_id) REFERENCES users(id) ON DELETE CASCADE
            );
        )";
        return executeRawSQL(fallbackSchema) && seedInitialData();
    }

    std::stringstream buffer;
    buffer << schemaFile.rdbuf();
    std::string sqlContent = buffer.str();

    bool success = executeRawSQL(sqlContent);
    if (success) {
        seedInitialData();
    }
    return success;
}

bool DatabaseManager::seedInitialData() {
    // Check if users table is already populated
    sqlite3_stmt* stmt = nullptr;
    const char* countSql = "SELECT COUNT(*) FROM users;";
    if (sqlite3_prepare_v2(db, countSql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    int userCount = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        userCount = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (userCount > 0) {
        return true; // Already seeded
    }

    Utils::printInfo("Seeding initial database records...");

    beginTransaction();

    // 1. Seed Categories
    executeRawSQL("INSERT INTO categories (name, description) VALUES ('Computer Science', 'Software development, algorithms, and computing');");
    executeRawSQL("INSERT INTO categories (name, description) VALUES ('Mathematics', 'Pure and applied mathematics');");
    executeRawSQL("INSERT INTO categories (name, description) VALUES ('Physics', 'Theoretical and experimental physics');");
    executeRawSQL("INSERT INTO categories (name, description) VALUES ('Literature', 'Classic and contemporary literature');");
    executeRawSQL("INSERT INTO categories (name, description) VALUES ('History', 'World history and historical analysis');");

    // 2. Seed Default Admin (admin / admin123)
    std::string adminHash = Utils::sha256("admin123");
    std::string adminSql = "INSERT INTO users (username, password_hash, full_name, email, phone, role) VALUES "
                           "('admin', '" + adminHash + "', 'System Librarian', 'admin@library.org', '+1-555-0100', 'ADMIN');";
    executeRawSQL(adminSql);

    // 3. Seed Sample Members (john_doe / user123, jane_smith / user123)
    std::string userHash = Utils::sha256("user123");
    
    // Member 1
    executeRawSQL("INSERT INTO users (username, password_hash, full_name, email, phone, role) VALUES "
                  "('john_doe', '" + userHash + "', 'John Doe', 'john.doe@example.com', '+1-555-0199', 'MEMBER');");
    int64_t johnId = getLastInsertedRowId();
    executeRawSQL("INSERT INTO members (user_id, membership_type, max_books_allowed, status) VALUES (" +
                  std::to_string(johnId) + ", 'REGULAR', 5, 'ACTIVE');");

    // Member 2
    executeRawSQL("INSERT INTO users (username, password_hash, full_name, email, phone, role) VALUES "
                  "('jane_smith', '" + userHash + "', 'Jane Smith', 'jane.smith@example.com', '+1-555-0288', 'MEMBER');");
    int64_t janeId = getLastInsertedRowId();
    executeRawSQL("INSERT INTO members (user_id, membership_type, max_books_allowed, status) VALUES (" +
                  std::to_string(janeId) + ", 'PREMIUM', 10, 'ACTIVE');");

    // 4. Seed Books
    executeRawSQL("INSERT INTO books (title, author, isbn, category_id, publisher, publication_year, total_copies, available_copies, price, rack_number) VALUES "
                  "('Clean Code: A Handbook of Agile Software Craftsmanship', 'Robert C. Martin', '9780132350884', 1, 'Prentice Hall', 2008, 5, 4, 45.00, 'CS-A1');");

    executeRawSQL("INSERT INTO books (title, author, isbn, category_id, publisher, publication_year, total_copies, available_copies, price, rack_number) VALUES "
                  "('Design Patterns: Elements of Reusable Object-Oriented Software', 'Erich Gamma, Richard Helm, Ralph Johnson, John Vlissides', '9780201633610', 1, 'Addison-Wesley', 1994, 3, 2, 54.99, 'CS-A2');");

    executeRawSQL("INSERT INTO books (title, author, isbn, category_id, publisher, publication_year, total_copies, available_copies, price, rack_number) VALUES "
                  "('The C++ Programming Language (4th Edition)', 'Bjarne Stroustrup', '9780321563842', 1, 'Addison-Wesley', 2013, 4, 4, 69.99, 'CS-B1');");

    executeRawSQL("INSERT INTO books (title, author, isbn, category_id, publisher, publication_year, total_copies, available_copies, price, rack_number) VALUES "
                  "('Introduction to Algorithms', 'Thomas H. Cormen, Charles E. Leiserson, Ronald L. Rivest, Clifford Stein', '9780262033848', 1, 'MIT Press', 2009, 6, 6, 85.00, 'CS-B2');");

    executeRawSQL("INSERT INTO books (title, author, isbn, category_id, publisher, publication_year, total_copies, available_copies, price, rack_number) VALUES "
                  "('Calculus: Early Transcendentals', 'James Stewart', '9781285741550', 2, 'Cengage Learning', 2015, 3, 3, 110.00, 'MATH-1');");

    // 5. Seed a Sample Issued Book record (John Doe borrowed Clean Code)
    std::string today = Utils::getCurrentDate();
    std::string dueDatePast = Utils::addDaysToDate(today, -5); // 5 days past due date to simulate an overdue record for testing fine calculations
    std::string issueDatePast = Utils::addDaysToDate(today, -19);

    executeRawSQL("INSERT INTO issues (book_id, member_id, issue_date, due_date, status, fine_amount) VALUES "
                  "(1, " + std::to_string(johnId) + ", '" + issueDatePast + "', '" + dueDatePast + "', 'OVERDUE', 5.00);");
    int64_t issueId = getLastInsertedRowId();

    // Create corresponding pending fine for John Doe
    executeRawSQL("INSERT INTO fines (issue_id, member_id, amount, paid_status, reason) VALUES (" +
                  std::to_string(issueId) + ", " + std::to_string(johnId) + ", 5.00, 'PENDING', 'Overdue Book Return (5 days late)');");

    commitTransaction();
    Utils::printSuccess("Initial database seeding completed successfully.");
    return true;
}
