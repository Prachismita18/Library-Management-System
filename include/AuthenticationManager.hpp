#ifndef AUTHENTICATION_MANAGER_HPP
#define AUTHENTICATION_MANAGER_HPP

#include "DatabaseManager.hpp"
#include "User.hpp"
#include "Admin.hpp"
#include "Member.hpp"
#include <memory>
#include <string>

class AuthenticationManager {
private:
    DatabaseManager& dbManager;

public:
    explicit AuthenticationManager(DatabaseManager& dbMgr);

    // Authentication method returning dynamic User polymorphic shared pointer
    std::shared_ptr<User> login(const std::string& username, const std::string& password);

    // Registration method for new library members
    bool registerMember(const std::string& username, const std::string& password,
                        const std::string& fullName, const std::string& email,
                        const std::string& phone, const std::string& membershipType = "REGULAR",
                        std::string* outErrorMsg = nullptr);

    // Utility methods
    bool usernameExists(const std::string& username);
    bool changePassword(int userId, const std::string& oldPassword, const std::string& newPassword);
};

#endif // AUTHENTICATION_MANAGER_HPP
