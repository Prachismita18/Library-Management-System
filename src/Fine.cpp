#include "Fine.hpp"
#include "Utils.hpp"
#include <iomanip>

Fine::Fine()
    : fineId(0), issueId(0), memberId(0), memberName(""), amount(0.0),
      paidStatus("PENDING"), paymentDate(""), reason("Overdue Return") {}

Fine::Fine(int fineId, int issueId, int memberId, const std::string& memberName,
           double amount, const std::string& paidStatus,
           const std::string& paymentDate, const std::string& reason)
    : fineId(fineId), issueId(issueId), memberId(memberId), memberName(memberName),
      amount(amount), paidStatus(paidStatus), paymentDate(paymentDate), reason(reason) {}

int Fine::getFineId() const { return fineId; }
int Fine::getIssueId() const { return issueId; }
int Fine::getMemberId() const { return memberId; }
std::string Fine::getMemberName() const { return memberName; }
double Fine::getAmount() const { return amount; }
std::string Fine::getPaidStatus() const { return paidStatus; }
std::string Fine::getPaymentDate() const { return paymentDate; }
std::string Fine::getReason() const { return reason; }

void Fine::setPaidStatus(const std::string& status) { paidStatus = status; }
void Fine::setPaymentDate(const std::string& pDate) { paymentDate = pDate; }

bool Fine::isPaid() const { return paidStatus == "PAID"; }

double Fine::calculateFineAmount(int overdueDays) {
    if (overdueDays <= 0) return 0.0;
    return overdueDays * DAILY_FINE_RATE;
}

void Fine::displayFine() const {
    std::cout << Color::BOLD << Color::YELLOW << "Fine #" << fineId << Color::RESET << "\n";
    std::cout << "  Member: [" << memberId << "] " << memberName << "\n";
    std::cout << "  Issue ID: #" << issueId << " | Reason: " << reason << "\n";
    std::cout << "  Amount: $" << std::fixed << std::setprecision(2) << amount << "\n";
    std::cout << "  Status: ";
    if (isPaid()) {
        std::cout << Color::GREEN << "PAID (Date: " << paymentDate << ")" << Color::RESET << "\n";
    } else {
        std::cout << Color::RED << "PENDING" << Color::RESET << "\n";
    }
}
