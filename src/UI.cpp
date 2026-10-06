#include "UI.hpp"
#include "Utils.hpp"
#include <iostream>
#include <iomanip>
#include <string>

UI::UI()
    : dbManager("database/library.db"), authManager(dbManager), library(dbManager), currentUser(nullptr) {
    dbManager.open();
    dbManager.initializeSchema("database/schema.sql");
}

void UI::run() {
    while (true) {
        Utils::clearScreen();
        renderBanner();

        std::cout << Color::BOLD << Color::CYAN << "MAIN NAVIGATION MENU" << Color::RESET << "\n";
        std::cout << "  1. Login to Account\n";
        std::cout << "  2. Register New Member Account\n";
        std::cout << "  3. View Demo Credentials & Project Info\n";
        std::cout << "  4. Exit Application\n\n";

        int choice = Utils::readInt("Select an option (1-4): ", 1, 4);

        if (choice == 1) {
            handleLogin();
        } else if (choice == 2) {
            handleRegister();
        } else if (choice == 3) {
            displayDemoInfo();
        } else if (choice == 4) {
            Utils::clearScreen();
            std::cout << Color::BOLD << Color::GREEN << "\nThank you for using Library Management System!\nExiting...\n\n" << Color::RESET;
            break;
        }
    }
}

void UI::renderBanner() {
    std::cout << Color::BOLD << Color::CYAN;
    std::cout << "========================================================================\n";
    std::cout << "   LIBRARY MANAGEMENT SYSTEM v2.0 (C++17 & SQLite3 Architecture)\n";
    std::cout << "========================================================================\n";
    std::cout << Color::RESET;
}

void UI::displayDemoInfo() {
    Utils::clearScreen();
    Utils::printHeader("DEMO CREDENTIALS & SYSTEM ARCHITECTURE");

    std::cout << Color::BOLD << Color::YELLOW << "Default Admin Account:" << Color::RESET << "\n";
    std::cout << "  Username: " << Color::CYAN << "admin" << Color::RESET << "\n";
    std::cout << "  Password: " << Color::CYAN << "admin123" << Color::RESET << "\n\n";

    std::cout << Color::BOLD << Color::YELLOW << "Sample Member Accounts:" << Color::RESET << "\n";
    std::cout << "  1. Username: " << Color::CYAN << "john_doe" << Color::RESET << "   | Password: " << Color::CYAN << "user123" << Color::RESET << " (Has overdue book & fine)\n";
    std::cout << "  2. Username: " << Color::CYAN << "jane_smith" << Color::RESET << " | Password: " << Color::CYAN << "user123" << Color::RESET << " (Premium member)\n\n";

    std::cout << Color::BOLD << Color::YELLOW << "Technical Highlights:" << Color::RESET << "\n";
    std::cout << "  - Standard: C++17 ISO Compliant\n";
    std::cout << "  - Data Persistence: SQLite3 relational database\n";
    std::cout << "  - Security: Standard Bitwise SHA-256 password hashing\n";
    std::cout << "  - Architecture: Modular OOP (Polymorphism, RAII, DAO Pattern)\n\n";

    Utils::pauseScreen();
}

void UI::handleLogin() {
    Utils::clearScreen();
    Utils::printHeader("ACCOUNT LOGIN");

    std::string username = Utils::readLine("Enter Username: ");
    std::string password = Utils::readPassword("Enter Password: ");

    currentUser = authManager.login(username, password);

    if (!currentUser) {
        Utils::printError("Invalid username or password!");
        Utils::pauseScreen();
        return;
    }

    Utils::printSuccess("Welcome, " + currentUser->getFullName() + " (" + currentUser->getRoleString() + ")!");
    Utils::pauseScreen();

    if (currentUser->isAdmin()) {
        adminMenu();
    } else {
        memberMenu();
    }

    currentUser = nullptr; // Logout
}

void UI::handleRegister() {
    Utils::clearScreen();
    Utils::printHeader("NEW MEMBER REGISTRATION");

    std::string username = Utils::readLine("Enter Desired Username: ");
    std::string password = Utils::readPassword("Enter Password: ");
    std::string fullName = Utils::readLine("Enter Full Name: ");
    std::string email = Utils::readLine("Enter Email Address: ");
    std::string phone = Utils::readLine("Enter Phone Number: ");

    std::cout << "\nMembership Types:\n";
    std::cout << "  1. REGULAR (5 books limit)\n";
    std::cout << "  2. PREMIUM (10 books limit)\n";
    std::cout << "  3. FACULTY (8 books limit)\n";
    int typeChoice = Utils::readInt("Choose Membership Type (1-3): ", 1, 3);
    std::string mType = (typeChoice == 2) ? "PREMIUM" : ((typeChoice == 3) ? "FACULTY" : "REGULAR");

    std::string errorMsg;
    bool success = authManager.registerMember(username, password, fullName, email, phone, mType, &errorMsg);

    if (success) {
        Utils::printSuccess("Account created successfully! You can now log in with your credentials.");
    } else {
        Utils::printError("Registration Failed: " + errorMsg);
    }
    Utils::pauseScreen();
}

// ==================== ADMIN MENU ====================

void UI::adminMenu() {
    while (true) {
        Utils::clearScreen();
        renderBanner();

        std::cout << Color::BOLD << Color::MAGENTA << "ADMINISTRATOR DASHBOARD & CONTROL PANEL" << Color::RESET << "\n";
        std::cout << "Logged in as: " << Color::YELLOW << currentUser->getFullName() << Color::RESET << "\n\n";

        std::cout << "  1. View Real-time Dashboard & Analytics\n";
        std::cout << "  2. Manage Book Catalog (View, Add, Search, Update, Delete)\n";
        std::cout << "  3. Manage Book Categories\n";
        std::cout << "  4. Manage Member Accounts (View, Search, Change Status, Delete)\n";
        std::cout << "  5. Issue Book to Member\n";
        std::cout << "  6. Process Book Return\n";
        std::cout << "  7. View All Borrowing History & Active Issues\n";
        std::cout << "  8. View Overdue Books Report\n";
        std::cout << "  9. Manage Fines & Collections\n";
        std::cout << "  10. Logout\n\n";

        int choice = Utils::readInt("Select Admin Action (1-10): ", 1, 10);

        if (choice == 1) renderDashboard();
        else if (choice == 2) adminManageBooks();
        else if (choice == 3) adminManageCategories();
        else if (choice == 4) adminManageMembers();
        else if (choice == 5) adminIssueBook();
        else if (choice == 6) adminReturnBook();
        else if (choice == 7) adminViewIssueHistory();
        else if (choice == 8) adminViewOverdueBooks();
        else if (choice == 9) adminManageFines();
        else if (choice == 10) break;
    }
}

void UI::renderDashboard() {
    Utils::clearScreen();
    Utils::printHeader("REAL-TIME SYSTEM DASHBOARD");

    DashboardStats stats = library.getDashboardStats();

    std::cout << Color::BOLD << Color::CYAN << "+----------------------------------------+--------------------+" << Color::RESET << "\n";
    std::cout << Color::BOLD << Color::CYAN << "| METRIC                                 | VALUE              |" << Color::RESET << "\n";
    std::cout << Color::BOLD << Color::CYAN << "+----------------------------------------+--------------------+" << Color::RESET << "\n";
    std::cout << "| Total Books in Library (Copies)        | " << std::left << std::setw(18) << stats.totalBooks << " |\n";
    std::cout << "| Available Books for Issue              | " << std::left << std::setw(18) << stats.availableBooks << " |\n";
    std::cout << "| Currently Issued Books                 | " << std::left << std::setw(18) << stats.issuedBooks << " |\n";
    std::cout << "| Total Registered Members               | " << std::left << std::setw(18) << stats.totalMembers << " |\n";
    std::cout << "| Overdue Books                          | " << std::left << Color::RED << std::setw(18) << stats.overdueBooks << Color::RESET << " |\n";
    std::cout << "| Total Fines Collected                  | $" << std::left << std::setw(17) << std::fixed << std::setprecision(2) << stats.totalFinesCollected << " |\n";
    std::cout << "| Total Fines Pending                    | $" << std::left << Color::RED << std::setw(17) << std::fixed << std::setprecision(2) << stats.totalFinesPending << Color::RESET << " |\n";
    std::cout << Color::BOLD << Color::CYAN << "+----------------------------------------+--------------------+" << Color::RESET << "\n\n";

    Utils::pauseScreen();
}

void UI::adminManageBooks() {
    while (true) {
        Utils::clearScreen();
        Utils::printHeader("BOOK CATALOG MANAGEMENT");

        auto books = library.getAllBooks();
        renderBookTable(books);

        std::cout << "\nActions:\n";
        std::cout << "  1. Add New Book\n";
        std::cout << "  2. Search / Filter Books\n";
        std::cout << "  3. Update Book Details\n";
        std::cout << "  4. Delete Book\n";
        std::cout << "  5. Back to Admin Menu\n\n";

        int choice = Utils::readInt("Select choice (1-5): ", 1, 5);

        if (choice == 1) adminAddBook();
        else if (choice == 2) {
            std::string kw = Utils::readLine("Enter search keyword (Title/Author/ISBN): ", true);
            auto filtered = library.searchBooks(kw, 0);
            Utils::clearScreen();
            Utils::printHeader("SEARCH RESULTS (" + std::to_string(filtered.size()) + " found)");
            renderBookTable(filtered);
            Utils::pauseScreen();
        }
        else if (choice == 3) adminUpdateBook();
        else if (choice == 4) adminDeleteBook();
        else if (choice == 5) break;
    }
}

void UI::adminAddBook() {
    Utils::clearScreen();
    Utils::printHeader("ADD NEW BOOK TO CATALOG");

    std::string title = Utils::readLine("Enter Book Title: ");
    std::string author = Utils::readLine("Enter Author Name: ");
    std::string isbn = Utils::readLine("Enter ISBN Number: ");
    std::string publisher = Utils::readLine("Enter Publisher: ");
    int year = Utils::readInt("Enter Publication Year: ", 1500, 2030);
    int copies = Utils::readInt("Enter Total Copies: ", 1, 1000);
    double price = Utils::readDouble("Enter Price ($): ", 0.0);
    std::string rack = Utils::readLine("Enter Rack/Shelf Location (e.g. CS-A1): ");

    auto categories = library.getAllCategories();
    int catId = 0;
    if (!categories.empty()) {
        std::cout << "\nAvailable Categories:\n";
        for (const auto& c : categories) {
            std::cout << "  [" << c.id << "] " << c.name << "\n";
        }
        catId = Utils::readInt("Select Category ID (0 for none): ", 0, 9999);
    }

    Book newBook(0, title, author, isbn, catId, "", publisher, year, copies, copies, price, rack);

    std::string err;
    if (library.addBook(newBook, &err)) {
        Utils::printSuccess("Book '" + title + "' added successfully to database!");
    } else {
        Utils::printError("Failed to add book: " + err);
    }
    Utils::pauseScreen();
}

void UI::adminUpdateBook() {
    int bookId = Utils::readInt("Enter Book ID to Update: ", 1, 999999);
    auto bookOpt = library.getBookById(bookId);

    if (!bookOpt.has_value()) {
        Utils::printError("Book ID #" + std::to_string(bookId) + " not found.");
        Utils::pauseScreen();
        return;
    }

    Book book = bookOpt.value();
    std::cout << "\nEditing Book: " << book.getTitle() << "\n";

    std::string title = Utils::readLine("New Title (leave empty to keep current): ", true);
    if (!title.empty()) book.setTitle(title);

    std::string author = Utils::readLine("New Author (leave empty to keep current): ", true);
    if (!author.empty()) book.setAuthor(author);

    std::string isbn = Utils::readLine("New ISBN (leave empty to keep current): ", true);
    if (!isbn.empty()) book.setIsbn(isbn);

    int totalCopies = Utils::readInt("New Total Copies (" + std::to_string(book.getTotalCopies()) + "): ", 1, 1000);
    int diff = totalCopies - book.getTotalCopies();
    book.setTotalCopies(totalCopies);
    book.setAvailableCopies(std::max(0, book.getAvailableCopies() + diff));

    double price = Utils::readDouble("New Price ($" + std::to_string(book.getPrice()) + "): ", 0.0);
    book.setPrice(price);

    std::string rack = Utils::readLine("New Rack (" + book.getRackNumber() + "): ", true);
    if (!rack.empty()) book.setRackNumber(rack);

    std::string err;
    if (library.updateBook(book, &err)) {
        Utils::printSuccess("Book updated successfully!");
    } else {
        Utils::printError("Update failed: " + err);
    }
    Utils::pauseScreen();
}

void UI::adminDeleteBook() {
    int bookId = Utils::readInt("Enter Book ID to Delete: ", 1, 999999);
    std::string err;
    if (library.deleteBook(bookId, &err)) {
        Utils::printSuccess("Book ID #" + std::to_string(bookId) + " deleted successfully.");
    } else {
        Utils::printError("Delete failed: " + err);
    }
    Utils::pauseScreen();
}

void UI::adminManageCategories() {
    Utils::clearScreen();
    Utils::printHeader("CATEGORY MANAGEMENT");

    auto categories = library.getAllCategories();
    std::cout << Color::BOLD << std::left << std::setw(6) << "ID" << std::setw(25) << "NAME" << "DESCRIPTION" << Color::RESET << "\n";
    std::cout << "---------------------------------------------------------\n";
    for (const auto& c : categories) {
        std::cout << std::left << std::setw(6) << c.id << std::setw(25) << c.name << c.description << "\n";
    }
    std::cout << "\n";

    std::string name = Utils::readLine("Enter new category name (leave empty to cancel): ", true);
    if (!name.empty()) {
        std::string desc = Utils::readLine("Enter description: ", true);
        std::string err;
        if (library.addCategory(name, desc, &err)) {
            Utils::printSuccess("Category '" + name + "' added successfully.");
        } else {
            Utils::printError("Failed to add category: " + err);
        }
        Utils::pauseScreen();
    }
}

void UI::adminManageMembers() {
    while (true) {
        Utils::clearScreen();
        Utils::printHeader("MEMBER ACCOUNT MANAGEMENT");

        auto members = library.getAllMembers();
        renderMemberTable(members);

        std::cout << "\nActions:\n";
        std::cout << "  1. Search Members\n";
        std::cout << "  2. Change Member Account Status (ACTIVE / SUSPENDED)\n";
        std::cout << "  3. Delete Member Account\n";
        std::cout << "  4. Back to Admin Menu\n\n";

        int choice = Utils::readInt("Select choice (1-4): ", 1, 4);

        if (choice == 1) {
            std::string kw = Utils::readLine("Enter search keyword (Username/Name/Email): ");
            auto filtered = library.searchMembers(kw);
            Utils::clearScreen();
            Utils::printHeader("MEMBER SEARCH RESULTS (" + std::to_string(filtered.size()) + " found)");
            renderMemberTable(filtered);
            Utils::pauseScreen();
        } else if (choice == 2) adminChangeMemberStatus();
        else if (choice == 3) adminDeleteMember();
        else if (choice == 4) break;
    }
}

void UI::adminChangeMemberStatus() {
    int memberId = Utils::readInt("Enter Member User ID: ", 1, 999999);
    auto member = library.getMemberById(memberId);
    if (!member) {
        Utils::printError("Member ID #" + std::to_string(memberId) + " not found.");
        Utils::pauseScreen();
        return;
    }

    std::cout << "Current Status of " << member->getFullName() << ": " << member->getStatus() << "\n";
    std::cout << "Select New Status:\n  1. ACTIVE\n  2. SUSPENDED\n";
    int stChoice = Utils::readInt("Choice (1-2): ", 1, 2);
    std::string newStatus = (stChoice == 1) ? "ACTIVE" : "SUSPENDED";

    std::string err;
    if (library.updateMemberStatus(memberId, newStatus, &err)) {
        Utils::printSuccess("Member status updated to " + newStatus + ".");
    } else {
        Utils::printError("Failed: " + err);
    }
    Utils::pauseScreen();
}

void UI::adminDeleteMember() {
    int memberId = Utils::readInt("Enter Member User ID to Delete: ", 1, 999999);
    std::string err;
    if (library.deleteMember(memberId, &err)) {
        Utils::printSuccess("Member ID #" + std::to_string(memberId) + " deleted successfully.");
    } else {
        Utils::printError("Delete failed: " + err);
    }
    Utils::pauseScreen();
}

void UI::adminIssueBook() {
    Utils::clearScreen();
    Utils::printHeader("ISSUE BOOK TO MEMBER");

    int bookId = Utils::readInt("Enter Book ID to Issue: ", 1, 999999);
    int memberId = Utils::readInt("Enter Member User ID: ", 1, 999999);
    int days = Utils::readInt("Enter Loan Period in Days (default 14): ", 1, 90);

    std::string err;
    if (library.issueBook(bookId, memberId, days, &err)) {
        Utils::printSuccess("Book ID #" + std::to_string(bookId) + " successfully issued to Member ID #" + std::to_string(memberId) + " for " + std::to_string(days) + " days.");
    } else {
        Utils::printError("Issue Operation Failed: " + err);
    }
    Utils::pauseScreen();
}

void UI::adminReturnBook() {
    Utils::clearScreen();
    Utils::printHeader("PROCESS BOOK RETURN");

    std::cout << "  1. Return by Issue Record ID\n";
    std::cout << "  2. Return by Member ID & Book ID\n";
    int mode = Utils::readInt("Choice (1-2): ", 1, 2);

    std::string err;
    bool success = false;

    if (mode == 1) {
        int issueId = Utils::readInt("Enter Issue Record ID: ", 1, 999999);
        success = library.returnBook(issueId, &err);
    } else {
        int memberId = Utils::readInt("Enter Member User ID: ", 1, 999999);
        int bookId = Utils::readInt("Enter Book ID: ", 1, 999999);
        success = library.returnBookByMemberAndBook(memberId, bookId, &err);
    }

    if (success) {
        Utils::printSuccess("Book return processed successfully!");
        if (!err.empty()) Utils::printWarning(err);
    } else {
        Utils::printError("Return Failed: " + err);
    }
    Utils::pauseScreen();
}

void UI::adminViewIssueHistory() {
    Utils::clearScreen();
    Utils::printHeader("ALL BORROWING RECORDS & HISTORY");
    auto records = library.getAllIssueRecords();
    renderIssueTable(records);
    Utils::pauseScreen();
}

void UI::adminViewOverdueBooks() {
    Utils::clearScreen();
    Utils::printHeader("OVERDUE BOOKS REPORT");
    auto overdue = library.getOverdueIssueRecords();
    renderIssueTable(overdue);
    Utils::pauseScreen();
}

void UI::adminManageFines() {
    Utils::clearScreen();
    Utils::printHeader("FINES MANAGEMENT & COLLECTION");

    auto fines = library.getAllFines();
    renderFineTable(fines);

    std::cout << "\nActions:\n";
    std::cout << "  1. Record Fine Payment\n";
    std::cout << "  2. Return to Admin Menu\n\n";

    int ch = Utils::readInt("Select choice (1-2): ", 1, 2);
    if (ch == 1) {
        int fineId = Utils::readInt("Enter Fine ID to mark as PAID: ", 1, 999999);
        std::string err;
        if (library.payFine(fineId, &err)) {
            Utils::printSuccess("Fine #" + std::to_string(fineId) + " payment recorded successfully.");
        } else {
            Utils::printError("Failed: " + err);
        }
        Utils::pauseScreen();
    }
}

// ==================== MEMBER MENU ====================

void UI::memberMenu() {
    auto member = std::dynamic_pointer_cast<Member>(currentUser);

    while (true) {
        Utils::clearScreen();
        renderBanner();

        std::cout << Color::BOLD << Color::CYAN << "MEMBER PORTAL" << Color::RESET << "\n";
        std::cout << "Welcome, " << Color::YELLOW << currentUser->getFullName() << Color::RESET;
        std::cout << " (Membership: " << member->getMembershipType() << ")\n\n";

        std::cout << "  1. Search & View Book Catalog\n";
        std::cout << "  2. View My Currently Borrowed Books\n";
        std::cout << "  3. View My Complete Borrowing History\n";
        std::cout << "  4. View My Fines & Pay Fine\n";
        std::cout << "  5. View My Account Profile & Password\n";
        std::cout << "  6. Logout\n\n";

        int choice = Utils::readInt("Select Member Action (1-6): ", 1, 6);

        if (choice == 1) memberSearchBooks();
        else if (choice == 2) memberViewMyBorrowedBooks();
        else if (choice == 3) memberViewMyHistory();
        else if (choice == 4) memberViewMyFines();
        else if (choice == 5) memberViewProfile();
        else if (choice == 6) break;
    }
}

void UI::memberSearchBooks() {
    Utils::clearScreen();
    Utils::printHeader("BOOK CATALOG SEARCH");

    std::string kw = Utils::readLine("Enter search keyword (Title/Author/ISBN or empty for all): ", true);
    auto books = library.searchBooks(kw, 0);

    renderBookTable(books);
    Utils::pauseScreen();
}

void UI::memberViewMyBorrowedBooks() {
    Utils::clearScreen();
    Utils::printHeader("MY CURRENTLY BORROWED BOOKS");

    auto activeRecords = library.getActiveIssuedBooks(currentUser->getId());
    renderIssueTable(activeRecords);
    Utils::pauseScreen();
}

void UI::memberViewMyHistory() {
    Utils::clearScreen();
    Utils::printHeader("MY COMPLETE BORROWING HISTORY");

    auto history = library.getMemberIssueHistory(currentUser->getId());
    renderIssueTable(history);
    Utils::pauseScreen();
}

void UI::memberViewMyFines() {
    Utils::clearScreen();
    Utils::printHeader("MY FINES AND PAYMENTS");

    auto fines = library.getMemberFines(currentUser->getId());
    renderFineTable(fines);

    std::cout << "\nActions:\n";
    std::cout << "  1. Pay Outstanding Fine Online\n";
    std::cout << "  2. Back to Member Menu\n\n";

    int ch = Utils::readInt("Choice (1-2): ", 1, 2);
    if (ch == 1) {
        int fineId = Utils::readInt("Enter Fine ID to Pay: ", 1, 999999);
        std::string err;
        if (library.payFine(fineId, &err)) {
            Utils::printSuccess("Payment of fine #" + std::to_string(fineId) + " successful! Thank you.");
        } else {
            Utils::printError("Payment failed: " + err);
        }
        Utils::pauseScreen();
    }
}

void UI::memberViewProfile() {
    Utils::clearScreen();
    Utils::printHeader("MY ACCOUNT PROFILE");

    currentUser->displayInfo();

    std::cout << "\nOptions:\n";
    std::cout << "  1. Change Password\n";
    std::cout << "  2. Back\n\n";

    int ch = Utils::readInt("Choice (1-2): ", 1, 2);
    if (ch == 1) memberChangePassword();
}

void UI::memberChangePassword() {
    std::string oldPass = Utils::readPassword("Enter Current Password: ");
    std::string newPass = Utils::readPassword("Enter New Password: ");
    std::string confirmPass = Utils::readPassword("Confirm New Password: ");

    if (newPass != confirmPass) {
        Utils::printError("New passwords do not match!");
        Utils::pauseScreen();
        return;
    }

    if (authManager.changePassword(currentUser->getId(), oldPass, newPass)) {
        Utils::printSuccess("Password updated successfully!");
    } else {
        Utils::printError("Failed to update password. Current password may be incorrect.");
    }
    Utils::pauseScreen();
}

// ==================== TABLE RENDERING UTILITIES ====================

void UI::renderBookTable(const std::vector<Book>& books) {
    if (books.empty()) {
        Utils::printInfo("No books found.");
        return;
    }

    std::cout << Color::BOLD << Color::CYAN
              << "+-----+------------------------------------+-------------------------+---------------+------------+---------+-------+\n"
              << "| ID  | TITLE                              | AUTHOR                  | ISBN          | CATEGORY   | COPIES  | RACK  |\n"
              << "+-----+------------------------------------+-------------------------+---------------+------------+---------+-------+\n"
              << Color::RESET;

    for (const auto& b : books) {
        std::cout << "| " << std::left << std::setw(3) << b.getId()
                  << " | " << std::setw(34) << Utils::truncate(b.getTitle(), 34)
                  << " | " << std::setw(23) << Utils::truncate(b.getAuthor(), 23)
                  << " | " << std::setw(13) << Utils::truncate(b.getIsbn(), 13)
                  << " | " << std::setw(10) << Utils::truncate(b.getCategoryName(), 10)
                  << " | " << std::right << std::setw(2) << b.getAvailableCopies() << "/" << std::left << std::setw(3) << b.getTotalCopies()
                  << " | " << std::setw(5) << b.getRackNumber() << " |\n";
    }

    std::cout << Color::BOLD << Color::CYAN
              << "+-----+------------------------------------+-------------------------+---------------+------------+---------+-------+\n"
              << Color::RESET;
}

void UI::renderMemberTable(const std::vector<std::shared_ptr<Member>>& members) {
    if (members.empty()) {
        Utils::printInfo("No members found.");
        return;
    }

    std::cout << Color::BOLD << Color::CYAN
              << "+-----+------------------+-------------------------+-------------------------+------------+----------+\n"
              << "| ID  | USERNAME         | FULL NAME               | EMAIL                   | TYPE       | STATUS   |\n"
              << "+-----+------------------+-------------------------+-------------------------+------------+----------+\n"
              << Color::RESET;

    for (const auto& m : members) {
        std::string st = m->getStatus();
        std::cout << "| " << std::left << std::setw(3) << m->getId()
                  << " | " << std::setw(16) << Utils::truncate(m->getUsername(), 16)
                  << " | " << std::setw(23) << Utils::truncate(m->getFullName(), 23)
                  << " | " << std::setw(23) << Utils::truncate(m->getEmail(), 23)
                  << " | " << std::setw(10) << m->getMembershipType()
                  << " | " << std::setw(8) << (st == "ACTIVE" ? Color::GREEN + "ACTIVE" + Color::RESET : Color::RED + "SUSPEND" + Color::RESET) << " |\n";
    }

    std::cout << Color::BOLD << Color::CYAN
              << "+-----+------------------+-------------------------+-------------------------+------------+----------+\n"
              << Color::RESET;
}

void UI::renderIssueTable(const std::vector<IssueRecord>& records) {
    if (records.empty()) {
        Utils::printInfo("No borrowing records found.");
        return;
    }

    std::cout << Color::BOLD << Color::CYAN
              << "+-----+-----------------------------+--------------------+------------+------------+------------+----------+\n"
              << "| ID  | BOOK TITLE                  | MEMBER NAME        | ISSUE DATE | DUE DATE   | RETURNED   | STATUS   |\n"
              << "+-----+-----------------------------+--------------------+------------+------------+------------+----------+\n"
              << Color::RESET;

    for (const auto& r : records) {
        std::string st = r.getStatus();
        std::string colStatus;
        if (st == "RETURNED") colStatus = Color::GREEN + "RETURNED" + Color::RESET;
        else if (r.isOverdue()) colStatus = Color::RED + "OVERDUE " + Color::RESET;
        else colStatus = Color::YELLOW + "ISSUED  " + Color::RESET;

        std::cout << "| " << std::left << std::setw(3) << r.getIssueId()
                  << " | " << std::setw(27) << Utils::truncate(r.getBookTitle(), 27)
                  << " | " << std::setw(18) << Utils::truncate(r.getMemberName(), 18)
                  << " | " << std::setw(10) << r.getIssueDate()
                  << " | " << std::setw(10) << r.getDueDate()
                  << " | " << std::setw(10) << (r.getReturnDate().empty() ? "-" : r.getReturnDate())
                  << " | " << colStatus << " |\n";
    }

    std::cout << Color::BOLD << Color::CYAN
              << "+-----+-----------------------------+--------------------+------------+------------+------------+----------+\n"
              << Color::RESET;
}

void UI::renderFineTable(const std::vector<Fine>& fines) {
    if (fines.empty()) {
        Utils::printInfo("No fine records found.");
        return;
    }

    std::cout << Color::BOLD << Color::CYAN
              << "+-----+----------+--------------------+-----------+------------+-----------------------------------+\n"
              << "| ID  | ISSUE #  | MEMBER NAME        | AMOUNT    | STATUS     | REASON                            |\n"
              << "+-----+----------+--------------------+-----------+------------+-----------------------------------+\n"
              << Color::RESET;

    for (const auto& f : fines) {
        std::string st = f.getPaidStatus();
        std::string stCol = (st == "PAID") ? Color::GREEN + "PAID    " + Color::RESET : Color::RED + "PENDING " + Color::RESET;

        std::cout << "| " << std::left << std::setw(3) << f.getFineId()
                  << " | " << std::setw(8) << f.getIssueId()
                  << " | " << std::setw(18) << Utils::truncate(f.getMemberName(), 18)
                  << " | $" << std::right << std::setw(8) << std::fixed << std::setprecision(2) << f.getAmount() << " "
                  << " | " << stCol
                  << " | " << std::left << std::setw(33) << Utils::truncate(f.getReason(), 33) << " |\n";
    }

    std::cout << Color::BOLD << Color::CYAN
              << "+-----+----------+--------------------+-----------+------------+-----------------------------------+\n"
              << Color::RESET;
}
