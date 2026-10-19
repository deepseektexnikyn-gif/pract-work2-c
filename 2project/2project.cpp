// ============================================================
//  Проект: «Управление тупым домом»
//  Дисциплина: ООП на C++, V семестр
//  Всё в одном файле: интерфейс, абстрактный класс, наследники,
//  контроллер, меню и точка входа.
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <limits>

// ============================================================
//  1. Интерфейс IPrintable
// ============================================================
class IPrintable {
public:
    virtual void printInfo() const = 0;
    virtual ~IPrintable() = default;
};

// ============================================================
//  2. Абстрактный базовый класс Device
// ============================================================
class Device : public IPrintable {
protected:
    std::string name_;
    std::string room_;
    bool isOn_;

public:
    Device(const std::string& name, const std::string& room, bool isOn)
        : name_(name), room_(room), isOn_(isOn) {
    }

    virtual ~Device() = default;

    // чисто виртуальные — делают класс абстрактным
    virtual void turnOn() = 0;
    virtual void turnOff() = 0;
    virtual std::string getType() const = 0;
    virtual std::string getState() const = 0;
    virtual double getParam() const = 0;
    virtual void setParam(double value) = 0;

    // общие геттеры
    std::string getName() const { return name_; }
    std::string getRoom() const { return room_; }
    bool isEnabled() const { return isOn_; }

    void printInfo() const override {
        std::cout << "[" << getType() << "] " << name_
            << " (" << room_ << "): " << getState()
            << ", параметр=" << getParam() << "\n";
    }
};

// ============================================================
//  3. Наследник Light
// ============================================================
class Light : public Device {
private:
    double brightness_; // 0..100

public:
    Light(const std::string& name, const std::string& room,
        bool isOn, double brightness)
        : Device(name, room, isOn), brightness_(brightness) {
    }

    void turnOn() override { isOn_ = true; }
    void turnOff() override { isOn_ = false; }

    std::string getType() const override { return "Light"; }
    std::string getState() const override {
        return isOn_ ? "горит" : "выключено";
    }
    double getParam() const override { return brightness_; }
    void setParam(double value) override { brightness_ = value; }
};

// ============================================================
//  4. Наследник Thermostat
// ============================================================
class Thermostat : public Device {
private:
    double targetTemp_;

public:
    Thermostat(const std::string& name, const std::string& room,
        bool isOn, double targetTemp)
        : Device(name, room, isOn), targetTemp_(targetTemp) {
    }

    void turnOn() override { isOn_ = true; }
    void turnOff() override { isOn_ = false; }

    std::string getType() const override { return "Thermostat"; }
    std::string getState() const override {
        return isOn_ ? "нагревает" : "выключен";
    }
    double getParam() const override { return targetTemp_; }
    void setParam(double value) override { targetTemp_ = value; }
};

// ============================================================
//  5. Наследник SmartSpeaker
// ============================================================
class SmartSpeaker : public Device {
private:
    int volume_; // 0..100

public:
    SmartSpeaker(const std::string& name, const std::string& room,
        bool isOn, int volume)
        : Device(name, room, isOn), volume_(volume) {
    }

    void turnOn() override { isOn_ = true; }
    void turnOff() override { isOn_ = false; }

    std::string getType() const override { return "SmartSpeaker"; }
    std::string getState() const override {
        return isOn_ ? "играет" : "не играет";
    }
    double getParam() const override { return static_cast<double>(volume_); }
    void setParam(double value) override { volume_ = static_cast<int>(value); }
};

// ============================================================
//  6. Наследник VacuumCleaner + «фишка» buildRoute
// ============================================================
class VacuumCleaner : public Device {
private:
    int battery_; // 0..100

public:
    VacuumCleaner(const std::string& name, const std::string& room,
        bool isOn, int battery)
        : Device(name, room, isOn), battery_(battery) {
    }

    void turnOn() override { isOn_ = true; }
    void turnOff() override { isOn_ = false; }

    std::string getType() const override { return "VacuumCleaner"; }
    std::string getState() const override {
        return isOn_ ? "убирает" : "на базе";
    }
    double getParam() const override { return static_cast<double>(battery_); }
    void setParam(double value) override { battery_ = static_cast<int>(value); }

    // «Фишка» часть 1: имитация маршрута уборки
    void buildRoute(const std::vector<std::string>& rooms) const {
        if (!isOn_) {
            std::cout << "  Пылесос выключен — маршрут не построен.\n";
            return;
        }
        std::cout << "  Маршрут уборки для " << name_ << ":\n";
        for (size_t i = 0; i < rooms.size(); ++i) {
            std::cout << "   " << (i + 1) << ". " << rooms[i] << "\n";
        }
        int left = battery_ - static_cast<int>(rooms.size()) * 10;
        if (left < 0) left = 0;
        std::cout << "  Заряд на финише: " << left << "%\n";
    }
};

// ============================================================
//  7. Контроллер DumbHomeController
// ============================================================
class DumbHomeController {
private:
    std::vector<std::unique_ptr<Device>> devices_;      // владеет устройствами
    std::map<std::string, std::vector<Device*>> rooms_; // комната -> устройства
    std::string filename_;

public:
    DumbHomeController() = default;

    bool loadFromFile(const std::string& filename) {
        filename_ = filename;
        devices_.clear();
        rooms_.clear();

        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Не удалось открыть файл: " << filename << "\n";
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string type, name, room, state, paramStr;

            std::getline(ss, type, ';');
            std::getline(ss, name, ';');
            std::getline(ss, room, ';');
            std::getline(ss, state, ';');
            std::getline(ss, paramStr, ';');

            bool isOn = (state == "on");
            double param = 0.0;
            try {
                param = std::stod(paramStr);
            }
            catch (...) {
                std::cerr << "Ошибка параметра в строке: " << line << "\n";
                continue;
            }

            std::unique_ptr<Device> dev;
            try {
                if (type == "Light") {
                    dev = std::make_unique<Light>(name, room, isOn, param);
                }
                else if (type == "Thermostat") {
                    dev = std::make_unique<Thermostat>(name, room, isOn, param);
                }
                else if (type == "SmartSpeaker") {
                    dev = std::make_unique<SmartSpeaker>(name, room, isOn,
                        static_cast<int>(param));
                }
                else if (type == "VacuumCleaner") {
                    dev = std::make_unique<VacuumCleaner>(name, room, isOn,
                        static_cast<int>(param));
                }
                else {
                    std::cerr << "Неизвестный тип: " << type << "\n";
                    continue;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Ошибка создания: " << e.what() << "\n";
                continue;
            }

            rooms_[room].push_back(dev.get());  // агрегация
            devices_.push_back(std::move(dev)); // композиция
        }

        std::cout << "Загружено устройств: " << devices_.size() << "\n";
        return true;
    }

    bool saveToFile() const {
        if (filename_.empty()) {
            std::cerr << "Имя файла не задано.\n";
            return false;
        }
        std::ofstream file(filename_);
        if (!file.is_open()) {
            std::cerr << "Не удалось открыть файл для записи: " << filename_ << "\n";
            return false;
        }
        for (const auto& d : devices_) {
            file << d->getType() << ";"
                << d->getName() << ";"
                << d->getRoom() << ";"
                << (d->isEnabled() ? "on" : "off") << ";"
                << d->getParam() << "\n";
        }
        std::cout << "Сохранено устройств: " << devices_.size() << "\n";
        return true;
    }

    void showAll() const {
        if (devices_.empty()) {
            std::cout << "Устройств нет.\n";
            return;
        }
        std::cout << "--- Все устройства ---\n";
        for (const auto& d : devices_) {
            d->printInfo(); // полиморфизм
        }
    }

    void showByRoom(const std::string& room) const {
        auto it = rooms_.find(room);
        if (it == rooms_.end() || it->second.empty()) {
            std::cout << "В комнате \"" << room << "\" нет устройств.\n";
            return;
        }
        std::cout << "--- Комната: " << room << " ---\n";
        for (const Device* d : it->second) {
            d->printInfo();
        }
    }

    void turnOnAllInRoom(const std::string& room) {
        auto it = rooms_.find(room);
        if (it == rooms_.end()) {
            std::cout << "Комната не найдена.\n";
            return;
        }
        for (Device* d : it->second) {
            d->turnOn();
        }
        std::cout << "Все устройства в комнате \"" << room << "\" включены.\n";
    }

    Device* findDeviceByName(const std::string& name) const {
        for (const auto& d : devices_) {
            if (d->getName() == name) return d.get();
        }
        return nullptr;
    }

    // «Фишка» часть 2: суммарное энергопотребление
    double totalPower() const {
        double total = 0.0;
        for (const auto& d : devices_) {
            if (!d->isEnabled()) continue;
            std::string type = d->getType();
            if (type == "Light")              total += d->getParam() * 1.0;
            else if (type == "Thermostat")    total += 1500.0;
            else if (type == "SmartSpeaker")  total += d->getParam() * 0.5;
            else if (type == "VacuumCleaner") total += 800.0;
        }
        return total;
    }

    // «Фишка» часть 3: самое активное устройство
    const Device* mostActiveDevice() const {
        const Device* best = nullptr;
        for (const auto& d : devices_) {
            if (!d->isEnabled()) continue;
            if (!best || d->getParam() > best->getParam()) {
                best = d.get();
            }
        }
        return best;
    }

    // «Фишка» часть 4: маршрут уборки
    void buildCleaningRoute() const {
        std::vector<std::string> roomOrder;
        for (const auto& pair : rooms_) {
            roomOrder.push_back(pair.first);
        }
        bool found = false;
        for (const auto& d : devices_) {
            if (d->getType() == "VacuumCleaner") {
                const VacuumCleaner* v =
                    dynamic_cast<const VacuumCleaner*>(d.get());
                if (v) {
                    v->buildRoute(roomOrder);
                    found = true;
                }
            }
        }
        if (!found) {
            std::cout << "Пылесосов в доме нет.\n";
        }
    }

    size_t size() const { return devices_.size(); }
};

// ============================================================
//  8. Меню и точка входа
// ============================================================
void printMenu() {
    std::cout << "\n===== УПРАВЛЕНИЕ ТУПЫМ ДОМОМ =====\n"
        << "1. Показать все устройства\n"
        << "2. Показать устройства по комнате\n"
        << "3. Включить устройство\n"
        << "4. Выключить устройство\n"
        << "5. Включить все в комнате\n"
        << "6. Суммарное энергопотребление (фишка)\n"
        << "7. Самое активное устройство (фишка)\n"
        << "8. Маршрут уборки пылесосов (фишка)\n"
        << "9. Сохранить в файл\n"
        << "0. Выход\n"
        << "Выбор: ";
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL,"rus");
    std::string filename = (argc > 1) ? argv[1] : "devices.txt";

    DumbHomeController home;
    if (!home.loadFromFile(filename)) {
        std::cerr << "Работа продолжена без данных.\n";
    }

    int choice = -1;
    while (choice != 0) {
        printMenu();
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Некорректный ввод.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::string name, room;
        switch (choice) {
        case 1:
            home.showAll();
            break;
        case 2:
            std::cout << "Комната: ";
            std::getline(std::cin, room);
            home.showByRoom(room);
            break;
        case 3: {
            std::cout << "Имя устройства: ";
            std::getline(std::cin, name);
            Device* d = home.findDeviceByName(name);
            if (d) { d->turnOn(); std::cout << "Включено.\n"; }
            else std::cout << "Не найдено.\n";
            break;
        }
        case 4: {
            std::cout << "Имя устройства: ";
            std::getline(std::cin, name);
            Device* d = home.findDeviceByName(name);
            if (d) { d->turnOff(); std::cout << "Выключено.\n"; }
            else std::cout << "Не найдено.\n";
            break;
        }
        case 5:
            std::cout << "Комната: ";
            std::getline(std::cin, room);
            home.turnOnAllInRoom(room);
            break;
        case 6:
            std::cout << "Суммарное энергопотребление: "
                << home.totalPower() << " Вт\n";
            break;
        case 7: {
            const Device* d = home.mostActiveDevice();
            if (d) {
                std::cout << "Самое активное: ";
                d->printInfo();
            }
            else {
                std::cout << "Нет включённых устройств.\n";
            }
            break;
        }
        case 8:
            home.buildCleaningRoute();
            break;
        case 9:
            home.saveToFile();
            break;
        case 0:
            std::cout << "Выход.\n";
            break;
        default:
            std::cout << "Неизвестная команда.\n";
        }
    }
    return 0;
}