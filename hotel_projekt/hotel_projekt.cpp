#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <limits>

using namespace std;

const int LEFT_WIDTH = 30; 
int CONSOLE_WIDTH = 120;
int CONSOLE_HEIGHT = 40;
int RIGHT_X = LEFT_WIDTH + 1;
int RIGHT_WIDTH = 80;

/* ------------------------ KLASA DATE ------------------------ */

class Date {
public:
    int d, m, y;

    Date() : d(1), m(1), y(2000) {}
    Date(int dd, int mm, int yy) : d(dd), m(mm), y(yy) {}

    bool operator<(const Date& other) const {
        if (y != other.y) return y < other.y;
        if (m != other.m) return m < other.m;
        return d < other.d;
    }

    bool operator==(const Date& other) const {
        return d == other.d && m == other.m && y == other.y;
    }

    string str() const {
        return to_string(d) + "." + to_string(m) + "." + to_string(y);
    }

    // Zeller
    string dayOfWeek() const {
        int dd = d;
        int mm = m;
        int yy = y;

        if (mm < 3) {
            mm += 12;
            yy--;
        }

        int K = yy % 100;
        int J = yy / 100;

        int h = (dd + (13 * (mm + 1)) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;

        string days[] = { "Sobota","Niedziela","Poniedziałek","Wtorek","Środa","Czwartek","Piątek" };
        return string(days[h]);
    }

    string dayOfWeekStr() const { return dayOfWeek(); }
};

int daysBetween(const Date& start, const Date& end) {
    return (end.y - start.y) * 365 + (end.m - start.m) * 30 + (end.d - start.d) + 1;
}

/* ------------------------ KLASA ROOM ------------------------ */

class Room {
public:
    int number;      // identyfikator/typ
    int capacity;    // maks. osób
    double price;    // cena bazowa za noc
    int count;       // ile takich pokoi

    Room() {}
    Room(int n, int c, double p, int cnt = 1) : number(n), capacity(c), price(p), count(cnt) {}

    string str() const {
        ostringstream ss;
        ss << "Pokój " << number << " | miejsca: " << capacity
            << " | cena: " << price << " | ilość: " << count;
        return ss.str();
    }
};

/* ------------------------ KLASA RESERVATION ------------------------ */

class Reservation {
public:
    int id;
    string lastname;
    int roomNumber;
    Date start;
    Date end;
    int persons;

    Reservation() {}
    Reservation(int i, string ln, int rn, Date s, Date e, int p = 1)
        : id(i), lastname(ln), roomNumber(rn), start(s), end(e), persons(p) {}

    bool conflicts(const Reservation& other) const {
        if (roomNumber != other.roomNumber) return false;
        return !(end < other.start || other.end < start);
    }

    string str() const {
        ostringstream ss;
        ss << "ID: " << id << " | " << lastname
            << " | pokój: " << roomNumber
            << " | " << start.str() << " - " << end.str()
            << " | os.: " << persons;
        return ss.str();
    }
};

/* ------------------------ GLOBALNE DANE ------------------------ */

vector<Room> rooms;
vector<Reservation> reservations;
int nextReservationID = 1;

/* ------------------------ KONSOLA / UI ------------------------ */



void fetchConsoleSize() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(h, &csbi)) {
        CONSOLE_WIDTH = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        CONSOLE_HEIGHT = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        RIGHT_X = LEFT_WIDTH + 1;
        RIGHT_WIDTH = max(20, CONSOLE_WIDTH - LEFT_WIDTH - 2);
    }
    else {
        CONSOLE_WIDTH = 120;
        CONSOLE_HEIGHT = 40;
        RIGHT_WIDTH = 80;
    }
}

void gotoXY(int x, int y) {
    COORD c;
    c.X = (SHORT)x;
    c.Y = (SHORT)y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void setCursorRight(int col, int row) {
    gotoXY(RIGHT_X + col, row);
}

void clearRightSide() {
    string blank(RIGHT_WIDTH, ' ');
    for (int y = 0; y < CONSOLE_HEIGHT; ++y) {
        gotoXY(RIGHT_X, y);
        cout << blank;
    }
}

// czyści lewą kolumnę i narysuje ramkę
void drawMenuBox() {
    // lewa kolumna tła (proste wypełnienie)
    for (int y = 0; y < CONSOLE_HEIGHT; ++y) {
        gotoXY(0, y);
        cout << string(LEFT_WIDTH, ' ');
    }
    // lewa ramka góra
    gotoXY(0, 0);
    cout << string(LEFT_WIDTH, '=');
    for (int y = 1; y < CONSOLE_HEIGHT - 1; ++y) {
        gotoXY(0, y);
        cout << "|";
        gotoXY(LEFT_WIDTH - 1, y);
        cout << "|";
    }
    gotoXY(0, CONSOLE_HEIGHT - 1);
    cout << string(LEFT_WIDTH, '=');
}

void drawMenu(const vector<string>& options, const string& title) {
    drawMenuBox();
    int row = 1;
    gotoXY(4, row++); cout << title;
    gotoXY(2, row++); cout << string(LEFT_WIDTH - 4, '-');
    for (auto& opt : options) {
        gotoXY(2, row++);
        if ((int)opt.size() > LEFT_WIDTH - 4) cout << opt.substr(0, LEFT_WIDTH - 5);
        else cout << opt;
    }
    // wskazówka
    int artStartY = CONSOLE_HEIGHT - 6; // np. rysunek zajmuje 5 wierszy
    gotoXY(12, artStartY);
    cout << " ^~^  , ";
    gotoXY(12, artStartY + 1);
    cout << "('Y') )";
    gotoXY(11, artStartY + 2);
    cout << " /   \\ / ";
    gotoXY(11, artStartY + 3);
    cout << "(\\|||/)";
    gotoXY(7, artStartY + 4);
    cout << "(Wybierz opcję)";

}

void pauseIfNeeded() {
    gotoXY(RIGHT_X + 100, 30); // ustaw kursor w prawym panelu
    cin.clear();              // usuń flagi błędów
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    //cout << "\n > Kontynuuj [ENTER]...";
    cin.get();                // czekaj na Enter
}


// wypisuje tekst po prawej, linia (y) i opcjonalnie przesunięcie kolumny
void printRight(int row, const string& text) {
    // podziel na linie jeśli za długie
    int maxw = RIGHT_WIDTH;
    int startCol = 2;
    int printed = 0;
    int len = (int)text.size();
    int cur = 0;
    while (cur < len) {
        int to = min(len - cur, maxw - startCol);
        string part = text.substr(cur, to);
        setCursorRight(startCol, row + printed);
        cout << part;
        cur += to;
        printed++;
    }
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

/* ------------------------ ZAPIS / ODCZYT (z obsługą spacji w nazwisku) ------------------------ */

void saveToFile() {
    // rooms: number capacity price count
    ofstream f("rooms.txt");
    for (auto& r : rooms)
        f << r.number << " " << r.capacity << " " << r.price << " " << r.count << "\n";
    f.close();

    ofstream rfile("reservations.txt");
    for (auto& r : reservations) {
        // format: id|lastname|room|startD startM startY endD endM endY persons
        rfile << r.id << "|" << r.lastname << "|" << r.roomNumber << "|"
            << r.start.d << " " << r.start.m << " " << r.start.y << " "
            << r.end.d << " " << r.end.m << " " << r.end.y << " "
            << r.persons << "\n";
    }
    rfile.close();

    ofstream idf("id.txt");
    idf << nextReservationID;
    idf.close();
}

void loadFromFile() {
    rooms.clear();
    reservations.clear();

    ifstream f("rooms.txt");
    int n, c, cnt;
    double p;
    while (f >> n >> c >> p >> cnt) {
        rooms.emplace_back(n, c, p, cnt);
    }
    f.close();

    ifstream rfile("reservations.txt");
    string line;
    while (getline(rfile, line)) {
        if (line.empty()) continue;
        size_t pos1 = line.find('|');
        size_t pos2 = line.find('|', pos1 + 1);
        size_t pos3 = line.find('|', pos2 + 1);

        if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos) continue;

        int id = stoi(line.substr(0, pos1));
        string ln = line.substr(pos1 + 1, pos2 - pos1 - 1);
        int rn = stoi(line.substr(pos2 + 1, pos3 - pos2 - 1));

        int sd, sm, sy, ed, em, ey, persons;
        string rest = line.substr(pos3 + 1);
        istringstream iss(rest);
        iss >> sd >> sm >> sy >> ed >> em >> ey >> persons;

        reservations.emplace_back(id, ln, rn, Date(sd, sm, sy), Date(ed, em, ey), persons);
        if (id >= nextReservationID) nextReservationID = id + 1;
    }
    rfile.close();

    ifstream idf("id.txt");
    if (idf) idf >> nextReservationID;
    idf.close();
}

/* ------------------------ POMOCNICZE (dostępność, kalendarz, ceny) ------------------------ */

bool isLeapYear(int y) {
    if (y % 400 == 0) return true;
    if (y % 100 == 0) return false;
    return (y % 4 == 0);
}

int daysInMonth(int m, int y) {
    switch (m) {
    case 1: return 31;
    case 2: return isLeapYear(y) ? 29 : 28;
    case 3: return 31;
    case 4: return 30;
    case 5: return 31;
    case 6: return 30;
    case 7: return 31;
    case 8: return 31;
    case 9: return 30;
    case 10: return 31;
    case 11: return 30;
    case 12: return 31;
    }
    return 30;
}

void printMonthCalendarRight(int month, int year, int topRow) {
    ostringstream header;
    header << "Kalendarz: " << month << "." << year;
    printRight(topRow, header.str());
    printRight(topRow + 1, "Pn Wt Śr Cz Pt So Nd");

    Date first(1, month, year);
    // compute index 0=Mon ... 6=Sun
    string w = first.dayOfWeekStr();
    int firstD = (w == "Poniedziałek" ? 0 :
        w == "Wtorek" ? 1 :
        w == "Środa" ? 2 :
        w == "Czwartek" ? 3 :
        w == "Piątek" ? 4 :
        w == "Sobota" ? 5 : 6);

    int days = daysInMonth(month, year);
    int row = topRow + 2;
    int col = 0;
    string line;
    ostringstream oss;
    for (int i = 0; i < firstD; ++i) oss << "   ";
    for (int d = 1; d <= days; ++d) {
        oss << setw(2) << d << " ";
        col++;
        if ((firstD + d) % 7 == 0) {
            printRight(row++, oss.str());
            oss.str(""); oss.clear();
        }
    }
    if (!oss.str().empty()) printRight(row++, oss.str());
}

bool isRoomFreeCounted(int roomNumber, Date start, Date end) {
    int booked = 0;
    int maxCount = 1;
    for (auto& room : rooms)
        if (room.number == roomNumber) maxCount = room.count;

    for (auto& r : reservations) {
        if (r.roomNumber == roomNumber) {
            if (!(end < r.start || r.end < start)) booked++;
        }
    }
    return booked < maxCount;
}

bool isWeekend(const Date& date) {
    string dw = date.dayOfWeekStr();
    return (dw == "Piątek" || dw == "Sobota" || dw == "Niedziela");
}

double calculatePrice(const Room& room, const Date& start, const Date& end, int numPersons) {
    int totalDays = daysBetween(start, end);
    double price = 0.0;

    Date current = start;
    for (int i = 0; i < totalDays; ++i) {
        double dayPrice = room.price;
        if (isWeekend(current)) dayPrice *= 1.2;
        price += dayPrice;

        // next day
        current.d++;
        if (current.d > daysInMonth(current.m, current.y)) {
            current.d = 1;
            current.m++;
            if (current.m > 12) { current.m = 1; current.y++; }
        }
    }

    if (totalDays > 7) price *= 0.9;
    if (numPersons > 2) price *= 0.95;

    return price;
}

/* ------------------------ PANELE (wyświetlają po prawej) ------------------------ */

void showReservationsListRight(int topRow) {
    int row = topRow;
    for (auto& r : reservations) {
        printRight(row++, r.str());
        if (row > CONSOLE_HEIGHT - 3) break;
    }
}

void showRoomsListRight(int topRow) {
    int row = topRow;
    for (auto& r : rooms) {
        printRight(row++, r.str());
        if (row > CONSOLE_HEIGHT - 3) break;
    }
}

/* ------------------------ FUNKCJE REZERWACJI ------------------------ */

void addReservationRight() {
    clearRightSide();

    // --- DODAWANIE REZERWACJI ---
    int sd, sm, sy, ed, em, ey;
    setCursorRight(2, 2); cout << "Data przyjazdu (d m r): ";
    gotoXY(RIGHT_X + 25, 2); cin >> sd >> sm >> sy;
    printMonthCalendarRight(sm, sy, 4);

    setCursorRight(2, 12); cout << "Data wyjazdu (d m r): ";
    gotoXY(RIGHT_X + 25, 12); cin >> ed >> em >> ey;

    Date start(sd, sm, sy), end(ed, em, ey);
    if (end < start) {
        printRight(16, "BŁĄD: Data wyjazdu musi byc pozniej.");
        pauseIfNeeded();
        return;
    }

    int top = 16;
    ostringstream s1; s1 << "Dni pobytu: " << daysBetween(start, end);
    printRight(top++, s1.str());
    printRight(top++, "Dzien przyjazdu: " + start.dayOfWeekStr());

    // lista wolnych pokoi
    vector<Room> freeRooms;
    for (auto& room : rooms) {
        if (isRoomFreeCounted(room.number, start, end)) freeRooms.push_back(room);
    }

    if (freeRooms.empty()) {
        printRight(top++, "Brak wolnych pokoi w tym terminie.");
        pauseIfNeeded();
        return;
    }

    printRight(top++, "Wolne pokoje:");
    for (auto& fr : freeRooms) printRight(top++, fr.str());

    int choiceRoom;
    setCursorRight(2, top + 1); cout << "Wybierz numer pokoju: ";
    gotoXY(RIGHT_X + 24, top + 1); cin >> choiceRoom;

    Room chosen;
    bool exists = false;
    for (auto& fr : freeRooms) {
        if (fr.number == choiceRoom) { exists = true; chosen = fr; break; }
    }
    if (!exists) {
        printRight(top + 2, "Nie ma takiego pokoju lub niedostepny.");
        pauseIfNeeded();
        return;
    }

    // dane klienta
    printRight(top + 2, "Podaj imie i nazwisko:");
    gotoXY(RIGHT_X + 2, top + 3);
    cin.ignore();
    string name;
    getline(cin, name);

    int numPersons;
    setCursorRight(2, top + 5); cout << "Liczba osob: ";
    gotoXY(RIGHT_X + 16, top + 5); cin >> numPersons;

    if (numPersons > chosen.capacity) {
        printRight(top + 7, "Za duzo osob dla tego pokoju!");
        pauseIfNeeded();
        return;
    }

    int id = nextReservationID++;
    reservations.emplace_back(id, name, choiceRoom, start, end, numPersons);
    saveToFile();

    double totalPrice = calculatePrice(chosen, start, end, numPersons);
    ostringstream conf;
    conf << "Rezerwacja dodana. ID: " << id
        << " | Cena: " << fixed << setprecision(2) << totalPrice;
    printRight(top + 7, conf.str());

    pauseIfNeeded();
}

void clientPanelUI() {
    setColor(1);
    vector<string> menu = { "1. Dodaj rezerwacje", "2. Wyloguj", "3. Inne" };

    while (true) {
        fetchConsoleSize();
        drawMenu(menu, "=== KLIENT ===");
        clearRightSide();

        setCursorRight(2, 2); cout << "Wybierz opcje: ";
        gotoXY(RIGHT_X + 16, 2);
        int choice; cin >> choice;

        clearRightSide();

        if (choice == 1) {
            addReservationRight();  // wywołanie nowej funkcji
        }
        else if (choice == 2) {
            setColor(7);
            return;
        }
        else {
            printRight(4, "Aby zmodyfikowac lub anulowac rezerwacje, zadzwon do recepcji pod numer: 111 111 111");
            pauseIfNeeded();
        }
    }
}


void cancelReservationRight() {
    clearRightSide();
    printRight(1, "=== ANULUJ REZERWACJE ===");
    setCursorRight(2, 3); cout << "Podaj ID rezerwacji: ";
    gotoXY(RIGHT_X + 22, 3);
    int id; cin >> id;
    auto it = remove_if(reservations.begin(), reservations.end(),
        [&](auto& r) { return r.id == id; });
    if (it != reservations.end()) {
        reservations.erase(it, reservations.end());
        saveToFile();
        printRight(5, "Usunieto rezerwacje.");
    }
    else printRight(5, "Nie znaleziono rezerwacji.");
}

void modifyReservationRight() {
    clearRightSide();
    printRight(1, "=== MODYFIKUJ REZERWACJE ===");
    setCursorRight(2, 3); cout << "Podaj ID rezerwacji: ";
    gotoXY(RIGHT_X + 22, 3);
    int id; cin >> id;
    for (auto& r : reservations) {
        if (r.id == id) {
            setCursorRight(2, 5); cout << "Nowy numer pokoju: ";
            gotoXY(RIGHT_X + 22, 5); cin >> r.roomNumber;
            setCursorRight(2, 6); cout << "Nowa data startu (d m r): ";
            gotoXY(RIGHT_X + 30, 6); cin >> r.start.d >> r.start.m >> r.start.y;
            setCursorRight(2, 7); cout << "Nowa data konca (d m r): ";
            gotoXY(RIGHT_X + 30, 7); cin >> r.end.d >> r.end.m >> r.end.y;
            saveToFile();
            printRight(9, "Zmieniono rezerwacje.");
            return;
        }
    }
    printRight(9, "Nie znaleziono rezerwacji.");
}

void searchRight() {
    clearRightSide();
    printRight(1, "=== SZUKAJ REZERWACJI ===");
    setCursorRight(2, 3); cout << "1. Po nazwisku  2. Po dacie  3. Po ID";
    gotoXY(RIGHT_X + 2, 4);
    int x; cin >> x;
    if (x == 1) {
        setCursorRight(2, 6); cout << "Nazwisko: ";
        gotoXY(RIGHT_X + 12, 6);
        string ln; cin >> ws; getline(cin, ln);
        int row = 8;
        for (auto& r : reservations) if (r.lastname == ln) { printRight(row++, r.str()); if (row > CONSOLE_HEIGHT - 2) break; }
    }
    else if (x == 2) {
        setCursorRight(2, 6); cout << "Data (d m r): ";
        gotoXY(RIGHT_X + 16, 6);
        int d, m, y; cin >> d >> m >> y;
        Date dt(d, m, y);
        int row = 8;
        for (auto& r : reservations) if (!(dt < r.start) && !(r.end < dt)) { printRight(row++, r.str()); if (row > CONSOLE_HEIGHT - 2) break; }
    }
    else if (x == 3) {
        setCursorRight(2, 6); cout << "ID: ";
        gotoXY(RIGHT_X + 6, 6); int id; cin >> id;
        int row = 8;
        for (auto& r : reservations) if (r.id == id) { printRight(row++, r.str()); break; }
    }
}

/* ------------------------ ADMIN / RECEPCJA PANELE (z menu po lewej) ------------------------ */

void adminPanelUI();
void receptionistPanelUI();

void adminPanelUI() {
    setColor(2);
    vector<string> menu = { "1. Dodaj pokój", "2. Lista pokoi", "3. Usuń pokój", "4. Wyloguj" };
    while (true) {
        fetchConsoleSize();
        drawMenu(menu, "=== ADMIN ===");
        clearRightSide();
        setCursorRight(2, 2); cout << "Wybierz opcje: ";
        gotoXY(RIGHT_X + 16, 2);
        int x; cin >> x;
        clearRightSide();

        if (x == 1) {
            int c, cnt; double p;
            // obliczamy nowy numer pokoju
            int newNumber = 1;
            for (auto& r : rooms) if (r.number >= newNumber) newNumber = r.number + 1;

            setCursorRight(2, 4); cout << "Numer pokoju (auto): " << newNumber;
            setCursorRight(2, 5); cout << "Miejsca: "; gotoXY(RIGHT_X + 12, 5); cin >> c;
            setCursorRight(2, 6); cout << "Cena: "; gotoXY(RIGHT_X + 8, 6); cin >> p;
            setCursorRight(2, 7); cout << "Ilość identycznych pokoi: "; gotoXY(RIGHT_X + 28, 7); cin >> cnt;

            rooms.emplace_back(newNumber, c, p, cnt);
            saveToFile();
            printRight(9, "Dodano pokoj o ID: " + to_string(newNumber));
        }
        else if (x == 2) {
            showRoomsListRight(4);
        }
        else if (x == 3) {
            showRoomsListRight(4); // pokaż listę, żeby wiedzieć jakie ID
            setCursorRight(2, 10); cout << "Podaj numer pokoju do usunięcia: ";
            gotoXY(RIGHT_X + 35, 10); int num; cin >> num;

            auto it = find_if(rooms.begin(), rooms.end(),
                [num](const Room& r) { return r.number == num; });
            if (it != rooms.end()) {
                setCursorRight(2, 12); cout << "Czy na pewno usunąć pokój " << num << "? (t/n): ";
                char confirm; cin >> confirm;
                if (confirm == 't' || confirm == 'T') {
                    rooms.erase(it);
                    saveToFile();
                    printRight(14, "Pokój usunięty.");
                }
                else {
                    printRight(14, "Usuwanie anulowane.");
                }
            }
            else {
                printRight(12, "Nie znaleziono pokoju o podanym numerze.");
            }
        }
        else if (x == 4) {
            setColor(7);
            return;
        }
        pauseIfNeeded();
    }
}


void receptionistPanelUI() {
    setColor(14);
    vector<string> menu = {
        "1. Dodaj rezerwację",
        "2. Usuń rezerwację",
        "3. Modyfikuj rezerwację",
        "4. Szukaj",
        "5. Wyświetl wszystkie",
        "6. Wyloguj"
    };

    while (true) {
        fetchConsoleSize();
        drawMenu(menu, "=== RECEPCJA ===");
        clearRightSide();
        setCursorRight(2, 2); cout << "Wybierz opcje: ";
        gotoXY(RIGHT_X + 16, 2);
        int x; cin >> x;
        clearRightSide();
        if (x == 1) addReservationRight();
        else if (x == 2) cancelReservationRight();
        else if (x == 3) modifyReservationRight();
        else if (x == 4) searchRight();
        else if (x == 5) showReservationsListRight(4);
        else if (x == 6) {
            setColor(7);
            return;
        }
        pauseIfNeeded();
    }
}

/* ------------------------ LOGOWANIE I MAIN UI ------------------------ */

bool checkPassword(const string& correct) {
    string pass;
    setCursorRight(2, 4); cout << "Haslo: ";
    gotoXY(RIGHT_X + 10, 4);
    cin >> pass;
    return pass == correct;
}

void mainUI() {
    vector<string> menu = { "1. Admin", "2. Recepcjonistka", "3. Klient", "4. Wyjscie" };
    while (true) {
        fetchConsoleSize();
        drawMenu(menu, "=== SYSTEM HOTELU ===");
        clearRightSide();
        setCursorRight(2, 2); cout << "Wybierz opcje: ";
        gotoXY(RIGHT_X + 18, 2);
        int x; cin >> x;
        clearRightSide();
        if (x == 1) {
            // admin login
            setCursorRight(2, 2); cout << "Logowanie admina";
            if (checkPassword("admin123")) adminPanelUI();
            else printRight(8, "Bledne haslo!");
        }
        else if (x == 2) {
            setCursorRight(2, 2); cout << "Logowanie recepcji";
            if (checkPassword("recepcja123")) receptionistPanelUI();
            else printRight(8, "Bledne haslo!");
        }
        else if (x == 3) {
            setCursorRight(2, 2); cout << "Logowanie klienta";
            if (checkPassword("klient123")) clientPanelUI();
            else printRight(8, "Bledne haslo!"); 
        }
        else if (x == 4) {
            printRight(4, "Zamykanie...");
            return;
        }
        pauseIfNeeded();
    }
}



/* ------------------------ MAIN ------------------------ */

int main() {
    setlocale(LC_ALL, "");
    fetchConsoleSize();
    clearRightSide();
    loadFromFile();


    mainUI();
    return 0;
}
