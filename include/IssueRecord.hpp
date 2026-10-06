#ifndef ISSUE_RECORD_HPP
#define ISSUE_RECORD_HPP

#include <string>
#include <iostream>

class IssueRecord {
private:
    int issueId;
    int bookId;
    std::string bookTitle;
    int memberId;
    std::string memberName;
    std::string issueDate;
    std::string dueDate;
    std::string returnDate;
    std::string status;
    double fineAmount;

public:
    IssueRecord();
    IssueRecord(int issueId, int bookId, const std::string& bookTitle,
                int memberId, const std::string& memberName,
                const std::string& issueDate, const std::string& dueDate,
                const std::string& returnDate, const std::string& status,
                double fineAmount = 0.0);

    // Getters
    int getIssueId() const;
    int getBookId() const;
    std::string getBookTitle() const;
    int getMemberId() const;
    std::string getMemberName() const;
    std::string getIssueDate() const;
    std::string getDueDate() const;
    std::string getReturnDate() const;
    std::string getStatus() const;
    double getFineAmount() const;

    // Setters
    void setReturnDate(const std::string& rDate);
    void setStatus(const std::string& newStatus);
    void setFineAmount(double fine);

    bool isReturned() const;
    bool isOverdue(const std::string& currentDate = "") const;
    int calculateOverdueDays(const std::string& checkDate = "") const;
    void displayRecord() const;
};

#endif // ISSUE_RECORD_HPP
