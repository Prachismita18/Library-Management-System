#ifndef ADMIN_HPP
#define ADMIN_HPP

#include "User.hpp"

class Admin : public User {
private:
    std::string adminLevel;

public:
    Admin();
    Admin(int id, const std::string& username, const std::string& passwordHash,
          const std::string& fullName, const std::string& email, const std::string& phone,
          const std::string& adminLevel = "SUPER_ADMIN", const std::string& createdAt = "");

    std::string getAdminLevel() const;
    void setAdminLevel(const std::string& level);

    void displayInfo() const override;
};

#endif // ADMIN_HPP
