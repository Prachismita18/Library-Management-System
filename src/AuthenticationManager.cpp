#include "AuthenticationManager.hpp"
#include "Utils.hpp"
#include <sqlite3.h>

AuthenticationManager::AuthenticationManager(DatabaseManager& dbMgr)
    : dbManager(dbMgr) {}

bool AuthenticationManager::usernameExists(const std::string& username) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return false;

    const char* sql = "SELECT COUNT(*) FROM users WHERE username = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = (sqlite3_column_int(stmt, 0) > 0);
    }
    sqlite3_finalize(stmt);
    return exists;
}

std::shared_ptr<User> AuthenticationManager::login(const std::string& username, const std::string& password) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return nullptr;

    std::string inputHash = Utils::sha256(password);

    const char* sql = "SELECT id, username, password_hash, full_name, email, phone, role, created_at FROM users WHERE username = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    std::shared_ptr<User> user = nullptr;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string uName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string passHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        std::string phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        std::string roleStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        std::string createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        if (inputHash == passHash) {
            if (roleStr == "ADMIN") {
                user = std::make_shared<Admin>(id, uName, passHash, fullName, email, phone, "SUPER_ADMIN", createdAt);
            } else {
                // Fetch member details
                const char* memSql = "SELECT membership_type, max_books_allowed, status FROM members WHERE user_id = ?;";
                sqlite3_stmt* memStmt = nullptr;
                std::string memType = "REGULAR";
                int maxBooks = 5;
                std::string status = "ACTIVE";

                if (sqlite3_prepare_v2(db, memSql, -1, &memStmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_int(memStmt, 1, id);
                    if (sqlite3_step(memStmt) == SQLITE_ROW) {
                        memType = reinterpret_cast<const char*>(sqlite3_column_text(memStmt, 0));
                        maxBooks = sqlite3_column_int(memStmt, 1);
                        status = reinterpret_cast<const char*>(sqlite3_column_text(memStmt, 2));
                    }
                    sqlite3_finalize(memStmt);
                }

                user = std::make_shared<Member>(id, uName, passHash, fullName, email, phone, memType, maxBooks, status, createdAt);
            }
        }
    }

    sqlite3_finalize(stmt);
    return user;
}

bool AuthenticationManager::registerMember(const std::string& username, const std::string& password,
                                            const std::string& fullName, const std::string& email,
                                            const std::string& phone, const std::string& membershipType,
                                            std::string* outErrorMsg) {
    if (usernameExists(username)) {
        if (outErrorMsg) *outErrorMsg = "Username '" + username + "' is already taken.";
        return false;
    }

    sqlite3* db = dbManager.getDbHandle();
    if (!db) {
        if (outErrorMsg) *outErrorMsg = "Database connection error.";
        return false;
    }

    dbManager.beginTransaction();

    std::string passHash = Utils::sha256(password);
    const char* userSql = "INSERT INTO users (username, password_hash, full_name, email, phone, role) VALUES (?, ?, ?, ?, ?, 'MEMBER');";
    sqlite3_stmt* uStmt = nullptr;

    if (sqlite3_prepare_v2(db, userSql, -1, &uStmt, nullptr) != SQLITE_OK) {
        dbManager.rollbackTransaction();
        if (outErrorMsg) *outErrorMsg = "Failed to prepare user statement.";
        return false;
    }

    sqlite3_bind_text(uStmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(uStmt, 2, passHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(uStmt, 3, fullName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(uStmt, 4, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(uStmt, 5, phone.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(uStmt) != SQLITE_DONE) {
        sqlite3_finalize(uStmt);
        dbManager.rollbackTransaction();
        if (outErrorMsg) *outErrorMsg = "Failed to insert user record.";
        return false;
    }
    sqlite3_finalize(uStmt);

    int64_t userId = dbManager.getLastInsertedRowId();

    int maxBooks = (membershipType == "PREMIUM") ? 10 : ((membershipType == "FACULTY") ? 8 : 5);

    const char* memSql = "INSERT INTO members (user_id, membership_type, max_books_allowed, status) VALUES (?, ?, ?, 'ACTIVE');";
    sqlite3_stmt* mStmt = nullptr;

    if (sqlite3_prepare_v2(db, memSql, -1, &mStmt, nullptr) != SQLITE_OK) {
        dbManager.rollbackTransaction();
        if (outErrorMsg) *outErrorMsg = "Failed to prepare member statement.";
        return false;
    }

    sqlite3_bind_int64(mStmt, 1, userId);
    sqlite3_bind_text(mStmt, 2, membershipType.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(mStmt, 3, maxBooks);

    if (sqlite3_step(mStmt) != SQLITE_DONE) {
        sqlite3_finalize(mStmt);
        dbManager.rollbackTransaction();
        if (outErrorMsg) *outErrorMsg = "Failed to insert member details.";
        return false;
    }
    sqlite3_finalize(mStmt);

    dbManager.commitTransaction();
    return true;
}

bool AuthenticationManager::changePassword(int userId, const std::string& oldPassword, const std::string& newPassword) {
    sqlite3* db = dbManager.getDbHandle();
    if (!db) return false;

    // Verify old password
    const char* checkSql = "SELECT password_hash FROM users WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, checkSql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, userId);
    bool oldCorrect = false;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string currentHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (currentHash == Utils::sha256(oldPassword)) {
            oldCorrect = true;
        }
    }
    sqlite3_finalize(stmt);

    if (!oldCorrect) return false;

    // Update password
    const char* updateSql = "UPDATE users SET password_hash = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(db, updateSql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    std::string newHash = Utils::sha256(newPassword);
    sqlite3_bind_text(stmt, 1, newHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, userId);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}
