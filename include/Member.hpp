#ifndef MEMBER_HPP
#define MEMBER_HPP

#include "User.hpp"

class Member : public User {
private:
    std::string membershipType;
    int maxBooksAllowed;
    std::string status;

public:
    Member();
    Member(int id, const std::string& username, const std::string& passwordHash,
           const std::string& fullName, const std::string& email, const std::string& phone,
           const std::string& membershipType = "REGULAR", int maxBooksAllowed = 5,
           const std::string& status = "ACTIVE", const std::string& createdAt = "");

    std::string getMembershipType() const;
    int getMaxBooksAllowed() const;
    std::string getStatus() const;

    void setMembershipType(const std::string& type);
    void setMaxBooksAllowed(int maxBooks);
    void setStatus(const std::string& newStatus);

    bool isActive() const;
    void displayInfo() const override;
};

#endif // MEMBER_HPP
