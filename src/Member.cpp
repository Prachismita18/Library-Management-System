#include "Member.hpp"
#include "Utils.hpp"

Member::Member()
    : User(), membershipType("REGULAR"), maxBooksAllowed(5), status("ACTIVE") {
    role = UserRole::MEMBER;
}

Member::Member(int id, const std::string& username, const std::string& passwordHash,
               const std::string& fullName, const std::string& email, const std::string& phone,
               const std::string& membershipType, int maxBooksAllowed,
               const std::string& status, const std::string& createdAt)
    : User(id, username, passwordHash, fullName, email, phone, UserRole::MEMBER, createdAt),
      membershipType(membershipType), maxBooksAllowed(maxBooksAllowed), status(status) {}

std::string Member::getMembershipType() const { return membershipType; }
int Member::getMaxBooksAllowed() const { return maxBooksAllowed; }
std::string Member::getStatus() const { return status; }

void Member::setMembershipType(const std::string& type) { membershipType = type; }
void Member::setMaxBooksAllowed(int maxBooks) { maxBooksAllowed = maxBooks; }
void Member::setStatus(const std::string& newStatus) { status = newStatus; }

bool Member::isActive() const { return status == "ACTIVE"; }

void Member::displayInfo() const {
    std::cout << Color::BOLD << Color::CYAN << "=== MEMBER PROFILE ===" << Color::RESET << "\n";
    User::displayInfo();
    std::cout << Color::BOLD << "Membership Type: " << Color::RESET << membershipType << "\n";
    std::cout << Color::BOLD << "Max Books Borrowable: " << Color::RESET << maxBooksAllowed << "\n";
    std::cout << Color::BOLD << "Account Status: " << Color::RESET;
    if (status == "ACTIVE") {
        std::cout << Color::GREEN << "ACTIVE" << Color::RESET << "\n";
    } else {
        std::cout << Color::RED << "SUSPENDED" << Color::RESET << "\n";
    }
}
