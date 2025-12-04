#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <iomanip>

using namespace std;

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

// Funkcja licząca dni między dwoma datami (przybliżenie)
int daysBetween(const Date& start, const Date& end) {
    return (end.y - start.y) * 365 + (end.m - start.m) * 30 + (end.d - start.d) + 1;
}

/* ------------------------ KLASA ROOM ------------------------ */

class Room {
public:
    int number;      // numer pokoju (unikalny lub typ pokoju)
    int capacity;    // ile osób mieści
    double price;    // cena za noc
    int count;       // ile takich pokoi w hotelu

    Room() {}
    Room(int n, int c, double p, int cnt = 1) : number(n), capacity(c), price(p), count(cnt) {}

    string str() const {
        return "Pokój " + to_string(number) + " | miejsca: " + to_string(capacity) +
            " | cena: " + to_string(price) + " | ilość: " + to_string(count);
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

    Reservation() {}
    Reservation(int i, string ln, int rn, Date s, Date e)
        : id(i), lastname(ln), roomNumber(rn), start(s), end(e) {}

    bool conflicts(const Reservation& other) const {
        if (roomNumber != other.roomNumber) return false;
        return !(end < other.start || other.end < start);
    }

    string str() const {
        return "ID: " + to_string(id) + " | " + lastname +
            " | pokój: " + to_string(roomNumber) +
            " | " + start.str() + " - " + end.str();
    }
};

/* ------------------------ GLOBALNE DANE ------------------------ */

vector<Room> rooms;
vector<Reservation> reservations;
int nextReservationID = 1;

/* ------------------------ ZAPIS / ODCZYT ------------------------ */

void saveToFile() {
    ofstream f("rooms.txt");
    for (auto& r : rooms)
        f << r.number << " " << r.capacity << " " << r.price << "\n";
    f.close();

    ofstream rfile("reservations.txt");
    for (auto& r : reservations)
        rfile << r.id << "|" << r.lastname << "|" << r.roomNumber << "|"
        << r.start.d << " " << r.start.m << " " << r.start.y << " "
        << r.end.d << " " << r.end.m << " " << r.end.y << "\n";
    rfile.close();


    ofstream idf("id.txt");
    idf << nextReservationID;
    idf.close();
}

void loadFromFile() {
    rooms.clear();
    reservations.clear();

    ifstream f("rooms.txt");
    int n, c;
    double p;
    while (f >> n >> c >> p)
        rooms.emplace_back(n, c, p);

    ifstream rfile("reservations.txt");
    string line;
    while (getline(rfile, line)) {
        size_t pos1 = line.find('|');
        size_t pos2 = line.find('|', pos1 + 1);
        size_t pos3 = line.find('|', pos2 + 1);

        int id = stoi(line.substr(0, pos1));
        string ln = line.substr(pos1 + 1, pos2 - pos1 - 1);
        int rn = stoi(line.substr(pos2 + 1, pos3 - pos2 - 1));

        int sd, sm, sy, ed, em, ey;
        sscanf_s(line.substr(pos3 + 1).c_str(), "%d %d %d %d %d %d", &sd, &sm, &sy, &ed, &em, &ey);


        reservations.emplace_back(id, ln, rn, Date(sd, sm, sy), Date(ed, em, ey));
    }


    ifstream idf("id.txt");
    if (idf) idf >> nextReservationID;
}

/* ------------------------ FUNKCJE POMOCNICZE ------------------------ */

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

void printMonthCalendar(int month, int year) {
    cout << "\nKalendarz: " << month << "." << year << "\n";
    cout << "Pn Wt Śr Cz Pt So Nd\n";

    Date first(1, month, year);
    int firstD = (first.dayOfWeekStr() == "Poniedziałek" ? 0 :
        first.dayOfWeekStr() == "Wtorek" ? 1 :
        first.dayOfWeekStr() == "Środa" ? 2 :
        first.dayOfWeekStr() == "Czwartek" ? 3 :
        first.dayOfWeekStr() == "Piątek" ? 4 :
        first.dayOfWeekStr() == "Sobota" ? 5 : 6);

    for (int i = 0; i < firstD; i++) cout << "   ";

    int days = daysInMonth(month, year);
    for (int d = 1; d <= days; d++) {
        cout << setw(2) << d << " ";
        if ((firstD + d) % 7 == 0) cout << "\n";
    }
    cout << "\n";
}

bool isRoomFree(int roomNumber, Date start, Date end) {
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

    // Prosta iteracja po dniach (przybliżenie)
    Date current = start;
    for (int i = 0; i < totalDays; i++) {
        double dayPrice = room.price;
        if (isWeekend(current)) dayPrice *= 1.2; // +20% weekend
        price += dayPrice;

        // przejście do następnego dnia
        current.d++;
        if (current.d > daysInMonth(current.m, current.y)) {
            current.d = 1;
            current.m++;
            if (current.m > 12) { current.m = 1; current.y++; }
        }
    }

    // Rabat >7 dni
    if (totalDays > 7) price *= 0.9;

    // Zniżka przy większej liczbie osób (>2)
    if (numPersons > 2) price *= 0.95;

    return price;
}


/* ------------------------ PANEL KLIENTA ------------------------ */

void clientPanel() {
    cout << "\n==== Rezerwacja pokoju ====\n";

    int sd, sm, sy, ed, em, ey;

    cout << "\nPodaj datę przyjazdu (d m r): ";
    cin >> sd >> sm >> sy;
    printMonthCalendar(sm, sy);

    cout << "\nPodaj datę wyjazdu (d m r): ";
    cin >> ed >> em >> ey;
    printMonthCalendar(em, ey);

    Date start(sd, sm, sy);
    Date end(ed, em, ey);

    if (end < start) {
        cout << "BŁĄD: Data wyjazdu musi być po dacie przyjazdu!\n";
        return;
    }

    cout << "\nDni pobytu: " << daysBetween(start, end) << "\n";
    cout << "Dzień tygodnia przyjazdu: " << start.dayOfWeekStr() << "\n";

    cout << "\nSprawdzanie dostępnych pokoi...\n";

    vector<Room> freeRooms;

    for (auto& room : rooms) {
        if (isRoomFree(room.number, start, end)) {
            freeRooms.push_back(room);
        }
    }

    if (freeRooms.empty()) {
        cout << "Brak wolnych pokoi w tym terminie.\n";
        return;
    }

    cout << "\nWolne pokoje:\n";
    for (auto& r : freeRooms) {
        cout << r.str() << "\n";
    }

    cout << "\nWybierz numer pokoju: ";
    int choice;
    cin >> choice;

    bool exists = false;
    for (auto& r : freeRooms) {
        if (r.number == choice) exists = true;
    }
    if (!exists) {
        cout << "Nie ma takiego pokoju na liście.\n";
        return;
    }

    cout << "Podaj imię i nazwisko: ";
    cin.ignore();
    string name;
    getline(cin, name);

    int id = reservations.empty() ? 1 : reservations.back().id + 1;

    int numPersons;
    cout << "Liczba osób: ";
    cin >> numPersons;

    reservations.emplace_back(id, name, choice, start, end);
    saveToFile();

    double totalPrice = calculatePrice(freeRooms[choice - 1], start, end, numPersons);


    cout << "\n✅ Rezerwacja zakończona!\n";
    cout << "ID: " << id << "\n";
    cout << "Klient: " << name << "\n";
    cout << "Pokój: " << choice << "\n";
    cout << "Od: " << start.str() << "\n";
    cout << "Do: " << end.str() << "\n";
    cout << "Liczba osób: " << numPersons << "\n";
    cout << "Cena całkowita: " << totalPrice << " zł\n";

}

/* ------------------------ ADMIN / RECEPCJA ------------------------ */

void addReservation() { clientPanel(); }

void cancelReservation() {
    int id;
    cout << "Podaj ID: ";
    cin >> id;
    auto it = remove_if(reservations.begin(), reservations.end(),
        [&](auto& r) { return r.id == id; });
    if (it != reservations.end()) {
        reservations.erase(it, reservations.end());
        saveToFile();
        cout << "Usunięto.\n";
    }
    else cout << "Nie znaleziono.\n";
}

void modifyReservation() {
    int id;
    cout << "Podaj ID: ";
    cin >> id;
    for (auto& r : reservations) {
        if (r.id == id) {
            cout << "Nowy pokój: "; cin >> r.roomNumber;
            cout << "Nowa data startu (d m r): "; cin >> r.start.d >> r.start.m >> r.start.y;
            cout << "Nowa data końca (d m r): "; cin >> r.end.d >> r.end.m >> r.end.y;
            saveToFile();
            cout << "Zmieniono.\n";
            return;
        }
    }
    cout << "Nie znaleziono.\n";
}

void search() {
    cout << "1. Po nazwisku\n2. Po dacie\n3. Po ID\nWybór: ";
    int x; cin >> x;

    if (x == 1) {
        string ln;
        cout << "Nazwisko: "; cin >> ln;
        for (auto& r : reservations)
            if (r.lastname == ln) cout << r.str() << "\n";
    }
    else if (x == 2) {
        int d, m, y;
        cout << "D M R: "; cin >> d >> m >> y;
        Date dt(d, m, y);
        for (auto& r : reservations)
            if (!(dt < r.start) && !(r.end < dt)) cout << r.str() << "\n";
    }
    else if (x == 3) {
        int id; cout << "ID: "; cin >> id;
        for (auto& r : reservations)
            if (r.id == id) cout << r.str() << "\n";
    }
}

void adminPanel() {
    while (true) {
        cout << "\n=== ADMIN ===\n1. Dodaj pokój\n2. Lista pokoi\n3. Wyloguj\n";
        int x; cin >> x;
        if (x == 1) {
            int n, c, cnt; double p;
            cout << "Numer: "; cin >> n;
            cout << "Miejsca: "; cin >> c;
            cout << "Cena: "; cin >> p;
            cout << "Ilość identycznych pokoi: "; cin >> cnt;
            rooms.emplace_back(n, c, p, cnt);

            saveToFile();
        }
        if (x == 2) for (auto& r : rooms) cout << r.str() << "\n";
        if (x == 3) return;
    }
}

void receptionistPanel() {
    while (true) {
        cout << "\n=== RECEPCJA ===\n1. Dodaj rezerwację\n2. Usuń rezerwację\n3. Modyfikuj rezerwację\n4. Szukaj\n5. Wyświetl wszystkie\n6. Wyloguj\n";
        int x; cin >> x;
        if (x == 1) addReservation();
        if (x == 2) cancelReservation();
        if (x == 3) modifyReservation();
        if (x == 4) search();
        if (x == 5) for (auto& r : reservations) cout << r.str() << "\n";
        if (x == 6) return;
    }
}

/* ------------------------ LOGOWANIE ------------------------ */

bool checkPassword(const string& correct) {
    string pass;
    cout << "Hasło: "; cin >> pass;
    return pass == correct;
}

void login() {
    while (true) {
        cout << "\n=== LOGOWANIE ===\n1. Admin\n2. Recepcjonistka\n3. Klient\n4. Wyjście\n";
        int x; cin >> x;
        if (x == 1) {
            if (checkPassword("admin123")) adminPanel();
            else cout << "Błędne hasło!\n";
        }
        if (x == 2) {
            if (checkPassword("recepcja123")) receptionistPanel();
            else cout << "Błędne hasło!\n";
        }
        if (x == 3) clientPanel();
        if (x == 4) exit(0);
    }
}

/* ------------------------ MAIN ------------------------ */

int main() {
    setlocale(LC_ALL, "");
    system("chcp 65001 > nul");

    loadFromFile();
    login();
    return 0;
}
