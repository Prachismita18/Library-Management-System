#ifndef UI_HPP
#define UI_HPP

#include "Library.hpp"
#include "AuthenticationManager.hpp"
#include <memory>

class UI {
private:
    DatabaseManager dbManager;
    AuthenticationManager authManager;
    Library library;
    std::shared_ptr<User> currentUser;

    // Helper rendering utilities
    void renderBanner();
    void renderDashboard();
    void renderBookTable(const std::vector<Book>& books);
    void renderMemberTable(const std::vector<std::shared_ptr<Member>>& members);
    void renderIssueTable(const std::vector<IssueRecord>& records);
    void renderFineTable(const std::vector<Fine>& fines);

    // Auth Flows
    void handleLogin();
    void handleRegister();
    void displayDemoInfo();

    // Admin Menus & Actions
    void adminMenu();
    void adminManageBooks();
    void adminAddBook();
    void adminUpdateBook();
    void adminDeleteBook();
    void adminManageCategories();
    void adminManageMembers();
    void adminChangeMemberStatus();
    void adminDeleteMember();
    void adminIssueBook();
    void adminReturnBook();
    void adminViewIssueHistory();
    void adminViewOverdueBooks();
    void adminManageFines();

    // Member Menus & Actions
    void memberMenu();
    void memberSearchBooks();
    void memberViewMyBorrowedBooks();
    void memberViewMyHistory();
    void memberViewMyFines();
    void memberViewProfile();
    void memberChangePassword();

public:
    UI();
    void run();
};

#endif // UI_HPP
