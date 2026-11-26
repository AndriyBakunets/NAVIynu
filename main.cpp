#include <iostream>
#include <limits>
#include <string>
using namespace std;


// ============================================================
// 1. ОСНОВНІ ТИПИ ДАНИХ (enum, struct, operator<<)
// ============================================================

// Статус операції для майбутнього стеку скасування (Undo)
enum class OperationType {
    ISSUE,  // ВИДАЧА
    RETURN  // ПОВЕРНЕННЯ
};

// Структура книги
struct Book {
    int id;
    string title;
    string author;
    int year;
    int totalCopies;
    int availableCopies;
    int borrowCount; // скільки разів книгу видавали за весь час (для "Найпопулярнішої")
};

// Перевантаження оператора << для зручного виводу книги через cout << book
ostream& operator<<(ostream& os, const Book& book) {
    os << "[" << book.id << "] "
       << book.title << " | "
       << book.author << " | "
       << book.year << " | "
       << "вільних: " << book.availableCopies << "/" << book.totalCopies
       << " (видач: " << book.borrowCount << ")";
    return os;
}

// ============================================================
// 2. ДИНАМІЧНИЙ МАСИВ КНИГ
// ============================================================

struct BookArray {
    Book* data;
    int size;
    int capacity;
};

// Ініціалізація порожнього динамічного масиву
void initBookArray(BookArray& arr) {
    arr.capacity = 4;
    arr.size = 0;
    arr.data = new Book[arr.capacity];
}

// Звільнення пам'яті динамічного масиву
void freeBookArray(BookArray& arr) {
    delete[] arr.data;
    arr.data = nullptr;
    arr.size = 0;
    arr.capacity = 0;
}

// Додавання книги в кінець масиву (з подвоєнням ємності при потребі)
void pushBook(BookArray& arr, const Book& book) {
    if (arr.size == arr.capacity) {
        int newCapacity = arr.capacity * 2;
        Book* newData = new Book[newCapacity];

        for (int i = 0; i < arr.size; i++) {
            newData[i] = arr.data[i];
        }

        delete[] arr.data;
        arr.data = newData;
        arr.capacity = newCapacity;
    }

    arr.data[arr.size] = book;
    arr.size++;
}

// Пошук індексу книги за id (повертає -1, якщо не знайдено)
int findBookIndexById(const BookArray& arr, int id) {
    for (int i = 0; i < arr.size; i++) {
        if (arr.data[i].id == id) {
            return i;
        }
    }
    return -1;
}

// ============================================================
// 3. ОДНОЗВ'ЯЗНИЙ СПИСОК ПОЗИЧОК
// ============================================================

struct LoanNode {
    int bookId;
    string readerName;
    int issueDay;
    LoanNode* next;
};

// Додати нову позичку на початок списку
void addLoan(LoanNode*& head, int bookId, const string& readerName, int issueDay) {
    LoanNode* newNode = new LoanNode;
    newNode->bookId = bookId;
    newNode->readerName = readerName;
    newNode->issueDay = issueDay;
    newNode->next = head;

    head = newNode;
}

// Порахувати, скільки книг зараз на руках у конкретного читача
int countReaderLoans(const LoanNode* head, const string& readerName) {
    int count = 0;
    const LoanNode* current = head;

    while (current != nullptr) {
        if (current->readerName == readerName) {
            count++;
        }
        current = current->next;
    }

    return count;
}

// Звільнити всю пам'ять списку позичок
void freeLoans(LoanNode*& head) {
    while (head != nullptr) {
        LoanNode* toDelete = head;
        head = head->next;
        delete toDelete;
    }
}

// ============================================================
// 4. ДОПОМІЖНІ ФУНКЦІЇ БЕЗПЕЧНОГО ВВОДУ
// ============================================================

// Безпечне зчитування цілого числа (не ламається від тексту)
int readInt(const string& prompt) {
    int value = 0;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); //numeric_limits<streamsize>::max() це тіпа те соме що ми писали 10000, '\n' але більш безпечний варіант
            return value;
        }
        cout << "Помилка вводу. Будь ласка, введіть ціле число.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Безпечне зчитування непорожнього рядка
string readNonEmptyLine(const string& prompt) {
    string line;
    while (true) {
        cout << prompt;
        getline(cin, line);
        if (!line.empty()) {
            return line;
        }
        cout << "Поле не може бути порожнім. Спробуйте ще раз.\n";
    }
}

// ============================================================
// 5. ОСНОВНІ ФУНКЦІЇ: ДОДАВАННЯ І ВИДАЧА КНИГ
// ============================================================

// Пункт меню 1: Додати книгу
void addBook(BookArray& books) {
    int id = readInt("Введіть ID книги (додатне ціле число): ");
    if (id <= 0) {
        cout << "Помилка: ID має бути більшим за 0.\n";
        return;
    }
    if (findBookIndexById(books, id) != -1) {
        cout << "Помилка: книга з ID " << id << " вже існує.\n";
        return;
    }

    string title = readNonEmptyLine("Введіть назву книги (латиницею): ");
    string author = readNonEmptyLine("Введіть автора (латиницею): ");
    int year = readInt("Введіть рік видання: ");
    if (year <= 0) {
        cout << "Помилка: рік має бути додатним.\n";
        return;
    }

    int totalCopies = readInt("Введіть кількість примірників: ");
    if (totalCopies <= 0) {
        cout << "Помилка: кількість примірників має бути не менше 1.\n";
        return;
    }

    Book newBook;
    newBook.id = id;
    newBook.title = title;
    newBook.author = author;
    newBook.year = year;
    newBook.totalCopies = totalCopies;
    newBook.availableCopies = totalCopies;
    newBook.borrowCount = 0;

    pushBook(books, newBook);
    cout << "Книгу успішно додано: " << newBook << "\n";
}

// Пункт меню 2: Показати всі книги
void ShowAllBooks(const BookArray& books) {
    if (books.size == 0) {
        cout << "У бібліотеці поки немає книг.\n";
        return;
    }
    for (int i = 0; i < books.size; i++) {
        cout << books.data[i] << "\n";
    }
}

// Пункт меню 4: Видати книгу читачеві
void issueBook(BookArray& books, LoanNode*& loansHead) {
    const int MAX_BOOKS_PER_READER = 3;

    int id = readInt("Введіть ID книги для видачі: ");
    int index = findBookIndexById(books, id);
    if (index == -1) {
        cout << "Помилка: книгу з ID " << id << " не знайдено.\n";
        return;
    }

    if (books.data[index].availableCopies <= 0) {
        cout << "Відмова: немає вільних примірників книги \"" << books.data[index].title << "\".\n";
        return;
    }

    string readerName = readNonEmptyLine("Введіть ім'я читача (латиницею): ");
    if (countReaderLoans(loansHead, readerName) >= MAX_BOOKS_PER_READER) {
        cout << "Відмова: у читача " << readerName << " вже є " << MAX_BOOKS_PER_READER << " книги.\n";
        return;
    }

    int day = readInt("Введіть день видачі (додатне ціле число): ");
    if (day <= 0) {
        cout << "Помилка: день має бути більшим за 0.\n";
        return;
    }

    // Оновлюємо дані книги та додаємо позичку в список
    books.data[index].availableCopies--;
    books.data[index].borrowCount++;
    addLoan(loansHead, id, readerName, day);

    cout << "Видано \"" << books.data[index].title << "\" читачеві " << readerName
         << " у день " << day << ". Залишилось вільних: "
         << books.data[index].availableCopies << "/" << books.data[index].totalCopies << "\n";
}


// 6. ТЕСТОВИЙ MAIN


int main() {
    BookArray books;
    initBookArray(books);

    LoanNode* loansHead = nullptr;
    bool running = true;

    while (running) {
        cout << "\n=== Dorm Library (Тестове меню Розробника 1) ===\n";
        cout << "1. Додати книгу\n";
        cout << "2. Показати всі книги\n";
        cout << "4. Видати книгу\n";
        cout << "0. Вихід\n";

        int choice = readInt("Оберіть пункт меню: ");

        switch (choice) {
            case 0:
                cout << "Вихід з програми...\n";
                running = false;
                break;
            case 1:
                addBook(books);
                break;
            case 2:
                ShowAllBooks(books);
                break;
            case 4:
                issueBook(books, loansHead);
                break;
            default:
                cout << "Невідомий пункт меню. Спробуйте ще раз.\n";
        }
    }

    // Обов'язкове звільнення всієї динамічної пам'яті при виході
    freeLoans(loansHead);
    freeBookArray(books);

    cout << "Пам'ять звільнено. До побачення!\n";
    return 0;
}
