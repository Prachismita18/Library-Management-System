#include "Book.hpp"
#include "Utils.hpp"
#include <iomanip>

Book::Book()
    : id(0), title(""), author(""), isbn(""), categoryId(0), categoryName("Uncategorized"),
      publisher(""), publicationYear(2023), totalCopies(1), availableCopies(1), price(0.0), rackNumber("A1") {}

Book::Book(int id, const std::string& title, const std::string& author, const std::string& isbn,
           int categoryId, const std::string& categoryName, const std::string& publisher,
           int publicationYear, int totalCopies, int availableCopies, double price, const std::string& rackNumber)
    : id(id), title(title), author(author), isbn(isbn), categoryId(categoryId),
      categoryName(categoryName), publisher(publisher), publicationYear(publicationYear),
      totalCopies(totalCopies), availableCopies(availableCopies), price(price), rackNumber(rackNumber) {}

int Book::getId() const { return id; }
std::string Book::getTitle() const { return title; }
std::string Book::getAuthor() const { return author; }
std::string Book::getIsbn() const { return isbn; }
int Book::getCategoryId() const { return categoryId; }
std::string Book::getCategoryName() const { return categoryName; }
std::string Book::getPublisher() const { return publisher; }
int Book::getPublicationYear() const { return publicationYear; }
int Book::getTotalCopies() const { return totalCopies; }
int Book::getAvailableCopies() const { return availableCopies; }
double Book::getPrice() const { return price; }
std::string Book::getRackNumber() const { return rackNumber; }

void Book::setId(int newId) { id = newId; }
void Book::setTitle(const std::string& newTitle) { title = newTitle; }
void Book::setAuthor(const std::string& newAuthor) { author = newAuthor; }
void Book::setIsbn(const std::string& newIsbn) { isbn = newIsbn; }
void Book::setCategoryId(int newCatId) { categoryId = newCatId; }
void Book::setCategoryName(const std::string& newCatName) { categoryName = newCatName; }
void Book::setPublisher(const std::string& newPublisher) { publisher = newPublisher; }
void Book::setPublicationYear(int year) { publicationYear = year; }
void Book::setTotalCopies(int copies) { totalCopies = copies; }
void Book::setAvailableCopies(int copies) { availableCopies = copies; }
void Book::setPrice(double newPrice) { price = newPrice; }
void Book::setRackNumber(const std::string& rack) { rackNumber = rack; }

bool Book::isAvailable() const { return availableCopies > 0; }

void Book::displaySummary() const {
    std::cout << Color::BOLD << Color::CYAN << "[" << id << "] " << title << Color::RESET << "\n";
    std::cout << "  Author: " << author << " | ISBN: " << isbn << "\n";
    std::cout << "  Category: " << categoryName << " | Rack: " << rackNumber << "\n";
    std::cout << "  Copies: " << availableCopies << "/" << totalCopies << " Available | Price: $" << std::fixed << std::setprecision(2) << price << "\n";
}
