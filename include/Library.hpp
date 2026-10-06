#ifndef LIBRARY_HPP
#define LIBRARY_HPP

#include "DatabaseManager.hpp"
#include "Book.hpp"
#include "User.hpp"
#include "Member.hpp"
#include "IssueRecord.hpp"
#include "Fine.hpp"
#include <vector>
#include <string>
#include <memory>
#include <optional>

struct DashboardStats {
    int totalBooks;
    int availableBooks;
    int issuedBooks;
    int totalMembers;
    int overdueBooks;
    double totalFinesCollected;
    double totalFinesPending;
};

struct Category {
    int id;
    std::string name;
    std::string description;
};

class Library {
private:
    DatabaseManager& dbManager;

    void updateOverdueStatusAll();

public:
    explicit Library(DatabaseManager& dbMgr);

    // Dashboard
    DashboardStats getDashboardStats();

    // Book Management
    bool addBook(const Book& book, std::string* outError = nullptr);
    bool updateBook(const Book& book, std::string* outError = nullptr);
    bool deleteBook(int bookId, std::string* outError = nullptr);
    std::optional<Book> getBookById(int bookId);
    std::optional<Book> getBookByIsbn(const std::string& isbn);
    std::vector<Book> getAllBooks();
    std::vector<Book> searchBooks(const std::string& keyword, int categoryId = 0);

    // Category Management
    std::vector<Category> getAllCategories();
    bool addCategory(const std::string& name, const std::string& description, std::string* outError = nullptr);

    // Member Management
    std::vector<std::shared_ptr<Member>> getAllMembers();
    std::shared_ptr<Member> getMemberById(int memberId);
    bool updateMemberStatus(int memberId, const std::string& newStatus, std::string* outError = nullptr);
    bool updateMemberDetails(int memberId, const std::string& fullName, const std::string& email, const std::string& phone, std::string* outError = nullptr);
    bool deleteMember(int memberId, std::string* outError = nullptr);
    std::vector<std::shared_ptr<Member>> searchMembers(const std::string& keyword);

    // Issue & Return Operations
    bool issueBook(int bookId, int memberId, int daysToBorrow = 14, std::string* outError = nullptr);
    bool returnBook(int issueId, std::string* outError = nullptr);
    bool returnBookByMemberAndBook(int memberId, int bookId, std::string* outError = nullptr);

    // Issue History & Overdue Records
    std::vector<IssueRecord> getAllIssueRecords();
    std::vector<IssueRecord> getMemberIssueHistory(int memberId);
    std::vector<IssueRecord> getActiveIssuedBooks(int memberId);
    std::vector<IssueRecord> getOverdueIssueRecords();

    // Fine Management
    std::vector<Fine> getAllFines();
    std::vector<Fine> getMemberFines(int memberId, bool pendingOnly = false);
    bool payFine(int fineId, std::string* outError = nullptr);
    int activeBorrowedCount(int memberId);
};

#endif // LIBRARY_HPP
