#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <iostream>

enum class UserRole {
    ADMIN,
    MEMBER
};

class User {
protected:
    int id;
    std::string username;
    std::string passwordHash;
    std::string fullName;
    std::string email;
    std::string phone;
    UserRole role;
    std::string createdAt;

public:
    User();
    User(int id, const std::string& username, const std::string& passwordHash,
         const std::string& fullName, const std::string& email, const std::string& phone,
         UserRole role, const std::string& createdAt = "");
    virtual ~User() = default;

    // Getters
    int getId() const;
    std::string getUsername() const;
    std::string getPasswordHash() const;
    std::string getFullName() const;
    std::string getEmail() const;
    std::string getPhone() const;
    UserRole getRole() const;
    std::string getRoleString() const;
    std::string getCreatedAt() const;

    // Setters
    void setFullName(const std::string& name);
    void setEmail(const std::string& newEmail);
    void setPhone(const std::string& newPhone);
    void setPasswordHash(const std::string& newHash);

    // Pure Virtual / Virtual methods for Polymorphism
    virtual void displayInfo() const;
    virtual bool isAdmin() const;
    virtual bool isMember() const;
};

#endif // USER_HPP
