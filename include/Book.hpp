#ifndef BOOK_HPP
#define BOOK_HPP

#include <string>
#include <iostream>

class Book {
private:
    int id;
    std::string title;
    std::string author;
    std::string isbn;
    int categoryId;
    std::string categoryName;
    std::string publisher;
    int publicationYear;
    int totalCopies;
    int availableCopies;
    double price;
    std::string rackNumber;

public:
    Book();
    Book(int id, const std::string& title, const std::string& author, const std::string& isbn,
         int categoryId, const std::string& categoryName, const std::string& publisher,
         int publicationYear, int totalCopies, int availableCopies, double price, const std::string& rackNumber);

    // Getters
    int getId() const;
    std::string getTitle() const;
    std::string getAuthor() const;
    std::string getIsbn() const;
    int getCategoryId() const;
    std::string getCategoryName() const;
    std::string getPublisher() const;
    int getPublicationYear() const;
    int getTotalCopies() const;
    int getAvailableCopies() const;
    double getPrice() const;
    std::string getRackNumber() const;

    // Setters
    void setId(int newId);
    void setTitle(const std::string& newTitle);
    void setAuthor(const std::string& newAuthor);
    void setIsbn(const std::string& newIsbn);
    void setCategoryId(int newCatId);
    void setCategoryName(const std::string& newCatName);
    void setPublisher(const std::string& newPublisher);
    void setPublicationYear(int year);
    void setTotalCopies(int copies);
    void setAvailableCopies(int copies);
    void setPrice(double newPrice);
    void setRackNumber(const std::string& rack);

    // Utilities
    bool isAvailable() const;
    void displaySummary() const;
};

#endif // BOOK_HPP
