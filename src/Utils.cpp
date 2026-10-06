#include "Utils.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <cstdint>
#include <cstdlib>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

// Pure C++ SHA-256 Implementation
namespace {
    uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    uint32_t choose(uint32_t e, uint32_t f, uint32_t g) { return (e & f) ^ (~e & g); }
    uint32_t majority(uint32_t a, uint32_t b, uint32_t c) { return (a & b) ^ (a & c) ^ (b & c); }
    uint32_t sig0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    uint32_t sig1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    uint32_t gamma0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    uint32_t gamma1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

    const uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
}

std::string Utils::sha256(const std::string& input) {
    uint32_t h[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    std::vector<uint8_t> data(input.begin(), input.end());
    uint64_t origLenBits = data.size() * 8;

    data.push_back(0x80);
    while ((data.size() % 64) != 56) {
        data.push_back(0x00);
    }

    for (int i = 7; i >= 0; --i) {
        data.push_back(static_cast<uint8_t>((origLenBits >> (i * 8)) & 0xFF));
    }

    for (size_t chunk = 0; chunk < data.size(); chunk += 64) {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (data[chunk + i * 4] << 24) |
                   (data[chunk + i * 4 + 1] << 16) |
                   (data[chunk + i * 4 + 2] << 8) |
                   (data[chunk + i * 4 + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            w[i] = gamma1(w[i - 2]) + w[i - 7] + gamma0(w[i - 15]) + w[i - 16];
        }

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], h_val = h[7];

        for (int i = 0; i < 64; ++i) {
            uint32_t temp1 = h_val + sig1(e) + choose(e, f, g) + K[i] + w[i];
            uint32_t temp2 = sig0(a) + majority(a, b, c);
            h_val = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += h_val;
    }

    std::ostringstream oss;
    for (int i = 0; i < 8; ++i) {
        oss << std::hex << std::setw(8) << std::setfill('0') << h[i];
    }
    return oss.str();
}

std::string Utils::getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &time_now);
#else
    localtime_r(&time_now, &tm_now);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d");
    return oss.str();
}

std::string Utils::addDaysToDate(const std::string& dateStr, int days) {
    std::tm tm_date = {};
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tm_date, "%Y-%m-%d");
    if (ss.fail()) return getCurrentDate();

    tm_date.tm_mday += days;
    std::mktime(&tm_date);

    std::ostringstream oss;
    oss << std::put_time(&tm_date, "%Y-%m-%d");
    return oss.str();
}

int Utils::daysBetweenDates(const std::string& startDateStr, const std::string& endDateStr) {
    std::tm tm_start = {}, tm_end = {};
    std::istringstream ss1(startDateStr), ss2(endDateStr);
    ss1 >> std::get_time(&tm_start, "%Y-%m-%d");
    ss2 >> std::get_time(&tm_end, "%Y-%m-%d");

    if (ss1.fail() || ss2.fail()) return 0;

    std::time_t t1 = std::mktime(&tm_start);
    std::time_t t2 = std::mktime(&tm_end);

    double diffSecs = std::difftime(t2, t1);
    return static_cast<int>(diffSecs / (60 * 60 * 24));
}

bool Utils::isOverdue(const std::string& dueDateStr, const std::string& currentDateStr) {
    std::string today = currentDateStr.empty() ? getCurrentDate() : currentDateStr;
    return daysBetweenDates(dueDateStr, today) > 0;
}

std::string Utils::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

std::string Utils::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string Utils::truncate(const std::string& str, size_t width) {
    if (str.length() <= width) return str;
    if (width <= 3) return str.substr(0, width);
    return str.substr(0, width - 3) + "...";
}

void Utils::clearScreen() {
#ifdef _WIN32
    auto res = std::system("cls");
    (void)res;
#else
    auto res = std::system("clear");
    (void)res;
#endif
}

void Utils::pauseScreen() {
    std::cout << "\n" << Color::CYAN << "Press Enter to continue..." << Color::RESET;
    std::cin.ignore(10000, '\n');
}

void Utils::printHeader(const std::string& title) {
    std::string bar(60, '=');
    std::cout << Color::BOLD << Color::CYAN << bar << Color::RESET << "\n";
    std::cout << Color::BOLD << Color::YELLOW << "  " << title << Color::RESET << "\n";
    std::cout << Color::BOLD << Color::CYAN << bar << Color::RESET << "\n\n";
}

void Utils::printSuccess(const std::string& msg) {
    std::cout << Color::BOLD << Color::GREEN << "[SUCCESS] " << Color::RESET << msg << "\n";
}

void Utils::printError(const std::string& msg) {
    std::cout << Color::BOLD << Color::RED << "[ERROR] " << Color::RESET << msg << "\n";
}

void Utils::printWarning(const std::string& msg) {
    std::cout << Color::BOLD << Color::YELLOW << "[WARNING] " << Color::RESET << msg << "\n";
}

void Utils::printInfo(const std::string& msg) {
    std::cout << Color::BOLD << Color::BLUE << "[INFO] " << Color::RESET << msg << "\n";
}

std::string Utils::readLine(const std::string& prompt, bool allowEmpty) {
    std::string input;
    while (true) {
        std::cout << Color::BOLD << prompt << Color::RESET;
        std::getline(std::cin, input);
        input = trim(input);
        if (allowEmpty || !input.empty()) {
            return input;
        }
        printError("Input cannot be empty. Please try again.");
    }
}

int Utils::readInt(const std::string& prompt, int minVal, int maxVal) {
    int val;
    std::string line;
    while (true) {
        std::cout << Color::BOLD << prompt << Color::RESET;
        std::getline(std::cin, line);
        line = trim(line);
        try {
            size_t pos;
            val = std::stoi(line, &pos);
            if (pos == line.length() && val >= minVal && val <= maxVal) {
                return val;
            }
        } catch (...) {}
        printError("Invalid integer. Please enter a value between " + std::to_string(minVal) + " and " + std::to_string(maxVal) + ".");
    }
}

double Utils::readDouble(const std::string& prompt, double minVal) {
    double val;
    std::string line;
    while (true) {
        std::cout << Color::BOLD << prompt << Color::RESET;
        std::getline(std::cin, line);
        line = trim(line);
        try {
            size_t pos;
            val = std::stod(line, &pos);
            if (pos == line.length() && val >= minVal) {
                return val;
            }
        } catch (...) {}
        printError("Invalid number. Please enter a value >= " + std::to_string(minVal) + ".");
    }
}

std::string Utils::readPassword(const std::string& prompt) {
    std::cout << Color::BOLD << prompt << Color::RESET;
    std::string password;
#ifdef _WIN32
    char ch;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        } else if (ch >= 32 && ch <= 126) {
            password.push_back(ch);
            std::cout << '*';
        }
    }
    std::cout << "\n";
#else
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::getline(std::cin, password);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << "\n";
#endif
    return trim(password);
}
