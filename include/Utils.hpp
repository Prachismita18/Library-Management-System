#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <algorithm>

namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string WHITE   = "\033[37m";
    const std::string BOLD_CYAN = "\033[1;\033[36m";
    const std::string BG_BLUE   = "\033[44m";
}

class Utils {
public:
    // Hashing
    static std::string sha256(const std::string& str);

    // Date Utilities
    static std::string getCurrentDate(); // YYYY-MM-DD
    static std::string addDaysToDate(const std::string& dateStr, int days);
    static int daysBetweenDates(const std::string& startDateStr, const std::string& endDateStr);
    static bool isOverdue(const std::string& dueDateStr, const std::string& currentDateStr = "");

    // String Utilities
    static std::string trim(const std::string& str);
    static std::string toLower(const std::string& str);
    static std::string truncate(const std::string& str, size_t width);

    // Terminal & IO Utilities
    static void clearScreen();
    static void pauseScreen();
    static void printHeader(const std::string& title);
    static void printSuccess(const std::string& msg);
    static void printError(const std::string& msg);
    static void printWarning(const std::string& msg);
    static void printInfo(const std::string& msg);

    // Safe Input Helpers
    static std::string readLine(const std::string& prompt, bool allowEmpty = false);
    static int readInt(const std::string& prompt, int minVal = -999999, int maxVal = 999999);
    static double readDouble(const std::string& prompt, double minVal = 0.0);
    static std::string readPassword(const std::string& prompt);
};

#endif // UTILS_HPP
