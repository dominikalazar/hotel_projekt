#include <iostream>
#include <vector>
#include <string>

using namespace std;

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
        cout << "Pokoj ID: " << id
            << ", Typ: " << type
            << ", Miejsca: " << capacity
            << ", Cena: " << basePrice << endl;
    }
};

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

    int getId() const { return reservationId; }
    string getClientName() const { return clientName; }
    int getRoomId() const { return roomId; }

    void printInfo() const {
        cout << "Rezerwacja ID: " << reservationId
            << ", Klient: " << clientName
            << ", Pokoj: " << roomId
            << ", Od: " << dateFrom
            << ", Do: " << dateTo
            << ", Osoby: " << peopleCount << endl;
    }
};

class HotelSystem {
private:
    vector<Room> rooms;
    vector<Reservation> reservations;

public:
    void addRoom() {
        int id, capacity;
        string type;
        double price;

        cout << "ID pokoju: ";
        cin >> id;

        cout << "Typ (single/double/suite): ";
        cin >> type;

        cout << "Pojemnosc: ";
        cin >> capacity;

        cout << "Cena bazowa: ";
        cin >> price;

        rooms.push_back(Room(id, type, capacity, price));

        cout << "Pokoj dodany." << endl;
    }

    void listRooms() {
        for (const auto& r : rooms) {
            r.printInfo();
        }
    }

    void addReservation() {
        int id, roomId, people;
        string name, from, to;

        cout << "ID rezerwacji: ";
        cin >> id;

        cout << "Nazwisko klienta: ";
        cin >> name;

        cout << "ID pokoju: ";
        cin >> roomId;

        cout << "Data od: ";
        cin >> from;

        cout << "Data do: ";
        cin >> to;

        cout << "Liczba osob: ";
        cin >> people;

        reservations.push_back(Reservation(id, name, roomId, from, to, people));

        cout << "Rezerwacja dodana." << endl;
    }

    void listReservations() {
        for (const auto& r : reservations) {
            r.printInfo();
        }
    }
};

int main() {
    HotelSystem system;
    int choice;

    do {
        cout << "\n--- SYSTEM HOTELU ---\n";
        cout << "1. Dodaj pokoj\n";
        cout << "2. Lista pokoi\n";
        cout << "3. Dodaj rezerwacje\n";
        cout << "4. Lista rezerwacji\n";
        cout << "0. Wyjdz\n";

        cout << "Wybor: ";
        cin >> choice;

        switch (choice) {
        case 1:
            system.addRoom();
            break;
        case 2:
            system.listRooms();
            break;
        case 3:
            system.addReservation();
            break;
        case 4:
            system.listReservations();
            break;
        case 0:
            cout << "Koniec programu." << endl;
            break;
        default:
            cout << "Nie ma takiej opcji." << endl;
        }

    } while (choice != 0);

    return 0;
}
