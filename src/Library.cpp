#include "Library.hpp"
#include "Utils.hpp"
#include <sqlite3.h>
#include <iostream>
#include <algorithm>

Library::Library(DatabaseManager& dbMgr) : dbManager(dbMgr) {
    updateOverdueStatusAll();
}

void Library::updateOverdueStatusAll() {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return;

    std::string today = Utils::getCurrentDate();
    std::string sql = "UPDATE issues SET status = 'OVERDUE' WHERE status = 'ISSUED' AND due_date < '" + today + "';";
    dbManager.executeRawSQL(sql);
}

DashboardStats Library::getDashboardStats() {
    updateOverdueStatusAll();
    DashboardStats stats = {0, 0, 0, 0, 0, 0.0, 0.0};
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return stats;

    sqlite3_stmt* stmt = nullptr;

    // Books Stats
    const char* bookSql = "SELECT COALESCE(SUM(total_copies), 0), COALESCE(SUM(available_copies), 0) FROM books;";
    if (sqlite3_prepare_v2(db, bookSql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.totalBooks = sqlite3_column_int(stmt, 0);
            stats.availableBooks = sqlite3_column_int(stmt, 1);
            stats.issuedBooks = stats.totalBooks - stats.availableBooks;
        }
        sqlite3_finalize(stmt);
    }

    // Members Count
    const char* memSql = "SELECT COUNT(*) FROM members;";
    if (sqlite3_prepare_v2(db, memSql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.totalMembers = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    // Overdue Books Count
    const char* ovSql = "SELECT COUNT(*) FROM issues WHERE status = 'OVERDUE';";
    if (sqlite3_prepare_v2(db, ovSql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.overdueBooks = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    // Fines Collected & Pending
    const char* fineSql = "SELECT paid_status, COALESCE(SUM(amount), 0.0) FROM fines GROUP BY paid_status;";
    if (sqlite3_prepare_v2(db, fineSql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            double amt = sqlite3_column_double(stmt, 1);
            if (status == "PAID") {
                stats.totalFinesCollected = amt;
            } else if (status == "PENDING") {
                stats.totalFinesPending = amt;
            }
        }
        sqlite3_finalize(stmt);
    }

    return stats;
}

std::vector<Category> Library::getAllCategories() {
    std::vector<Category> list;
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return list;

    const char* sql = "SELECT id, name, description FROM categories ORDER BY name ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Category c;
            c.id = sqlite3_column_int(stmt, 0);
            c.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            c.description = desc ? desc : "";
            list.push_back(c);
        }
        sqlite3_finalize(stmt);
    }
    return list;
}

bool Library::addCategory(const std::string& name, const std::string& description, std::string* outError) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return false;

    const char* sql = "INSERT INTO categories (name, description) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!ok && outError) {
        *outError = "Category name already exists or is invalid.";
    }
    sqlite3_finalize(stmt);
    return ok;
}

bool Library::addBook(const Book& book, std::string* outError) {
    if (getBookByIsbn(book.getIsbn()).has_value()) {
        if (outError) *outError = "Book with ISBN '" + book.getIsbn() + "' already exists.";
        return false;
    }

    sqlite3* db = dbManager.getDbHandle();
    if (!db) return false;

    const char* sql = "INSERT INTO books (title, author, isbn, category_id, publisher, publication_year, total_copies, available_copies, price, rack_number) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, book.getTitle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, book.getAuthor().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, book.getIsbn().c_str(), -1, SQLITE_TRANSIENT);
    if (book.getCategoryId() > 0) {
        sqlite3_bind_int(stmt, 4, book.getCategoryId());
    } else {
        sqlite3_bind_null(stmt, 4);
    }
    sqlite3_bind_text(stmt, 5, book.getPublisher().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, book.getPublicationYear());
    sqlite3_bind_int(stmt, 7, book.getTotalCopies());
    sqlite3_bind_int(stmt, 8, book.getAvailableCopies());
    sqlite3_bind_double(stmt, 9, book.getPrice());
    sqlite3_bind_text(stmt, 10, book.getRackNumber().c_str(), -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!ok && outError) {
        *outError = "Failed to insert book record into database.";
    }
    sqlite3_finalize(stmt);
    return ok;
}

bool Library::updateBook(const Book& book, std::string* outError) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return false;

    const char* sql = "UPDATE books SET title = ?, author = ?, isbn = ?, category_id = ?, publisher = ?, "
                      "publication_year = ?, total_copies = ?, available_copies = ?, price = ?, rack_number = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, book.getTitle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, book.getAuthor().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, book.getIsbn().c_str(), -1, SQLITE_TRANSIENT);
    if (book.getCategoryId() > 0) sqlite3_bind_int(stmt, 4, book.getCategoryId());
    else sqlite3_bind_null(stmt, 4);
    sqlite3_bind_text(stmt, 5, book.getPublisher().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, book.getPublicationYear());
    sqlite3_bind_int(stmt, 7, book.getTotalCopies());
    sqlite3_bind_int(stmt, 8, book.getAvailableCopies());
    sqlite3_bind_double(stmt, 9, book.getPrice());
    sqlite3_bind_text(stmt, 10, book.getRackNumber().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 11, book.getId());

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!ok && outError) *outError = "Failed to update book record.";
    sqlite3_finalize(stmt);
    return ok;
}

bool Library::deleteBook(int bookId, std::string* outError) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return false;

    // Check if currently issued
    const char* chkSql = "SELECT COUNT(*) FROM issues WHERE book_id = ? AND status IN ('ISSUED', 'OVERDUE');";
    sqlite3_stmt* cStmt = nullptr;
    if (sqlite3_prepare_v2(db, chkSql, -1, &cStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(cStmt, 1, bookId);
        if (sqlite3_step(cStmt) == SQLITE_ROW && sqlite3_column_int(cStmt, 0) > 0) {
            sqlite3_finalize(cStmt);
            if (outError) *outError = "Cannot delete book because copies are currently issued to members.";
            return false;
        }
        sqlite3_finalize(cStmt);
    }

    const char* sql = "DELETE FROM books WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, bookId);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!ok && outError) *outError = "Failed to delete book.";
    sqlite3_finalize(stmt);
    return ok;
}

std::optional<Book> Library::getBookById(int bookId) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return std::nullopt;

    const char* sql = "SELECT b.id, b.title, b.author, b.isbn, b.category_id, COALESCE(c.name, 'Uncategorized'), "
                      "b.publisher, b.publication_year, b.total_copies, b.available_copies, b.price, b.rack_number "
                      "FROM books b LEFT JOIN categories c ON b.category_id = c.id WHERE b.id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;

    sqlite3_bind_int(stmt, 1, bookId);

    std::optional<Book> book = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string isbn = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int catId = sqlite3_column_type(stmt, 4) == SQLITE_NULL ? 0 : sqlite3_column_int(stmt, 4);
        std::string catName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        std::string publisher = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        int year = sqlite3_column_int(stmt, 7);
        int total = sqlite3_column_int(stmt, 8);
        int avail = sqlite3_column_int(stmt, 9);
        double price = sqlite3_column_double(stmt, 10);
        std::string rack = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        book = Book(id, title, author, isbn, catId, catName, publisher, year, total, avail, price, rack);
    }
    sqlite3_finalize(stmt);
    return book;
}

std::optional<Book> Library::getBookByIsbn(const std::string& isbn) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return std::nullopt;

    const char* sql = "SELECT b.id, b.title, b.author, b.isbn, b.category_id, COALESCE(c.name, 'Uncategorized'), "
                      "b.publisher, b.publication_year, b.total_copies, b.available_copies, b.price, b.rack_number "
                      "FROM books b LEFT JOIN categories c ON b.category_id = c.id WHERE b.isbn = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;

    sqlite3_bind_text(stmt, 1, isbn.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<Book> book = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string bisbn = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int catId = sqlite3_column_type(stmt, 4) == SQLITE_NULL ? 0 : sqlite3_column_int(stmt, 4);
        std::string catName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        std::string publisher = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        int year = sqlite3_column_int(stmt, 7);
        int total = sqlite3_column_int(stmt, 8);
        int avail = sqlite3_column_int(stmt, 9);
        double price = sqlite3_column_double(stmt, 10);
        std::string rack = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        book = Book(id, title, author, bisbn, catId, catName, publisher, year, total, avail, price, rack);
    }
    sqlite3_finalize(stmt);
    return book;
}

std::vector<Book> Library::getAllBooks() {
    return searchBooks("", 0);
}

std::vector<Book> Library::searchBooks(const std::string& keyword, int categoryId) {
    std::vector<Book> books;
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return books;

    std::string sql = "SELECT b.id, b.title, b.author, b.isbn, b.category_id, COALESCE(c.name, 'Uncategorized'), "
                      "b.publisher, b.publication_year, b.total_copies, b.available_copies, b.price, b.rack_number "
                      "FROM books b LEFT JOIN categories c ON b.category_id = c.id WHERE 1=1";

    if (!keyword.empty()) {
        sql += " AND (b.title LIKE ? OR b.author LIKE ? OR b.isbn LIKE ?)";
    }
    if (categoryId > 0) {
        sql += " AND b.category_id = ?";
    }
    sql += " ORDER BY b.title ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return books;

    int bindIdx = 1;
    if (!keyword.empty()) {
        std::string pattern = "%" + keyword + "%";
        sqlite3_bind_text(stmt, bindIdx++, pattern.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, bindIdx++, pattern.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, bindIdx++, pattern.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (categoryId > 0) {
        sqlite3_bind_int(stmt, bindIdx++, categoryId);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string isbn = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int catId = sqlite3_column_type(stmt, 4) == SQLITE_NULL ? 0 : sqlite3_column_int(stmt, 4);
        std::string catName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        std::string publisher = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        int year = sqlite3_column_int(stmt, 7);
        int total = sqlite3_column_int(stmt, 8);
        int avail = sqlite3_column_int(stmt, 9);
        double price = sqlite3_column_double(stmt, 10);
        std::string rack = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        books.emplace_back(id, title, author, isbn, catId, catName, publisher, year, total, avail, price, rack);
    }
    sqlite3_finalize(stmt);
    return books;
}

std::vector<std::shared_ptr<Member>> Library::getAllMembers() {
    return searchMembers("");
}

std::vector<std::shared_ptr<Member>> Library::searchMembers(const std::string& keyword) {
    std::vector<std::shared_ptr<Member>> members;
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return members;

    std::string sql = "SELECT u.id, u.username, u.password_hash, u.full_name, u.email, u.phone, u.created_at, "
                      "m.membership_type, m.max_books_allowed, m.status FROM users u JOIN members m ON u.id = m.user_id";

    if (!keyword.empty()) {
        sql += " WHERE u.username LIKE ? OR u.full_name LIKE ? OR u.email LIKE ?";
    }
    sql += " ORDER BY u.full_name ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return members;

    if (!keyword.empty()) {
        std::string pattern = "%" + keyword + "%";
        sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, pattern.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, pattern.c_str(), -1, SQLITE_TRANSIENT);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        std::string phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        std::string created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        std::string mType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        int maxB = sqlite3_column_int(stmt, 8);
        std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

        members.push_back(std::make_shared<Member>(id, username, hash, name, email, phone, mType, maxB, status, created));
    }
    sqlite3_finalize(stmt);
    return members;
}

std::shared_ptr<Member> Library::getMemberById(int memberId) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return nullptr;

    const char* sql = "SELECT u.id, u.username, u.password_hash, u.full_name, u.email, u.phone, u.created_at, "
                      "m.membership_type, m.max_books_allowed, m.status FROM users u JOIN members m ON u.id = m.user_id WHERE u.id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return nullptr;

    sqlite3_bind_int(stmt, 1, memberId);

    std::shared_ptr<Member> member = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        std::string phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        std::string created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        std::string mType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        int maxB = sqlite3_column_int(stmt, 8);
        std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

        member = std::make_shared<Member>(id, username, hash, name, email, phone, mType, maxB, status, created);
    }
    sqlite3_finalize(stmt);
    return member;
}

bool Library::updateMemberStatus(int memberId, const std::string& newStatus, std::string* outError) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return false;

    const char* sql = "UPDATE members SET status = ? WHERE user_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, newStatus.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, memberId);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!ok && outError) *outError = "Failed to update member status.";
    sqlite3_finalize(stmt);
    return ok;
}

bool Library::updateMemberDetails(int memberId, const std::string& fullName, const std::string& email, const std::string& phone, std::string* outError) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return false;

    const char* sql = "UPDATE users SET full_name = ?, email = ?, phone = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, fullName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, memberId);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!ok && outError) *outError = "Failed to update user profile.";
    sqlite3_finalize(stmt);
    return ok;
}

bool Library::deleteMember(int memberId, std::string* outError) {
    if (activeBorrowedCount(memberId) > 0) {
        if (outError) *outError = "Cannot delete member with active borrowed books.";
        return false;
    }

    sqlite3* db = dbManager.getDbHandle();
    if (!db) return false;

    const char* sql = "DELETE FROM users WHERE id = ? AND role = 'MEMBER';";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, memberId);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!ok && outError) *outError = "Failed to delete member.";
    sqlite3_finalize(stmt);
    return ok;
}

int Library::activeBorrowedCount(int memberId) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return 0;

    const char* sql = "SELECT COUNT(*) FROM issues WHERE member_id = ? AND status IN ('ISSUED', 'OVERDUE');";
    sqlite3_stmt* stmt = nullptr;
    int count = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, memberId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return count;
}

bool Library::issueBook(int bookId, int memberId, int daysToBorrow, std::string* outError) {
    updateOverdueStatusAll();

    auto bookOpt = getBookById(bookId);
    if (!bookOpt.has_value()) {
        if (outError) *outError = "Book ID #" + std::to_string(bookId) + " not found.";
        return false;
    }

    Book book = bookOpt.value();
    if (book.getAvailableCopies() <= 0) {
        if (outError) *outError = "No available copies for '" + book.getTitle() + "'.";
        return false;
    }

    auto member = getMemberById(memberId);
    if (!member) {
        if (outError) *outError = "Member ID #" + std::to_string(memberId) + " not found.";
        return false;
    }

    if (!member->isActive()) {
        if (outError) *outError = "Member account is SUSPENDED. Cannot issue books.";
        return false;
    }

    int currentBorrowed = activeBorrowedCount(memberId);
    if (currentBorrowed >= member->getMaxBooksAllowed()) {
        if (outError) *outError = "Member has reached maximum borrowing limit (" +
                                  std::to_string(member->getMaxBooksAllowed()) + " books).";
        return false;
    }

    // Check if member already has an active issue for THIS same book
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return false;

    const char* dupCheckSql = "SELECT COUNT(*) FROM issues WHERE book_id = ? AND member_id = ? AND status IN ('ISSUED', 'OVERDUE');";
    sqlite3_stmt* dcStmt = nullptr;
    if (sqlite3_prepare_v2(db, dupCheckSql, -1, &dcStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(dcStmt, 1, bookId);
        sqlite3_bind_int(dcStmt, 2, memberId);
        if (sqlite3_step(dcStmt) == SQLITE_ROW && sqlite3_column_int(dcStmt, 0) > 0) {
            sqlite3_finalize(dcStmt);
            if (outError) *outError = "Member already has an active issue record for this book.";
            return false;
        }
        sqlite3_finalize(dcStmt);
    }

    dbManager.beginTransaction();

    std::string today = Utils::getCurrentDate();
    std::string dueDate = Utils::addDaysToDate(today, daysToBorrow);

    // 1. Create issue record
    const char* issueSql = "INSERT INTO issues (book_id, member_id, issue_date, due_date, status) VALUES (?, ?, ?, ?, 'ISSUED');";
    sqlite3_stmt* iStmt = nullptr;
    if (sqlite3_prepare_v2(db, issueSql, -1, &iStmt, nullptr) != SQLITE_OK) {
        dbManager.rollbackTransaction();
        if (outError) *outError = "Database error preparing issue statement.";
        return false;
    }

    sqlite3_bind_int(iStmt, 1, bookId);
    sqlite3_bind_int(iStmt, 2, memberId);
    sqlite3_bind_text(iStmt, 3, today.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(iStmt, 4, dueDate.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(iStmt) != SQLITE_DONE) {
        sqlite3_finalize(iStmt);
        dbManager.rollbackTransaction();
        if (outError) *outError = "Failed to record book issue.";
        return false;
    }
    sqlite3_finalize(iStmt);

    // 2. Decrement available copies
    const char* decSql = "UPDATE books SET available_copies = available_copies - 1 WHERE id = ?;";
    sqlite3_stmt* bStmt = nullptr;
    if (sqlite3_prepare_v2(db, decSql, -1, &bStmt, nullptr) != SQLITE_OK) {
        dbManager.rollbackTransaction();
        return false;
    }

    sqlite3_bind_int(bStmt, 1, bookId);
    if (sqlite3_step(bStmt) != SQLITE_DONE) {
        sqlite3_finalize(bStmt);
        dbManager.rollbackTransaction();
        if (outError) *outError = "Failed to update available copies.";
        return false;
    }
    sqlite3_finalize(bStmt);

    dbManager.commitTransaction();
    return true;
}

bool Library::returnBook(int issueId, std::string* outError) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return false;

    const char* fetchSql = "SELECT book_id, member_id, due_date, status FROM issues WHERE issue_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, fetchSql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, issueId);

    int bookId = 0;
    int memberId = 0;
    std::string dueDate;
    std::string currentStatus;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        bookId = sqlite3_column_int(stmt, 0);
        memberId = sqlite3_column_int(stmt, 1);
        dueDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        currentStatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    } else {
        sqlite3_finalize(stmt);
        if (outError) *outError = "Issue record #" + std::to_string(issueId) + " not found.";
        return false;
    }
    sqlite3_finalize(stmt);

    if (currentStatus == "RETURNED") {
        if (outError) *outError = "This book was already returned.";
        return false;
    }

    dbManager.beginTransaction();

    std::string today = Utils::getCurrentDate();
    int overdueDays = Utils::daysBetweenDates(dueDate, today);
    if (overdueDays < 0) overdueDays = 0;

    double fineAmt = Fine::calculateFineAmount(overdueDays);

    // 1. Update issue record
    const char* upIssueSql = "UPDATE issues SET return_date = ?, status = 'RETURNED', fine_amount = ? WHERE issue_id = ?;";
    sqlite3_stmt* uStmt = nullptr;
    if (sqlite3_prepare_v2(db, upIssueSql, -1, &uStmt, nullptr) != SQLITE_OK) {
        dbManager.rollbackTransaction();
        return false;
    }

    sqlite3_bind_text(uStmt, 1, today.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(uStmt, 2, fineAmt);
    sqlite3_bind_int(uStmt, 3, issueId);

    if (sqlite3_step(uStmt) != SQLITE_DONE) {
        sqlite3_finalize(uStmt);
        dbManager.rollbackTransaction();
        if (outError) *outError = "Failed to update return record.";
        return false;
    }
    sqlite3_finalize(uStmt);

    // 2. Increment available copies
    const char* incSql = "UPDATE books SET available_copies = available_copies + 1 WHERE id = ?;";
    sqlite3_stmt* bStmt = nullptr;
    if (sqlite3_prepare_v2(db, incSql, -1, &bStmt, nullptr) != SQLITE_OK) {
        dbManager.rollbackTransaction();
        return false;
    }

    sqlite3_bind_int(bStmt, 1, bookId);
    if (sqlite3_step(bStmt) != SQLITE_DONE) {
        sqlite3_finalize(bStmt);
        dbManager.rollbackTransaction();
        if (outError) *outError = "Failed to update book copies.";
        return false;
    }
    sqlite3_finalize(bStmt);

    // 3. Create Fine record if overdue
    if (fineAmt > 0) {
        const char* fineSql = "INSERT INTO fines (issue_id, member_id, amount, paid_status, reason) VALUES (?, ?, ?, 'PENDING', ?);";
        sqlite3_stmt* fStmt = nullptr;
        if (sqlite3_prepare_v2(db, fineSql, -1, &fStmt, nullptr) == SQLITE_OK) {
            std::string reason = "Overdue Book Return (" + std::to_string(overdueDays) + " days late)";
            sqlite3_bind_int(fStmt, 1, issueId);
            sqlite3_bind_int(fStmt, 2, memberId);
            sqlite3_bind_double(fStmt, 3, fineAmt);
            sqlite3_bind_text(fStmt, 4, reason.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(fStmt);
            sqlite3_finalize(fStmt);
        }
    }

    dbManager.commitTransaction();

    if (fineAmt > 0 && outError) {
        *outError = "Book returned successfully. Overdue fine generated: $" + std::to_string(fineAmt) + " (" + std::to_string(overdueDays) + " days late).";
    }

    return true;
}

bool Library::returnBookByMemberAndBook(int memberId, int bookId, std::string* outError) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return false;

    const char* sql = "SELECT issue_id FROM issues WHERE member_id = ? AND book_id = ? AND status IN ('ISSUED', 'OVERDUE') ORDER BY issue_id ASC LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    int issueId = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, memberId);
        sqlite3_bind_int(stmt, 2, bookId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            issueId = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (issueId == 0) {
        if (outError) *outError = "No active borrowed record found for this member and book combination.";
        return false;
    }

    return returnBook(issueId, outError);
}

std::vector<IssueRecord> Library::getAllIssueRecords() {
    std::vector<IssueRecord> records;
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return records;

    const char* sql = "SELECT i.issue_id, i.book_id, b.title, i.member_id, u.full_name, "
                      "i.issue_date, i.due_date, i.return_date, i.status, i.fine_amount "
                      "FROM issues i JOIN books b ON i.book_id = b.id JOIN users u ON i.member_id = u.id "
                      "ORDER BY i.issue_id DESC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int issueId = sqlite3_column_int(stmt, 0);
            int bookId = sqlite3_column_int(stmt, 1);
            std::string bTitle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            int memId = sqlite3_column_int(stmt, 3);
            std::string mName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            std::string iDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            std::string dDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            const char* rTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            std::string rDate = rTxt ? rTxt : "";
            std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            double fineAmt = sqlite3_column_double(stmt, 9);

            records.emplace_back(issueId, bookId, bTitle, memId, mName, iDate, dDate, rDate, status, fineAmt);
        }
        sqlite3_finalize(stmt);
    }
    return records;
}

std::vector<IssueRecord> Library::getMemberIssueHistory(int memberId) {
    std::vector<IssueRecord> records;
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return records;

    const char* sql = "SELECT i.issue_id, i.book_id, b.title, i.member_id, u.full_name, "
                      "i.issue_date, i.due_date, i.return_date, i.status, i.fine_amount "
                      "FROM issues i JOIN books b ON i.book_id = b.id JOIN users u ON i.member_id = u.id "
                      "WHERE i.member_id = ? ORDER BY i.issue_id DESC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, memberId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int issueId = sqlite3_column_int(stmt, 0);
            int bookId = sqlite3_column_int(stmt, 1);
            std::string bTitle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            int memId = sqlite3_column_int(stmt, 3);
            std::string mName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            std::string iDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            std::string dDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            const char* rTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            std::string rDate = rTxt ? rTxt : "";
            std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            double fineAmt = sqlite3_column_double(stmt, 9);

            records.emplace_back(issueId, bookId, bTitle, memId, mName, iDate, dDate, rDate, status, fineAmt);
        }
        sqlite3_finalize(stmt);
    }
    return records;
}

std::vector<IssueRecord> Library::getActiveIssuedBooks(int memberId) {
    std::vector<IssueRecord> all = getMemberIssueHistory(memberId);
    std::vector<IssueRecord> active;
    for (const auto& rec : all) {
        if (!rec.isReturned()) active.push_back(rec);
    }
    return active;
}

std::vector<IssueRecord> Library::getOverdueIssueRecords() {
    updateOverdueStatusAll();
    std::vector<IssueRecord> records;
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return records;

    const char* sql = "SELECT i.issue_id, i.book_id, b.title, i.member_id, u.full_name, "
                      "i.issue_date, i.due_date, i.return_date, i.status, i.fine_amount "
                      "FROM issues i JOIN books b ON i.book_id = b.id JOIN users u ON i.member_id = u.id "
                      "WHERE i.status = 'OVERDUE' ORDER BY i.due_date ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int issueId = sqlite3_column_int(stmt, 0);
            int bookId = sqlite3_column_int(stmt, 1);
            std::string bTitle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            int memId = sqlite3_column_int(stmt, 3);
            std::string mName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            std::string iDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            std::string dDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            const char* rTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            std::string rDate = rTxt ? rTxt : "";
            std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            double fineAmt = sqlite3_column_double(stmt, 9);

            records.emplace_back(issueId, bookId, bTitle, memId, mName, iDate, dDate, rDate, status, fineAmt);
        }
        sqlite3_finalize(stmt);
    }
    return records;
}

std::vector<Fine> Library::getAllFines() {
    std::vector<Fine> fines;
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return fines;

    const char* sql = "SELECT f.fine_id, f.issue_id, f.member_id, u.full_name, f.amount, f.paid_status, f.payment_date, f.reason "
                      "FROM fines f JOIN users u ON f.member_id = u.id ORDER BY f.fine_id DESC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int fineId = sqlite3_column_int(stmt, 0);
            int issueId = sqlite3_column_int(stmt, 1);
            int memId = sqlite3_column_int(stmt, 2);
            std::string mName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            double amt = sqlite3_column_double(stmt, 4);
            std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            const char* pTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            std::string pDate = pTxt ? pTxt : "";
            const char* rTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            std::string reason = rTxt ? rTxt : "Overdue Return";

            fines.emplace_back(fineId, issueId, memId, mName, amt, status, pDate, reason);
        }
        sqlite3_finalize(stmt);
    }
    return fines;
}

std::vector<Fine> Library::getMemberFines(int memberId, bool pendingOnly) {
    std::vector<Fine> fines;
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return fines;

    std::string sql = "SELECT f.fine_id, f.issue_id, f.member_id, u.full_name, f.amount, f.paid_status, f.payment_date, f.reason "
                      "FROM fines f JOIN users u ON f.member_id = u.id WHERE f.member_id = ?";
    if (pendingOnly) {
        sql += " AND f.paid_status = 'PENDING'";
    }
    sql += " ORDER BY f.fine_id DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, memberId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int fineId = sqlite3_column_int(stmt, 0);
            int issueId = sqlite3_column_int(stmt, 1);
            int memId = sqlite3_column_int(stmt, 2);
            std::string mName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            double amt = sqlite3_column_double(stmt, 4);
            std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            const char* pTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            std::string pDate = pTxt ? pTxt : "";
            const char* rTxt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            std::string reason = rTxt ? rTxt : "Overdue Return";

            fines.emplace_back(fineId, issueId, memId, mName, amt, status, pDate, reason);
        }
        sqlite3_finalize(stmt);
    }
    return fines;
}

bool Library::payFine(int fineId, std::string* outError) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return false;

    std::string today = Utils::getCurrentDate();
    const char* sql = "UPDATE fines SET paid_status = 'PAID', payment_date = ? WHERE fine_id = ? AND paid_status = 'PENDING';";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, today.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, fineId);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!ok && outError) {
        *outError = "Fine ID #" + std::to_string(fineId) + " was not found or already paid.";
    }
    sqlite3_finalize(stmt);
    return ok;
}
