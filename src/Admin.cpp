#include "Admin.hpp"
#include "Utils.hpp"

Admin::Admin() : User(), adminLevel("SUPER_ADMIN") {
    role = UserRole::ADMIN;
}

Admin::Admin(int id, const std::string& username, const std::string& passwordHash,
             const std::string& fullName, const std::string& email, const std::string& phone,
             const std::string& adminLevel, const std::string& createdAt)
    : User(id, username, passwordHash, fullName, email, phone, UserRole::ADMIN, createdAt),
      adminLevel(adminLevel) {}

std::string Admin::getAdminLevel() const { return adminLevel; }
void Admin::setAdminLevel(const std::string& level) { adminLevel = level; }

void Admin::displayInfo() const {
    std::cout << Color::BOLD << Color::MAGENTA << "=== ADMIN PROFILE ===" << Color::RESET << "\n";
    User::displayInfo();
    std::cout << Color::BOLD << "Admin Privilege Level: " << Color::RESET << adminLevel << "\n";
}
