#include <iostream>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

class Date {
public:
    int day, month, year;

    Date() { day = month = year = 0; }
    Date(int d, int m, int y) : day(d), month(m), year(y) {}

    bool operator<=(const Date& other) const {
        if (year < other.year) return true;
        if (year > other.year) return false;

        if (month < other.month) return true;
        if (month > other.month) return false;

        return day <= other.day;
    }

    bool operator>=(const Date& other) const {
        if (year > other.year) return true;
        if (year < other.year) return false;

        if (month > other.month) return true;
        if (month < other.month) return false;

        return day >= other.day;
    }

    bool operator==(const Date& other) const {
        return day == other.day && month == other.month && year == other.year;
    }
};

// pełne obliczenie dnia tygodnia (Zeller)
int dayOfWeek(int d, int m, int y) {
    if (m < 3) {
        m += 12;
        y -= 1;
    }
    int K = y % 100;
    int J = y / 100;

    int h = (d + (13 * (m + 1)) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;
    int dayIndex = (h + 5) % 7; // 0=pon, ..., 6=niedz
    return dayIndex;
}

class Room {
public:
    int id;
    string type;
    int capacity;
    double basePrice;

    Room(int id, string type, int capacity, double price)
        : id(id), type(type), capacity(capacity), basePrice(price) {}

    Room() {}
};

class Reservation {
public:
    int id;
    string lastName;
    int roomId;
    int people;
    Date from;
    Date to;

    Reservation(int id, string lname, int rid, int p, Date f, Date t)
        : id(id), lastName(lname), roomId(rid), people(p), from(f), to(t) {}

    Reservation() {}

    bool conflictsWith(const Reservation& other) {
        if (roomId != other.roomId) return false;
        return !(to <= other.from || from >= other.to);
    }
};

vector<Room> rooms;
vector<Reservation> reservations;
int nextResId = 1;

bool isRoomAvailable(int roomId, Date f, Date t) {
    for (const auto& r : reservations) {
        if (r.roomId == roomId) {
            Reservation temp(0, "", roomId, 0, f, t);
            if (temp.conflictsWith(r)) return false;
        }
    }
    return true;
}

void addReservation() {
    string ln;
    int rid, people;
    int d1, m1, y1, d2, m2, y2;

    cout << "Nazwisko: ";
    cin >> ln;

    cout << "ID pokoju: ";
    cin >> rid;

    cout << "Liczba osob: ";
    cin >> people;

    cout << "Data od (d m r): ";
    cin >> d1 >> m1 >> y1;

    cout << "Data do (d m r): ";
    cin >> d2 >> m2 >> y2;

    Date from(d1, m1, y1);
    Date to(d2, m2, y2);

    Room* selected = nullptr;
    for (auto& r : rooms) if (r.id == rid) selected = &r;

    if (selected == nullptr) {
        cout << "Taki pokoj nie istnieje!\n";
        return;
    }

    if (people > selected->capacity) {
        cout << "Za duzo osob do tego pokoju!\n";
        return;
    }

    if (!isRoomAvailable(rid, from, to)) {
        cout << "Pokoj zajety w tym terminie!\n";
        return;
    }

    reservations.push_back(Reservation(nextResId++, ln, rid, people, from, to));
    cout << "Rezerwacja dodana.\n";
}

void cancelReservation() {
    int id;
    cout << "Podaj ID rezerwacji do anulowania: ";
    cin >> id;

    for (int i = 0; i < reservations.size(); i++) {
        if (reservations[i].id == id) {
            reservations.erase(reservations.begin() + i);
            cout << "Rezerwacja anulowana.\n";
            return;
        }
    }
    cout << "Nie znaleziono takiej rezerwacji.\n";
}

void modifyReservation() {
    int id;
    cout << "Podaj ID rezerwacji do modyfikacji: ";
    cin >> id;

    for (auto& r : reservations) {
        if (r.id == id) {
            cout << "Nowa liczba osob: ";
            cin >> r.people;

            cout << "Nowa data od (d m r): ";
            cin >> r.from.day >> r.from.month >> r.from.year;

            cout << "Nowa data do (d m r): ";
            cin >> r.to.day >> r.to.month >> r.to.year;

            if (!isRoomAvailable(r.roomId, r.from, r.to)) {
                cout << "Pokoj w tym terminie zajety! Zmiana anulowana.\n";
                return;
            }

            cout << "Zmodyfikowano.\n";
            return;
        }
    }
    cout << "Brak takiej rezerwacji.\n";
}

void searchByLastName() {
    string ln;
    cout << "Nazwisko: ";
    cin >> ln;

    for (const auto& r : reservations) {
        if (r.lastName == ln) {
            cout << "ID: " << r.id << " pokoj: " << r.roomId << "\n";
        }
    }
}

void searchByDate() {
    int d, m, y;
    cout << "Data (d m r): ";
    cin >> d >> m >> y;

    Date x(d, m, y);

    for (const auto& r : reservations) {
        if (x >= r.from && x <= r.to) {
            cout << "ID: " << r.id
                << " pokoj: " << r.roomId
                << " nazwisko: " << r.lastName << "\n";
        }
    }
}

void searchById() {
    int id;
    cout << "ID: ";
    cin >> id;

    for (const auto& r : reservations) {
        if (r.id == id) {
            cout << "Znaleziono: " << r.lastName
                << ", pokoj " << r.roomId << "\n";
            return;
        }
    }
    cout << "Brak.\n";
}

void saveRooms() {
    ofstream file("rooms.txt");
    if (!file) { cout << "Blad zapisu rooms.txt\n"; return; }

    for (auto& r : rooms) {
        file << r.id << " " << r.type << " "
            << r.capacity << " " << r.basePrice << "\n";
    }

    file.close();
    cout << "Zapisano pokoje.\n";
}

void loadRooms() {
    ifstream file("rooms.txt");
    if (!file) return;

    rooms.clear();

    int id, cap;
    string type;
    double price;

    while (file >> id >> type >> cap >> price) {
        rooms.push_back(Room(id, type, cap, price));
    }

    file.close();
    cout << "Wczytano pokoje.\n";
}

void saveReservations() {
    ofstream file("reservations.txt");
    if (!file) { cout << "Blad zapisu reservations.txt\n"; return; }

    for (auto& r : reservations) {
        file << r.id << " " << r.lastName << " " << r.roomId << " "
            << r.people << " "
            << r.from.day << " " << r.from.month << " " << r.from.year << " "
            << r.to.day << " " << r.to.month << " " << r.to.year << "\n";
    }

    file.close();
    cout << "Zapisano rezerwacje.\n";
}

void loadReservations() {
    ifstream file("reservations.txt");
    if (!file) return;

    reservations.clear();

    int id, rid, people;
    int d1, m1, y1, d2, m2, y2;
    string ln;

    while (file >> id >> ln >> rid >> people
        >> d1 >> m1 >> y1
        >> d2 >> m2 >> y2)
    {
        reservations.push_back(
            Reservation(id, ln, rid, people,
                Date(d1, m1, y1),
                Date(d2, m2, y2))
        );

        if (id >= nextResId) nextResId = id + 1;
    }

    file.close();
    cout << "Wczytano rezerwacje.\n";
}

void menu() {
    cout << "\n--- MENU ---\n";
    cout << "1. Dodaj rezerwacje\n";
    cout << "2. Anuluj rezerwacje\n";
    cout << "3. Modyfikuj rezerwacje\n";
    cout << "4. Szukaj po nazwisku\n";
    cout << "5. Szukaj po dacie\n";
    cout << "6. Szukaj po ID\n";
    cout << "7. Zapisz dane\n";
    cout << "8. Wczytaj dane\n";
    cout << "0. Wyjdz\n";
}

int main() {
    loadRooms();
    loadReservations();

    if (rooms.size() == 0) {
        rooms.push_back(Room(1, "single", 1, 100));
        rooms.push_back(Room(2, "double", 2, 150));
        rooms.push_back(Room(3, "suite", 4, 300));
    }

    int op;
    while (true) {
        menu();
        cout << "Wybor: ";
        cin >> op;

        if (op == 0) break;
        if (op == 1) addReservation();
        if (op == 2) cancelReservation();
        if (op == 3) modifyReservation();
        if (op == 4) searchByLastName();
        if (op == 5) searchByDate();
        if (op == 6) searchById();
        if (op == 7) { saveRooms(); saveReservations(); }
        if (op == 8) { loadRooms(); loadReservations(); }
    }

    return 0;
}
