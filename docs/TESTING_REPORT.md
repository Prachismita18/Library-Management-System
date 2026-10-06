# 🧪 System Testing & QA Verification Report

This document records the automated testing suite execution, unit tests, integration validation, and edge case verifications for the **Library Management System**.

---

## 🎯 1. Test Execution Summary

- **Total Test Cases Executed**: 8 Main Automated Test Modules
- **Passed**: 8 / 8 (100% Success Rate)
- **Failed**: 0
- **Test Framework**: Native C++ Assertions & Custom Automated Test Harness (`tests/test_main.cpp`)

```
========================================
 RUNNING AUTOMATED SYSTEM UNIT TESTS
========================================

[TEST 1] Hashing SHA-256... PASSED
[TEST 2] Date Utilities... PASSED
[TEST 3] Admin Authentication... PASSED
[TEST 4] Member Registration & Auth... PASSED
[TEST 5] Book Addition & Retrieval... PASSED
[TEST 6] Book Issue Workflow... PASSED
[TEST 7] Book Return & Fine Calculation... PASSED
[TEST 8] Dashboard Analytics... PASSED

========================================
 ALL TESTS PASSED SUCCESSFULLY! (8/8)
========================================
```

---

## 📊 2. Comprehensive Test Cases Matrix

| Test ID | Module | Scenario / Description | Input / Trigger | Expected Result | Status |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC-01** | `Utils` | SHA-256 Hashing Consistency | `Utils::sha256("admin123")` | Deterministic 64-char hex string matching expected hash | **PASS** |
| **TC-02** | `Utils` | Date Difference Calculation | `2026-01-01` to `2026-01-15` | Returns `14` days difference | **PASS** |
| **TC-03** | `Utils` | Date Addition | `2026-01-01` + 10 days | Returns `2026-01-11` | **PASS** |
| **TC-04** | `Auth` | Valid Admin Login | `admin` / `admin123` | Returns `shared_ptr<Admin>` with `isAdmin() == true` | **PASS** |
| **TC-05** | `Auth` | Invalid Login Attempt | `admin` / `wrongpass` | Returns `nullptr` without crashing | **PASS** |
| **TC-06** | `Auth` | Duplicate Username Prevention | Register `admin` again | Fails registration with error message | **PASS** |
| **TC-07** | `Library`| Book Addition & ISBN Lookup | Add ISBN `9998887776665` | Book correctly saved & retrieved from DB | **PASS** |
| **TC-08** | `Library`| Book Issue & Stock Decrement | Issue Book ID #1 to Member | `available_copies` decreases from 5 to 4 | **PASS** |
| **TC-09** | `Library`| Max Borrowing Limit Enforcement | Member with max 5 attempts 6th issue | Rejects issue with limit reached message | **PASS** |
| **TC-10** | `Library`| Book Return & Stock Increment | Return Book ID #1 | `available_copies` restored to 5 | **PASS** |
| **TC-11** | `Library`| Overdue Fine Calculation | Return book 5 days past due date | Generates Fine record of `$5.00` ($1.00/day) | **PASS** |
| **TC-12** | `Library`| Dashboard Analytics Aggregation| `getDashboardStats()` | Matches database row counts accurately | **PASS** |

---

## 🛡️ 3. Edge Case Handling & Input Validation

1. **SQL Injection Prevention**:
   - Inputs like `' OR '1'='1` tested in search and login fields. Handled safely via SQLite parameterized queries (`sqlite3_bind_text`).

2. **Invalid Console Inputs**:
   - Alphabetic strings entered when an integer choice is requested (e.g. typing `"abc"` into menu options) are caught by `Utils::readInt` stream validation without terminal infinite loops.

3. **OutOfStock Prevention**:
   - Attempting to issue a book when `available_copies == 0` correctly blocks checkout and displays a user notification.

4. **Suspended Member Block**:
   - Suspended accounts cannot borrow new books until an administrator updates their status back to `ACTIVE`.
