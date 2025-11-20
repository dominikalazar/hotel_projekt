#include <iostream>
#include <vector>
#include <string>

using namespace std;

// ---------------------- DATY -----------------------------

int dayOfWeek(int y, int m, int d) {
    if (m < 3) {
        m += 12;
        y -= 1;
    }

    int K = y % 100;
    int J = y / 100;

    int h = (d + (13 * (m + 1)) / 5 + K + (K / 4) + (J / 4) + (5 * J)) % 7;

    // Zeller:
    // 0 = sobota
    // 1 = niedziela
    // 2 = poniedziałek
    // 3 = wtorek
    // 4 = środa
    // 5 = czwartek
    // 6 = piątek

    return h;
}

void parseDate(const string& date, int& y, int& m, int& d) {
    y = stoi(date.substr(0, 4));
    m = stoi(date.substr(5, 2));
    d = stoi(date.substr(8, 2));
}

bool isWeekend(const string& date) {
    int y, m, d;
    parseDate(date, y, m, d);

    int w = dayOfWeek(y, m, d);

    return (w == 0 || w == 1); // sobota, niedziela
}

string nextDay(const string& date) {
    int y, m, d;
    parseDate(date, y, m, d);

    int daysInMonth[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    if (leap) daysInMonth[1] = 29;

    d++;

    if (d > daysInMonth[m - 1]) {
        d = 1;
        m++;
        if (m > 12) {
            m = 1;
            y++;
        }
    }

    string ys = to_string(y);
    string ms = (m < 10 ? "0" : "") + to_string(m);
    string ds = (d < 10 ? "0" : "") + to_string(d);

    return ys + "-" + ms + "-" + ds;
}

// ---------------------- ROOM -----------------------------

class Room {
private:
    int id;
    string type;
    int capacity;
    double basePrice;

public:
    Room() {}

    Room(int id, const string& type, int capacity, double basePrice) {
        this->id = id;
        this->type = type;
        this->capacity = capacity;
        this->basePrice = basePrice;
    }

    int getId() const { return id; }
    string getType() const { return type; }
    int getCapacity() const { return capacity; }
    double getBasePrice() const { return basePrice; }

    void printInfo() const {
        cout << "ID: " << id
            << " | Typ: " << type
            << " | Miejsca: " << capacity
            << " | Cena bazowa: " << basePrice << " zl\n";
    }
};

// ---------------------- RESERVATION -----------------------------

class Reservation {
private:
    int reservationId;
    string clientName;
    int roomId;
    string dateFrom;
    string dateTo;
    int peopleCount;

public:
    Reservation() {}

    Reservation(int id, const string& name, int roomId,
        const string& from, const string& to, int people) {
        this->reservationId = id;
        this->clientName = name;
        this->roomId = roomId;
        this->dateFrom = from;
        this->dateTo = to;
        this->peopleCount = people;
    }

    int getRoomId() const { return roomId; }
    string getFrom() const { return dateFrom; }
    string getTo() const { return dateTo; }

    bool datesOverlap(const string& f1, const string& t1,
        const string& f2, const string& t2) const {
        return (f1 <= t2 && t1 >= f2);
    }

    bool conflictsWith(const Reservation& other) const {
        if (roomId != other.roomId) return false;
        return datesOverlap(dateFrom, dateTo,
            other.dateFrom, other.dateTo);
    }

    void printInfo() const {
        cout << "Rezerwacja " << reservationId
            << " | Klient: " << clientName
            << " | Pokoj: " << roomId
            << " | Od: " << dateFrom
            << " | Do: " << dateTo
            << " | Osoby: " << peopleCount << endl;
    }
};

// ---------------------- HOTEL SYSTEM -----------------------------

class HotelSystem {
private:
    vector<Room> rooms;
    vector<Reservation> reservations;

public:

    // --------- Cena za całą rezerwację (dokładnie dzień po dniu) ---------

    double calculatePrice(double basePrice,
        const string& from,
        const string& to) {

        string current = from;
        double total = 0.0;

        while (true) {
            double dayPrice = basePrice;

            if (isWeekend(current)) {
                dayPrice *= 1.20; // narzut weekendowy
            }

            total += dayPrice;

            if (current == to) break;

            current = nextDay(current);
        }

        // policz ilość dni (już bez znaczenia – tylko do rabatu)
        int days = 1;
        string t = from;
        while (t != to) {
            t = nextDay(t);
            days++;
        }

        if (days > 7) {
            total *= 0.90;
        }

        return total;
    }

    // --------- Pokoje ---------

    void addRoom() {
        int id, capacity;
        string type;
        double price;

        cout << "ID: ";
        cin >> id;

        cout << "Typ: ";
        cin >> type;

        cout << "Pojemnosc: ";
        cin >> capacity;

        cout << "Cena bazowa: ";
        cin >> price;

        rooms.push_back(Room(id, type, capacity, price));

        cout << "Pokoj dodany.\n";
    }

    Room* findRoom(int id) {
        for (auto& r : rooms) {
            if (r.getId() == id) return &r;
        }
        return nullptr;
    }

    void listRooms() {
        for (const auto& r : rooms) r.printInfo();
    }

    // --------- Dostępność ---------

    bool isRoomAvailable(int roomId, const string& from, const string& to) {
        Reservation temp(0, "", roomId, from, to, 0);

        for (const auto& r : reservations) {
            if (temp.conflictsWith(r)) return false;
        }
        return true;
    }

    // --------- Rezerwacje ---------

    void addReservation() {
        int id, roomId, people;
        string name, from, to;

        cout << "ID rezerwacji: ";
        cin >> id;

        cout << "Nazwisko: ";
        cin >> name;

        cout << "ID pokoju: ";
        cin >> roomId;

        Room* r = findRoom(roomId);
        if (!r) {
            cout << "Pokoj nie istnieje!\n";
            return;
        }

        cout << "Osoby: ";
        cin >> people;

        if (people > r->getCapacity()) {
            cout << "Za duzo osob na ten pokoj!\n";
            return;
        }

        cout << "Data od (YYYY-MM-DD): ";
        cin >> from;

        cout << "Data do (YYYY-MM-DD): ";
        cin >> to;

        if (!isRoomAvailable(roomId, from, to)) {
            cout << "Pokoj zajety w tym terminie!\n";
            return;
        }

        double price = calculatePrice(r->getBasePrice(), from, to);

        cout << "Cena calej rezerwacji: " << price << " zl\n";

        reservations.push_back(Reservation(id, name, roomId, from, to, people));

        cout << "Rezerwacja dodana.\n";
    }

    void listReservations() {
        for (const auto& r : reservations) r.printInfo();
    }
};

// ---------------------- MAIN -----------------------------

int main() {
    HotelSystem system;
    int choice;

    do {
        cout << "\n--- HOTEL ---\n";
        cout << "1. Dodaj pokoj\n";
        cout << "2. Lista pokoi\n";
        cout << "3. Dodaj rezerwacje\n";
        cout << "4. Lista rezerwacji\n";
        cout << "0. Wyjdz\n";

        cout << "Wybor: ";
        cin >> choice;

        switch (choice) {
        case 1: system.addRoom(); break;
        case 2: system.listRooms(); break;
        case 3: system.addReservation(); break;
        case 4: system.listReservations(); break;
        case 0: cout << "Koniec programu.\n"; break;
        default: cout << "Nie ma takiej opcji.\n";
        }
    } while (choice != 0);

    return 0;
}
