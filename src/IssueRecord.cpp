#include "IssueRecord.hpp"
#include "Utils.hpp"
#include <iomanip>

IssueRecord::IssueRecord()
    : issueId(0), bookId(0), bookTitle(""), memberId(0), memberName(""),
      issueDate(""), dueDate(""), returnDate(""), status("ISSUED"), fineAmount(0.0) {}

IssueRecord::IssueRecord(int issueId, int bookId, const std::string& bookTitle,
                         int memberId, const std::string& memberName,
                         const std::string& issueDate, const std::string& dueDate,
                         const std::string& returnDate, const std::string& status,
                         double fineAmount)
    : issueId(issueId), bookId(bookId), bookTitle(bookTitle), memberId(memberId),
      memberName(memberName), issueDate(issueDate), dueDate(dueDate),
      returnDate(returnDate), status(status), fineAmount(fineAmount) {}

int IssueRecord::getIssueId() const { return issueId; }
int IssueRecord::getBookId() const { return bookId; }
std::string IssueRecord::getBookTitle() const { return bookTitle; }
int IssueRecord::getMemberId() const { return memberId; }
std::string IssueRecord::getMemberName() const { return memberName; }
std::string IssueRecord::getIssueDate() const { return issueDate; }
std::string IssueRecord::getDueDate() const { return dueDate; }
std::string IssueRecord::getReturnDate() const { return returnDate; }
std::string IssueRecord::getStatus() const { return status; }
double IssueRecord::getFineAmount() const { return fineAmount; }

void IssueRecord::setReturnDate(const std::string& rDate) { returnDate = rDate; }
void IssueRecord::setStatus(const std::string& newStatus) { status = newStatus; }
void IssueRecord::setFineAmount(double fine) { fineAmount = fine; }

bool IssueRecord::isReturned() const { return status == "RETURNED"; }

bool IssueRecord::isOverdue(const std::string& currentDate) const {
    if (isReturned()) return false;
    std::string today = currentDate.empty() ? Utils::getCurrentDate() : currentDate;
    return Utils::daysBetweenDates(dueDate, today) > 0;
}

int IssueRecord::calculateOverdueDays(const std::string& checkDate) const {
    std::string refDate = !returnDate.empty() ? returnDate : (checkDate.empty() ? Utils::getCurrentDate() : checkDate);
    int days = Utils::daysBetweenDates(dueDate, refDate);
    return days > 0 ? days : 0;
}

void IssueRecord::displayRecord() const {
    std::cout << Color::BOLD << Color::CYAN << "Issue #" << issueId << Color::RESET << "\n";
    std::cout << "  Book: [" << bookId << "] " << bookTitle << "\n";
    std::cout << "  Member: [" << memberId << "] " << memberName << "\n";
    std::cout << "  Issue Date: " << issueDate << " | Due Date: " << dueDate << "\n";
    std::cout << "  Return Date: " << (returnDate.empty() ? "Not Returned Yet" : returnDate) << "\n";
    std::cout << "  Status: ";
    if (status == "RETURNED") {
        std::cout << Color::GREEN << "RETURNED" << Color::RESET;
    } else if (isOverdue()) {
        std::cout << Color::RED << "OVERDUE (" << calculateOverdueDays() << " days late)" << Color::RESET;
    } else {
        std::cout << Color::YELLOW << "ISSUED" << Color::RESET;
    }
    std::cout << " | Fine: $" << std::fixed << std::setprecision(2) << fineAmount << "\n";
}
