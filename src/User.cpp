#include "User.hpp"
#include "Utils.hpp"

User::User()
    : id(0), username(""), passwordHash(""), fullName(""), email(""), phone(""),
      role(UserRole::MEMBER), createdAt("") {}

User::User(int id, const std::string& username, const std::string& passwordHash,
           const std::string& fullName, const std::string& email, const std::string& phone,
           UserRole role, const std::string& createdAt)
    : id(id), username(username), passwordHash(passwordHash), fullName(fullName),
      email(email), phone(phone), role(role), createdAt(createdAt) {}

int User::getId() const { return id; }
std::string User::getUsername() const { return username; }
std::string User::getPasswordHash() const { return passwordHash; }
std::string User::getFullName() const { return fullName; }
std::string User::getEmail() const { return email; }
std::string User::getPhone() const { return phone; }
UserRole User::getRole() const { return role; }

std::string User::getRoleString() const {
    return (role == UserRole::ADMIN) ? "ADMIN" : "MEMBER";
}

std::string User::getCreatedAt() const { return createdAt; }

void User::setFullName(const std::string& name) { fullName = name; }
void User::setEmail(const std::string& newEmail) { email = newEmail; }
void User::setPhone(const std::string& newPhone) { phone = newPhone; }
void User::setPasswordHash(const std::string& newHash) { passwordHash = newHash; }

void User::displayInfo() const {
    std::cout << Color::BOLD << "User ID: " << Color::RESET << id << "\n";
    std::cout << Color::BOLD << "Username: " << Color::RESET << username << "\n";
    std::cout << Color::BOLD << "Full Name: " << Color::RESET << fullName << "\n";
    std::cout << Color::BOLD << "Email: " << Color::RESET << email << "\n";
    std::cout << Color::BOLD << "Phone: " << Color::RESET << phone << "\n";
    std::cout << Color::BOLD << "Role: " << Color::RESET << getRoleString() << "\n";
}

bool User::isAdmin() const { return role == UserRole::ADMIN; }
bool User::isMember() const { return role == UserRole::MEMBER; }
