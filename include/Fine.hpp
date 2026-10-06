#ifndef FINE_HPP
#define FINE_HPP

#include <string>
#include <iostream>

class Fine {
private:
    int fineId;
    int issueId;
    int memberId;
    std::string memberName;
    double amount;
    std::string paidStatus;
    std::string paymentDate;
    std::string reason;

public:
    static constexpr double DAILY_FINE_RATE = 1.00; // $1.00 per day overdue

    Fine();
    Fine(int fineId, int issueId, int memberId, const std::string& memberName,
         double amount, const std::string& paidStatus = "PENDING",
         const std::string& paymentDate = "", const std::string& reason = "Overdue Return");

    // Getters
    int getFineId() const;
    int getIssueId() const;
    int getMemberId() const;
    std::string getMemberName() const;
    double getAmount() const;
    std::string getPaidStatus() const;
    std::string getPaymentDate() const;
    std::string getReason() const;

    // Setters
    void setPaidStatus(const std::string& status);
    void setPaymentDate(const std::string& pDate);

    bool isPaid() const;
    static double calculateFineAmount(int overdueDays);
    void displayFine() const;
};

#endif // FINE_HPP
