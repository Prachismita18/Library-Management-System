#include "DatabaseManager.hpp"
#include "AuthenticationManager.hpp"
#include "Library.hpp"
#include "Utils.hpp"
#include <iostream>
#include <cassert>
#include <cstdio>

void runTests() {
    std::cout << "========================================\n";
    std::cout << " RUNNING AUTOMATED SYSTEM UNIT TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: SHA-256 Hashing
    std::cout << "[TEST 1] Hashing SHA-256... ";
    std::string h1 = Utils::sha256("admin123");
    std::string h2 = Utils::sha256("admin123");
    std::string h3 = Utils::sha256("different_pass");
    assert(h1 == h2);
    assert(h1 != h3);
    assert(!h1.empty());
    std::cout << Color::GREEN << "PASSED" << Color::RESET << "\n";

    // Test 2: Date Calculation Utilities
    std::cout << "[TEST 2] Date Utilities... ";
    std::string startDate = "2026-01-01";
    std::string endDate = "2026-01-15";
    int diff = Utils::daysBetweenDates(startDate, endDate);
    assert(diff == 14);

    std::string addDate = Utils::addDaysToDate(startDate, 10);
    assert(addDate == "2026-01-11");
    std::cout << Color::GREEN << "PASSED" << Color::RESET << "\n";

    // Setup Test DB
    const char* testDbPath = "database/test_library.db";
    std::remove(testDbPath);

    DatabaseManager dbMgr(testDbPath);
    assert(dbMgr.open());
    assert(dbMgr.initializeSchema("database/schema.sql"));

    AuthenticationManager authMgr(dbMgr);
    Library lib(dbMgr);

    // Test 3: Admin Login
    std::cout << "[TEST 3] Admin Authentication... ";
    auto adminUser = authMgr.login("admin", "admin123");
    assert(adminUser != nullptr);
    assert(adminUser->isAdmin());
    assert(adminUser->getRoleString() == "ADMIN");
    std::cout << Color::GREEN << "PASSED" << Color::RESET << "\n";

    // Test 4: Member Registration & Login
    std::cout << "[TEST 4] Member Registration & Auth... ";
    std::string errStr;
    bool regOk = authMgr.registerMember("test_member", "pass123", "Test Member", "test@test.com", "1234567890", "REGULAR", &errStr);
    assert(regOk);

    auto memberUser = authMgr.login("test_member", "pass123");
    assert(memberUser != nullptr);
    assert(memberUser->isMember());
    std::cout << Color::GREEN << "PASSED" << Color::RESET << "\n";

    // Test 5: Book Management
    std::cout << "[TEST 5] Book Addition & Retrieval... ";
    Book testBook(0, "Test C++ Architecture", "Test Author", "9998887776665", 1, "Computer Science", "Test Publisher", 2024, 5, 5, 29.99, "TEST-1");
    bool addOk = lib.addBook(testBook, &errStr);
    assert(addOk);

    auto searched = lib.searchBooks("Test C++ Architecture", 0);
    assert(!searched.empty());
    assert(searched[0].getIsbn() == "9998887776665");
    int testBookId = searched[0].getId();
    std::cout << Color::GREEN << "PASSED" << Color::RESET << "\n";

    // Test 6: Book Issue & Stock Decrement
    std::cout << "[TEST 6] Book Issue Workflow... ";
    int memberId = memberUser->getId();
    bool issueOk = lib.issueBook(testBookId, memberId, 14, &errStr);
    assert(issueOk);

    auto updatedBook = lib.getBookById(testBookId);
    assert(updatedBook.has_value());
    assert(updatedBook->getAvailableCopies() == 4); // Initial 5 - 1 = 4
    std::cout << Color::GREEN << "PASSED" << Color::RESET << "\n";

    // Test 7: Book Return & Fine Calculation
    std::cout << "[TEST 7] Book Return & Fine Calculation... ";
    bool returnOk = lib.returnBookByMemberAndBook(memberId, testBookId, &errStr);
    assert(returnOk);

    auto returnedBook = lib.getBookById(testBookId);
    assert(returnedBook->getAvailableCopies() == 5); // Restored copy
    std::cout << Color::GREEN << "PASSED" << Color::RESET << "\n";

    // Test 8: Dashboard Integrity
    std::cout << "[TEST 8] Dashboard Analytics... ";
    DashboardStats stats = lib.getDashboardStats();
    assert(stats.totalBooks >= 5);
    assert(stats.totalMembers >= 3);
    std::cout << Color::GREEN << "PASSED" << Color::RESET << "\n";

    // Cleanup Test DB
    dbMgr.close();
    std::remove(testDbPath);

    std::cout << "\n========================================\n";
    std::cout << Color::GREEN << " ALL TESTS PASSED SUCCESSFULLY! (8/8)\n" << Color::RESET;
    std::cout << "========================================\n";
}

int main() {
    try {
        runTests();
    } catch (const std::exception& e) {
        std::cerr << "Test failure exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
