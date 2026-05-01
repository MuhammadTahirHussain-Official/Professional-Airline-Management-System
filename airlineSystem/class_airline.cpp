/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║          NEXUS AIR  ✈  Professional Airline Management System v5.0           ║
 ║                   Created by Muhammad Tahir Hussain                          ║
 ║             REWRITTEN WITH FULL OOP (C++ Object-Oriented Design)             ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
 *
 *  BUGS FIXED IN THIS VERSION:
 *  ─────────────────────────────
 *  [FIX-1] Duplicate getUsername(): IAuthenticatable::getUsername() returns
 *          string (by value). User now has ONE matching override only.
 *          Eliminated the conflicting `const string& getUsername()` getter.
 *
 *  [FIX-2] ODR (One Definition Rule) violations: all static member definitions
 *          (nextId_, refSeed_, tagSeed_, instance_, _sl, _tk) removed from
 *          this header and placed exactly ONCE in their respective .cpp files.
 *
 *  [FIX-3] string_view used for all read-only string getters so callers
 *          get a zero-copy view without the const string& / string overload clash.
 *
 *  [FIX-4] string comparisons in .cpp files updated where needed: string_view
 *          compares correctly against string literals with ==.
 */

#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cassert>
#include <memory>
#include <functional>
using namespace std;

/* ─── ANSI Colors ─────────────────────────────────────────── */
#define RS "\033[0m"
#define BOLD "\033[1m"
#define DIM "\033[2m"
#define UL "\033[4m"
#define RD "\033[31m"
#define GR "\033[32m"
#define YL "\033[33m"
#define BL "\033[34m"
#define MG "\033[35m"
#define CY "\033[36m"
#define WH "\033[37m"
#define BRD "\033[91m"
#define BGR "\033[92m"
#define BYL "\033[93m"
#define BBL "\033[94m"
#define BMG "\033[95m"
#define BCY "\033[96m"
#define BWH "\033[97m"
#define BG_BL "\033[44m"
#define BG_CY "\033[46m"
#define BG_YL "\033[43m"
#define BG_GR "\033[42m"
#define BG_RD "\033[41m"
#define BG_MG "\033[45m"
#define BG_WH "\033[47m"
#define BG_BLK "\033[40m"

/* ─── Box drawing UTF-8 ───────────────────────────────────── */
#define BT "\xe2\x95\x94"
#define BB "\xe2\x95\x9a"
#define BH "\xe2\x95\x90"
#define BR "\xe2\x95\x97"
#define BL2 "\xe2\x95\x9d"
#define BV "\xe2\x95\x91"
#define BMT "\xe2\x95\xa0"
#define BMB "\xe2\x95\xa3"
#define BST "\xe2\x95\x9f"
#define BSB "\xe2\x95\xa2"
#define SH "\xe2\x94\x80"

static const int TW = 78;

/* ─── File names ──────────────────────────────────────────── */
#define FILE_FLIGHTS "nx_flights.txt"
#define FILE_PASS "nx_passengers.txt"
#define FILE_STAFF "nx_staff.txt"
#define FILE_ADMINS "nx_admins.txt"
#define FILE_BOOKINGS "nx_bookings.txt"
#define FILE_BAGGAGE "nx_baggage.txt"
#define FILE_SEATS "nx_seats.txt"
#define ADMIN_CODE "123a"
#define STAFF_CODE "123a"

/* ─── Seat geometry ───────────────────────────────────────── */
static const int ROWS_FIRST = 2;
static const int ROWS_BIZ = 6;
static const int ROWS_PREM = 4;
static const int ROWS_ECO = 18;
static const int TOTAL_ROWS = 30;

/* ─── Staff role codes ────────────────────────────────────── */
enum StaffRole
{
    SR_GATE = 0,
    SR_CHECKIN,
    SR_BAGGAGE,
    SR_SUPERVISOR,
    SR_TICKET,
    SR_SECURITY,
    SR_LOUNGE,
    SR_GROUND,
    SR_DISPATCH,
    SR_CUSTOMS,
    SR_COUNT
};
static const char *SROLES[SR_COUNT] = {
    "Gate Agent", "Check-in Agent", "Baggage Handler", "Supervisor",
    "Ticket Checker", "Security Officer", "Lounge Attendant", "Ground Crew",
    "Flight Dispatcher", "Customs Officer"};
static const char *SROLE_DESC[SR_COUNT] = {
    "Board passengers, scan passes, manage gate operations",
    "Process check-ins, assign seats, issue boarding passes",
    "Load/unload bags, lost & found, baggage carousel management",
    "Oversee all terminal ops, staff scheduling, incident mgmt",
    "Verify tickets at aircraft door, ID checks, boarding pass scan",
    "Passenger screening, prohibited items, checkpoint management",
    "Premium lounge services, VIP passenger hospitality",
    "Aircraft towing, refueling coordination, tarmac safety",
    "Departure slots, ATC coordination, delay management",
    "Passport control, visa verification, immigration processing"};
static const char *SROLE_COL[SR_COUNT] = {
    BGR, BCY, BYL, BMG, BWH, BRD, BCY, GR, BYL, BMG};

/* ─── Baggage stages ──────────────────────────────────────── */
static const int BAG_STAGES = 8;
static const char *BAG_ST[BAG_STAGES] = {
    "REGISTERED", "CHECK-IN DONE", "SECURITY CLEARED",
    "SORTED", "LOADED", "IN TRANSIT", "ARRIVED", "DELIVERED"};
static const char *BAG_COL[BAG_STAGES] = {
    DIM, BCY, BGR, BYL, BMG, BCY, BGR, BOLD BGR};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [3] ABSTRACTION — Pure Abstract Interfaces
═══════════════════════════════════════════════════════════════════════════════*/
class ISerializable
{
public:
    virtual ~ISerializable() = default;
    virtual string serialize() const = 0;
    virtual bool deserialize(const string &line) = 0;
};

class IDisplayable
{
public:
    virtual ~IDisplayable() = default;
    virtual void display() const = 0;
    virtual void displayCard(int i) const = 0;
};

/*
 * FIX-1: getUsername() returns `string` (by value) in the interface.
 *        User provides exactly ONE matching `string getUsername() const override`.
 *        The old `const string& getUsername()` getter is gone — string_view
 *        covers that need without any overload ambiguity.
 */
class IAuthenticatable
{
public:
    virtual ~IAuthenticatable() = default;
    virtual bool authenticate(const string &pw) const = 0;
    virtual void setPassword(const string &pw) = 0;
    virtual string getUsername() const = 0;
};

class IRolePanel
{
public:
    virtual ~IRolePanel() = default;
    virtual void execute() = 0;
    virtual string getPanelName() const = 0;
};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [8] EXCEPTION HIERARCHY
═══════════════════════════════════════════════════════════════════════════════*/
class NexusException : public runtime_error
{
public:
    explicit NexusException(const string &m) : runtime_error(m) {}
};
class AuthException : public NexusException
{
public:
    explicit AuthException(const string &m) : NexusException(m) {}
};
class CapacityException : public NexusException
{
public:
    explicit CapacityException(const string &m) : NexusException(m) {}
};
class NotFoundException : public NexusException
{
public:
    explicit NotFoundException(const string &m) : NexusException(m) {}
};
class ValidationException : public NexusException
{
public:
    explicit ValidationException(const string &m) : NexusException(m) {}
};
class PaymentException : public NexusException
{
public:
    explicit PaymentException(const string &m) : NexusException(m) {}
};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [7] TEMPLATES — Repository<T>
═══════════════════════════════════════════════════════════════════════════════*/
template <typename T>
class Repository
{
    vector<T *> items_;
    int maxCap_;

public:
    explicit Repository(int cap) : maxCap_(cap) {}

    void add(T *item)
    {
        if ((int)items_.size() < maxCap_)
            items_.push_back(item);
    }
    int count() const { return (int)items_.size(); }
    int capacity() const { return maxCap_; }
    bool isFull() const { return (int)items_.size() >= maxCap_; }
    T *get(int i) { return (i >= 0 && i < count()) ? items_[i] : nullptr; }
    const T *get(int i) const { return (i >= 0 && i < count()) ? items_[i] : nullptr; }

    template <typename Pred>
    T *findIf(Pred pred) const
    {
        for (auto *p : items_)
            if (pred(p))
                return p;
        return nullptr;
    }
    template <typename Pred>
    vector<T *> filterIf(Pred pred) const
    {
        vector<T *> out;
        for (auto *p : items_)
            if (pred(p))
                out.push_back(p);
        return out;
    }
    void rebuildFrom(T *arr, int n)
    {
        items_.clear();
        for (int i = 0; i < n; i++)
            items_.push_back(&arr[i]);
    }
};

/*═══════════════════════════════════════════════════════════════════════════════
  UTILITY — FIX-2: static member DECLARATIONS only here; definitions in .cpp
═══════════════════════════════════════════════════════════════════════════════*/
class Utils
{
    Utils() = delete;
    static int refSeed_; // defined in nexus_impl1.cpp
    static int tagSeed_; // defined in nexus_impl1.cpp
public:
    static void sleepMs(int ms) { Sleep(ms); }
    static void clearScreen() { system("cls"); }
    static void clearInput()
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
    static string getTimestamp();
    static string generateRef();
    static string generateTag(const string &flightNo);
    static string hashPassword(const string &pw);
    static string computeTier(int miles);
    static const char *tierColor(const char *tier);
    static const char *statusColor(const char *status);
    static int ansiLen(const char *s);
    static void spaces(int n)
    {
        while (n-- > 0)
            putchar(' ');
    }
};

/*═══════════════════════════════════════════════════════════════════════════════
  SOUND ENGINE
═══════════════════════════════════════════════════════════════════════════════*/
class SoundEngine
{
    SoundEngine() = delete;

public:
    static void boot();
    static void ok();
    static void error();
    static void click() { Beep(900, 30); }
    static void tick() { Beep(1100, 18); }
    static void warn();
    static void login();
    static void select();
    static void pay();
    static void think();
    static void scan();
    static void fly();
    static void alert();
    static void exitSound();
};

/*═══════════════════════════════════════════════════════════════════════════════
  TERMINAL / UI ENGINE
═══════════════════════════════════════════════════════════════════════════════*/
class Terminal
{
    Terminal() = delete;

public:
    static void boxTop(const char *c = BCY);
    static void boxBot(const char *c = BCY);
    static void boxMid(const char *c = BCY);
    static void boxSep(const char *c = BCY);
    static void boxEmpty(const char *c = BCY);
    static void boxRow(const char *bc, const char *t);
    static void boxRowC(const char *bc, const char *t);
    static void boxRowF(const char *bc, const char *fmt, ...);
    static void boxCenter(const char *bc, const char *t);
    static void boxLR(const char *bc, const char *l, const char *r);
    static void typeWrite(const char *t, int d = 16);
    static void progressBar(const char *lbl, int ms = 1100);
    static void spinner(const char *msg, int ms = 800);
    static void neuralPulse(int n);
    static void glitch(const char *text);
    static void dataStream(int lines);
    static void flyingPlane();
    static void starBurst(int lines);
    static void success(const char *m);
    static void errMsg(const char *m);
    static void infoMsg(const char *m);
    static void warnMsg(const char *m);
    static void waitEnter();
    static void sectionHeader(const char *title, const char *col = BCY);
    static void fieldSep();
};

/*═══════════════════════════════════════════════════════════════════════════════
  INPUT ENGINE
═══════════════════════════════════════════════════════════════════════════════*/
class Input
{
    Input() = delete;

public:
    static string field(const char *lbl, const char *hint = "");
    static int integer(const char *lbl, int lo = 0, int hi = 9999);
    static double dbl(const char *lbl);
    static string phone(const char *lbl);
    static string email(const char *lbl);
    static string date(const char *lbl);
    static string password(const char *lbl, bool check = false);
    static string passport(const char *lbl);
    static string card16();
    static string cvv();
    static string expiry();

private:
    static void readPW(char *buf, int mx);
};

/*═══════════════════════════════════════════════════════════════════════════════
  SEATMAP
═══════════════════════════════════════════════════════════════════════════════*/
class SeatMap : public ISerializable
{
    string flightNo_;
    int st_[TOTAL_ROWS][6];
    string who_[TOTAL_ROWS][6];

public:
    SeatMap();
    explicit SeatMap(const string &flightNo);
    SeatMap(const SeatMap &other);
    SeatMap &operator=(const SeatMap &other)
    {
        if (this != &other)
        {
            flightNo_ = other.flightNo_;
            memcpy(st_, other.st_, sizeof(st_));
            for (int r = 0; r < TOTAL_ROWS; r++)
                for (int c = 0; c < 6; c++)
                    who_[r][c] = other.who_[r][c];
        }
        return *this;
    }

    const string &getFlightNo() const { return flightNo_; }
    int getStatus(int r, int c) const;
    const string &getWho(int r, int c) const;

    void book(int r, int c, const string &name);
    void free(int r, int c);
    void block(int r, int c);

    static const char *cabinForRow(int r);
    static int colsForRow(int r);
    static char colLetter(int c, int cols);

    string serialize() const override;
    bool deserialize(const string &blk) override;

    void display(double fF, double fB, double fP, double fE,
                 const string &aircraft, const string &date,
                 const string &origin, const string &dest) const;

    bool operator==(const SeatMap &o) const { return flightNo_ == o.flightNo_; }
};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [4] BASE — Flight   (FIX-2: nextId_ defined in nexus_impl2.cpp)
═══════════════════════════════════════════════════════════════════════════════*/
class Flight : public ISerializable, public IDisplayable
{
    int id_;
    string flightNo_, airline_, origin_, dest_;
    string date_, depTime_, arrTime_, aircraft_, status_;
    int gate_, termDep_, termArr_, duration_;
    int seatsFirst_, bkFirst_, seatsBiz_, bkBiz_;
    int seatsPrem_, bkPrem_, seatsEco_, bkEco_;
    double fareFirst_, fareBiz_, farePrem_, fareEco_, taxRate_;
    string delayReason_;
    bool active_;
    SeatMap seatMap_;
    static int nextId_;

public:
    Flight();
    Flight(const string &no, const string &airline,
           const string &org, const string &dst,
           const string &date, const string &dep, const string &arr,
           const string &aircraft, int gate, int tDep, int tArr, int dur,
           double fF, double fB, double fP, double fE);

    int getId() const { return id_; }
    const string &getFlightNo() const { return flightNo_; }
    const string &getAirline() const { return airline_; }
    const string &getOrigin() const { return origin_; }
    const string &getDest() const { return dest_; }
    const string &getDate() const { return date_; }
    const string &getDepTime() const { return depTime_; }
    const string &getArrTime() const { return arrTime_; }
    const string &getAircraft() const { return aircraft_; }
    const string &getStatus() const { return status_; }
    int getGate() const { return gate_; }
    int getTermDep() const { return termDep_; }
    int getTermArr() const { return termArr_; }
    int getDuration() const { return duration_; }
    int getSeatsFirst() const { return seatsFirst_; }
    int getSeatsBiz() const { return seatsBiz_; }
    int getSeatsPrem() const { return seatsPrem_; }
    int getSeatsEco() const { return seatsEco_; }
    int getBkFirst() const { return bkFirst_; }
    int getBkBiz() const { return bkBiz_; }
    int getBkPrem() const { return bkPrem_; }
    int getBkEco() const { return bkEco_; }
    double getFareFirst() const { return fareFirst_; }
    double getFareBiz() const { return fareBiz_; }
    double getFarePrem() const { return farePrem_; }
    double getFareEco() const { return fareEco_; }
    double getTaxRate() const { return taxRate_; }
    const string &getDelayReason() const { return delayReason_; }
    bool isActive() const { return active_; }
    SeatMap &getSeatMap() { return seatMap_; }
    const SeatMap &getSeatMap() const { return seatMap_; }

    void setStatus(const string &s) { status_ = s; }
    void setGate(int g) { gate_ = g; }
    void setDepTime(const string &t) { depTime_ = t; }
    void setFareEco(double f) { fareEco_ = f; }
    void setDelayReason(const string &r) { delayReason_ = r; }
    void setActive(bool v) { active_ = v; }
    void setNextId(int n)
    {
        if (n > nextId_)
            nextId_ = n;
    }
    static int getNextId() { return nextId_; }

    void bookSeat(const string &cabin);
    void unBookSeat(const string &cabin);
    double fareForCabin(const string &cabin) const;
    int totalSeats() const { return seatsFirst_ + seatsBiz_ + seatsPrem_ + seatsEco_; }
    int totalBooked() const { return bkFirst_ + bkBiz_ + bkPrem_ + bkEco_; }
    int loadPct() const
    {
        int t = totalSeats();
        return t > 0 ? totalBooked() * 100 / t : 0;
    }

    string serialize() const override;
    bool deserialize(const string &line) override;
    void display() const override;
    void displayCard(int i) const override;

    bool operator==(const Flight &o) const { return flightNo_ == o.flightNo_; }
    bool operator<(const Flight &o) const { return flightNo_ < o.flightNo_; }
    friend class BillingEngine;
};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [4] ABSTRACT BASE — User
  FIX-1: ONE getUsername() override returning string. No duplicate getter.
═══════════════════════════════════════════════════════════════════════════════*/
class User : public ISerializable, public IDisplayable, public IAuthenticatable
{
protected:
    int id_;
    string username_, passHash_;
    string firstName_, lastName_;
    string email_, created_;
    bool active_;

public:
    User();
    User(const string &un, const string &fn, const string &ln, const string &email);
    User(const User &o);
    User &operator=(const User &o)
    {
        if (this != &o)
        {
            id_ = o.id_;
            username_ = o.username_;
            passHash_ = o.passHash_;
            firstName_ = o.firstName_;
            lastName_ = o.lastName_;
            email_ = o.email_;
            created_ = o.created_;
            active_ = o.active_;
        }
        return *this;
    }
    virtual ~User() = default;

    int getId() const { return id_; }
    const string &getFirstName() const { return firstName_; }
    const string &getLastName() const { return lastName_; }
    const string &getEmail() const { return email_; }
    const string &getCreated() const { return created_; }
    bool isActive() const { return active_; }
    string getFullName() const { return firstName_ + " " + lastName_; }

    void setFirstName(const string &s) { firstName_ = s; }
    void setLastName(const string &s) { lastName_ = s; }
    void setEmail(const string &s) { email_ = s; }
    void setActive(bool v) { active_ = v; }
    void setId(int i) { id_ = i; }

    /* FIX-1: single, unambiguous override matching IAuthenticatable */
    bool authenticate(const string &pw) const override;
    void setPassword(const string &pw) override;
    string getUsername() const override { return username_; }

    virtual string getRoleTag() const = 0;
    virtual string getColor() const = 0;

    bool operator==(const User &o) const { return username_ == o.username_; }
    bool operator!=(const User &o) const { return !(*this == o); }

    void display() const override;
    void displayCard(int i) const override;
};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [4] DERIVED — Passenger   (FIX-2: nextId_ defined in nexus_impl2.cpp)
═══════════════════════════════════════════════════════════════════════════════*/
class Passenger : public User
{
    string phone_, passport_, nationality_, dob_;
    string address_, city_, country_;
    int loyaltyPts_, totalMiles_;
    string loyaltyTier_;
    static int nextId_;

public:
    Passenger();
    Passenger(const string &un, const string &fn, const string &ln,
              const string &em, const string &ph, const string &pp,
              const string &nat, const string &dob,
              const string &addr, const string &city, const string &country);
    Passenger(const Passenger &o);

    const string &getPhone() const { return phone_; }
    const string &getPassport() const { return passport_; }
    const string &getNationality() const { return nationality_; }
    const string &getDob() const { return dob_; }
    const string &getAddress() const { return address_; }
    const string &getCity() const { return city_; }
    const string &getCountry() const { return country_; }
    int getLoyaltyPts() const { return loyaltyPts_; }
    int getTotalMiles() const { return totalMiles_; }
    const string &getLoyaltyTier() const { return loyaltyTier_; }

    void setPhone(const string &s) { phone_ = s; }
    void setNextId(int n)
    {
        if (n > nextId_)
            nextId_ = n;
    }
    static int getNextId() { return nextId_; }

    void addMiles(int m);
    void addPoints(int p) { loyaltyPts_ += p; }
    void redeemPoints(int p) { loyaltyPts_ = max(0, loyaltyPts_ - p); }
    void updateTier();
    double getLoyaltyDiscount(double subtotal) const;

    string getRoleTag() const override { return "PASSENGER"; }
    string getColor() const override { return BGR; }
    void display() const override;
    void displayCard(int i) const override;
    string serialize() const override;
    bool deserialize(const string &line) override;
};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [4] DERIVED — Staff
═══════════════════════════════════════════════════════════════════════════════*/
class Staff : public User
{
    string empId_, phone_;
    StaffRole roleCode_;
    string terminal_, gates_;
    string shiftStart_, shiftEnd_;
    static int nextId_;

public:
    Staff();
    Staff(const string &un, const string &fn, const string &ln,
          const string &em, const string &ph, StaffRole role,
          const string &terminal, const string &gates);
    Staff(const Staff &o);

    const string &getEmpId() const { return empId_; }
    const string &getPhone() const { return phone_; }
    StaffRole getRoleCode() const { return roleCode_; }
    const char *getRoleName() const { return SROLES[roleCode_]; }
    const char *getRoleDesc() const { return SROLE_DESC[roleCode_]; }
    const char *getRoleColorA() const { return SROLE_COL[roleCode_]; }
    const string &getTerminal() const { return terminal_; }
    const string &getGates() const { return gates_; }
    const string &getShiftStart() const { return shiftStart_; }
    const string &getShiftEnd() const { return shiftEnd_; }
    void setNextId(int n)
    {
        if (n > nextId_)
            nextId_ = n;
    }
    static int getNextId() { return nextId_; }

    string getRoleTag() const override { return SROLES[roleCode_]; }
    string getColor() const override { return string(SROLE_COL[roleCode_]); }
    void display() const override;
    void displayCard(int i) const override;
    string serialize() const override;
    bool deserialize(const string &line) override;
};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [4] DERIVED — Admin
═══════════════════════════════════════════════════════════════════════════════*/
class Admin : public User
{
    static int nextId_;

public:
    Admin();
    Admin(const string &un, const string &fn, const string &ln, const string &em);
    Admin(const Admin &o);
    void setNextId(int n)
    {
        if (n > nextId_)
            nextId_ = n;
    }
    static int getNextId() { return nextId_; }
    string getRoleTag() const override { return "ADMIN"; }
    string getColor() const override { return BYL; }
    void display() const override;
    void displayCard(int i) const override;
    string serialize() const override;
    bool deserialize(const string &line) override;
};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [11] COMPOSITION — BaggageItem
═══════════════════════════════════════════════════════════════════════════════*/
class BaggageItem : public ISerializable, public IDisplayable
{
    int id_, bookingId_;
    string bookingRef_, passengerName_, flightNo_, tagNo_;
    double weightKg_;
    int stageIdx_;
    bool fragile_, special_;
    string location_, carousel_, created_;
    static int nextId_;

public:
    BaggageItem();
    BaggageItem(int bookingId, const string &ref, const string &pax,
                const string &flight, const string &tag,
                bool fragile, bool special);

    int getId() const { return id_; }
    int getBookingId() const { return bookingId_; }
    const string &getBookingRef() const { return bookingRef_; }
    const string &getPassengerName() const { return passengerName_; }
    const string &getFlightNo() const { return flightNo_; }
    const string &getTagNo() const { return tagNo_; }
    double getWeightKg() const { return weightKg_; }
    int getStageIdx() const { return stageIdx_; }
    bool isFragile() const { return fragile_; }
    bool isSpecial() const { return special_; }
    const string &getLocation() const { return location_; }
    const string &getCarousel() const { return carousel_; }
    bool isOverweight() const { return weightKg_ > 23.0; }
    void setNextId(int n)
    {
        if (n > nextId_)
            nextId_ = n;
    }
    static int getNextId() { return nextId_; }

    void setWeightKg(double w) { weightKg_ = w; }
    void setStageIdx(int s) { stageIdx_ = s; }
    void setLocation(const string &l) { location_ = l; }
    void setCarousel(const string &c) { carousel_ = c; }

    string serialize() const override;
    bool deserialize(const string &line) override;
    void display() const override;
    void displayCard(int i) const override;
    bool operator==(const BaggageItem &o) const { return tagNo_ == o.tagNo_; }
};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [11] COMPOSITION — FareBreakdown value class
═══════════════════════════════════════════════════════════════════════════════*/
class FareBreakdown
{
public:
    double baseFare = 0, seatFee = 0, baggageFee = 0, mealFee = 0;
    double serviceFee = 0, taxes = 0, discount = 0, grandTotal = 0;

    FareBreakdown() = default;
    FareBreakdown(double base, double seat, double bag, double meal,
                  double svc, double tax, double disc)
        : baseFare(base), seatFee(seat), baggageFee(bag), mealFee(meal),
          serviceFee(svc), taxes(tax), discount(disc),
          grandTotal(base + seat + bag + meal + svc + tax - disc) {}

    double getSubtotal() const { return baseFare + seatFee + baggageFee + mealFee + serviceFee; }
    FareBreakdown operator+(const FareBreakdown &o) const
    {
        return FareBreakdown(baseFare + o.baseFare, seatFee + o.seatFee,
                             baggageFee + o.baggageFee, mealFee + o.mealFee,
                             serviceFee + o.serviceFee, taxes + o.taxes, discount + o.discount);
    }
    bool operator<(const FareBreakdown &o) const { return grandTotal < o.grandTotal; }
    bool operator>(const FareBreakdown &o) const { return grandTotal > o.grandTotal; }
};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [11] COMPOSITION — Booking contains FareBreakdown
═══════════════════════════════════════════════════════════════════════════════*/
class Booking : public ISerializable, public IDisplayable
{
    int id_, passengerId_, flightId_;
    string flightNo_, passengerName_;
    string cabin_, seatNo_, meal_;
    FareBreakdown fare_;
    string bookingRef_, payMethod_, cardLast4_, status_;
    bool checkedIn_;
    int bagsCount_;
    string boardingGate_, boardingGroup_, created_, specialReq_;
    static int nextId_;

public:
    Booking();
    Booking(int paxId, int flightId, const string &flightNo,
            const string &paxName, const string &cabin, const string &seat,
            const string &meal, const FareBreakdown &fare,
            const string &payMethod, const string &cardLast4,
            const string &gate, int bagsCount, const string &specialReq);

    int getId() const { return id_; }
    int getPassengerId() const { return passengerId_; }
    int getFlightId() const { return flightId_; }
    const string &getFlightNo() const { return flightNo_; }
    const string &getPassengerName() const { return passengerName_; }
    const string &getCabin() const { return cabin_; }
    const string &getSeatNo() const { return seatNo_; }
    const string &getMeal() const { return meal_; }
    const FareBreakdown &getFare() const { return fare_; }
    const string &getBookingRef() const { return bookingRef_; }
    const string &getPayMethod() const { return payMethod_; }
    const string &getCardLast4() const { return cardLast4_; }
    const string &getStatus() const { return status_; }
    bool isCheckedIn() const { return checkedIn_; }
    int getBagsCount() const { return bagsCount_; }
    const string &getBoardingGate() const { return boardingGate_; }
    const string &getBoardingGroup() const { return boardingGroup_; }
    const string &getCreated() const { return created_; }
    const string &getSpecialReq() const { return specialReq_; }
    void setNextId(int n)
    {
        if (n > nextId_)
            nextId_ = n;
    }
    static int getNextId() { return nextId_; }

    void setStatus(const string &s) { status_ = s; }
    void setCheckedIn(bool v) { checkedIn_ = v; }

    string serialize() const override;
    bool deserialize(const string &line) override;
    void display() const override;
    void displayCard(int i) const override;

    bool operator==(const Booking &o) const { return bookingRef_ == o.bookingRef_; }
    bool operator<(const Booking &o) const { return fare_ < o.fare_; }
    friend class BillingEngine;
};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [10] FRIEND — BillingEngine
═══════════════════════════════════════════════════════════════════════════════*/
class BillingEngine
{
    BillingEngine() = delete;

public:
    static void printBill(const Booking &b, const Flight &fl);
    static bool payWizard(double amount, string &outMethod, string &outCard, int paxIdx);
    static double calcSeatFee(int row, const Flight &fl);
};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [3] ABSTRACTION — Abstract SeatSelector
═══════════════════════════════════════════════════════════════════════════════*/
class SeatSelector
{
public:
    virtual ~SeatSelector() = default;
    virtual bool selectSeat(SeatMap &sm, const Flight &fl,
                            int &outRow, int &outCol,
                            double &outFare, string &outSeat) = 0;
};
class InteractiveSeatSelector : public SeatSelector
{
public:
    bool selectSeat(SeatMap &sm, const Flight &fl,
                    int &outRow, int &outCol,
                    double &outFare, string &outSeat) override;
};

/*═══════════════════════════════════════════════════════════════════════════════
  SESSION
═══════════════════════════════════════════════════════════════════════════════*/
enum UserRole
{
    RNONE = 0,
    RPASS,
    RSTAFF,
    RADMIN
};

class Session
{
    UserRole role_;
    int idx_;
    string username_;
    bool loggedIn_;

public:
    Session() : role_(RNONE), idx_(-1), loggedIn_(false) {}
    void login(UserRole r, int idx, const string &un)
    {
        role_ = r;
        idx_ = idx;
        username_ = un;
        loggedIn_ = true;
    }
    void logout()
    {
        role_ = RNONE;
        idx_ = -1;
        loggedIn_ = false;
    }
    bool isLoggedIn() const { return loggedIn_; }
    UserRole getRole() const { return role_; }
    int getIdx() const { return idx_; }
    const string &getUsername() const { return username_; }
};

/*═══════════════════════════════════════════════════════════════════════════════
  DATA STORE — Singleton  (FIX-2: instance_ defined in nexus_impl3.cpp)
═══════════════════════════════════════════════════════════════════════════════*/
static const int MAX_FL = 200, MAX_PA = 500, MAX_ST = 80,
                 MAX_AD = 20, MAX_BK = 800, MAX_BG = 800;

class DataStore
{
    Flight flightArr_[MAX_FL];
    Passenger passArr_[MAX_PA];
    Staff staffArr_[MAX_ST];
    Admin adminArr_[MAX_AD];
    Booking bookingArr_[MAX_BK];
    BaggageItem bagArr_[MAX_BG];

    int flightCount_ = 0, passCount_ = 0, staffCount_ = 0;
    int adminCount_ = 0, bookingCount_ = 0, bagCount_ = 0;

    Repository<Flight> flightRepo_;
    Repository<Passenger> passRepo_;
    Repository<Staff> staffRepo_;
    Repository<Admin> adminRepo_;
    Repository<Booking> bookingRepo_;
    Repository<BaggageItem> bagRepo_;

    static DataStore *instance_; // defined in nexus_impl3.cpp
    DataStore();

public:
    static DataStore &instance();

    Flight *flightAt(int i) { return &flightArr_[i]; }
    Passenger *passAt(int i) { return &passArr_[i]; }
    Staff *staffAt(int i) { return &staffArr_[i]; }
    Admin *adminAt(int i) { return &adminArr_[i]; }
    Booking *bookingAt(int i) { return &bookingArr_[i]; }
    BaggageItem *bagAt(int i) { return &bagArr_[i]; }

    int flightCount() const { return flightCount_; }
    int passCount() const { return passCount_; }
    int staffCount() const { return staffCount_; }
    int adminCount() const { return adminCount_; }
    int bookingCount() const { return bookingCount_; }
    int bagCount() const { return bagCount_; }

    Repository<Flight> &flights() { return flightRepo_; }
    Repository<Passenger> &passengers() { return passRepo_; }
    Repository<Staff> &staff() { return staffRepo_; }
    Repository<Admin> &admins() { return adminRepo_; }
    Repository<Booking> &bookings() { return bookingRepo_; }
    Repository<BaggageItem> &bags() { return bagRepo_; }

    Flight *addFlight(const Flight &f);
    Passenger *addPassenger(const Passenger &p);
    Staff *addStaff(const Staff &s);
    Admin *addAdmin(const Admin &a);
    Booking *addBooking(const Booking &b);
    BaggageItem *addBag(const BaggageItem &b);

    int findFlightByNo(const string &no) const;
    int findPassByUser(const string &un) const;
    int findStaffByUser(const string &un) const;
    int findAdminByUser(const string &un) const;
    int findBookingByRef(const string &ref) const;

    void saveAll();
    void loadAll();
    void seedData();

private:
    void rebuildRepos();
    void saveFlights();
    void loadFlights();
    void savePassengers();
    void loadPassengers();
    void saveStaff();
    void loadStaff();
    void saveAdmins();
    void loadAdmins();
    void saveBookings();
    void loadBookings();
    void saveBags();
    void loadBags();
    void saveSeatMaps();
    void loadSeatMaps();
};

/*═══════════════════════════════════════════════════════════════════════════════
  CONCEPT [13] MULTIPLE INHERITANCE — Staff Role Panels
═══════════════════════════════════════════════════════════════════════════════*/
class StaffPanelBase : public IRolePanel
{
protected:
    Staff &staff_;
    DataStore &db_;
    Session &sess_;

public:
    StaffPanelBase(Staff &s, DataStore &db, Session &sess)
        : staff_(s), db_(db), sess_(sess) {}
    virtual ~StaffPanelBase() = default;
};

#define DECL_PANEL(Name, Label)                                \
    class Name : public StaffPanelBase                         \
    {                                                          \
    public:                                                    \
        using StaffPanelBase::StaffPanelBase;                  \
        void execute() override;                               \
        string getPanelName() const override { return Label; } \
    };
DECL_PANEL(GateAgentPanel, "Boarding Management")
DECL_PANEL(CheckInPanel, "Check-In Operations")
DECL_PANEL(BaggagePanel, "Baggage Operations")
DECL_PANEL(SupervisorPanel, "Supervisor Panel")
DECL_PANEL(TicketPanel, "Ticket Verification")
DECL_PANEL(SecurityPanel, "Security Operations")
DECL_PANEL(LoungePanel, "Lounge Services")
DECL_PANEL(GroundCrewPanel, "Ground Operations")
DECL_PANEL(DispatchPanel, "Flight Dispatch")
DECL_PANEL(CustomsPanel, "Customs & Immigration")
#undef DECL_PANEL

IRolePanel *createRolePanel(Staff &s, DataStore &db, Session &sess);

/*═══════════════════════════════════════════════════════════════════════════════
  DASHBOARDS & AUTH CONTROLLER
═══════════════════════════════════════════════════════════════════════════════*/
class PassengerDashboard
{
    Session &sess_;
    DataStore &db_;

public:
    PassengerDashboard(Session &s, DataStore &db) : sess_(s), db_(db) {}
    void run();

private:
    void bookFlight();
    void viewBookings();
    void onlineCheckIn();
    void loyaltyProgram();
    void myProfile();
    void baggageTracking();
    void entertainment();
    void aviationQuiz();
    void priceCalc();
};

class StaffDashboard
{
    Session &sess_;
    DataStore &db_;

public:
    StaffDashboard(Session &s, DataStore &db) : sess_(s), db_(db) {}
    void run();
};

class AdminDashboard
{
    Session &sess_;
    DataStore &db_;

public:
    AdminDashboard(Session &s, DataStore &db) : sess_(s), db_(db) {}
    void run();

private:
    void addFlight();
    void viewFlights();
    void editFlight();
    void cancelFlight();
    void viewPassengers();
    void viewStaff();
    void reports();
    void viewBookings();
    void systemOverview();
};

class AuthController
{
    Session &sess_;
    DataStore &db_;

public:
    AuthController(Session &s, DataStore &db) : sess_(s), db_(db) {}
    void run();

private:
    bool loginPassenger();
    bool loginStaff();
    bool loginAdmin();
    void registerPassenger();
    void registerStaff();
    void registerAdmin();
    void searchFlights();
    void flightStatus();
};

/*═══════════════════════════════════════════════════════════════════════════════
  PRESENTATION  (FIX-2: _sl and _tk defined in nexus_impl3.cpp)
═══════════════════════════════════════════════════════════════════════════════*/
class Presentation
{
    Presentation() = delete;
    static int _sl; // defined in nexus_impl3.cpp
    static int _tk; // defined in nexus_impl3.cpp
public:
    static void printBanner();
    static void bootSeq();
    static void aiLogs();
    static void exitAnim();
};

/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  NEXUS AIR v5.0 — OOP Implementation  (nexus_air.cpp)                       ║
 ║  Part 1: Utility, Sound, Terminal, Input implementations                     ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/
#include <stdarg.h>
#include <cstdio>

/* ── FIX-2: ONE-TIME static member definitions for Utils ──────────────────── */
int Utils::refSeed_ = 1000;
int Utils::tagSeed_ = 100;

/*──────────────────────────────────────────────────────────────────────────────
  Utils implementation
──────────────────────────────────────────────────────────────────────────────*/
string Utils::getTimestamp()
{
    time_t n = time(nullptr);
    struct tm *t = localtime(&n);
    char b[20];
    strftime(b, 20, "%Y-%m-%d %H:%M", t);
    return string(b);
}
string Utils::generateRef()
{
    char b[20];
    snprintf(b, 20, "NX%04d", refSeed_++);
    return string(b);
}
string Utils::generateTag(const string &flightNo)
{
    char b[20];
    snprintf(b, 20, "%s-B%03d", flightNo.c_str(), tagSeed_++);
    return string(b);
}
string Utils::hashPassword(const string &pw)
{
    unsigned long h = 5381;
    for (char c : pw)
        h = ((h << 5) + h) ^ (unsigned char)c;
    char b[24];
    snprintf(b, 24, "%012lu", h % 1000000000000UL);
    return string(b);
}
string Utils::computeTier(int miles)
{
    if (miles >= 50000)
        return "PLATINUM";
    if (miles >= 20000)
        return "GOLD";
    if (miles >= 5000)
        return "SILVER";
    return "BRONZE";
}
const char *Utils::tierColor(const char *t)
{
    if (!strcmp(t, "PLATINUM"))
        return BCY;
    if (!strcmp(t, "GOLD"))
        return BYL;
    if (!strcmp(t, "SILVER"))
        return BWH;
    return YL;
}
const char *Utils::statusColor(const char *s)
{
    if (!strcmp(s, "ON_TIME"))
        return BGR;
    if (!strcmp(s, "DELAYED"))
        return BYL;
    if (!strcmp(s, "CANCELLED"))
        return BRD;
    if (!strcmp(s, "BOARDING"))
        return BMG;
    if (!strcmp(s, "DEPARTED"))
        return BCY;
    if (!strcmp(s, "LANDED"))
        return GR;
    return BWH;
}
int Utils::ansiLen(const char *s)
{
    int n = 0;
    while (*s)
    {
        if ((unsigned char)*s == 0x1b && *(s + 1) == '[')
        {
            s += 2;
            while (*s && *s != 'm')
                s++;
            if (*s)
                s++;
        }
        else
        {
            unsigned char c = (unsigned char)*s;
            if (c < 0x80 || c >= 0xC0)
                n++;
            s++;
        }
    }
    return n;
}

/*──────────────────────────────────────────────────────────────────────────────
  SoundEngine implementation
──────────────────────────────────────────────────────────────────────────────*/
void SoundEngine::boot()
{
    int m[] = {400, 500, 600, 700, 900, 1100};
    for (int i = 0; i < 6; i++)
    {
        Beep(m[i], 80);
        Sleep(20);
    }
}
void SoundEngine::ok()
{
    Beep(800, 70);
    Sleep(25);
    Beep(1050, 70);
    Sleep(25);
    Beep(1300, 160);
}
void SoundEngine::error()
{
    Beep(300, 180);
    Sleep(50);
    Beep(230, 180);
    Sleep(50);
    Beep(190, 300);
}
void SoundEngine::warn()
{
    Beep(700, 130);
    Sleep(70);
    Beep(580, 130);
}
void SoundEngine::login()
{
    Beep(880, 65);
    Sleep(25);
    Beep(1100, 65);
    Sleep(25);
    Beep(1320, 140);
}
void SoundEngine::select()
{
    Beep(950, 38);
    Sleep(12);
    Beep(1060, 38);
}
void SoundEngine::pay()
{
    int m[] = {523, 659, 784, 1047, 1319};
    for (int i = 0; i < 5; i++)
    {
        Beep(m[i], 85);
        Sleep(28);
    }
}
void SoundEngine::think()
{
    Beep(600, 45);
    Sleep(90);
    Beep(650, 45);
    Sleep(90);
    Beep(600, 45);
    Sleep(90);
    Beep(700, 45);
}
void SoundEngine::scan()
{
    Beep(1200, 55);
    Sleep(18);
    Beep(1450, 55);
}
void SoundEngine::fly()
{
    for (int f = 350; f <= 1150; f += 55)
        Beep(f, 18);
}
void SoundEngine::alert()
{
    Beep(1000, 100);
    Beep(800, 100);
    Sleep(60);
    Beep(1000, 100);
}
void SoundEngine::exitSound()
{
    int m[] = {1047, 988, 880, 784, 698, 659, 587, 523};
    for (int i = 0; i < 8; i++)
    {
        Beep(m[i], 95);
        Sleep(35);
    }
}

/*──────────────────────────────────────────────────────────────────────────────
  Terminal implementation
──────────────────────────────────────────────────────────────────────────────*/
void Terminal::boxTop(const char *c)
{
    printf("%s" BT, c);
    for (int i = 0; i < TW; i++)
        printf(BH);
    printf(BR RS "\n");
}
void Terminal::boxBot(const char *c)
{
    printf("%s" BB, c);
    for (int i = 0; i < TW; i++)
        printf(BH);
    printf(BL2 RS "\n");
}
void Terminal::boxMid(const char *c)
{
    printf("%s" BMT, c);
    for (int i = 0; i < TW; i++)
        printf(BH);
    printf(BMB RS "\n");
}
void Terminal::boxSep(const char *c)
{
    printf("%s" BST, c);
    for (int i = 0; i < TW; i++)
        printf(SH);
    printf(BSB RS "\n");
}
void Terminal::boxEmpty(const char *c)
{
    printf("%s" BV RS, c);
    Utils::spaces(TW);
    printf("%s" BV RS "\n", c);
}
static void _row(const char *bc, const char *t, int pw)
{
    int p = TW - pw;
    if (p < 0)
        p = 0;
    printf("%s" BV RS "%s", bc, t);
    Utils::spaces(p);
    printf("%s" BV RS "\n", bc);
}
void Terminal::boxRow(const char *bc, const char *t) { _row(bc, t, (int)strlen(t)); }
void Terminal::boxRowC(const char *bc, const char *t) { _row(bc, t, Utils::ansiLen(t)); }
void Terminal::boxRowF(const char *bc, const char *fmt, ...)
{
    char buf[600];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, 600, fmt, ap);
    va_end(ap);
    boxRow(bc, buf);
}
void Terminal::boxCenter(const char *bc, const char *t)
{
    int tw = Utils::ansiLen(t), lp = (TW - tw) / 2;
    if (lp < 0)
        lp = 0;
    int rp = TW - tw - lp;
    if (rp < 0)
        rp = 0;
    printf("%s" BV RS, bc);
    Utils::spaces(lp);
    printf("%s", t);
    Utils::spaces(rp);
    printf("%s" BV RS "\n", bc);
}
void Terminal::boxLR(const char *bc, const char *l, const char *r)
{
    int lw = Utils::ansiLen(l), rw = Utils::ansiLen(r), g = TW - lw - rw;
    if (g < 1)
        g = 1;
    printf("%s" BV RS "%s", bc, l);
    Utils::spaces(g);
    printf("%s", r);
    printf("%s" BV RS "\n", bc);
}

void Terminal::typeWrite(const char *t, int d)
{
    for (int i = 0; t[i]; i++)
    {
        printf("%c", t[i]);
        fflush(stdout);
        Utils::sleepMs(d);
    }
}
void Terminal::progressBar(const char *lbl, int ms)
{
    int bw = TW - 14;
    printf("\n");
    boxTop(BYL);
    {
        char b[200];
        snprintf(b, 200, "  ⏳  " YL "%s" RS, lbl);
        boxRowC(BYL, b);
    }
    boxMid(BYL);
    printf(BYL BV RS "    [" BGR);
    for (int i = 0; i < bw; i++)
        printf(" ");
    printf(RS "]    " BYL BV RS);
    printf("\r" BYL BV RS "    [" BGR);
    fflush(stdout);
    for (int i = 0; i < bw; i++)
    {
        printf("█");
        fflush(stdout);
        SoundEngine::tick();
        Utils::sleepMs(ms / bw);
    }
    printf(RS "]  " BGR "✔" RS " Done");
    Utils::spaces(TW - (4 + 1 + bw + 1 + 9));
    printf(BYL BV RS "\n");
    boxBot(BYL);
    fflush(stdout);
}
void Terminal::spinner(const char *msg, int ms)
{
    const char *fr[] = {"◐", "◓", "◑", "◒"};
    int steps = ms / 80;
    for (int i = 0; i < steps; i++)
    {
        printf("\r  " BCY "%s" RS "  " YL "%s" RS "   ", fr[i % 4], msg);
        fflush(stdout);
        Utils::sleepMs(80);
    }
    printf("\r  " BGR "✔" RS "  " GR "%s — Done!" RS "   \n", msg);
    SoundEngine::ok();
}
void Terminal::neuralPulse(int n)
{
    const char *d[] = {"●○○○○", "○●○○○", "○○●○○", "○○○●○", "○○○○●"};
    int lp = (TW + 2 - 10) / 2;
    if (lp < 0)
        lp = 0;
    for (int i = 0; i < n; i++)
    {
        printf("\r");
        Utils::spaces(lp);
        printf(BCY "%s" RS " ", d[i % 5]);
        fflush(stdout);
        Utils::sleepMs(110);
    }
    printf("\r");
    Utils::spaces(lp + 12);
    printf("\r");
    fflush(stdout);
}
void Terminal::glitch(const char *text)
{
    const char *gc = "@#$%&*!?~^";
    int len = (int)strlen(text), gl = (int)strlen(gc);
    int lp = (TW + 2 - len) / 2;
    if (lp < 0)
        lp = 0;
    for (int p = 0; p < 3; p++)
    {
        printf("\r");
        Utils::spaces(lp);
        printf(BCY);
        for (int i = 0; i < len; i++)
            printf("%c", text[i] == ' ' ? ' ' : gc[rand() % gl]);
        printf(RS);
        fflush(stdout);
        Utils::sleepMs(80);
    }
    printf("\r");
    Utils::spaces(lp);
    printf(BRD BOLD "%s" RS "\n", text);
    fflush(stdout);
}
void Terminal::dataStream(int lines)
{
    const char *s[] = {"0xNX01  >>  LOADING NEXUS AIR FLIGHT DATABASE",
                       "0xNX02  >>  MAPPING PASSENGER RECORDS",
                       "0xNX03  >>  SYNCING SEAT INVENTORY",
                       "0xNX04  >>  CALIBRATING BAGGAGE TRACKING",
                       "0xNX05  >>  INITIALISING PAYMENT ENGINE",
                       "0xNX06  >>  PRIMING AUTHENTICATION LAYER"};
    int lp = (TW + 2 - 46) / 2;
    if (lp < 0)
        lp = 0;
    for (int i = 0; i < lines; i++)
    {
        Utils::spaces(lp);
        printf(DIM "  %s" RS "\n", s[i % 6]);
        fflush(stdout);
        Utils::sleepMs(85);
    }
}
void Terminal::flyingPlane()
{
    printf("\n");
    for (int p = 0; p < TW + 2; p++)
    {
        printf("\r");
        Utils::spaces(p);
        printf(BYL "✈" RS);
        fflush(stdout);
        Utils::sleepMs(11);
    }
    printf("\n");
    SoundEngine::fly();
}
void Terminal::starBurst(int lines)
{
    const char *stars[] = {"✦", "★", "◆", "⬟", "◉", "✸", "✺", "❋"};
    const char *cols[] = {BYL, BGR, BCY, BMG, BWH, BRD, BYL, BCY};
    for (int w = 0; w < lines; w++)
    {
        printf("\r");
        for (int c = 0; c < TW + 2; c++)
        {
            if (rand() % 5 == 0)
            {
                int s = rand() % 8;
                printf("%s%s" RS, cols[s], stars[s]);
            }
            else
                printf(" ");
        }
        printf("\n");
        fflush(stdout);
        Utils::sleepMs(155);
    }
}
void Terminal::success(const char *m)
{
    printf("\n");
    boxTop(BGR);
    {
        char b[256];
        snprintf(b, 256, "  ✅  " BGR "SUCCESS:" RS "  " BWH "%s" RS, m);
        boxRowC(BGR, b);
    }
    boxBot(BGR);
    printf("\n");
    SoundEngine::ok();
}
void Terminal::errMsg(const char *m)
{
    printf("\n");
    boxTop(BRD);
    {
        char b[256];
        snprintf(b, 256, "  ❌  " BRD "ERROR:" RS "  " BWH "%s" RS, m);
        boxRowC(BRD, b);
    }
    boxBot(BRD);
    printf("\n");
    SoundEngine::error();
}
void Terminal::infoMsg(const char *m)
{
    printf("\n");
    boxTop(BCY);
    {
        char b[256];
        snprintf(b, 256, "  ℹ   " BCY "INFO:" RS "  " BWH "%s" RS, m);
        boxRowC(BCY, b);
    }
    boxBot(BCY);
    printf("\n");
}
void Terminal::warnMsg(const char *m)
{
    printf("\n");
    boxTop(BYL);
    {
        char b[256];
        snprintf(b, 256, "  ⚠   " BYL "WARNING:" RS "  " BWH "%s" RS, m);
        boxRowC(BYL, b);
    }
    boxBot(BYL);
    printf("\n");
    SoundEngine::warn();
}
void Terminal::waitEnter()
{
    printf("\n");
    printf(DIM "╔");
    for (int i = 0; i < TW; i++)
        printf("─");
    printf("╗" RS "\n");
    printf(DIM "║  Press ENTER to continue...                                                  ║" RS "\n");
    printf(DIM "╚");
    for (int i = 0; i < TW; i++)
        printf("─");
    printf("╝" RS "\n");
    fflush(stdout);
    SoundEngine::click();
    getchar();
}
void Terminal::sectionHeader(const char *title, const char *col)
{
    printf("\n");
    boxTop(col);
    char b[256];
    snprintf(b, 256, "%s✈  %s  ✈" RS, col, title);
    boxCenter(col, b);
    boxBot(col);
    printf("\n");
}
void Terminal::fieldSep()
{
    printf("  " DIM);
    for (int i = 0; i < TW - 2; i++)
        printf("─");
    printf(RS "\n");
}

/*──────────────────────────────────────────────────────────────────────────────
  Input implementation
──────────────────────────────────────────────────────────────────────────────*/
void Input::readPW(char *buf, int mx)
{
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD m = 0;
    GetConsoleMode(h, &m);
    SetConsoleMode(h, m & ~ENABLE_ECHO_INPUT);
    printf(DIM);
    fgets(buf, mx, stdin);
    buf[strcspn(buf, "\n")] = 0;
    SetConsoleMode(h, m);
    printf(RS "\n");
}
string Input::field(const char *lbl, const char *hint)
{
    printf("  " BYL " ▶ " RS BOLD BWH "%-24s" RS " " BCY, lbl);
    if (*hint)
        printf(DIM "(%s) " RS, hint);
    fflush(stdout);
    char buf[256];
    fgets(buf, 256, stdin);
    buf[strcspn(buf, "\n")] = 0;
    int s = 0;
    while (buf[s] == ' ')
        s++;
    printf(RS);
    SoundEngine::tick();
    return string(buf + s);
}
int Input::integer(const char *lbl, int lo, int hi)
{
    for (;;)
    {
        printf("  " BYL " ▶ " RS BOLD BWH "%-24s" RS " " BCY, lbl);
        fflush(stdout);
        int v;
        if (scanf("%d", &v) != 1)
        {
            Utils::clearInput();
            continue;
        }
        Utils::clearInput();
        printf(RS);
        SoundEngine::tick();
        if (v >= lo && v <= hi)
            return v;
        printf("  " BRD " ✘ Enter %d-%d\n" RS, lo, hi);
        SoundEngine::error();
    }
}
double Input::dbl(const char *lbl)
{
    printf("  " BYL " ▶ " RS BOLD BWH "%-24s" RS " " BCY, lbl);
    fflush(stdout);
    double v;
    if (scanf("%lf", &v) != 1)
        v = 0;
    Utils::clearInput();
    printf(RS);
    SoundEngine::tick();
    return v;
}
string Input::phone(const char *lbl)
{
    for (;;)
    {
        string s = field(lbl, "+Code-Number");
        bool ok = true;
        int dc = 0;
        for (char c : s)
        {
            if (c == '+' || c == '-' || c == ' ')
                continue;
            if (!isdigit(c))
            {
                ok = false;
                break;
            }
            dc++;
        }
        if (ok && dc >= 7 && dc <= 15)
            return s;
        printf("  " BRD " ✘ Invalid phone (7-15 digits)\n" RS);
        SoundEngine::error();
    }
}
string Input::email(const char *lbl)
{
    for (;;)
    {
        string s = field(lbl, "user@domain.com");
        if (s.find('@') != string::npos && s.find('.') != string::npos && s.size() > 5)
            return s;
        printf("  " BRD " ✘ Invalid email format.\n" RS);
        SoundEngine::error();
    }
}
string Input::date(const char *lbl)
{
    for (;;)
    {
        string s = field(lbl, "DD-MMM-YYYY e.g. 15-MAR-1990");
        if (s.size() >= 9)
            return s;
        printf("  " BRD " ✘ Use format: DD-MMM-YYYY\n" RS);
        SoundEngine::error();
    }
}
string Input::password(const char *lbl, bool chk)
{
    for (;;)
    {
        printf("  " BYL " ▶ " RS BOLD BWH "%-24s" RS " " DIM, lbl);
        fflush(stdout);
        char buf[64];
        readPW(buf, 64);
        printf(RS);
        if ((int)strlen(buf) < 4)
        {
            printf("  " BRD " ✘ Min 4 characters.\n" RS);
            SoundEngine::error();
            continue;
        }
        if (chk && (int)strlen(buf) < 8)
        {
            printf("  " BYL " ⚠ Weak password. Continue? [Y/N]: " RS " " BCY);
            char c;
            scanf(" %c", &c);
            Utils::clearInput();
            printf(RS);
            if (tolower(c) != 'y')
                continue;
        }
        return string(buf);
    }
}
string Input::passport(const char *lbl)
{
    for (;;)
    {
        string s = field(lbl, "e.g. AB1234567");
        if (s.size() >= 6 && s.size() <= 12)
        {
            bool ok = true;
            for (char c : s)
                if (!isalnum(c))
                {
                    ok = false;
                    break;
                }
            if (ok)
                return s;
        }
        printf("  " BRD " ✘ Passport: 6-12 alphanumeric.\n" RS);
        SoundEngine::error();
    }
}
string Input::card16()
{
    for (;;)
    {
        printf("  " BYL " ▶ " RS BOLD BWH "Card Number (16 digits):   " RS " " BCY);
        fflush(stdout);
        char buf[32];
        fgets(buf, 32, stdin);
        buf[strcspn(buf, "\n")] = 0;
        string raw = "";
        for (char c : string(buf))
            if (isdigit(c))
                raw += c;
        if (raw.size() == 16)
        {
            printf(RS);
            SoundEngine::tick();
            return raw;
        }
        printf("  " BRD " ✘ Exactly 16 digits required.\n" RS);
        SoundEngine::error();
    }
}
string Input::cvv()
{
    for (;;)
    {
        printf("  " BYL " ▶ " RS BOLD BWH "CVV:                       " RS " " DIM);
        fflush(stdout);
        char buf[8];
        readPW(buf, 8);
        printf(RS);
        string s(buf);
        if (s.size() >= 3 && s.size() <= 4)
        {
            bool ok = true;
            for (char c : s)
                if (!isdigit(c))
                {
                    ok = false;
                    break;
                }
            if (ok)
                return s;
        }
        printf("  " BRD " ✘ CVV: 3-4 digits.\n" RS);
        SoundEngine::error();
    }
}
string Input::expiry()
{
    for (;;)
    {
        string s = field("Expiry (MM/YY):", "e.g. 09/27");
        if (s.size() == 5 && s[2] == '/')
        {
            bool ok = true;
            for (int i = 0; i < 5; i++)
                if (i != 2 && !isdigit(s[i]))
                {
                    ok = false;
                    break;
                }
            if (ok)
                return s;
        }
        printf("  " BRD " ✘ Use MM/YY format.\n" RS);
        SoundEngine::error();
    }
}
/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  NEXUS AIR v5.0 — OOP Implementation  (nexus_impl2.cpp)                     ║
 ║  Part 2: SeatMap, Flight, User hierarchy, Booking, BaggageItem               ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

/* ── FIX-2: ONE-TIME static member definitions ─────────────────────────── */
int Flight::nextId_ = 1001;
int Passenger::nextId_ = 2001;
int Staff::nextId_ = 3001;
int Admin::nextId_ = 4001;
int BaggageItem::nextId_ = 6001;
int Booking::nextId_ = 5001;

/*──────────────────────────────────────────────────────────────────────────────
  SeatMap — CONCEPT [1][2] Class & Encapsulation, [15] Constructor overloading
──────────────────────────────────────────────────────────────────────────────*/
SeatMap::SeatMap() : flightNo_("")
{
    memset(st_, 0, sizeof(st_));
}
SeatMap::SeatMap(const string &flightNo) : flightNo_(flightNo)
{
    memset(st_, 0, sizeof(st_));
}
SeatMap::SeatMap(const SeatMap &o) : flightNo_(o.flightNo_)
{
    memcpy(st_, o.st_, sizeof(st_));
    for (int r = 0; r < TOTAL_ROWS; r++)
        for (int c = 0; c < 6; c++)
            who_[r][c] = o.who_[r][c];
}

int SeatMap::getStatus(int r, int c) const
{
    if (r < 0 || r >= TOTAL_ROWS || c < 0 || c >= 6)
        return -1;
    return st_[r][c];
}
const string &SeatMap::getWho(int r, int c) const
{
    static string empty;
    if (r < 0 || r >= TOTAL_ROWS || c < 0 || c >= 6)
        return empty;
    return who_[r][c];
}
void SeatMap::book(int r, int c, const string &name)
{
    if (r < 0 || r >= TOTAL_ROWS || c < 0 || c >= 6)
        return;
    st_[r][c] = 1;
    who_[r][c] = name;
}
void SeatMap::free(int r, int c)
{
    if (r < 0 || r >= TOTAL_ROWS || c < 0 || c >= 6)
        return;
    st_[r][c] = 0;
    who_[r][c] = "";
}
void SeatMap::block(int r, int c)
{
    if (r < 0 || r >= TOTAL_ROWS || c < 0 || c >= 6)
        return;
    st_[r][c] = 2;
}

const char *SeatMap::cabinForRow(int r)
{
    if (r < ROWS_FIRST)
        return "FIRST";
    if (r < ROWS_FIRST + ROWS_BIZ)
        return "BUSINESS";
    if (r < ROWS_FIRST + ROWS_BIZ + ROWS_PREM)
        return "PREMIUM_ECO";
    return "ECONOMY";
}
int SeatMap::colsForRow(int r)
{
    return (r < ROWS_FIRST + ROWS_BIZ) ? 4 : 6;
}
char SeatMap::colLetter(int c, int cols)
{
    if (cols == 4)
    {
        const char *l = "ABCD";
        return l[c];
    }
    const char *l = "ABCDEF";
    return l[c];
}

void SeatMap::display(double fF, double fB, double fP, double fE,
                      const string &ac, const string &dt,
                      const string &org, const string &dst) const
{
    Presentation::printBanner();
    printf("\n");
    Terminal::boxTop(BCY);
    {
        char b[200];
        snprintf(b, 200, "  ✈  " BOLD BCY "NEXUS AIR — INTERACTIVE SEAT MAP" RS "  Flight " BYL "%s" RS, flightNo_.c_str());
        Terminal::boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Aircraft: " RS BWH "%s" RS "   Date: " BWH "%s" RS "   Route: " BWH "%s → %s" RS, ac.c_str(), dt.c_str(), org.c_str(), dst.c_str());
        Terminal::boxRowC(BCY, b);
    }
    Terminal::boxSep(BCY);
    Terminal::boxRowC(BCY, "  Legend:  " BGR "[◉]" RS " Available   " BRD "[X]" RS " Taken   " DIM "[▪]" RS " Blocked");
    Terminal::boxSep(BCY);
    {
        char b[300];
        snprintf(b, 300, "  " BMG "FIRST $%.0f" RS "  " BYL "BUSINESS $%.0f" RS "  " BCY "PREM-ECO $%.0f" RS "  " BGR "ECONOMY $%.0f" RS, fF, fB, fP, fE);
        Terminal::boxRowC(BCY, b);
    }
    Terminal::boxSep(BCY);
    Terminal::boxRowC(BCY, "  " DIM "Class             Row  |  A    B    C  |  D    E    F  | Fare/seat" RS);
    Terminal::boxSep(BCY);

    const char *lastCab = "";
    int dispRow = 10;
    double fares[] = {fF, fB, fP, fE};
    const char *cabNames[] = {"FIRST", "BUSINESS", "PREMIUM_ECO", "ECONOMY"};
    const char *cabCols[] = {BMG, BYL, BCY, BGR};
    const char *cabLabels[] = {"  ✦ FIRST CLASS", "  ◆ BUSINESS CLASS", "  ▲ PREMIUM ECONOMY", "  ● ECONOMY CLASS"};

    for (int r = 0; r < TOTAL_ROWS; r++)
    {
        int cols = colsForRow(r);
        const char *cab = cabinForRow(r);
        int cabIdx = 0;
        for (int ci = 0; ci < 4; ci++)
            if (!strcmp(cab, cabNames[ci]))
            {
                cabIdx = ci;
                break;
            }
        double fare = fares[cabIdx];
        const char *cabCol = cabCols[cabIdx];

        if (strcmp(cab, lastCab))
        {
            if (*lastCab)
                Terminal::boxSep(BCY);
            char b2[120];
            snprintf(b2, 120, "%s%s" RS "  " DIM "Base fare: " RS BWH "$%.0f" RS, cabCol, cabLabels[cabIdx], fare);
            Terminal::boxRowC(BCY, b2);
            Terminal::boxSep(BCY);
            lastCab = cab;
        }
        char line[512] = "";
        char tmp[64];
        snprintf(tmp, 64, "  %s%-17s" RS "  " DIM "%2d" RS "  |  ", cabCol, cab, dispRow);
        strcat(line, tmp);
        for (int c = 0; c < 6; c++)
        {
            if (c == 3)
                strcat(line, " |  ");
            if (c >= cols)
            {
                strcat(line, "     ");
                continue;
            }
            int s = st_[r][c];
            const char *sym = (s == 0) ? (BGR "[◉]" RS) : (s == 1) ? (BRD "[X]" RS)
                                                                   : (DIM "[▪]" RS);
            strncat(line, sym, sizeof(line) - strlen(line) - 1);
            strncat(line, "  ", sizeof(line) - strlen(line) - 1);
        }
        char fs[24];
        snprintf(fs, 24, " | $%.0f", fare);
        strncat(line, fs, sizeof(line) - strlen(line) - 1);
        Terminal::boxRowC(BCY, line);
        dispRow++;
    }
    Terminal::boxBot(BCY);
}

string SeatMap::serialize() const
{
    string out = flightNo_ + "\n";
    for (int r = 0; r < TOTAL_ROWS; r++)
    {
        for (int c = 0; c < 6; c++)
            out += to_string(st_[r][c]) + "|" + who_[r][c] + "|";
        out += "\n";
    }
    return out;
}
bool SeatMap::deserialize(const string &block)
{
    /* block is multi-line — first line is flightNo, then 30 seat rows */
    istringstream ss(block);
    getline(ss, flightNo_);
    for (int r = 0; r < TOTAL_ROWS; r++)
    {
        string line;
        if (!getline(ss, line))
            break;
        char *p = &line[0];
        for (int c = 0; c < 6; c++)
        {
            st_[r][c] = atoi(p);
            p = strchr(p, '|');
            if (!p)
                break;
            p++;
            char *end = strchr(p, '|');
            if (end)
            {
                who_[r][c] = string(p, end - p);
                p = end + 1;
            }
        }
    }
    return true;
}

/*──────────────────────────────────────────────────────────────────────────────
  Flight — CONCEPT [1][2][4][5][6][9][11]
──────────────────────────────────────────────────────────────────────────────*/
Flight::Flight() : id_(nextId_++), gate_(1), termDep_(1), termArr_(1),
                   duration_(120), seatsFirst_(ROWS_FIRST * 4), bkFirst_(0),
                   seatsBiz_(ROWS_BIZ * 4), bkBiz_(0), seatsPrem_(ROWS_PREM * 6),
                   bkPrem_(0), seatsEco_(ROWS_ECO * 6), bkEco_(0),
                   fareFirst_(2500), fareBiz_(1200), farePrem_(700), fareEco_(300),
                   taxRate_(16.72), delayReason_("N/A"), active_(true)
{
    status_ = "ON_TIME";
}
Flight::Flight(const string &no, const string &airline,
               const string &org, const string &dst,
               const string &date, const string &dep, const string &arr,
               const string &aircraft, int gate, int tDep, int tArr, int dur,
               double fF, double fB, double fP, double fE)
    : id_(nextId_++), flightNo_(no), airline_(airline), origin_(org), dest_(dst),
      date_(date), depTime_(dep), arrTime_(arr), aircraft_(aircraft),
      status_("ON_TIME"), gate_(gate), termDep_(tDep), termArr_(tArr), duration_(dur),
      seatsFirst_(ROWS_FIRST * 4), bkFirst_(0), seatsBiz_(ROWS_BIZ * 4), bkBiz_(0),
      seatsPrem_(ROWS_PREM * 6), bkPrem_(0), seatsEco_(ROWS_ECO * 6), bkEco_(0),
      fareFirst_(fF), fareBiz_(fB), farePrem_(fP), fareEco_(fE), taxRate_(16.72),
      delayReason_("N/A"), active_(true), seatMap_(no) {}

double Flight::fareForCabin(const string &cabin) const
{
    if (cabin == "FIRST")
        return fareFirst_;
    if (cabin == "BUSINESS")
        return fareBiz_;
    if (cabin == "PREMIUM_ECO")
        return farePrem_;
    return fareEco_;
}
void Flight::bookSeat(const string &cabin)
{
    if (cabin == "FIRST")
        bkFirst_++;
    else if (cabin == "BUSINESS")
        bkBiz_++;
    else if (cabin == "PREMIUM_ECO")
        bkPrem_++;
    else
        bkEco_++;
}
void Flight::unBookSeat(const string &cabin)
{
    if (cabin == "FIRST" && bkFirst_ > 0)
        bkFirst_--;
    else if (cabin == "BUSINESS" && bkBiz_ > 0)
        bkBiz_--;
    else if (cabin == "PREMIUM_ECO" && bkPrem_ > 0)
        bkPrem_--;
    else if (bkEco_ > 0)
        bkEco_--;
}
string Flight::serialize() const
{
    char buf[600];
    snprintf(buf, 600, "%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%.2f|%.2f|%.2f|%.2f|%.2f|%s|%d",
             id_, flightNo_.c_str(), airline_.c_str(), origin_.c_str(), dest_.c_str(),
             date_.c_str(), depTime_.c_str(), arrTime_.c_str(), aircraft_.c_str(), status_.c_str(),
             gate_, termDep_, termArr_, duration_,
             seatsFirst_, bkFirst_, seatsBiz_, bkBiz_, seatsPrem_, bkPrem_, seatsEco_, bkEco_,
             fareFirst_, fareBiz_, farePrem_, fareEco_, taxRate_,
             delayReason_.c_str(), (int)active_);
    return string(buf);
}
bool Flight::deserialize(const string &line)
{
    char fno[16], air[48], org[48], dst[48], dt[16], dep[8], arr[8], ac[40], st[20], dr[80];
    int g, td, ta, dur, sf, bf, sb, bb, sp, bp, se, be, act;
    double ff, fb, fp, fe, tx;
    sscanf(line.c_str(),
           "%d|%15[^|]|%47[^|]|%47[^|]|%47[^|]|%15[^|]|%7[^|]|%7[^|]|%39[^|]|%19[^|]"
           "|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d"
           "|%lf|%lf|%lf|%lf|%lf|%79[^|]|%d",
           &id_, fno, air, org, dst, dt, dep, arr, ac, st,
           &g, &td, &ta, &dur, &sf, &bf, &sb, &bb, &sp, &bp, &se, &be,
           &ff, &fb, &fp, &fe, &tx, dr, &act);
    flightNo_ = fno;
    airline_ = air;
    origin_ = org;
    dest_ = dst;
    date_ = dt;
    depTime_ = dep;
    arrTime_ = arr;
    aircraft_ = ac;
    status_ = st;
    delayReason_ = dr;
    gate_ = g;
    termDep_ = td;
    termArr_ = ta;
    duration_ = dur;
    seatsFirst_ = sf;
    bkFirst_ = bf;
    seatsBiz_ = sb;
    bkBiz_ = bb;
    seatsPrem_ = sp;
    bkPrem_ = bp;
    seatsEco_ = se;
    bkEco_ = be;
    fareFirst_ = ff;
    fareBiz_ = fb;
    farePrem_ = fp;
    fareEco_ = fe;
    taxRate_ = tx;
    active_ = (act != 0);
    seatMap_ = SeatMap(flightNo_);
    if (id_ >= nextId_)
        nextId_ = id_ + 1;
    return true;
}
void Flight::display() const { displayCard(0); }
void Flight::displayCard(int idx) const
{
    const char *sc = Utils::statusColor(status_.c_str());
    printf("\n  " BCY "[%d]" RS "  " BOLD BWH "✈ %s" RS "  │  " CY "%s" RS "\n",
           idx + 1, flightNo_.c_str(), airline_.c_str());
    printf("      " YL "%s" RS "  →  " BYL "%s" RS "\n", origin_.c_str(), dest_.c_str());
    printf("      " DIM "Date: " RS BWH "%s  Dep:%s  Arr:%s  Duration:%dh%02dm" RS "\n",
           date_.c_str(), depTime_.c_str(), arrTime_.c_str(), duration_ / 60, duration_ % 60);
    printf("      " DIM "Status: " RS "%s" BOLD "%s" RS "   Gate: " BWH "%d" RS "   Aircraft: " BWH "%s" RS "\n",
           sc, status_.c_str(), gate_, aircraft_.c_str());
    printf("      " BMG "First:$%.0f" RS "  " BYL "Biz:$%.0f" RS "  " BCY "PrmEco:$%.0f" RS "  " BGR "Eco:$%.0f" RS "   Load: %d%%\n",
           fareFirst_, fareBiz_, farePrem_, fareEco_, loadPct());
    printf("      " DIM "──────────────────────────────────────────────────────────────" RS "\n");
}

/*──────────────────────────────────────────────────────────────────────────────
  User base class — CONCEPT [4][16]
──────────────────────────────────────────────────────────────────────────────*/
User::User() : id_(0), active_(true) { created_ = Utils::getTimestamp(); }
User::User(const string &un, const string &fn, const string &ln, const string &em)
    : id_(0), username_(un), firstName_(fn), lastName_(ln), email_(em), active_(true)
{
    created_ = Utils::getTimestamp();
}
User::User(const User &o) : id_(o.id_), username_(o.username_), passHash_(o.passHash_),
                            firstName_(o.firstName_), lastName_(o.lastName_), email_(o.email_),
                            created_(o.created_), active_(o.active_) {}

bool User::authenticate(const string &pw) const
{
    return Utils::hashPassword(pw) == passHash_;
}
void User::setPassword(const string &pw)
{
    passHash_ = Utils::hashPassword(pw);
}
void User::display() const { displayCard(0); }
void User::displayCard(int i) const
{
    printf("  " BCY "[%d] " RS BOLD BWH "%s" RS "  " DIM "(%s)" RS "\n",
           i + 1, getFullName().c_str(), getRoleTag().c_str());
}

/*──────────────────────────────────────────────────────────────────────────────
  Passenger — CONCEPT [4][5][6][12]
──────────────────────────────────────────────────────────────────────────────*/
Passenger::Passenger() : User(), loyaltyPts_(500), totalMiles_(0), loyaltyTier_("BRONZE")
{
    id_ = nextId_++;
}
Passenger::Passenger(const string &un, const string &fn, const string &ln,
                     const string &em, const string &ph, const string &pp,
                     const string &nat, const string &dob,
                     const string &addr, const string &city, const string &country)
    : User(un, fn, ln, em), phone_(ph), passport_(pp), nationality_(nat), dob_(dob),
      address_(addr), city_(city), country_(country),
      loyaltyPts_(500), totalMiles_(0), loyaltyTier_("BRONZE")
{
    id_ = nextId_++;
}
Passenger::Passenger(const Passenger &o) : User(o),
                                           phone_(o.phone_), passport_(o.passport_), nationality_(o.nationality_),
                                           dob_(o.dob_), address_(o.address_), city_(o.city_), country_(o.country_),
                                           loyaltyPts_(o.loyaltyPts_), totalMiles_(o.totalMiles_), loyaltyTier_(o.loyaltyTier_) {}

void Passenger::addMiles(int m)
{
    totalMiles_ += m;
    loyaltyPts_ += m / 10;
    updateTier();
}
void Passenger::updateTier() { loyaltyTier_ = Utils::computeTier(totalMiles_); }
double Passenger::getLoyaltyDiscount(double subtotal) const
{
    if (loyaltyTier_ == "PLATINUM")
        return subtotal * 0.10;
    if (loyaltyTier_ == "GOLD")
        return subtotal * 0.05;
    return 0.0;
}
void Passenger::display() const
{
    printf("  " BWH "P%-5d" WH "%-20.19s " DIM "%-25.24s " RS "%s%-10s" RS WH "%-6d" RS "\n",
           id_, getFullName().c_str(), email_.c_str(),
           Utils::tierColor(loyaltyTier_.c_str()), loyaltyTier_.c_str(), totalMiles_);
}
void Passenger::displayCard(int i) const { display(); }
string Passenger::serialize() const
{
    char buf[600];
    snprintf(buf, 600, "%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%d|%d|%s|%s|%d",
             id_, username_.c_str(), passHash_.c_str(), firstName_.c_str(), lastName_.c_str(),
             email_.c_str(), phone_.c_str(), passport_.c_str(), nationality_.c_str(), dob_.c_str(),
             address_.c_str(), city_.c_str(), country_.c_str(),
             loyaltyPts_, totalMiles_, loyaltyTier_.c_str(), created_.c_str(), (int)active_);
    return string(buf);
}
bool Passenger::deserialize(const string &line)
{
    char un[64], ph2[24], fn[64], ln[64], em[64], ph[64], pp[64], nat[64], dob[16];
    char addr[80], city[40], cntry[40], tier[12], crt[20];
    int pts, miles, act;
    sscanf(line.c_str(),
           "%d|%63[^|]|%23[^|]|%63[^|]|%63[^|]|%63[^|]|%63[^|]|%63[^|]|%63[^|]|%15[^|]"
           "|%79[^|]|%39[^|]|%39[^|]|%d|%d|%11[^|]|%19[^|]|%d",
           &id_, un, ph2, fn, ln, em, ph, pp, nat, dob, addr, city, cntry, &pts, &miles, tier, crt, &act);
    username_ = un;
    passHash_ = ph2;
    firstName_ = fn;
    lastName_ = ln;
    email_ = em;
    phone_ = ph;
    passport_ = pp;
    nationality_ = nat;
    dob_ = dob;
    address_ = addr;
    city_ = city;
    country_ = cntry;
    loyaltyPts_ = pts;
    totalMiles_ = miles;
    loyaltyTier_ = tier;
    created_ = crt;
    active_ = (act != 0);
    if (id_ >= nextId_)
        nextId_ = id_ + 1;
    return true;
}

/*──────────────────────────────────────────────────────────────────────────────
  Staff — CONCEPT [4][5]
──────────────────────────────────────────────────────────────────────────────*/
Staff::Staff() : User(), roleCode_(SR_GATE), shiftStart_("08:00"), shiftEnd_("16:00")
{
    id_ = nextId_++;
    char tmp[16];
    snprintf(tmp, 16, "NX-EMP-%04d", id_);
    empId_ = tmp;
}
Staff::Staff(const string &un, const string &fn, const string &ln,
             const string &em, const string &ph, StaffRole role,
             const string &terminal, const string &gates)
    : User(un, fn, ln, em), phone_(ph), roleCode_(role),
      terminal_(terminal), gates_(gates),
      shiftStart_("08:00"), shiftEnd_("16:00")
{
    id_ = nextId_++;
    char tmp[16];
    snprintf(tmp, 16, "NX-EMP-%04d", id_);
    empId_ = tmp;
}
Staff::Staff(const Staff &o) : User(o), empId_(o.empId_), phone_(o.phone_),
                               roleCode_(o.roleCode_), terminal_(o.terminal_), gates_(o.gates_),
                               shiftStart_(o.shiftStart_), shiftEnd_(o.shiftEnd_) {}

void Staff::display() const
{
    printf("  " BCY "%-10s" RS WH "%-20s %s%-20s" RS WH "%-12s" RS "\n",
           empId_.c_str(), getFullName().c_str(),
           SROLE_COL[roleCode_], SROLES[roleCode_], terminal_.c_str());
}
void Staff::displayCard(int i) const { display(); }
string Staff::serialize() const
{
    char buf[400];
    snprintf(buf, 400, "%d|%s|%s|%s|%s|%s|%s|%s|%d|%s|%s|%s|%s|%s|%d",
             id_, empId_.c_str(), username_.c_str(), passHash_.c_str(),
             firstName_.c_str(), lastName_.c_str(), email_.c_str(), phone_.c_str(),
             (int)roleCode_, terminal_.c_str(), gates_.c_str(),
             shiftStart_.c_str(), shiftEnd_.c_str(), created_.c_str(), (int)active_);
    return string(buf);
}
bool Staff::deserialize(const string &line)
{
    char emp[16], un[64], ph2[24], fn[64], ln[64], em[64], ph[64];
    char ter[20], gts[40], ss[8], se[8], crt[20];
    int rc, act;
    sscanf(line.c_str(),
           "%d|%15[^|]|%63[^|]|%23[^|]|%63[^|]|%63[^|]|%63[^|]|%63[^|]"
           "|%d|%19[^|]|%39[^|]|%7[^|]|%7[^|]|%19[^|]|%d",
           &id_, emp, un, ph2, fn, ln, em, ph, &rc, ter, gts, ss, se, crt, &act);
    empId_ = emp;
    username_ = un;
    passHash_ = ph2;
    firstName_ = fn;
    lastName_ = ln;
    email_ = em;
    phone_ = ph;
    roleCode_ = (StaffRole)rc;
    terminal_ = ter;
    gates_ = gts;
    shiftStart_ = ss;
    shiftEnd_ = se;
    created_ = crt;
    active_ = (act != 0);
    if (id_ >= nextId_)
        nextId_ = id_ + 1;
    return true;
}

/*──────────────────────────────────────────────────────────────────────────────
  Admin — CONCEPT [4][5]
──────────────────────────────────────────────────────────────────────────────*/
Admin::Admin() : User() { id_ = nextId_++; }
Admin::Admin(const string &un, const string &fn, const string &ln, const string &em)
    : User(un, fn, ln, em) { id_ = nextId_++; }
Admin::Admin(const Admin &o) : User(o) {}

void Admin::display() const
{
    printf("  " BYL "A%-5d" RS BWH "%-20s" RS " " DIM "%-30s" RS "\n",
           id_, getFullName().c_str(), email_.c_str());
}
void Admin::displayCard(int i) const { display(); }
string Admin::serialize() const
{
    char buf[300];
    snprintf(buf, 300, "%d|%s|%s|%s|%s|%s|%s|%d",
             id_, username_.c_str(), passHash_.c_str(),
             firstName_.c_str(), lastName_.c_str(), email_.c_str(), created_.c_str(), (int)active_);
    return string(buf);
}
bool Admin::deserialize(const string &line)
{
    char un[64], ph2[24], fn[64], ln[64], em[64], crt[20];
    int act;
    sscanf(line.c_str(), "%d|%63[^|]|%23[^|]|%63[^|]|%63[^|]|%63[^|]|%19[^|]|%d",
           &id_, un, ph2, fn, ln, em, crt, &act);
    username_ = un;
    passHash_ = ph2;
    firstName_ = fn;
    lastName_ = ln;
    email_ = em;
    created_ = crt;
    active_ = (act != 0);
    if (id_ >= nextId_)
        nextId_ = id_ + 1;
    return true;
}

/*──────────────────────────────────────────────────────────────────────────────
  BaggageItem — CONCEPT [11][6]
──────────────────────────────────────────────────────────────────────────────*/
BaggageItem::BaggageItem() : id_(nextId_++), bookingId_(0),
                             weightKg_(0), stageIdx_(0), fragile_(false), special_(false),
                             location_("Awaiting Check-in"), carousel_("TBA")
{
    created_ = Utils::getTimestamp();
}
BaggageItem::BaggageItem(int bkId, const string &ref, const string &pax,
                         const string &flight, const string &tag,
                         bool fragile, bool special)
    : id_(nextId_++), bookingId_(bkId), bookingRef_(ref), passengerName_(pax),
      flightNo_(flight), tagNo_(tag), weightKg_(0), stageIdx_(0),
      fragile_(fragile), special_(special),
      location_("Awaiting Check-in"), carousel_("TBA")
{
    created_ = Utils::getTimestamp();
}
void BaggageItem::display() const { displayCard(0); }
void BaggageItem::displayCard(int) const
{
    printf("\n");
    Terminal::boxTop(BCY);
    {
        char b[200];
        snprintf(b, 200, "  🟢  Tag: " BWH "%s" RS "   Flight: " BWH "%s" RS "   Pax: " BWH "%s" RS, tagNo_.c_str(), flightNo_.c_str(), passengerName_.c_str());
        Terminal::boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Weight: " BWH "%.1fkg" RS "   Fragile: %s   Special: %s", weightKg_, fragile_ ? (BRD "YES" RS) : (BGR "NO" RS), special_ ? (BYL "YES" RS) : (DIM "NO" RS));
        Terminal::boxRowC(BCY, b);
    }
    Terminal::boxSep(BCY);
    for (int i = 0; i < BAG_STAGES; i++)
    {
        char b[100];
        if (i <= stageIdx_)
            snprintf(b, 100, "  %s[%d] %s" RS " %s", BAG_COL[i], i + 1, BAG_ST[i], (i == stageIdx_) ? " ← CURRENT" : "");
        else
            snprintf(b, 100, "  " DIM "[%d] %s" RS, i + 1, BAG_ST[i]);
        Terminal::boxRowC(BCY, b);
    }
    Terminal::boxSep(BCY);
    {
        char b[200];
        snprintf(b, 200, "  Location: " BWH "%s" RS, location_.c_str());
        Terminal::boxRowC(BCY, b);
    }
    if (stageIdx_ >= 6 && !carousel_.empty() && carousel_ != "TBA")
    {
        char b[80];
        snprintf(b, 80, "  Carousel: " BYL "%s" RS, carousel_.c_str());
        Terminal::boxRowC(BCY, b);
    }
    Terminal::boxBot(BCY);
}
string BaggageItem::serialize() const
{
    char buf[400];
    snprintf(buf, 400, "%d|%d|%s|%s|%s|%s|%.2f|%d|%s|%s|%s|%d|%d",
             id_, bookingId_, bookingRef_.c_str(), passengerName_.c_str(), flightNo_.c_str(),
             tagNo_.c_str(), weightKg_, stageIdx_, location_.c_str(), carousel_.c_str(),
             created_.c_str(), (int)fragile_, (int)special_);
    return string(buf);
}
bool BaggageItem::deserialize(const string &line)
{
    char ref[20], pax[80], fno[16], tag[20], loc[48], car[8], crt[20];
    double w;
    int si, fr, sp;
    sscanf(line.c_str(), "%d|%d|%19[^|]|%79[^|]|%15[^|]|%19[^|]|%lf|%d|%47[^|]|%7[^|]|%19[^|]|%d|%d",
           &id_, &bookingId_, ref, pax, fno, tag, &w, &si, loc, car, crt, &fr, &sp);
    bookingRef_ = ref;
    passengerName_ = pax;
    flightNo_ = fno;
    tagNo_ = tag;
    weightKg_ = w;
    stageIdx_ = si;
    location_ = loc;
    carousel_ = car;
    created_ = crt;
    fragile_ = (fr != 0);
    special_ = (sp != 0);
    if (id_ >= nextId_)
        nextId_ = id_ + 1;
    return true;
}

/*──────────────────────────────────────────────────────────────────────────────
  Booking — CONCEPT [11][6][10]
──────────────────────────────────────────────────────────────────────────────*/
Booking::Booking() : id_(nextId_++), passengerId_(0), flightId_(0),
                     status_("CONFIRMED"), checkedIn_(false), bagsCount_(0), boardingGroup_("2")
{
    created_ = Utils::getTimestamp();
    bookingRef_ = Utils::generateRef();
}
Booking::Booking(int paxId, int fltId, const string &fno, const string &paxName,
                 const string &cabin, const string &seat, const string &meal,
                 const FareBreakdown &fare, const string &payMethod,
                 const string &card, const string &gate, int bags, const string &req)
    : id_(nextId_++), passengerId_(paxId), flightId_(fltId),
      flightNo_(fno), passengerName_(paxName), cabin_(cabin), seatNo_(seat),
      meal_(meal), fare_(fare), payMethod_(payMethod), cardLast4_(card),
      status_("CONFIRMED"), checkedIn_(false), bagsCount_(bags),
      boardingGate_(gate), boardingGroup_("2"), specialReq_(req)
{
    created_ = Utils::getTimestamp();
    bookingRef_ = Utils::generateRef();
}
void Booking::display() const { displayCard(0); }
void Booking::displayCard(int) const
{
    const char *sc = (!strcmp(status_.c_str(), "CONFIRMED")) ? BGR : (!strcmp(status_.c_str(), "CANCELLED")) ? BRD
                                                                                                             : DIM;
    printf("\n  " BCY "✈ %s" RS "   Ref: " BWH "%s" RS "   Seat: " BWH "%s" RS "   Class: " BWH "%s" RS "\n",
           flightNo_.c_str(), bookingRef_.c_str(), seatNo_.c_str(), cabin_.c_str());
    printf("      " DIM "Total: " RS BGR "$%.2f" RS "   Status: %s" BOLD "%s" RS "   CheckIn: %s\n",
           fare_.grandTotal, sc, status_.c_str(), checkedIn_ ? (BGR "✔ Done" RS) : (BYL "Pending" RS));
    printf("      " DIM "───────────────────────────────────────────────────────────" RS "\n");
}
string Booking::serialize() const
{
    char buf[800];
    snprintf(buf, 800, "%d|%d|%d|%s|%s|%s|%s|%s|%.2f|%.2f|%.2f|%.2f|%.2f|%.2f|%.2f|%.2f|%s|%s|%s|%s|%d|%d|%s|%s|%s|%s",
             id_, passengerId_, flightId_, flightNo_.c_str(), passengerName_.c_str(),
             cabin_.c_str(), seatNo_.c_str(), meal_.c_str(),
             fare_.baseFare, fare_.seatFee, fare_.baggageFee, fare_.mealFee,
             fare_.serviceFee, fare_.taxes, fare_.discount, fare_.grandTotal,
             bookingRef_.c_str(), payMethod_.c_str(), cardLast4_.c_str(), status_.c_str(),
             (int)checkedIn_, bagsCount_, boardingGate_.c_str(), boardingGroup_.c_str(),
             created_.c_str(), specialReq_.c_str());
    return string(buf);
}
bool Booking::deserialize(const string &line)
{
    char fno[16], pax[80], cab[24], seat[8], meal[40], ref[20], pay[24], card[8], st[16], gate[8], grp[4], crt[20], req[60];
    double bf, sf, bgf, mf, svc, tx, disc, gt;
    int ci, bc;
    sscanf(line.c_str(),
           "%d|%d|%d|%15[^|]|%79[^|]|%23[^|]|%7[^|]|%39[^|]"
           "|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf"
           "|%19[^|]|%23[^|]|%7[^|]|%15[^|]|%d|%d|%7[^|]|%3[^|]|%19[^|]|%59[^\n]",
           &id_, &passengerId_, &flightId_, fno, pax, cab, seat, meal,
           &bf, &sf, &bgf, &mf, &svc, &tx, &disc, &gt,
           ref, pay, card, st, &ci, &bc, gate, grp, crt, req);
    flightNo_ = fno;
    passengerName_ = pax;
    cabin_ = cab;
    seatNo_ = seat;
    meal_ = meal;
    fare_ = FareBreakdown(bf, sf, bgf, mf, svc, tx, disc);
    fare_.grandTotal = gt;
    bookingRef_ = ref;
    payMethod_ = pay;
    cardLast4_ = card;
    status_ = st;
    checkedIn_ = (ci != 0);
    bagsCount_ = bc;
    boardingGate_ = gate;
    boardingGroup_ = grp;
    created_ = crt;
    specialReq_ = req;
    if (id_ >= nextId_)
        nextId_ = id_ + 1;
    return true;
}
/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  NEXUS AIR v5.0 — OOP Implementation  (nexus_impl3.cpp)                     ║
 ║  Part 3: BillingEngine, SeatSelector, DataStore, Presentation               ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

/* ── FIX-2: ONE-TIME static member definitions ─────────────────────────── */
DataStore *DataStore::instance_ = nullptr;
int Presentation::_sl = 0;
int Presentation::_tk = 0;

/*──────────────────────────────────────────────────────────────────────────────
  BillingEngine — CONCEPT [10] FRIEND CLASS
──────────────────────────────────────────────────────────────────────────────*/
void BillingEngine::printBill(const Booking &b, const Flight &fl)
{
    printf("\n");
    Terminal::boxTop(BYL);
    Terminal::boxCenter(BYL, BYL BOLD "  NEXUS AIR — OFFICIAL BILLING STATEMENT  " RS);
    Terminal::boxSep(BYL);
    {
        char buf[200];
        snprintf(buf, 200, "  " DIM "Booking Ref  : " RS BWH "%s" RS "   Issued: " BWH "%s" RS, b.bookingRef_.c_str(), b.created_.c_str());
        Terminal::boxRowC(BYL, buf);
    }
    {
        char buf[200];
        snprintf(buf, 200, "  " DIM "Passenger    : " RS BWH "%s" RS, b.passengerName_.c_str());
        Terminal::boxRowC(BYL, buf);
    }
    {
        char buf[200];
        snprintf(buf, 200, "  " DIM "Flight       : " RS BWH "%s" RS "  " BWH "%s" RS " → " BWH "%s" RS, fl.flightNo_.c_str(), fl.origin_.c_str(), fl.dest_.c_str());
        Terminal::boxRowC(BYL, buf);
    }
    {
        char buf[200];
        snprintf(buf, 200, "  " DIM "Date/Time    : " RS BWH "%s  %s" RS "   Seat: " BYL "%s" RS "   Class: " BCY "%s" RS, fl.date_.c_str(), fl.depTime_.c_str(), b.seatNo_.c_str(), b.cabin_.c_str());
        Terminal::boxRowC(BYL, buf);
    }
    Terminal::boxMid(BYL);

    /* Itemized lines using lambda */
    auto line = [&](const char *desc, double amt, const char *col = DIM)
    {
        char ld[50], rd[20];
        snprintf(ld, 50, "  %-36s", desc);
        snprintf(rd, 20, "$%10.2f", amt);
        char full[100];
        snprintf(full, 100, "%s%s" RS "%s%s" RS, col, ld, DIM, rd);
        Terminal::boxRowC(BYL, full);
    };
    line(("Base Fare (" + b.cabin_ + ")").c_str(), b.fare_.baseFare);
    if (b.fare_.seatFee > 0)
        line("Seat Selection Fee", b.fare_.seatFee);
    if (b.fare_.baggageFee > 0)
        line(("Checked Baggage (" + to_string(b.bagsCount_) + " bag(s))").c_str(), b.fare_.baggageFee);
    if (b.fare_.mealFee > 0)
        line("Meal Preference Fee", b.fare_.mealFee);
    line("Service & Handling Fee", b.fare_.serviceFee);
    line("Government Taxes (16.72%)", b.fare_.taxes);
    if (b.fare_.discount > 0)
    {
        char ld[50], rd[20];
        snprintf(ld, 50, "  %-36s", "Nexus Miles Discount");
        snprintf(rd, 20, "-$%9.2f", b.fare_.discount);
        char full[100];
        snprintf(full, 100, BGR "  %-36s" RS DIM "%s" RS, ld, rd);
        Terminal::boxRowC(BYL, full);
    }
    Terminal::boxSep(BYL);
    {
        char ld[50], rd[20];
        snprintf(ld, 50, "  %-36s", "GRAND TOTAL");
        snprintf(rd, 20, "$%10.2f", b.fare_.grandTotal);
        char full[100];
        snprintf(full, 100, BOLD "%-36s" RS BGR BOLD "%s" RS, ld, rd);
        Terminal::boxRowC(BYL, full);
    }
    Terminal::boxSep(BYL);
    {
        char buf[100];
        snprintf(buf, 100, "  Payment: " BWH "%s" RS "   Card: " BWH "****%s" RS, b.payMethod_.c_str(), b.cardLast4_.c_str());
        Terminal::boxRowC(BYL, buf);
    }
    Terminal::boxBot(BYL);
}

double BillingEngine::calcSeatFee(int r, const Flight &fl)
{
    const char *c = SeatMap::cabinForRow(r);
    if (!strcmp(c, "FIRST"))
        return 0;
    if (!strcmp(c, "BUSINESS"))
        return 50;
    if (!strcmp(c, "PREMIUM_ECO"))
        return 35;
    if (r == 14 || r == 15)
        return 25;
    if (r <= 13)
        return 15;
    return 0;
}

bool BillingEngine::payWizard(double amount, string &outMethod, string &outCard, int paxIdx)
{
    Presentation::printBanner();
    Terminal::sectionHeader("NEXUS AIR — SECURE PAYMENT GATEWAY", BCY);
    printf("\n");
    Terminal::boxTop(BCY);
    Terminal::boxCenter(BCY, BCY BOLD "  🔒  SECURE PAYMENT PORTAL  " RS);
    Terminal::boxSep(BCY);
    {
        char b[100];
        snprintf(b, 100, "  " DIM "Amount Due: " RS BGR BOLD "  $%.2f  " RS, amount);
        Terminal::boxRowC(BCY, b);
    }
    Terminal::boxSep(BCY);
    Terminal::boxRowC(BCY, "  " BCY "[1]" RS "  💳  Credit / Debit Card   " DIM "(Visa · MasterCard · Amex)" RS);
    Terminal::boxRowC(BCY, "  " BCY "[2]" RS "  🌐  PayPal");
    Terminal::boxRowC(BCY, "  " BCY "[3]" RS "  🏦  Bank Transfer  " DIM "(IBAN/SWIFT)" RS);
    Terminal::boxRowC(BCY, "  " BCY "[4]" RS "  ⭐  Nexus Miles  " DIM "(Points Redemption)" RS);
    Terminal::boxRowC(BCY, "  " BRD "[0]" RS "  Cancel");
    Terminal::boxBot(BCY);

    int ch = Input::integer("Select Payment:", 0, 4);
    if (ch == 0)
        return false;
    SoundEngine::click();

    if (ch == 1)
    {
        Presentation::printBanner();
        Terminal::sectionHeader("CARD PAYMENT — STEP 1 of 4", BCY);
        printf("\n");
        Terminal::boxTop(BCY);
        Terminal::boxCenter(BCY, BCY "  💳  ENTER CARD DETAILS  " RS);
        Terminal::boxSep(BCY);
        Terminal::boxRowC(BCY, "  " DIM "All data is encrypted with 256-bit SSL" RS);
        Terminal::boxSep(BCY);
        printf("  " BYL "[1]" RS " Visa   " BYL "[2]" RS " MasterCard   " BYL "[3]" RS " Amex   " BYL "[4]" RS " Discover\n");
        int ct = Input::integer("Card Type:", 1, 4);
        const char *ct_names[] = {"Visa", "MasterCard", "American Express", "Discover"};
        Terminal::boxBot(BCY);

        Presentation::printBanner();
        Terminal::sectionHeader("CARD PAYMENT — STEP 2 of 4", BCY);
        printf("\n");
        Terminal::boxTop(BCY);
        Terminal::boxCenter(BCY, BCY "  💳  CARD INFORMATION  " RS);
        Terminal::boxSep(BCY);
        string cardNo = Input::card16();
        Input::field("Cardholder Name:", "As printed on card");
        Input::expiry();
        Input::cvv();
        Terminal::boxBot(BCY);

        Presentation::printBanner();
        Terminal::sectionHeader("CARD PAYMENT — STEP 3 of 4", BCY);
        printf("\n");
        Terminal::boxTop(BCY);
        Terminal::boxCenter(BCY, BCY "  🏠  BILLING ADDRESS  " RS);
        Terminal::boxSep(BCY);
        Input::field("Billing Address:");
        Input::field("City:");
        Input::field("Country:");
        Input::field("Postal Code:");
        Terminal::boxBot(BCY);

        Presentation::printBanner();
        Terminal::sectionHeader("CARD PAYMENT — STEP 4 of 4", BCY);
        Terminal::progressBar("Securely processing payment", 1800);
        printf("\n");
        Terminal::boxTop(BGR);
        Terminal::boxCenter(BGR, BGR BOLD "  PAYMENT AUTHORISATION  " RS);
        Terminal::boxMid(BGR);
        const char *steps[] = {"Contacting issuing bank...", "Verifying card details...", "Checking available funds...", "Authorising transaction...", "Encrypting & finalising..."};
        for (int i = 0; i < 5; i++)
        {
            char b[100];
            snprintf(b, 100, "  " DIM "[%d/5]  " RS BCY "%s" RS, i + 1, steps[i]);
            Terminal::boxRowC(BGR, b);
            fflush(stdout);
            Utils::sleepMs(500);
        }
        Terminal::boxBot(BGR);
        SoundEngine::pay();
        outMethod = ct_names[ct - 1];
        outCard = cardNo.substr(12, 4);
        return true;
    }
    if (ch == 2)
    {
        Presentation::printBanner();
        Terminal::sectionHeader("PAYPAL PAYMENT", BCY);
        printf("\n");
        Terminal::boxTop(BCY);
        Terminal::boxCenter(BCY, BCY "  🌐  PAYPAL  " RS);
        Terminal::boxSep(BCY);
        Input::email("PayPal Email:");
        Input::password("PayPal Password:");
        Terminal::boxBot(BCY);
        Terminal::spinner("Connecting to PayPal...", 1200);
        SoundEngine::pay();
        outMethod = "PayPal";
        outCard = "PPAY";
        return true;
    }
    if (ch == 3)
    {
        Presentation::printBanner();
        Terminal::sectionHeader("BANK TRANSFER", BCY);
        printf("\n");
        Terminal::boxTop(BCY);
        Terminal::boxCenter(BCY, BCY "  🏦  NEXUS AIR BANK DETAILS  " RS);
        Terminal::boxSep(BCY);
        Terminal::boxRowC(BCY, "  " DIM "Bank   : " RS BWH "Nexus Air Financial Services" RS);
        Terminal::boxRowC(BCY, "  " DIM "IBAN   : " RS BWH "NX00 0000 0000 1234 5678 9012" RS);
        Terminal::boxRowC(BCY, "  " DIM "SWIFT  : " RS BWH "NXAIUSXX" RS);
        {
            char b[100];
            snprintf(b, 100, "  " DIM "Amount : " RS BGR "$%.2f" RS, amount);
            Terminal::boxRowC(BCY, b);
        }
        Terminal::boxBot(BCY);
        Input::field("Your Transfer Reference:");
        Terminal::spinner("Verifying bank transfer...", 1400);
        SoundEngine::pay();
        outMethod = "Bank Transfer";
        outCard = "BANK";
        return true;
    }
    /* Miles */
    if (paxIdx >= 0)
    {
        DataStore &db = DataStore::instance();
        Passenger *p = db.passAt(paxIdx);
        int needed = (int)(amount * 10);
        printf("\n  " CY "Your Nexus Miles: " RS BWH "%d" RS "   Required: " BYL "%d" RS "\n", p->getLoyaltyPts(), needed);
        if (p->getLoyaltyPts() >= needed)
        {
            p->redeemPoints(needed);
            db.saveAll();
            SoundEngine::pay();
            outMethod = "Nexus Miles";
            outCard = "MILE";
            return true;
        }
        Terminal::errMsg("Insufficient Nexus Miles for this booking.");
    }
    else
    {
        Terminal::errMsg("Miles redemption requires passenger login.");
    }
    return false;
}

/*──────────────────────────────────────────────────────────────────────────────
  InteractiveSeatSelector — CONCEPT [3][5] Abstraction + Override
──────────────────────────────────────────────────────────────────────────────*/
bool InteractiveSeatSelector::selectSeat(SeatMap &sm, const Flight &fl,
                                         int &outRow, int &outCol,
                                         double &outFare, string &outSeat)
{
    sm.display(fl.getFareFirst(), fl.getFareBiz(), fl.getFarePrem(), fl.getFareEco(),
               fl.getAircraft(), fl.getDate(), fl.getOrigin(), fl.getDest());
    printf("\n");
    Terminal::boxTop(BYL);
    Terminal::boxRowC(BYL, "  ✈  SEAT SELECTION  — Enter seat e.g. " BWH "12A" RS "  or  " BRD "CANCEL" RS " to go back");
    Terminal::boxRowC(BYL, "  " DIM "Row 10-11: First   12-17: Business   18-21: Prem-Eco   22-39: Economy" RS);
    Terminal::boxBot(BYL);

    for (;;)
    {
        printf("  " BYL " ▶ " RS BOLD BWH "Seat Number:              " RS " " BCY);
        fflush(stdout);
        char inp[16];
        fgets(inp, 16, stdin);
        inp[strcspn(inp, "\n")] = 0;
        printf(RS);
        if (!strncmp(inp, "CANCEL", 6) || !strncmp(inp, "cancel", 6) || !strcmp(inp, "0"))
            return false;
        if (strlen(inp) < 2)
        {
            Terminal::warnMsg("Invalid. Use row+col e.g. 12A");
            continue;
        }

        char colCh = toupper(inp[strlen(inp) - 1]);
        char rowStr[8] = {0};
        strncpy(rowStr, inp, strlen(inp) - 1);
        int dispRow = atoi(rowStr);
        int r = dispRow - 10;
        if (r < 0 || r >= TOTAL_ROWS)
        {
            Terminal::warnMsg("Row out of range (10-39)");
            SoundEngine::error();
            continue;
        }

        int cols = SeatMap::colsForRow(r), c = -1;
        for (int i = 0; i < cols; i++)
            if (SeatMap::colLetter(i, cols) == colCh)
            {
                c = i;
                break;
            }
        if (c < 0)
        {
            Terminal::warnMsg("Invalid column for this class");
            SoundEngine::error();
            continue;
        }
        if (sm.getStatus(r, c) == 1)
        {
            Terminal::errMsg("Seat already booked!");
            continue;
        }
        if (sm.getStatus(r, c) == 2)
        {
            Terminal::errMsg("Seat is blocked.");
            continue;
        }

        const char *cab = SeatMap::cabinForRow(r);
        double fare = fl.fareForCabin(string(cab));
        const char *cabCol = (!strcmp(cab, "FIRST")) ? BMG : (!strcmp(cab, "BUSINESS"))  ? BYL
                                                         : (!strcmp(cab, "PREMIUM_ECO")) ? BCY
                                                                                         : BGR;
        char seatLbl[8];
        snprintf(seatLbl, 8, "%d%c", dispRow, SeatMap::colLetter(c, cols));

        printf("\n");
        Terminal::boxTop(BGR);
        {
            char b[200];
            snprintf(b, 200, "  ☆  Seat " BYL "%s" RS "   Class: %s%s" RS "   Seat Fee: " BGR "$%.2f" RS, seatLbl, cabCol, cab, fare);
            Terminal::boxRowC(BGR, b);
        }
        Terminal::boxRowC(BGR, "  " DIM "Note: seat fare is charged on top of the class base fare." RS);
        Terminal::boxBot(BGR);
        SoundEngine::select();

        printf("  " BYL " ▶ " RS BWH "Confirm seat " BYL "%s" RS "? [Y/N]: " RS " " BCY, seatLbl);
        fflush(stdout);
        char conf;
        scanf(" %c", &conf);
        Utils::clearInput();
        printf(RS);
        if (tolower(conf) == 'y')
        {
            outRow = r;
            outCol = c;
            outFare = fare;
            outSeat = string(seatLbl);
            SoundEngine::ok();
            return true;
        }
    }
}

/*──────────────────────────────────────────────────────────────────────────────
  DataStore — CONCEPT [9] Singleton, [7] Template Repository
──────────────────────────────────────────────────────────────────────────────*/
DataStore::DataStore()
    : flightRepo_(MAX_FL), passRepo_(MAX_PA), staffRepo_(MAX_ST),
      adminRepo_(MAX_AD), bookingRepo_(MAX_BK), bagRepo_(MAX_BG) {}

DataStore &DataStore::instance()
{
    if (!instance_)
        instance_ = new DataStore();
    return *instance_;
}

Flight *DataStore::addFlight(const Flight &f)
{
    if (flightCount_ >= MAX_FL)
        throw CapacityException("Flight capacity reached");
    flightArr_[flightCount_] = f;
    Flight *p = &flightArr_[flightCount_++];
    rebuildRepos();
    return p;
}
Passenger *DataStore::addPassenger(const Passenger &p)
{
    if (passCount_ >= MAX_PA)
        throw CapacityException("Passenger capacity reached");
    passArr_[passCount_] = p;
    Passenger *ptr = &passArr_[passCount_++];
    rebuildRepos();
    return ptr;
}
Staff *DataStore::addStaff(const Staff &s)
{
    if (staffCount_ >= MAX_ST)
        throw CapacityException("Staff capacity reached");
    staffArr_[staffCount_] = s;
    Staff *p = &staffArr_[staffCount_++];
    rebuildRepos();
    return p;
}
Admin *DataStore::addAdmin(const Admin &a)
{
    if (adminCount_ >= MAX_AD)
        throw CapacityException("Admin capacity reached");
    adminArr_[adminCount_] = a;
    Admin *p = &adminArr_[adminCount_++];
    rebuildRepos();
    return p;
}
Booking *DataStore::addBooking(const Booking &b)
{
    if (bookingCount_ >= MAX_BK)
        throw CapacityException("Booking capacity reached");
    bookingArr_[bookingCount_] = b;
    Booking *p = &bookingArr_[bookingCount_++];
    rebuildRepos();
    return p;
}
BaggageItem *DataStore::addBag(const BaggageItem &b)
{
    if (bagCount_ >= MAX_BG)
        throw CapacityException("Baggage capacity reached");
    bagArr_[bagCount_] = b;
    BaggageItem *p = &bagArr_[bagCount_++];
    rebuildRepos();
    return p;
}

void DataStore::rebuildRepos()
{
    /* CONCEPT [7] Template — rebuild all repositories after any mutation */
    flightRepo_.rebuildFrom(flightArr_, flightCount_);
    passRepo_.rebuildFrom(passArr_, passCount_);
    staffRepo_.rebuildFrom(staffArr_, staffCount_);
    adminRepo_.rebuildFrom(adminArr_, adminCount_);
    bookingRepo_.rebuildFrom(bookingArr_, bookingCount_);
    bagRepo_.rebuildFrom(bagArr_, bagCount_);
}

int DataStore::findFlightByNo(const string &no) const
{
    for (int i = 0; i < flightCount_; i++)
        if (flightArr_[i].getFlightNo() == no && flightArr_[i].isActive())
            return i;
    return -1;
}
int DataStore::findPassByUser(const string &un) const
{
    for (int i = 0; i < passCount_; i++)
        if (passArr_[i].getUsername() == un && passArr_[i].isActive())
            return i;
    return -1;
}
int DataStore::findStaffByUser(const string &un) const
{
    for (int i = 0; i < staffCount_; i++)
        if (staffArr_[i].getUsername() == un && staffArr_[i].isActive())
            return i;
    return -1;
}
int DataStore::findAdminByUser(const string &un) const
{
    for (int i = 0; i < adminCount_; i++)
        if (adminArr_[i].getUsername() == un && adminArr_[i].isActive())
            return i;
    return -1;
}
int DataStore::findBookingByRef(const string &ref) const
{
    for (int i = 0; i < bookingCount_; i++)
        if (bookingArr_[i].getBookingRef() == ref)
            return i;
    return -1;
}

/* File I/O */
void DataStore::saveFlights()
{
    FILE *f = fopen(FILE_FLIGHTS, "w");
    if (!f)
        return;
    fprintf(f, "COUNT:%d\n", flightCount_);
    for (int i = 0; i < flightCount_; i++)
        fprintf(f, "%s\n", flightArr_[i].serialize().c_str());
    fclose(f);
}
void DataStore::loadFlights()
{
    FILE *f = fopen(FILE_FLIGHTS, "r");
    if (!f)
        return;
    char line[512];
    if (!fgets(line, 512, f))
    {
        fclose(f);
        return;
    }
    int cnt = 0;
    sscanf(line, "COUNT:%d", &cnt);
    if (cnt > MAX_FL)
        cnt = MAX_FL;
    for (int i = 0; i < cnt; i++)
    {
        if (!fgets(line, 512, f))
            break;
        line[strcspn(line, "\n")] = 0;
        flightArr_[flightCount_].deserialize(string(line));
        flightCount_++;
    }
    fclose(f);
}
void DataStore::savePassengers()
{
    FILE *f = fopen(FILE_PASS, "w");
    if (!f)
        return;
    fprintf(f, "COUNT:%d\n", passCount_);
    for (int i = 0; i < passCount_; i++)
        fprintf(f, "%s\n", passArr_[i].serialize().c_str());
    fclose(f);
}
void DataStore::loadPassengers()
{
    FILE *f = fopen(FILE_PASS, "r");
    if (!f)
        return;
    char line[600];
    if (!fgets(line, 600, f))
    {
        fclose(f);
        return;
    }
    int cnt = 0;
    sscanf(line, "COUNT:%d", &cnt);
    if (cnt > MAX_PA)
        cnt = MAX_PA;
    for (int i = 0; i < cnt; i++)
    {
        if (!fgets(line, 600, f))
            break;
        line[strcspn(line, "\n")] = 0;
        passArr_[passCount_].deserialize(string(line));
        passCount_++;
    }
    fclose(f);
}
void DataStore::saveStaff()
{
    FILE *f = fopen(FILE_STAFF, "w");
    if (!f)
        return;
    fprintf(f, "COUNT:%d\n", staffCount_);
    for (int i = 0; i < staffCount_; i++)
        fprintf(f, "%s\n", staffArr_[i].serialize().c_str());
    fclose(f);
}
void DataStore::loadStaff()
{
    FILE *f = fopen(FILE_STAFF, "r");
    if (!f)
        return;
    char line[512];
    if (!fgets(line, 512, f))
    {
        fclose(f);
        return;
    }
    int cnt = 0;
    sscanf(line, "COUNT:%d", &cnt);
    if (cnt > MAX_ST)
        cnt = MAX_ST;
    for (int i = 0; i < cnt; i++)
    {
        if (!fgets(line, 512, f))
            break;
        line[strcspn(line, "\n")] = 0;
        staffArr_[staffCount_].deserialize(string(line));
        staffCount_++;
    }
    fclose(f);
}
void DataStore::saveAdmins()
{
    FILE *f = fopen(FILE_ADMINS, "w");
    if (!f)
        return;
    fprintf(f, "COUNT:%d\n", adminCount_);
    for (int i = 0; i < adminCount_; i++)
        fprintf(f, "%s\n", adminArr_[i].serialize().c_str());
    fclose(f);
}
void DataStore::loadAdmins()
{
    FILE *f = fopen(FILE_ADMINS, "r");
    if (!f)
        return;
    char line[400];
    if (!fgets(line, 400, f))
    {
        fclose(f);
        return;
    }
    int cnt = 0;
    sscanf(line, "COUNT:%d", &cnt);
    if (cnt > MAX_AD)
        cnt = MAX_AD;
    for (int i = 0; i < cnt; i++)
    {
        if (!fgets(line, 400, f))
            break;
        line[strcspn(line, "\n")] = 0;
        adminArr_[adminCount_].deserialize(string(line));
        adminCount_++;
    }
    fclose(f);
}
void DataStore::saveBookings()
{
    FILE *f = fopen(FILE_BOOKINGS, "w");
    if (!f)
        return;
    fprintf(f, "COUNT:%d\n", bookingCount_);
    for (int i = 0; i < bookingCount_; i++)
        fprintf(f, "%s\n", bookingArr_[i].serialize().c_str());
    fclose(f);
}
void DataStore::loadBookings()
{
    FILE *f = fopen(FILE_BOOKINGS, "r");
    if (!f)
        return;
    char line[800];
    if (!fgets(line, 800, f))
    {
        fclose(f);
        return;
    }
    int cnt = 0;
    sscanf(line, "COUNT:%d", &cnt);
    if (cnt > MAX_BK)
        cnt = MAX_BK;
    for (int i = 0; i < cnt; i++)
    {
        if (!fgets(line, 800, f))
            break;
        line[strcspn(line, "\n")] = 0;
        bookingArr_[bookingCount_].deserialize(string(line));
        bookingCount_++;
    }
    fclose(f);
}
void DataStore::saveBags()
{
    FILE *f = fopen(FILE_BAGGAGE, "w");
    if (!f)
        return;
    fprintf(f, "COUNT:%d\n", bagCount_);
    for (int i = 0; i < bagCount_; i++)
        fprintf(f, "%s\n", bagArr_[i].serialize().c_str());
    fclose(f);
}
void DataStore::loadBags()
{
    FILE *f = fopen(FILE_BAGGAGE, "r");
    if (!f)
        return;
    char line[512];
    if (!fgets(line, 512, f))
    {
        fclose(f);
        return;
    }
    int cnt = 0;
    sscanf(line, "COUNT:%d", &cnt);
    if (cnt > MAX_BG)
        cnt = MAX_BG;
    for (int i = 0; i < cnt; i++)
    {
        if (!fgets(line, 512, f))
            break;
        line[strcspn(line, "\n")] = 0;
        bagArr_[bagCount_].deserialize(string(line));
        bagCount_++;
    }
    fclose(f);
}
void DataStore::saveSeatMaps()
{
    FILE *f = fopen(FILE_SEATS, "w");
    if (!f)
        return;
    fprintf(f, "COUNT:%d\n", flightCount_);
    for (int i = 0; i < flightCount_; i++)
    {
        string s = flightArr_[i].getSeatMap().serialize();
        fprintf(f, "%s", s.c_str());
    }
    fclose(f);
}
void DataStore::loadSeatMaps()
{
    FILE *f = fopen(FILE_SEATS, "r");
    if (!f)
        return;
    char line[1024];
    if (!fgets(line, 1024, f))
    {
        fclose(f);
        return;
    }
    int cnt = 0;
    sscanf(line, "COUNT:%d", &cnt);
    for (int i = 0; i < cnt && i < flightCount_; i++)
    {
        /* Read 1 + TOTAL_ROWS lines into a block string */
        string block = "";
        for (int r = 0; r < TOTAL_ROWS + 1; r++)
        {
            if (!fgets(line, 1024, f))
                break;
            block += string(line);
        }
        flightArr_[i].getSeatMap().deserialize(block);
    }
    fclose(f);
}
void DataStore::saveAll()
{
    saveFlights();
    savePassengers();
    saveStaff();
    saveAdmins();
    saveBookings();
    saveBags();
    saveSeatMaps();
}
void DataStore::loadAll()
{
    loadFlights();
    loadPassengers();
    loadStaff();
    loadAdmins();
    loadBookings();
    loadBags();
    loadSeatMaps();
    rebuildRepos();
}
void DataStore::seedData()
{
    if (flightCount_ > 0)
        return;
    struct FD
    {
        const char *no, *org, *dst, *date, *dep, *arr, *ac;
        int gate, td, ta, dur;
        double f1, f2, f3, f4;
    };
    FD fd[] = {
        {"NX101", "Karachi (KHI)", "Dubai (DXB)", "20-MAR-2026", "09:00", "11:00", "Airbus A320-Neo", 10, 1, 3, 120, 2500, 1200, 700, 280},
        {"NX202", "Lahore (LHE)", "London (LHR)", "21-MAR-2026", "14:00", "18:45", "Boeing 777-300ER", 22, 2, 5, 480, 3200, 1800, 1100, 650},
        {"NX303", "Islamabad (ISB)", "New York (JFK)", "21-MAR-2026", "18:30", "06:45+1", "Boeing 787-9 Dreamliner", 42, 3, 8, 810, 4500, 2400, 1600, 890},
        {"NX404", "Karachi (KHI)", "Kuala Lumpur (KUL)", "22-MAR-2026", "23:00", "09:30+1", "Airbus A330-900Neo", 15, 1, 4, 540, 3100, 1700, 950, 420},
        {"NX505", "Dubai (DXB)", "London (LHR)", "22-MAR-2026", "10:30", "14:15", "Airbus A380-800", 30, 3, 5, 405, 3800, 2100, 1300, 720},
    };
    for (int i = 0; i < 5; i++)
    {
        Flight fl(fd[i].no, "Nexus Air", fd[i].org, fd[i].dst, fd[i].date, fd[i].dep, fd[i].arr,
                  fd[i].ac, fd[i].gate, fd[i].td, fd[i].ta, fd[i].dur,
                  fd[i].f1, fd[i].f2, fd[i].f3, fd[i].f4);
        addFlight(fl);
    }
    saveAll();
}

/*──────────────────────────────────────────────────────────────────────────────
  Presentation — Banner & Boot animations
──────────────────────────────────────────────────────────────────────────────*/
void Presentation::printBanner()
{
    const char *art[] = {
        " ███╗   ██╗███████╗██╗  ██╗██╗   ██╗███████╗     █████╗ ██╗██████╗  ",
        " ████╗  ██║██╔════╝╚██╗██╔╝██║   ██║██╔════╝    ██╔══██╗██║██╔══██╗ ",
        " ██╔██╗ ██║█████╗   ╚███╔╝ ██║   ██║███████╗    ███████║██║██████╔╝ ",
        " ██║╚██╗██║██╔══╝   ██╔██╗ ██║   ██║╚════██║    ██╔══██║██║██╔══██╗ ",
        " ██║ ╚████║███████╗██╔╝ ██╗╚██████╔╝███████║    ██║  ██║██║██║  ██║ ",
        " ╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝    ╚═╝  ╚═╝╚═╝╚═╝  ╚═╝ "};
    int aw = 70, lp = (TW - aw) / 2, rp = TW - aw - lp;
    if (lp < 0)
        lp = 0;
    if (rp < 0)
        rp = 0;

    static const char *SL[][2] = {
        {"⚡  NEXUS AIR — CONNECTING THE WORLD, ONE FLIGHT AT A TIME  ⚡", "🌏 Precision. Innovation. Excellence in every journey."},
        {"🚀  BEYOND THE HORIZON WITH NEXUS AIR.", "💺 From booking to landing — a seamless premium experience"},
        {"🌟  WHERE EVERY DESTINATION IS A PROMISE.", "✈  Nexus Air — your trusted partner at 35,000 feet"},
        {"🎯  TECHNOLOGY MEETS ALTITUDE. NEXUS AIR LEADS.", "🛫 Innovative airline management for the modern world"},
        {"🏆  THE GOLD STANDARD IN AIRLINE EXCELLENCE.", "🌊 Smooth operations, happy passengers, on-time every time"},
    };
    static const char *tks[] = {"  🌌 CONNECTING THE WORLD ONE NEXUS FLIGHT AT A TIME",
                                "  🔭 WHERE INNOVATION MEETS THE OPEN SKY",
                                "  🎨 PRECISION MANAGEMENT FOR EVERY ROUTE",
                                "  🦋 YOUR PASSENGERS TAKE WING WITH NEXUS AIR",
                                "  🌊 SMOOTH SKIES AHEAD — POWERED BY NEXUS",
                                "  🏛  BUILT ON TRUST. ENGINEERED FOR EXCELLENCE.",
                                "  🎭 EVERY GREAT JOURNEY BEGINS AT NEXUS AIR",
                                "  🎵 YOUR OPERATIONS, ORCHESTRATED TO PERFECTION"};
    static const char *rks[] = {"Fly. Manage. Excel.  ", "Every flight, on time  ", "Nexus: precision altitude  ",
                                "The sky is no limit  ", "Your airline, reimagined  ",
                                "Fly bold. Fly Nexus.  ", "Where ideas take off  ", "The future flies Nexus  "};
    int ns = 5, nt = 8;
    const char *s1 = SL[_sl % ns][0];
    const char *s2 = SL[_sl % ns][1];
    _sl++;
    const char *lt2 = tks[_tk % nt];
    const char *rt2 = rks[_tk % nt];
    _tk++;

    Utils::clearScreen();
    printf("\n");
    printf(BCY BT);
    for (int i = 0; i < TW; i++)
        printf(BH);
    printf(BR RS "\n");
    {
        const char *lt = "  ✈ NEXUS AIR";
        const char *rt = "v5.0  ";
        int lw = 14, rw = (int)strlen(rt), g = TW - lw - rw;
        if (g < 1)
            g = 1;
        printf(BCY BV RS BCY "  ✈ NEXUS AIR" RS);
        Utils::spaces(g);
        printf(BYL "%s" RS BCY BV RS "\n", rt);
    }
    printf(BCY BST);
    for (int i = 0; i < TW; i++)
        printf(SH);
    printf(BSB RS "\n");
    printf(BCY BV RS);
    Utils::spaces(TW);
    printf(BCY BV RS "\n");
    for (int r = 0; r < 6; r++)
    {
        printf(BCY BV RS);
        Utils::spaces(lp);
        printf(BYL "%s" RS, art[r]);
        Utils::spaces(rp);
        printf(BCY BV RS "\n");
    }
    printf(BCY BV RS);
    Utils::spaces(TW);
    printf(BCY BV RS "\n");
    printf(BCY BST);
    for (int i = 0; i < TW; i++)
        printf(SH);
    printf(BSB RS "\n");
    {
        char b[256];
        snprintf(b, 256, BMG "%s" RS, s1);
        Terminal::boxCenter(BCY, b);
    }
    {
        char b[256];
        snprintf(b, 256, DIM "%s" RS, s2);
        Terminal::boxCenter(BCY, b);
    }
    Terminal::boxCenter(BCY, DIM "  ✦  Created by  " BWH "Muhammad Tahir Hussain" DIM "  ✦  " RS);
    printf(BCY BV RS);
    Utils::spaces(TW);
    printf(BCY BV RS "\n");
    {
        int lw = (int)strlen(lt2), rw = (int)strlen(rt2), g = TW - lw - rw;
        if (g < 1)
            g = 1;
        printf(BCY BV RS BMG "%s" RS, lt2);
        Utils::spaces(g);
        printf(BCY "%s" RS BCY BV RS "\n", rt2);
    }
    printf(BCY BB);
    for (int i = 0; i < TW; i++)
        printf(BH);
    printf(BL2 RS "\n\n");
}

void Presentation::bootSeq()
{
    Utils::clearScreen();
    printf("\n");
    const char *art[] = {
        " ███╗   ██╗███████╗██╗  ██╗██╗   ██╗███████╗     █████╗ ██╗██████╗  ",
        " ████╗  ██║██╔════╝╚██╗██╔╝██║   ██║██╔════╝    ██╔══██╗██║██╔══██╗ ",
        " ██╔██╗ ██║█████╗   ╚███╔╝ ██║   ██║███████╗    ███████║██║██████╔╝ ",
        " ██║╚██╗██║██╔══╝   ██╔██╗ ██║   ██║╚════██║    ██╔══██║██║██╔══██╗ ",
        " ██║ ╚████║███████╗██╔╝ ██╗╚██████╔╝███████║    ██║  ██║██║██║  ██║ ",
        " ╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝    ╚═╝  ╚═╝╚═╝╚═╝  ╚═╝ "};
    int aw = 70, lp = (TW - aw) / 2, rp = TW - aw - lp;
    if (lp < 0)
        lp = 0;
    if (rp < 0)
        rp = 0;
    printf(BCY BT);
    for (int i = 0; i < TW; i++)
        printf(BH);
    printf(BR RS "\n");
    printf(BCY BV RS);
    Utils::spaces(TW);
    printf(BCY BV RS "\n");
    for (int r = 0; r < 6; r++)
    {
        printf(BCY BV RS);
        Utils::spaces(lp);
        printf(BYL "%s" RS, art[r]);
        Utils::spaces(rp);
        printf(BCY BV RS "\n");
    }
    printf(BCY BV RS);
    Utils::spaces(TW);
    printf(BCY BV RS "\n");
    printf(BCY BB);
    for (int i = 0; i < TW; i++)
        printf(BH);
    printf(BL2 RS "\n");
    fflush(stdout);
    Utils::sleepMs(280);
    printf("\n");
    int slp = (TW + 2 - 44) / 2;
    if (slp < 0)
        slp = 0;
    Utils::spaces(slp);
    printf(BYL "  ⚡  Initialising NEXUS AIR Engine  " BGR);
    for (int i = 0; i < 6; i++)
    {
        printf(".");
        fflush(stdout);
        SoundEngine::boot();
        Utils::sleepMs(290);
    }
    printf("  " BWH "[ ONLINE ]" RS "\n\n");
    Utils::sleepMs(190);
}
void Presentation::aiLogs()
{
    const char *ll[] = {"  🔧  Loading NEXUS AIR flight database modules",
                        "  ⚡  Starting booking & payment engine",
                        "  📖  Mounting passenger records & seat maps",
                        "  🔐  Loading authentication system",
                        "  🛡   Setting up role-based access control",
                        "  ✈   Synchronising real-time flight & baggage status",
                        "  ✅  NEXUS AIR v5.0 is ONLINE and READY"};
    const char *cc[] = {DIM, DIM, DIM, DIM, DIM, DIM, BGR};
    Terminal::boxTop(DIM);
    for (int i = 0; i < 7; i++)
    {
        char b[128];
        snprintf(b, 128, "%s%s" RS, cc[i], ll[i]);
        Terminal::boxRowC(DIM, b);
        fflush(stdout);
        Utils::sleepMs(75);
    }
    Terminal::boxBot(DIM);
    printf("\n");
}
/* ── helper: animated count-down ──────────────────────────── */
static void exitCountdown(int from)
{
    for (int i = from; i >= 1; i--)
    {
        printf("\r  " BYL "Shutting down in " BGR BOLD "%d" RS BYL "..." RS "   ", i);
        fflush(stdout);
        Beep(600 + (from - i) * 80, 120);
        Utils::sleepMs(900);
    }
    printf("\r");
    Utils::spaces(50);
    printf("\r");
}

/* ── helper: horizontal scrolling runway ──────────────────── */
static void runwayAnimation()
{
    const char *plane = " ✈ ";
    printf("\n");
    /* runway line */
    printf("  ");
    for (int i = 0; i < TW - 2; i++)
        printf(DIM "─" RS);
    printf("\n");
    /* plane taxiing */
    for (int p = 0; p <= TW - 6; p += 2)
    {
        printf("\r  ");
        Utils::spaces(p);
        printf(BYL BOLD "%s" RS, plane);
        fflush(stdout);
        Utils::sleepMs(30);
    }
    /* take-off sweep upward */
    SoundEngine::fly();
    for (int row = 1; row <= 4; row++)
    {
        printf("\033[%dA", 1); /* cursor up */
        printf("\r  ");
        Utils::spaces(TW - 6 - (row * 2));
        printf(BYL BOLD "%s" RS, plane);
        fflush(stdout);
        Utils::sleepMs(80);
    }
    printf("\n\n\n\n");
}

/* ── helper: fireworks burst ──────────────────────────────── */
static void fireworks(int bursts)
{
    const char *sparks[] = {"*", "✦", "✸", "✺", "◉", "★", "⬟"};
    const char *cols[] = {BRD, BGR, BYL, BCY, BMG, BWH, BYL};
    int ns = 7;
    for (int b = 0; b < bursts; b++)
    {
        int cx = 5 + rand() % (TW - 10);
        /* rise */
        for (int r = 0; r < 3; r++)
        {
            printf("\r");
            Utils::spaces(cx);
            printf(BYL "│" RS);
            fflush(stdout);
            Utils::sleepMs(60);
            printf("\r");
            Utils::spaces(cx);
            printf("  ");
        }
        /* explode */
        Beep(1000 + rand() % 400, 60);
        printf("\r");
        Utils::spaces(max(0, cx - 3));
        for (int s = 0; s < 7; s++)
        {
            int si = rand() % ns;
            printf("%s%s" RS, cols[si], sparks[si]);
        }
        fflush(stdout);
        Utils::sleepMs(200);
        printf("\r");
        Utils::spaces(TW);
        printf("\r");
    }
}

/* ── helper: wave text effect ─────────────────────────────── */
static void waveText(const char *text, const char *col, int waves = 3)
{
    int len = (int)strlen(text);
    for (int w = 0; w < waves * len; w++)
    {
        printf("\r  ");
        for (int c = 0; c < len; c++)
        {
            if (c == (w % len))
                printf("%s" BOLD "%c" RS, col, text[c]);
            else
                printf(DIM "%c" RS, text[c]);
        }
        fflush(stdout);
        Utils::sleepMs(40);
    }
    printf("\r  ");
    printf("%s" BOLD, col);
    for (int c = 0; c < len; c++)
        printf("%c", text[c]);
    printf(RS "\n");
}

/* ── helper: matrix-style digit rain column ───────────────── */
static void digitRain(int cols_count, int rows)
{
    for (int r = 0; r < rows; r++)
    {
        printf("  ");
        for (int c = 0; c < cols_count; c++)
        {
            if (rand() % 3 == 0)
                printf(BGR "%d" RS, rand() % 10);
            else if (rand() % 5 == 0)
                printf(BCY "%d" RS, rand() % 10);
            else
                printf(DIM "%d" RS, rand() % 10);
        }
        printf("\n");
        fflush(stdout);
        Utils::sleepMs(55);
    }
}

/* ── helper: typewriter line inside box ──────────────────── */
static void boxTypeLine(const char *bc, const char *text, const char *col, int delay = 18)
{
    int rawlen = (int)strlen(text);
    int pad = TW - rawlen;
    if (pad < 0)
        pad = 0;
    printf("%s" BV RS, bc);
    printf("%s", col);
    for (int i = 0; text[i]; i++)
    {
        printf("%c", text[i]);
        fflush(stdout);
        Utils::sleepMs(delay);
    }
    printf(RS);
    Utils::spaces(pad);
    printf("%s" BV RS "\n", bc);
}

void Presentation::exitAnim()
{
    Utils::clearScreen();

    /* ── PHASE 1: confirmation prompt ── */
    printf("\n\n");
    Terminal::boxTop(BRD);
    Terminal::boxCenter(BRD, BRD BOLD "  ⚠   ARE YOU SURE YOU WANT TO EXIT?  ⚠  " RS);
    Terminal::boxSep(BRD);
    Terminal::boxRowC(BRD, "  " BRD "All unsaved session data will be cleared." RS);
    Terminal::boxRowC(BRD, "  " DIM "Your flight data and bookings are already saved to disk." RS);
    Terminal::boxSep(BRD);
    Terminal::boxRowC(BRD, "  " BGR "[Y]" RS "  Yes, exit NEXUS AIR");
    Terminal::boxRowC(BRD, "  " BCY "[N]" RS "  No, take me back");
    Terminal::boxBot(BRD);
    printf("\n  " BYL " ▶ " RS BOLD BWH "Your choice [Y/N]: " RS " " BCY);
    fflush(stdout);
    char conf;
    scanf(" %c", &conf);
    Utils::clearInput();
    printf(RS);
    if (tolower(conf) != 'y')
    {
        Terminal::infoMsg("Good call! The skies still need you.");
        Utils::sleepMs(800);
        return;
    }

    Utils::clearScreen();

    /* ── PHASE 2: digit rain intro ── */
    printf("\n");
    digitRain(38, 5);
    Utils::sleepMs(200);

    /* ── PHASE 3: animated banner ── */
    Utils::clearScreen();
    printf("\n");
    Terminal::starBurst(2);
    printf("\n");

    /* Fireworks */
    for (int f = 0; f < 8; f++)
        fireworks(1);

    /* ── PHASE 4: main farewell box typed out ── */
    Utils::clearScreen();
    printf("\n");

    /* ASCII art plane fade-in */
    const char *art[] = {
        "        " BCY "      __|__          " RS,
        "        " BYL "   --(@)--       ✈  " RS,
        "        " BCY "      / \\           " RS};
    for (int r = 0; r < 3; r++)
    {
        printf("  %s\n", art[r]);
        Utils::sleepMs(120);
    }
    printf("\n");

    /* Runway take-off */
    runwayAnimation();
    Utils::sleepMs(300);

    /* ── PHASE 5: farewell messages box ── */
    Terminal::boxTop(BMG);
    Terminal::boxCenter(BMG, BYL BOLD "  ✈   NEXUS AIR — SIGNING OFF   ✈  " RS);
    Terminal::boxMid(BMG);

    struct FareLine
    {
        const char *text;
        const char *col;
        int delay;
    };
    FareLine lines[] = {
        {"  ✨  Thank you for using NEXUS AIR Management System!", BWH, 20},
        {"  🛫  Every flight you managed took someone home.", BCY, 18},
        {"  🌍  You connected cities, people, and dreams today.", BGR, 18},
        {"  📊  Your data is saved — every booking, every record.", BYL, 16},
        {"  🏆  You ran a world-class airline. Well done.", BMG, 18},
        {"", RS, 0},
        {"  ✦   Created by  Muhammad Tahir Hussain  ✦", DIM, 14},
        {"", RS, 0},
        {"  🚀  NEXUS AIR v5.0  —  Until next time, Captain!", BYL, 22},
    };
    for (int i = 0; i < 9; i++)
    {
        if (lines[i].delay == 0)
        {
            Terminal::boxEmpty(BMG);
        }
        else
        {
            boxTypeLine(BMG, lines[i].text, lines[i].col, lines[i].delay);
            SoundEngine::tick();
            Utils::sleepMs(80);
        }
    }
    Terminal::boxMid(BMG);

    /* ── PHASE 6: animated progress "Saving & closing" ── */
    {
        int bw = TW - 20;
        const char *stages[] = {"Saving records", "Closing connections", "Clearing session", "Powering down"};
        for (int s = 0; s < 4; s++)
        {
            char lbl[60];
            snprintf(lbl, 60, "  %-20s", stages[s]);
            printf(BMG BV RS BYL "%s" RS "  [", lbl);
            int seg = bw / 4;
            for (int j = 0; j < seg; j++)
            {
                printf(BGR "█" RS);
                fflush(stdout);
                SoundEngine::tick();
                Utils::sleepMs(18);
            }
            printf("] " BGR "✔" RS);
            Utils::spaces(TW - (int)strlen(lbl) - 2 - seg - 4);
            printf(BMG BV RS "\n");
        }
    }
    Terminal::boxMid(BMG);

    /* ── PHASE 7: live stats row ── */
    {
        DataStore &db = DataStore::instance();
        char stats[200];
        snprintf(stats, 200, "  Flights: " BWH "%d" RS "   Passengers: " BWH "%d" RS "   Bookings: " BWH "%d" RS "   Staff: " BWH "%d" RS,
                 db.flightCount(), db.passCount(), db.bookingCount(), db.staffCount());
        Terminal::boxRowC(BMG, stats);
    }
    Terminal::boxMid(BMG);

    /* ── PHASE 8: wave goodbye text ── */
    printf(BMG BV RS);
    waveText("  GOODBYE FROM NEXUS AIR!  See you on the next flight...", BYL, 2);
    /* fix: re-close the row */
    printf(BMG BV RS "\n");

    /* ── PHASE 9: countdown bar ── */
    {
        int bw = TW - 8;
        printf(BMG BV RS "  [ ");
        for (int i = 0; i < bw; i++)
            printf(DIM "░" RS);
        printf(" ]  " BMG BV RS);
        printf("\r" BMG BV RS "  [ ");
        fflush(stdout);
        for (int i = 0; i < bw; i++)
        {
            /* colour shifts from red→yellow→green */
            const char *c = (i < bw / 3) ? BRD : (i < 2 * bw / 3) ? BYL
                                                                  : BGR;
            printf("%s█" RS, c);
            fflush(stdout);
            Utils::sleepMs(12);
        }
        printf(" ]  " BGR BOLD "✔ DONE" RS);
        Utils::spaces(TW - (4 + bw + 8));
        printf(BMG BV RS "\n");
    }
    Terminal::boxBot(BMG);

    /* ── PHASE 10: final fireworks burst ── */
    printf("\n");
    SoundEngine::exitSound();
    for (int f = 0; f < 12; f++)
    {
        fireworks(1);
        Utils::sleepMs(60);
    }

    /* ── PHASE 11: final scrolling plane ── */
    printf("\n");
    for (int p = 0; p < TW + 2; p++)
    {
        printf("\r");
        Utils::spaces(p);
        printf(BYL BOLD "✈ " RS DIM "NEXUS AIR" RS);
        fflush(stdout);
        Utils::sleepMs(14);
    }
    printf("\n\n");

    /* ── PHASE 12: shrinking banner fade-out ── */
    const char *fadeLines[] = {
        "  ★ ★ ★  N E X U S   A I R  ★ ★ ★  ",
        "   ★ ★  N E X U S  A I R  ★ ★   ",
        "    ★  N E X U S  A I R  ★    ",
        "      N E X U S           ",
        "         N E X           ",
        "           N           ",
        "                   "};
    const char *fadeCols[] = {BWH, BYL, BCY, BCY, BCY, DIM, DIM};
    for (int i = 0; i < 7; i++)
    {
        int lp = (TW - (int)strlen(fadeLines[i])) / 2;
        if (lp < 0)
            lp = 0;
        printf("\r");
        Utils::spaces(lp);
        printf("%s" BOLD "%s" RS, fadeCols[i], fadeLines[i]);
        fflush(stdout);
        Utils::sleepMs(180);
        printf("\r");
        Utils::spaces(TW + 2);
        printf("\r");
    }

    Utils::sleepMs(400);
    Utils::clearScreen();
}
/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  NEXUS AIR v5.0 — OOP Implementation  (nexus_impl4.cpp)                     ║
 ║  Part 4: Staff Panels (Polymorphic), Dashboards, AuthController, main()     ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

/*──────────────────────────────────────────────────────────────────────────────
  CONCEPT [5] POLYMORPHISM — Factory creates the correct IRolePanel at runtime
──────────────────────────────────────────────────────────────────────────────*/
IRolePanel *createRolePanel(Staff &s, DataStore &db, Session &sess)
{
    switch (s.getRoleCode())
    {
    case SR_GATE:
        return new GateAgentPanel(s, db, sess);
    case SR_CHECKIN:
        return new CheckInPanel(s, db, sess);
    case SR_BAGGAGE:
        return new BaggagePanel(s, db, sess);
    case SR_SUPERVISOR:
        return new SupervisorPanel(s, db, sess);
    case SR_TICKET:
        return new TicketPanel(s, db, sess);
    case SR_SECURITY:
        return new SecurityPanel(s, db, sess);
    case SR_LOUNGE:
        return new LoungePanel(s, db, sess);
    case SR_GROUND:
        return new GroundCrewPanel(s, db, sess);
    case SR_DISPATCH:
        return new DispatchPanel(s, db, sess);
    case SR_CUSTOMS:
        return new CustomsPanel(s, db, sess);
    default:
        return new GateAgentPanel(s, db, sess);
    }
}

/*──────────────────────────────────────────────────────────────────────────────
  GateAgentPanel — CONCEPT [5] overrides execute()
──────────────────────────────────────────────────────────────────────────────*/
void GateAgentPanel::execute()
{
    Presentation::printBanner();
    Terminal::sectionHeader("BOARDING MANAGEMENT", BMG);
    string fno = Input::field("Flight Number:");
    int fi = db_.findFlightByNo(fno);
    if (fi < 0)
    {
        Terminal::errMsg("Flight not found.");
        Terminal::waitEnter();
        return;
    }
    Flight &fl = *db_.flightAt(fi);

    printf("\n");
    Terminal::boxTop(BMG);
    {
        char b[200];
        snprintf(b, 200, "  " BOLD BMG "✈  %s — %s → %s" RS, fl.getFlightNo().c_str(), fl.getOrigin().c_str(), fl.getDest().c_str());
        Terminal::boxRowC(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Gate: " RS BWH "%d" RS "   Status: " RS "%s" BOLD "%s" RS, fl.getGate(), Utils::statusColor(fl.getStatus().c_str()), fl.getStatus().c_str());
        Terminal::boxRowC(BMG, b);
    }
    Terminal::boxSep(BMG);

    int total = 0, ci = 0;
    for (int i = 0; i < db_.bookingCount(); i++)
    {
        Booking *bk = db_.bookingAt(i);
        if (bk->getFlightNo() != fno || bk->getStatus() != "CONFIRMED")
            continue;
        total++;
        if (bk->isCheckedIn())
            ci++;
        const char *chk = bk->isCheckedIn() ? (BGR "✔" RS) : (BRD "✘" RS);
        char b[200];
        snprintf(b, 200, "  %s %-22s Seat:%-5s  Class:%-16s",
                 chk, bk->getPassengerName().c_str(), bk->getSeatNo().c_str(), bk->getCabin().c_str());
        Terminal::boxRow(BMG, b);
    }
    Terminal::boxSep(BMG);
    {
        char b[100];
        snprintf(b, 100, "  " DIM "Checked-In: " RS BGR "%d" RS "/" BWH "%d" RS, ci, total);
        Terminal::boxRowC(BMG, b);
    }
    Terminal::boxBot(BMG);

    printf("\n  " BCY "[S]" RS " Scan ticket   " BCY "[B]" RS " Start boarding   " BCY "[D]" RS " Mark departed\n");
    string ch = Input::field("Action:");
    if (ch == "S" || ch == "s")
    {
        string ref = Input::field("Boarding pass ref:");
        SoundEngine::scan();
        int bi = db_.findBookingByRef(ref);
        if (bi >= 0 && db_.bookingAt(bi)->getFlightNo() == fno)
        {
            if (!db_.bookingAt(bi)->isCheckedIn())
                Terminal::warnMsg("Passenger NOT checked in!");
            else
            {
                char b[80];
                snprintf(b, 80, "Passenger %s — BOARDING APPROVED", db_.bookingAt(bi)->getPassengerName().c_str());
                Terminal::success(b);
            }
        }
        else
            Terminal::errMsg("Booking not found.");
    }
    else if (ch == "B" || ch == "b")
    {
        fl.setStatus("BOARDING");
        db_.saveAll();
        Terminal::success(("Flight " + fno + " — BOARDING STARTED").c_str());
    }
    else if (ch == "D" || ch == "d")
    {
        fl.setStatus("DEPARTED");
        db_.saveAll();
        Terminal::success(("Flight " + fno + " — MARKED DEPARTED").c_str());
    }
    Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────
  CheckInPanel
──────────────────────────────────────────────────────────────────────────────*/
void CheckInPanel::execute()
{
    Presentation::printBanner();
    Terminal::sectionHeader("CHECK-IN OPERATIONS", BMG);
    string fno = Input::field("Flight Number:");
    int fi = db_.findFlightByNo(fno);
    if (fi < 0)
    {
        Terminal::errMsg("Flight not found.");
        Terminal::waitEnter();
        return;
    }
    Flight &fl = *db_.flightAt(fi);

    int ci = 0, total = 0;
    for (int i = 0; i < db_.bookingCount(); i++)
    {
        Booking *b = db_.bookingAt(i);
        if (b->getFlightNo() != fno || b->getStatus() != "CONFIRMED")
            continue;
        total++;
        if (b->isCheckedIn())
            ci++;
    }
    printf("\n  Flight: " BWH "%s" RS "  %s → %s  Gate %d\n",
           fl.getFlightNo().c_str(), fl.getOrigin().c_str(), fl.getDest().c_str(), fl.getGate());
    printf("  Check-in: " BGR "%d" RS " / " BWH "%d" RS " passengers\n\n", ci, total);
    printf("  " BCY "[F]" RS " Force check-in   " BCY "[V]" RS " View all   " BCY "[B]" RS " Back\n");
    string ch = Input::field("Action:");

    if (ch == "F" || ch == "f")
    {
        string ref = Input::field("Booking Reference:");
        int bi = db_.findBookingByRef(ref);
        if (bi < 0)
            Terminal::errMsg("Booking not found.");
        else if (db_.bookingAt(bi)->getFlightNo() == fno)
        {
            db_.bookingAt(bi)->setCheckedIn(true);
            db_.saveAll();
            Terminal::success("Passenger checked in manually.");
            SoundEngine::scan();
        }
        else
            Terminal::errMsg("Booking not on this flight.");
    }
    else if (ch == "V" || ch == "v")
    {
        for (int i = 0; i < db_.bookingCount(); i++)
        {
            Booking *b = db_.bookingAt(i);
            if (b->getFlightNo() != fno || b->getStatus() != "CONFIRMED")
                continue;
            const char *sc2 = b->isCheckedIn() ? (BGR "✔ In" RS) : (BYL "⏳ Out" RS);
            printf("  %s %-22s Seat:%-5s\n", sc2, b->getPassengerName().c_str(), b->getSeatNo().c_str());
        }
    }
    Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────
  BaggagePanel
──────────────────────────────────────────────────────────────────────────────*/
void BaggagePanel::execute()
{
    Presentation::printBanner();
    Terminal::sectionHeader("BAGGAGE OPERATIONS", BMG);
    printf("  " BCY "[1]" RS " View bags by flight\n");
    printf("  " BCY "[2]" RS " Update bag status\n");
    printf("  " BCY "[3]" RS " Report missing bag\n");
    printf("  " BCY "[4]" RS " Weight check\n");
    printf("  " BCY "[0]" RS " Back\n\n");
    int ch = Input::integer("Choice:", 0, 4);

    if (ch == 1)
    {
        string fno = Input::field("Flight Number:");
        bool found = false;
        for (int i = 0; i < db_.bagCount(); i++)
        {
            BaggageItem *bg = db_.bagAt(i);
            if (bg->getFlightNo() == fno)
            {
                bg->display();
                found = true;
            }
        }
        if (!found)
            Terminal::infoMsg("No bags on record for this flight.");
    }
    else if (ch == 2)
    {
        string tag = Input::field("Bag Tag:");
        bool found = false;
        for (int i = 0; i < db_.bagCount(); i++)
        {
            BaggageItem *bg = db_.bagAt(i);
            if (bg->getTagNo() != tag)
                continue;
            printf("  Current: %s%s" RS "\n", BAG_COL[bg->getStageIdx()], BAG_ST[bg->getStageIdx()]);
            for (int j = 0; j < BAG_STAGES; j++)
                printf("  " BCY "[%d] " RS "%s\n", j + 1, BAG_ST[j]);
            int sc = Input::integer("New stage:", 1, BAG_STAGES);
            bg->setStageIdx(sc - 1);
            if (sc == 7)
                bg->setLocation("Arrival Hall");
            else if (sc == 5)
                bg->setLocation("Cargo Hold");
            else if (sc == 4)
                bg->setLocation("Sorting Belt");
            db_.saveAll();
            Terminal::success("Bag status updated.");
            SoundEngine::scan();
            found = true;
            break;
        }
        if (!found)
            Terminal::errMsg("Tag not found.");
    }
    else if (ch == 3)
    {
        string tag = Input::field("Bag Tag:");
        for (int i = 0; i < db_.bagCount(); i++)
        {
            BaggageItem *bg = db_.bagAt(i);
            if (bg->getTagNo() == tag)
            {
                bg->setStageIdx(0);
                bg->setLocation("MISSING — Investigation Open");
                db_.saveAll();
                Terminal::warnMsg("Bag reported MISSING. Passenger will be notified.");
                break;
            }
        }
    }
    else if (ch == 4)
    {
        string fno = Input::field("Flight Number:");
        double total = 0;
        int count = 0;
        for (int i = 0; i < db_.bagCount(); i++)
        {
            BaggageItem *bg = db_.bagAt(i);
            if (bg->getFlightNo() != fno)
                continue;
            total += bg->getWeightKg();
            count++;
            if (bg->isOverweight())
            {
                string _w = "OVERWEIGHT bag: ";
                _w += bg->getTagNo();
                Terminal::warnMsg(_w.c_str());
            }
        }
        char b[100];
        snprintf(b, 100, "%d bags, total %.1fkg for flight %s.", count, total, fno.c_str());
        Terminal::infoMsg(b);
    }
    if (ch != 0)
        Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────
  SupervisorPanel
──────────────────────────────────────────────────────────────────────────────*/
void SupervisorPanel::execute()
{
    for (;;)
    {
        Presentation::printBanner();
        Terminal::sectionHeader("SUPERVISOR PANEL", BMG);
        printf("  " BCY "[1]" RS " View all flights\n");
        printf("  " BCY "[2]" RS " Update flight status\n");
        printf("  " BCY "[3]" RS " View staff on duty\n");
        printf("  " BCY "[4]" RS " Baggage overview\n");
        printf("  " BCY "[0]" RS " Back\n\n");
        int ch = Input::integer("Choice:", 0, 4);
        if (ch == 0)
            return;

        if (ch == 1)
        {
            for (int i = 0; i < db_.flightCount(); i++)
                if (db_.flightAt(i)->isActive())
                    db_.flightAt(i)->displayCard(i);
            Terminal::waitEnter();
        }
        else if (ch == 2)
        {
            string fno = Input::field("Flight:");
            int fi = db_.findFlightByNo(fno);
            if (fi < 0)
                Terminal::errMsg("Not found.");
            else
            {
                printf("  " BCY "[1]" RS " ON_TIME  [2] DELAYED  [3] BOARDING  [4] DEPARTED  [5] CANCELLED\n");
                int sc = Input::integer("Status:", 1, 5);
                const char *sts[] = {"ON_TIME", "DELAYED", "BOARDING", "DEPARTED", "CANCELLED"};
                db_.flightAt(fi)->setStatus(sts[sc - 1]);
                db_.saveAll();
                Terminal::success("Status updated.");
            }
            Terminal::waitEnter();
        }
        else if (ch == 3)
        {
            printf("\n  " BOLD CY "%-10s %-20s %-20s %-14s\n" RS, "EMP ID", "NAME", "ROLE", "TERMINAL");
            printf("  " DIM "───────────────────────────────────────────────────────────────────\n" RS);
            for (int i = 0; i < db_.staffCount(); i++)
            {
                Staff *s = db_.staffAt(i);
                if (!s->isActive())
                    continue;
                printf("  " BCY "%-10s" RS WH "%-20s %s%-20s" RS WH "%-14s\n" RS,
                       s->getEmpId().c_str(), s->getFullName().c_str(),
                       SROLE_COL[s->getRoleCode()], s->getRoleName(), s->getTerminal().c_str());
            }
            Terminal::waitEnter();
        }
        else if (ch == 4)
        {
            int stageCounts[BAG_STAGES] = {};
            for (int i = 0; i < db_.bagCount(); i++)
                stageCounts[db_.bagAt(i)->getStageIdx()]++;
            printf("\n");
            Terminal::boxTop(BCY);
            for (int i = 0; i < BAG_STAGES; i++)
            {
                char b[100];
                snprintf(b, 100, "  %s%-20s" RS " : " BWH "%d bags" RS, BAG_COL[i], BAG_ST[i], stageCounts[i]);
                Terminal::boxRowC(BCY, b);
            }
            Terminal::boxBot(BCY);
            Terminal::waitEnter();
        }
    }
}

/*──────────────────────────────────────────────────────────────────────────────
  TicketPanel
──────────────────────────────────────────────────────────────────────────────*/
void TicketPanel::execute()
{
    Presentation::printBanner();
    Terminal::sectionHeader("TICKET VERIFICATION", BMG);
    printf("  " DIM "Scan or enter boarding pass reference to verify.\n\n" RS);
    string ref = Input::field("Boarding Pass Ref:");
    SoundEngine::scan();
    int bi = db_.findBookingByRef(ref);
    if (bi < 0)
    {
        Terminal::errMsg("Booking NOT FOUND — deny boarding!");
        Terminal::waitEnter();
        return;
    }

    Booking *bk = db_.bookingAt(bi);
    const char *bc = (bk->getStatus() == "CONFIRMED") ? BGR : BRD;
    printf("\n");
    Terminal::boxTop(bc);
    {
        char b[200];
        snprintf(b, 200, "  PASSENGER : " BWH "%s" RS, bk->getPassengerName().c_str());
        Terminal::boxRow(bc, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  FLIGHT    : " BWH "%s" RS "   SEAT: " BWH "%s" RS, bk->getFlightNo().c_str(), bk->getSeatNo().c_str());
        Terminal::boxRowC(bc, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  STATUS    : %s%s" RS, (bk->getStatus() == "CONFIRMED") ? (BGR BOLD) : (BRD BOLD), bk->getStatus().c_str());
        Terminal::boxRowC(bc, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  CHECKED-IN: %s", bk->isCheckedIn() ? (BGR "YES — ALLOW BOARDING" RS) : (BRD "NO — SEND TO CHECK-IN" RS));
        Terminal::boxRowC(bc, b);
    }
    Terminal::boxBot(bc);
    if (bk->getStatus() == "CONFIRMED" && bk->isCheckedIn())
        SoundEngine::ok();
    else
        SoundEngine::error();
    Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────
  SecurityPanel
──────────────────────────────────────────────────────────────────────────────*/
void SecurityPanel::execute()
{
    Presentation::printBanner();
    Terminal::sectionHeader("SECURITY OPERATIONS", BMG);
    printf("  " BCY "[1]" RS " Verify passenger ticket & ID\n");
    printf("  " BCY "[2]" RS " Random screening log\n");
    printf("  " BCY "[3]" RS " Prohibited items report\n");
    printf("  " BCY "[0]" RS " Back\n\n");
    int ch = Input::integer("Choice:", 0, 3);

    if (ch == 1)
    {
        string ref = Input::field("Booking Reference:");
        int bi = db_.findBookingByRef(ref);
        if (bi < 0)
            Terminal::errMsg("Booking not found.");
        else
        {
            Booking *bk = db_.bookingAt(bi);
            printf("\n");
            Terminal::boxTop(BGR);
            {
                char b[200];
                snprintf(b, 200, "  " BWH "Passenger : %s" RS, bk->getPassengerName().c_str());
                Terminal::boxRowC(BGR, b);
            }
            {
                char b[200];
                snprintf(b, 200, "  Flight   : %s   Seat: %s", bk->getFlightNo().c_str(), bk->getSeatNo().c_str());
                Terminal::boxRow(BGR, b);
            }
            {
                char b[200];
                snprintf(b, 200, "  Status   : %s%s" RS, (bk->getStatus() == "CONFIRMED") ? BGR : BRD, bk->getStatus().c_str());
                Terminal::boxRowC(BGR, b);
            }
            {
                char b[200];
                snprintf(b, 200, "  CheckIn  : %s", bk->isCheckedIn() ? "YES" : "NO");
                Terminal::boxRow(BGR, b);
            }
            Terminal::boxBot(BGR);
            SoundEngine::scan();
        }
    }
    else if (ch == 2)
    {
        Terminal::success("Screening log entry recorded.");
    }
    else if (ch == 3)
    {
        string desc = Input::field("Prohibited item found (description):");
        char b[100];
        snprintf(b, 100, "Item reported: %s. Security notified.", desc.c_str());
        Terminal::warnMsg(b);
        SoundEngine::alert();
    }
    if (ch != 0)
        Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────
  LoungePanel
──────────────────────────────────────────────────────────────────────────────*/
void LoungePanel::execute()
{
    Presentation::printBanner();
    Terminal::sectionHeader("EXECUTIVE LOUNGE SERVICES", BMG);
    printf("  " BCY "[1]" RS " Verify lounge access\n");
    printf("  " BCY "[2]" RS " Log complimentary service\n");
    printf("  " BCY "[0]" RS " Back\n\n");
    int ch = Input::integer("Choice:", 0, 2);

    if (ch == 1)
    {
        string ref = Input::field("Booking Ref:");
        int bi = db_.findBookingByRef(ref);
        if (bi < 0)
            Terminal::errMsg("Booking not found.");
        else
        {
            Booking *bk = db_.bookingAt(bi);
            bool ok = (bk->getCabin() == "FIRST" || bk->getCabin() == "BUSINESS");
            if (ok)
            {
                Terminal::success("Lounge access GRANTED — First or Business class.");
                SoundEngine::ok();
            }
            else
            {
                /* CONCEPT [7] Template findIf — search passengers by predicate */
                Passenger *p = db_.passengers().findIf([&](Passenger *px)
                                                       { return px->getId() == bk->getPassengerId(); });
                bool tierOk = p && (p->getLoyaltyTier() == "GOLD" || p->getLoyaltyTier() == "PLATINUM");
                if (tierOk)
                    Terminal::success("Lounge access GRANTED — Elite tier member.");
                else
                    Terminal::errMsg("Lounge access DENIED — Economy / Bronze / Silver tier.");
            }
        }
    }
    else if (ch == 2)
    {
        string svc = Input::field("Service rendered:");
        Terminal::success(("Service logged: " + svc).c_str());
    }
    if (ch != 0)
        Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────
  GroundCrewPanel
──────────────────────────────────────────────────────────────────────────────*/
void GroundCrewPanel::execute()
{
    Presentation::printBanner();
    Terminal::sectionHeader("GROUND OPERATIONS", BMG);
    string fno = Input::field("Flight Number:");
    int fi = db_.findFlightByNo(fno);
    if (fi < 0)
    {
        Terminal::errMsg("Flight not found.");
        Terminal::waitEnter();
        return;
    }
    Flight *fl = db_.flightAt(fi);
    printf("\n  " BWH "Aircraft: %s" RS "   Gate: %d   Status: %s" BOLD "%s" RS "\n\n",
           fl->getAircraft().c_str(), fl->getGate(),
           Utils::statusColor(fl->getStatus().c_str()), fl->getStatus().c_str());
    printf("  " BCY "[1]" RS " Log aircraft ready   " BCY "[2]" RS " Fueling complete   " BCY "[3]" RS " Pushback authorised\n");
    int ch = Input::integer("Action:", 1, 3);
    if (ch == 1)
        Terminal::success("Aircraft ready for boarding logged.");
    else if (ch == 2)
    {
        SoundEngine::scan();
        Terminal::success("Fueling complete. Quantity logged.");
    }
    else if (ch == 3)
    {
        fl->setStatus("DEPARTED");
        db_.saveAll();
        SoundEngine::fly();
        Terminal::success("Pushback authorised. Flight departed.");
    }
    Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────
  DispatchPanel
──────────────────────────────────────────────────────────────────────────────*/
void DispatchPanel::execute()
{
    Presentation::printBanner();
    Terminal::sectionHeader("FLIGHT DISPATCH", BMG);
    string fno = Input::field("Flight Number:");
    int fi = db_.findFlightByNo(fno);
    if (fi < 0)
    {
        Terminal::errMsg("Flight not found.");
        Terminal::waitEnter();
        return;
    }
    Flight *fl = db_.flightAt(fi);
    printf("\n  Flight: " BWH "%s" RS "  Status: %s" BOLD "%s" RS "\n\n",
           fl->getFlightNo().c_str(), Utils::statusColor(fl->getStatus().c_str()), fl->getStatus().c_str());
    printf("  " BCY "[1]" RS " Update status   " BCY "[2]" RS " Set delay   " BCY "[3]" RS " Clear for departure\n");
    int ch = Input::integer("Action:", 1, 3);
    if (ch == 1)
    {
        printf("  " BCY "[1]" RS " ON_TIME  [2] DELAYED  [3] BOARDING  [4] DEPARTED  [5] LANDED\n");
        int sc = Input::integer("Status:", 1, 5);
        const char *sts[] = {"ON_TIME", "DELAYED", "BOARDING", "DEPARTED", "LANDED"};
        fl->setStatus(sts[sc - 1]);
        db_.saveAll();
        Terminal::success(("Flight " + fno + " status → " + string(sts[sc - 1])).c_str());
    }
    else if (ch == 2)
    {
        string reason = Input::field("Delay reason:");
        fl->setDelayReason(reason);
        fl->setStatus("DELAYED");
        db_.saveAll();
        Terminal::warnMsg(("Flight " + fno + " delayed: " + reason).c_str());
    }
    else if (ch == 3)
    {
        printf("  " BWH "%d passengers on board. Clear for departure?\n" RS, fl->totalBooked());
        string conf = Input::field("[YES/NO]:");
        if (conf == "YES")
        {
            fl->setStatus("DEPARTED");
            db_.saveAll();
            Terminal::success("Flight cleared for departure.");
            SoundEngine::fly();
        }
    }
    Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────
  CustomsPanel
──────────────────────────────────────────────────────────────────────────────*/
void CustomsPanel::execute()
{
    Presentation::printBanner();
    Terminal::sectionHeader("CUSTOMS & IMMIGRATION", BMG);
    string ref = Input::field("Passport / Booking Ref:");
    int bi = db_.findBookingByRef(ref);
    if (bi < 0)
    {
        Terminal::errMsg("Booking not found.");
        Terminal::waitEnter();
        return;
    }

    Booking *bk = db_.bookingAt(bi);
    /* CONCEPT [7] Template findIf — locate passenger by ID predicate */
    Passenger *p = db_.passengers().findIf([&](Passenger *px)
                                           { return px->getId() == bk->getPassengerId(); });
    if (!p)
    {
        Terminal::errMsg("Passenger record not found.");
        Terminal::waitEnter();
        return;
    }

    printf("\n");
    Terminal::boxTop(BCY);
    {
        char b[200];
        snprintf(b, 200, "  Name        : " BWH "%s" RS, p->getFullName().c_str());
        Terminal::boxRow(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Nationality : " BWH "%s" RS, p->getNationality().c_str());
        Terminal::boxRow(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Passport    : " BWH "%s" RS, p->getPassport().c_str());
        Terminal::boxRow(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  DOB         : " BWH "%s" RS, p->getDob().c_str());
        Terminal::boxRow(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Destination : " BWH "%s" RS, bk->getFlightNo().c_str());
        Terminal::boxRow(BCY, b);
    }
    Terminal::boxBot(BCY);
    SoundEngine::scan();

    printf("  " BCY "[A]" RS " Admit   " BCY "[F]" RS " Flag for inspection   " BCY "[D]" RS " Deny entry\n");
    string dec = Input::field("Decision:");
    if (dec == "A" || dec == "a")
        Terminal::success("Passenger admitted. Immigration stamp issued.");
    else if (dec == "F" || dec == "f")
    {
        Terminal::warnMsg("Passenger flagged. Escort to secondary inspection.");
        SoundEngine::warn();
    }
    else if (dec == "D" || dec == "d")
    {
        Terminal::errMsg("Entry DENIED. Deportation process initiated.");
        SoundEngine::error();
    }
    Terminal::waitEnter();
}

/*══════════════════════════════════════════════════════════════════════════════
  PassengerDashboard — full passenger feature set
══════════════════════════════════════════════════════════════════════════════*/
void PassengerDashboard::bookFlight()
{
    Passenger *p = db_.passAt(sess_.getIdx());
    Presentation::printBanner();
    Terminal::sectionHeader("BOOK A FLIGHT", BMG);

    int shown = 0;
    for (int i = 0; i < db_.flightCount(); i++)
        if (db_.flightAt(i)->isActive())
        {
            db_.flightAt(i)->displayCard(shown);
            shown++;
        }
    if (!shown)
    {
        Terminal::infoMsg("No active flights available.");
        Terminal::waitEnter();
        return;
    }

    int ch = Input::integer("Select flight [1-N] or 0 to cancel:", 0, shown);
    if (ch < 1)
    {
        Terminal::infoMsg("Booking cancelled.");
        Terminal::waitEnter();
        return;
    }

    int fIdx = -1, cnt = 0;
    for (int i = 0; i < db_.flightCount(); i++)
        if (db_.flightAt(i)->isActive())
        {
            cnt++;
            if (cnt == ch)
            {
                fIdx = i;
                break;
            }
        }
    if (fIdx < 0)
    {
        Terminal::errMsg("Invalid.");
        Terminal::waitEnter();
        return;
    }
    Flight *fl = db_.flightAt(fIdx);

    /* CONCEPT [3][5] Abstract SeatSelector — polymorphic seat selection */
    InteractiveSeatSelector selector;
    int outRow = -1, outCol = -1;
    double seatBase = 0;
    string seatNo;
    if (!selector.selectSeat(fl->getSeatMap(), *fl, outRow, outCol, seatBase, seatNo))
    {
        Terminal::infoMsg("Booking cancelled.");
        Terminal::waitEnter();
        return;
    }
    string cabin = string(SeatMap::cabinForRow(outRow));
    double seatFee = BillingEngine::calcSeatFee(outRow, *fl);

    /* ── Baggage ──────────────────────────────────────────────────────────── */
    Presentation::printBanner();
    Terminal::sectionHeader("BAGGAGE SELECTION — STEP 2", BMG);
    Terminal::boxTop(BMG);
    Terminal::boxCenter(BMG, BMG "  🧳  CHECKED BAGGAGE OPTIONS  " RS);
    Terminal::boxSep(BMG);
    Terminal::boxRowC(BMG, "  " BCY "[0]" RS "  Hand luggage only  (free)  " DIM "7kg cabin bag included" RS);
    Terminal::boxRowC(BMG, "  " BCY "[1]" RS "  1 bag  " BGR "+$30" RS "        " DIM "up to 23kg" RS);
    Terminal::boxRowC(BMG, "  " BCY "[2]" RS "  2 bags " BGR "+$55" RS "        " DIM "up to 23kg each" RS);
    Terminal::boxRowC(BMG, "  " BCY "[3]" RS "  3 bags " BGR "+$75" RS "        " DIM "up to 23kg each" RS);
    Terminal::boxBot(BMG);
    int bags = Input::integer("Bags (0-3):", 0, 3);
    double bagFees[] = {0, 30, 55, 75};
    double bagFee = bagFees[bags];
    bool fragile = false, special = false;
    if (bags > 0)
    {
        printf("  " BYL " ▶ " RS BWH "Any fragile items? [Y/N]: " RS " " BCY);
        fflush(stdout);
        char c;
        scanf(" %c", &c);
        Utils::clearInput();
        printf(RS);
        fragile = (tolower(c) == 'y');
        SoundEngine::tick();
        printf("  " BYL " ▶ " RS BWH "Oversized/sports equipment? [Y/N]: " RS " " BCY);
        fflush(stdout);
        scanf(" %c", &c);
        Utils::clearInput();
        printf(RS);
        special = (tolower(c) == 'y');
        SoundEngine::tick();
        if (special)
            bagFee += 30;
    }

    /* ── Meal ─────────────────────────────────────────────────────────────── */
    Presentation::printBanner();
    Terminal::sectionHeader("MEAL SELECTION — STEP 3", BMG);
    const char *meals[] = {
        "No Preference (Standard)", "Vegetarian (VGML)", "Vegan (VOML)",
        "Halal (MOML)", "Kosher (KSML)", "Gluten-Free (GFML)", "Diabetic (DBML)", "Child Meal (CHML)"};
    double mealFees[] = {0, 0, 0, 5, 8, 10, 10, 0};
    Terminal::boxTop(BMG);
    Terminal::boxCenter(BMG, BMG "  🍽  MEAL PREFERENCE  " RS);
    Terminal::boxSep(BMG);
    for (int i = 0; i < 8; i++)
    {
        char b[100];
        snprintf(b, 100, "  " BCY "[%d] " RS BWH "%s" RS "  " DIM "%s" RS, i + 1, meals[i], mealFees[i] > 0 ? "(+fee)" : "(free)");
        Terminal::boxRowC(BMG, b);
    }
    Terminal::boxBot(BMG);
    int mc = Input::integer("Select meal [1-8]:", 1, 8) - 1;
    string meal = meals[mc];
    double mealFee = mealFees[mc];

    /* ── Special requests ─────────────────────────────────────────────────── */
    Presentation::printBanner();
    Terminal::sectionHeader("SPECIAL REQUESTS — STEP 4", BMG);
    string specialReq = Input::field("Special Request (wheelchair/medical/other)", "or press Enter to skip");

    /* ── CONCEPT [11] FareBreakdown composition ───────────────────────────── */
    double baseFare = seatBase;
    double serviceFee = 15.0;
    double subtotal = baseFare + seatFee + bagFee + mealFee + serviceFee;
    double taxes = subtotal * (fl->getTaxRate() / 100.0);
    double discount = p->getLoyaltyDiscount(subtotal);
    double grandTotal = subtotal + taxes - discount;
    FareBreakdown fare(baseFare, seatFee, bagFee, mealFee, serviceFee, taxes, discount);
    fare.grandTotal = grandTotal;

    /* ── Review ───────────────────────────────────────────────────────────── */
    Presentation::printBanner();
    Terminal::sectionHeader("BOOKING REVIEW — STEP 5", BMG);
    Terminal::boxTop(BMG);
    Terminal::boxCenter(BMG, BMG BOLD "  📋  BOOKING SUMMARY  " RS);
    Terminal::boxSep(BMG);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Flight   : " RS BWH "%s  %s → %s" RS, fl->getFlightNo().c_str(), fl->getOrigin().c_str(), fl->getDest().c_str());
        Terminal::boxRowC(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Date     : " RS BWH "%s  Dep:%s" RS, fl->getDate().c_str(), fl->getDepTime().c_str());
        Terminal::boxRowC(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Seat     : " RS BYL "%s" RS "   Class: " BCY "%s" RS, seatNo.c_str(), cabin.c_str());
        Terminal::boxRowC(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Meal     : " RS BWH "%s" RS, meal.c_str());
        Terminal::boxRowC(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Bags     : " RS BWH "%d checked bag(s)" RS, bags);
        Terminal::boxRowC(BMG, b);
    }
    Terminal::boxSep(BMG);
    {
        char b[100];
        snprintf(b, 100, "  %-34s $%.2f", "Base Fare:", baseFare);
        Terminal::boxRow(BMG, b);
    }
    {
        char b[100];
        snprintf(b, 100, "  %-34s $%.2f", "Seat Fee:", seatFee);
        Terminal::boxRow(BMG, b);
    }
    if (bagFee > 0)
    {
        char b[100];
        snprintf(b, 100, "  %-34s $%.2f", "Baggage:", bagFee);
        Terminal::boxRow(BMG, b);
    }
    if (mealFee > 0)
    {
        char b[100];
        snprintf(b, 100, "  %-34s $%.2f", "Meal:", mealFee);
        Terminal::boxRow(BMG, b);
    }
    {
        char b[100];
        snprintf(b, 100, "  %-34s $%.2f", "Service Fee:", serviceFee);
        Terminal::boxRow(BMG, b);
    }
    {
        char b[100];
        snprintf(b, 100, "  %-34s $%.2f", "Taxes (16.72%):", taxes);
        Terminal::boxRow(BMG, b);
    }
    if (discount > 0)
    {
        char b[100];
        snprintf(b, 100, "  %-34s -$%.2f", "Loyalty Discount:", discount);
        Terminal::boxRow(BMG, b);
    }
    Terminal::boxSep(BMG);
    {
        char b[100];
        snprintf(b, 100, "  " BOLD "%-34s " BGR "$%.2f" RS, "GRAND TOTAL:", grandTotal);
        Terminal::boxRowC(BMG, b);
    }
    Terminal::boxBot(BMG);

    if (Input::field("Type CONFIRM to proceed to payment:") != "CONFIRM")
    {
        Terminal::infoMsg("Booking cancelled.");
        Terminal::waitEnter();
        return;
    }

    /* ── Payment ──────────────────────────────────────────────────────────── */
    string payMethod, cardLast4;
    if (!BillingEngine::payWizard(grandTotal, payMethod, cardLast4, sess_.getIdx()))
    {
        Terminal::infoMsg("Payment not completed. Booking cancelled.");
        Terminal::waitEnter();
        return;
    }

    Terminal::progressBar("Finalising your booking", 1000);
    Terminal::spinner("Issuing E-Ticket...", 600);

    /* Mark seat booked on the seatmap */
    fl->getSeatMap().book(outRow, outCol, p->getFullName());
    fl->bookSeat(cabin);

    /* Award miles */
    int milesEarned = fl->getDuration() * 8;
    p->addMiles(milesEarned);

    /* Create & persist booking */
    char gateStr[8];
    snprintf(gateStr, 8, "%d", fl->getGate());
    Booking bk(p->getId(), fl->getId(), fl->getFlightNo(), p->getFullName(),
               cabin, seatNo, meal, fare, payMethod, cardLast4,
               string(gateStr), bags, specialReq);
    Booking *bkPtr = db_.addBooking(bk);

    /* Create bag records */
    for (int nb = 0; nb < bags; nb++)
    {
        string tag = Utils::generateTag(fl->getFlightNo());
        BaggageItem bg(bkPtr->getId(), bkPtr->getBookingRef(),
                       p->getFullName(), fl->getFlightNo(), tag, fragile, special);
        db_.addBag(bg);
    }
    db_.saveAll();

    /* ── E-Ticket confirmation ────────────────────────────────────────────── */
    Presentation::printBanner();
    Terminal::sectionHeader("BOOKING CONFIRMED!", BGR);
    Terminal::boxTop(BGR);
    Terminal::boxCenter(BGR, BOLD BGR "✔  E-TICKET ISSUED SUCCESSFULLY  ✔" RS);
    Terminal::boxSep(BGR);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Booking Ref : " RS BOLD BWH "%s" RS, bkPtr->getBookingRef().c_str());
        Terminal::boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Ticket No   : " RS BWH "TKT-%d" RS, bkPtr->getId());
        Terminal::boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Passenger   : " RS BWH "%s" RS, bkPtr->getPassengerName().c_str());
        Terminal::boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Frequent Flyer: " RS "%s" BOLD "%s" RS, Utils::tierColor(p->getLoyaltyTier().c_str()), p->getLoyaltyTier().c_str());
        Terminal::boxRowC(BGR, b);
    }
    Terminal::boxSep(BGR);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Flight      : " RS BWH "%s  %s → %s" RS, fl->getFlightNo().c_str(), fl->getOrigin().c_str(), fl->getDest().c_str());
        Terminal::boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Seat        : " RS BYL "%s  %s" RS, seatNo.c_str(), cabin.c_str());
        Terminal::boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Gate/Group  : " RS BWH "%s  Group 2" RS, gateStr);
        Terminal::boxRowC(BGR, b);
    }
    Terminal::boxSep(BGR);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Grand Total : " RS BGR BOLD "$%.2f" RS "  via %s", grandTotal, payMethod.c_str());
        Terminal::boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Miles Earned: " RS BYL "+%d Nexus Miles" RS, milesEarned);
        Terminal::boxRowC(BGR, b);
    }
    Terminal::boxSep(BGR);
    Terminal::boxRowC(BGR, "  " YL "⚠  Check-in opens 24 hrs before departure" RS);
    Terminal::boxRowC(BGR, "  " YL "⚠  Arrive at airport 3 hours before departure" RS);
    Terminal::boxBot(BGR);

    /* CONCEPT [10] Friend class prints bill */
    BillingEngine::printBill(*bkPtr, *fl);
    Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────*/
void PassengerDashboard::viewBookings()
{
    Passenger *p = db_.passAt(sess_.getIdx());
    Presentation::printBanner();
    char h[80];
    snprintf(h, 80, "MY BOOKINGS — %s", p->getFullName().c_str());
    Terminal::sectionHeader(h, BCY);

    int found = 0;
    for (int i = 0; i < db_.bookingCount(); i++)
    {
        Booking *bk = db_.bookingAt(i);
        if (bk->getPassengerId() != p->getId())
            continue;
        printf("\n  " BCY "[%d] " RS BOLD BWH "✈ %s" RS "\n", ++found, bk->getFlightNo().c_str());
        bk->displayCard(found);
    }
    if (!found)
        Terminal::infoMsg("No bookings found.");

    printf("\n  " BCY "[C]" RS " Cancel booking   " BCY "[B]" RS " View bill   " BCY "[ENTER]" RS " Back\n");
    string ch = Input::field("Choice (C/B/Enter):");
    if (ch == "C" || ch == "c")
    {
        string ref = Input::field("Booking Reference:");
        int bi = db_.findBookingByRef(ref);
        if (bi < 0 || db_.bookingAt(bi)->getPassengerId() != p->getId())
            Terminal::errMsg("Booking not found.");
        else if (db_.bookingAt(bi)->getStatus() == "CANCELLED")
            Terminal::infoMsg("Already cancelled.");
        else
        {
            Booking *bk = db_.bookingAt(bi);
            bk->setStatus("CANCELLED");
            /* CONCEPT [7] Template findIf to locate the flight */
            Flight *fl = db_.flights().findIf([&](Flight *f)
                                              { return f->getFlightNo() == bk->getFlightNo(); });
            if (fl)
            {
                SeatMap &sm = fl->getSeatMap();
                for (int r = 0; r < TOTAL_ROWS; r++)
                    for (int c = 0; c < 6; c++)
                        if (sm.getWho(r, c) == bk->getPassengerName())
                            sm.free(r, c);
                fl->unBookSeat(bk->getCabin());
            }
            db_.saveAll();
            Terminal::success(("Booking " + ref + " cancelled. Refund initiated.").c_str());
        }
    }
    else if (ch == "B" || ch == "b")
    {
        string ref = Input::field("Booking Reference for bill:");
        int bi = db_.findBookingByRef(ref);
        if (bi < 0 || db_.bookingAt(bi)->getPassengerId() != p->getId())
            Terminal::errMsg("Booking not found.");
        else
        {
            int fi = db_.findFlightByNo(db_.bookingAt(bi)->getFlightNo());
            if (fi >= 0)
                BillingEngine::printBill(*db_.bookingAt(bi), *db_.flightAt(fi));
        }
    }
    Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────*/
void PassengerDashboard::onlineCheckIn()
{
    Passenger *p = db_.passAt(sess_.getIdx());
    Presentation::printBanner();
    Terminal::sectionHeader("ONLINE CHECK-IN", BYL);

    vector<int> elig;
    for (int i = 0; i < db_.bookingCount(); i++)
    {
        Booking *b = db_.bookingAt(i);
        if (b->getPassengerId() == p->getId() &&
            b->getStatus() == "CONFIRMED" && !b->isCheckedIn())
            elig.push_back(i);
    }
    if (elig.empty())
    {
        Terminal::infoMsg("No flights available for check-in.");
        Terminal::waitEnter();
        return;
    }

    for (int e = 0; e < (int)elig.size(); e++)
    {
        Booking *bk = db_.bookingAt(elig[e]);
        printf("  " BCY "[%d] " RS BOLD "✈ %s" RS "   Ref: " BWH "%s" RS "   Seat: " BWH "%s" RS "\n\n",
               e + 1, bk->getFlightNo().c_str(), bk->getBookingRef().c_str(), bk->getSeatNo().c_str());
    }
    int ch = Input::integer("Select flight to check in:", 1, (int)elig.size());
    Booking *bk = db_.bookingAt(elig[ch - 1]);

    Terminal::sectionHeader("STEP 1 — IDENTITY VERIFICATION", BYL);
    printf("  Passenger : " BWH "%s" RS "\n  Passport  : " BWH "%s" RS "\n  DOB       : " BWH "%s" RS "\n\n",
           p->getFullName().c_str(), p->getPassport().c_str(), p->getDob().c_str());
    if (Input::field("Confirm details are correct [YES/NO]:") != "YES")
    {
        Terminal::infoMsg("Check-in cancelled.");
        Terminal::waitEnter();
        return;
    }

    Terminal::sectionHeader("STEP 2 — BAGGAGE DROP", BYL);
    if (bk->getBagsCount() > 0)
    {
        printf("  " BWH "You have %d checked bag(s) on this booking.\n" RS, bk->getBagsCount());
        printf("  " BCY "[1]" RS " Drop all bags now   " BCY "[2]" RS " Skip bag drop (airport only)\n");
        int bch = Input::integer("Choice:", 1, 2);
        if (bch == 1)
        {
            double totalW = 0;
            for (int i = 0; i < db_.bagCount(); i++)
            {
                BaggageItem *bg = db_.bagAt(i);
                if (bg->getBookingRef() != bk->getBookingRef())
                    continue;
                bg->setStageIdx(1);
                bg->setLocation("Check-in Counter");
                printf("  " BYL " ▶ " RS BWH "Weight for bag %s (kg): " RS " " BCY, bg->getTagNo().c_str());
                fflush(stdout);
                double w;
                scanf("%lf", &w);
                Utils::clearInput();
                printf(RS);
                if (w < 0.1 || w > 32)
                {
                    Terminal::warnMsg("Weight out of range. Using 20kg.");
                    w = 20;
                }
                bg->setWeightKg(w);
                totalW += w;
                SoundEngine::scan();
            }
            db_.saveAll();
            if (totalW > 0)
            {
                char b[80];
                snprintf(b, 80, "%.1fkg total bag weight recorded.", totalW);
                Terminal::infoMsg(b);
            }
        }
    }
    else
        Terminal::infoMsg("No checked bags on this booking.");

    Terminal::sectionHeader("STEP 3 — TRAVEL DOCUMENTS", BYL);
    printf("  " BWH "Destination Requirements:\n" RS);
    printf("  • Valid passport required  • Check visa requirements for destination\n\n");
    Input::field("Emergency Contact Name:");
    Input::phone("Emergency Contact Phone:");
    if (Input::field("Type CONFIRM to complete check-in:") != "CONFIRM")
    {
        Terminal::infoMsg("Check-in cancelled.");
        Terminal::waitEnter();
        return;
    }

    Terminal::spinner("Processing check-in...", 900);
    bk->setCheckedIn(true);
    db_.saveAll();

    /* Find gate */
    int gate = 0;
    Flight *fl = db_.flights().findIf([&](Flight *f)
                                      { return f->getFlightNo() == bk->getFlightNo(); });
    if (fl)
        gate = fl->getGate();

    /* Print boarding pass */
    Presentation::printBanner();
    Terminal::sectionHeader("BOARDING PASS", BGR);
    Terminal::boxTop(BCY);
    Terminal::boxCenter(BCY, BOLD BGR "✔  CHECK-IN COMPLETE — BOARDING PASS READY  ✔" RS);
    Terminal::boxSep(BCY);
    {
        char b[200];
        snprintf(b, 200, "  " BWH "NEXUS AIR" RS "              Flight: " BYL "%s" RS "   Gate: " BYL "%d" RS, bk->getFlightNo().c_str(), gate);
        Terminal::boxRowC(BCY, b);
    }
    Terminal::boxSep(BCY);
    {
        char b[200];
        snprintf(b, 200, "  " CY "PASSENGER   : " RS BOLD BWH "%s" RS, bk->getPassengerName().c_str());
        Terminal::boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " CY "SEAT        : " RS BOLD BYL "%s" RS "   CLASS: " BWH "%s" RS, bk->getSeatNo().c_str(), bk->getCabin().c_str());
        Terminal::boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " CY "BOARDING    : " RS BWH "Group %s" RS "   Seq: %03d" RS, bk->getBoardingGroup().c_str(), bk->getId() % 999);
        Terminal::boxRowC(BCY, b);
    }
    Terminal::boxSep(BCY);
    printf(BCY BV RS "  ");
    printf(BG_WH "\033[30m"
                 "  +----------+  " RS);
    Utils::spaces(TW - 18);
    printf(BCY BV RS "\n");
    printf(BCY BV RS "  ");
    printf(BG_WH "\033[30m"
                 "  | QR CODE |  " RS BCY "  [ SCAN AT GATE ]" RS);
    Utils::spaces(TW - 33);
    printf(BCY BV RS "\n");
    printf(BCY BV RS "  ");
    printf(BG_WH "\033[30m"
                 "  +----------+  " RS);
    Utils::spaces(TW - 18);
    printf(BCY BV RS "\n");
    Terminal::boxSep(BCY);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "BP-%s-%s-%04d" RS, bk->getFlightNo().c_str(), bk->getSeatNo().c_str(), bk->getId());
        Terminal::boxRowC(BCY, b);
    }
    Terminal::boxBot(BCY);
    Terminal::warnMsg("Please arrive at gate 45 minutes before departure.");
    Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────*/
void PassengerDashboard::loyaltyProgram()
{
    Passenger *p = db_.passAt(sess_.getIdx());
    p->updateTier();
    Presentation::printBanner();
    Terminal::sectionHeader("NEXUS MILES LOYALTY PROGRAM", BYL);
    printf("  " DIM "Member: " RS BWH "%s" RS "   ID: " BWH "NM%d" RS "\n\n", p->getFullName().c_str(), p->getId() * 7 + 654321);
    printf("  " DIM "Current Tier : " RS "%s" BOLD "%s" RS "\n", Utils::tierColor(p->getLoyaltyTier().c_str()), p->getLoyaltyTier().c_str());
    int target = (p->getLoyaltyTier() == "SILVER") ? 20000 : (p->getLoyaltyTier() == "GOLD") ? 50000
                                                                                             : 50000;
    int needed = (int)fmax(0, target - p->getTotalMiles());
    int bLen = 40, filled = target > 0 ? (int)fmin(bLen, 1.0 * p->getTotalMiles() / target * bLen) : bLen;
    printf("  " DIM "Miles to Next: " RS BYL "%d" RS "\n", needed);
    printf("  [");
    for (int i = 0; i < bLen; i++)
        printf("%s", i < filled ? (BGR "█" RS) : (DIM "░" RS));
    printf("]\n\n");
    printf("  " BYL "Tier Benefits:\n" RS);
    printf("  " YL "BRONZE" RS " → 0 miles     • 1x points\n");
    printf("  " BWH "SILVER" RS " → 5,000       • 1.25x points  • Priority check-in\n");
    printf("  " BYL "GOLD  " RS " → 20,000      • 1.5x points   • Lounge access  • Extra bag\n");
    printf("  " BCY "PLAT  " RS " → 50,000      • 2x points     • Suite upgrade  • Chauffeur\n\n");
    printf("  " BCY "[1] " RS "Redeem 10,000 pts for $100 discount\n");
    printf("  " BCY "[2] " RS "Seat Upgrade (8,000 pts)\n");
    printf("  " BCY "[0] " RS "Back\n\n");
    int ch = Input::integer("Select:", 0, 2);
    if (ch == 1 && p->getLoyaltyPts() >= 10000)
    {
        p->redeemPoints(10000);
        db_.saveAll();
        Terminal::success("10,000 pts redeemed! $100 discount on next booking.");
    }
    else if (ch == 2 && p->getLoyaltyPts() >= 8000)
    {
        p->redeemPoints(8000);
        db_.saveAll();
        Terminal::success("8,000 pts redeemed! Seat upgrade applied to next booking.");
    }
    else if (ch != 0)
        Terminal::warnMsg("Insufficient points or invalid option.");
    Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────*/
void PassengerDashboard::myProfile()
{
    Passenger *p = db_.passAt(sess_.getIdx());
    Presentation::printBanner();
    Terminal::sectionHeader("MY PROFILE", BMG);
    printf("  " CY "Full Name   : " RS BWH "%s" RS "\n", p->getFullName().c_str());
    printf("  " CY "Email       : " RS BWH "%s" RS "\n", p->getEmail().c_str());
    printf("  " CY "Phone       : " RS BWH "%s" RS "\n", p->getPhone().c_str());
    printf("  " CY "Passport    : " RS BWH "%s" RS "\n", p->getPassport().c_str());
    printf("  " CY "Nationality : " RS BWH "%s" RS "\n", p->getNationality().c_str());
    printf("  " CY "DOB         : " RS BWH "%s" RS "\n", p->getDob().c_str());
    printf("  " CY "Username    : " RS BWH "%s" RS "\n\n", p->getUsername().c_str());
    printf("  " BCY "[E]" RS " Edit  " BCY "[P]" RS " Change Password  " BCY "[B]" RS " Back\n");
    string ch = Input::field("Choice:");
    if (ch == "E" || ch == "e")
    {
        string em = Input::email("New Email:");
        string ph = Input::phone("New Phone:");
        p->setEmail(em);
        p->setPhone(ph);
        db_.saveAll();
        Terminal::success("Profile updated!");
    }
    else if (ch == "P" || ch == "p")
    {
        string op = Input::password("Current Password:");
        if (!p->authenticate(op))
            Terminal::errMsg("Wrong current password.");
        else
        {
            string np = Input::password("New Password:", true);
            string nc = Input::password("Confirm New Password:");
            if (np != nc)
                Terminal::errMsg("Passwords do not match.");
            else
            {
                p->setPassword(np);
                db_.saveAll();
                Terminal::success("Password changed!");
            }
        }
    }
    Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────*/
void PassengerDashboard::baggageTracking()
{
    Presentation::printBanner();
    Terminal::sectionHeader("BAGGAGE TRACKING", BCY);
    string ref = Input::field("Enter Booking Reference:");
    Terminal::spinner("Locating your baggage...", 700);
    int found = 0;
    for (int i = 0; i < db_.bagCount(); i++)
    {
        BaggageItem *bg = db_.bagAt(i);
        if (bg->getBookingRef() == ref)
        {
            bg->display();
            found++;
        }
    }
    if (!found)
        Terminal::infoMsg("No baggage found for this booking reference.");
    Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────*/
/* ══════════════════════════════════════════════════════════════
   ENTERTAINMENT HELPERS
══════════════════════════════════════════════════════════════*/

/* animated score bar */
static void animateScore(int score, int total)
{
    printf("\n  " DIM "Tallying score" RS);
    for (int i = 0; i < 3; i++)
    {
        Utils::sleepMs(350);
        printf(BYL "." RS);
        fflush(stdout);
    }
    Utils::sleepMs(400);
    printf("\r");
    Utils::spaces(TW);
    printf("\r");
    int pct = total > 0 ? score * 100 / total : 0;
    int bw = 40, filled = total > 0 ? bw * score / total : 0;
    printf("\n  Score: [");
    for (int i = 0; i < bw; i++)
    {
        const char *c = (i < filled) ? ((pct >= 75) ? BGR : (pct >= 50) ? BYL
                                                                        : BRD)
                                     : DIM;
        printf("%s%s" RS, c, i < filled ? "█" : "░");
        fflush(stdout);
        Utils::sleepMs(28);
    }
    printf("] " BOLD "%d/%d" RS " (%d%%)\n\n", score, total, pct);
}

/* per-question result */
static void questionResult(bool correct, const char *correctAns)
{
    if (correct)
    {
        SoundEngine::ok();
        const char *msgs[] = {"  ✅  CORRECT! Brilliant!", "  ✅  NAILED IT! +1 point",
                              "  ✅  SPOT ON! Great aviator!", "  ✅  PERFECT! You know your stuff!"};
        printf(BGR BOLD "\n%s\n" RS, msgs[rand() % 4]);
    }
    else
    {
        SoundEngine::error();
        const char *msgs[] = {"  ❌  Wrong! Don't worry — learn and grow.",
                              "  ❌  Not quite! The skies forgive mistakes.",
                              "  ❌  Oops! Every pilot learns from errors."};
        printf(BRD BOLD "\n%s\n" RS, msgs[rand() % 3]);
        printf("  " DIM "Correct answer: " RS BGR BOLD "%s\n" RS, correctAns);
    }
}

/* grade badge */
static void gradeBadge(int score, int total)
{
    int pct = total > 0 ? score * 100 / total : 0;
    Terminal::boxTop(BYL);
    Terminal::boxCenter(BYL, BYL BOLD "  🏆  RESULTS  🏆  " RS);
    Terminal::boxSep(BYL);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Score: " RS BGR BOLD "%d / %d" RS "  Accuracy: " BWH "%d%%" RS, score, total, pct);
        Terminal::boxRowC(BYL, b);
    }
    Terminal::boxSep(BYL);
    if (pct == 100)
    {
        Terminal::boxCenter(BYL, BCY BOLD "  🌟🌟🌟  PERFECT — AVIATION LEGEND!  🌟🌟🌟  " RS);
        Beep(1047, 120);
        Beep(1319, 120);
        Beep(1568, 120);
        Beep(2093, 300);
    }
    else if (pct >= 75)
    {
        Terminal::boxCenter(BYL, BGR BOLD "  🥇  AVIATION EXPERT  🥇  " RS);
    }
    else if (pct >= 50)
    {
        Terminal::boxCenter(BYL, BYL BOLD "  🥈  SKILLED AVIATOR  🥈  " RS);
    }
    else if (pct >= 25)
    {
        Terminal::boxCenter(BYL, BCY BOLD "  🥉  TRAINEE PILOT  🥉  " RS);
    }
    else
    {
        Terminal::boxCenter(BYL, DIM BOLD "  📚  GROUND SCHOOL STUDENT  " RS);
    }
    Terminal::boxBot(BYL);
}

/* ══════════════════════════════════════════════════════════════
   AVIATION QUIZ — 20-question pool, 10 random per game
══════════════════════════════════════════════════════════════*/
void PassengerDashboard::aviationQuiz()
{
    struct Q
    {
        const char *q, *a[4];
        int ans;
    };
    Q pool[] = {
        {"What does 'KHI' stand for?",
         {"Karachi Intl Airport", "Khi Hub Index", "Kolkata Hub Int", "Kuala Hub Int"},
         0},
        {"What is Mach 1?",
         {"Speed of Light", "Speed of Sound", "Max aircraft speed", "Runway speed"},
         1},
        {"What does 'ETA' mean?",
         {"Extra Travel Allowance", "Estimated Time of Arrival", "Engine Thrust Angle", "Exit Terminal Access"},
         1},
        {"Colour of the flight data recorder?",
         {"Black", "Red", "Orange", "Yellow"},
         2},
        {"What does 'IATA' stand for?",
         {"Intl Air Transport Association", "Intl Airline Traffic Assoc", "Intl Airport Trade Auth", "Intl ATC Association"},
         0},
        {"The Airbus A380 has how many decks?",
         {"1", "2", "3", "4"},
         1},
        {"What is a 'red-eye' flight?",
         {"Emergency flight", "Overnight flight", "First class only", "Cancelled flight"},
         1},
        {"What does ATC stand for?",
         {"Air Traffic Control", "Aircraft Turbine Clearance", "Altitude Terminal Check", "Auto Thrust Control"},
         0},
        {"Which aircraft is the 'Jumbo Jet'?",
         {"Airbus A380", "Boeing 747", "Boeing 777", "Concorde"},
         1},
        {"Cruising altitude for most commercial jets?",
         {"15,000-20,000 ft", "25,000-30,000 ft", "35,000-42,000 ft", "50,000+ ft"},
         2},
        {"World's longest non-stop flight?",
         {"Dubai-New York", "Sydney-Dallas", "Singapore-New York (SQ21)", "London-Sydney"},
         2},
        {"What does a 'METAR' report provide?",
         {"Flight schedules", "Airport weather", "Runway capacity", "Passenger manifest"},
         1},
        {"What does 'PAX' mean in aviation?",
         {"Passenger", "Parking Axle Index", "Pilot Auxiliary", "Precision Approach X"},
         0},
        {"Which instrument shows aircraft attitude?",
         {"Altimeter", "Attitude Indicator", "Airspeed Indicator", "VSI"},
         1},
        {"Phonetic alphabet for 'B'?",
         {"Bravo", "Beta", "Bacon", "Broadway"},
         0},
        {"First commercial jet service began?",
         {"1945", "1952", "1958", "1969"},
         2},
        {"V1 speed refers to?",
         {"Take-off decision speed", "Vertical limit", "VHF channel 1", "Velocity at 1000ft"},
         0},
        {"A 'holding pattern' is?",
         {"Aircraft storage", "Racetrack flight while waiting", "Emergency sequence", "Gate assignment"},
         1},
        {"ETOPS stands for?",
         {"Extended-range Twin-engine Operational Performance Standards", "Engine Turn-Off Protocol", "Extra Take-Off Procedures", "Emergency Twin-Op System"},
         0},
        {"Standard emergency transponder code?",
         {"1200", "7500", "7700", "7000"},
         2},
    };
    int poolSize = 20, total = 10, score = 0;
    for (int i = poolSize - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        Q t = pool[i];
        pool[i] = pool[j];
        pool[j] = t;
    }

    Presentation::printBanner();
    Terminal::sectionHeader("AVIATION KNOWLEDGE QUIZ", BYL);
    printf("\n");
    Terminal::boxTop(BYL);
    Terminal::boxCenter(BYL, BYL BOLD "  ✈   NEXUS AIR KNOWLEDGE CHALLENGE   ✈  " RS);
    Terminal::boxSep(BYL);
    Terminal::boxRowC(BYL, "  " DIM "10 questions  •  4 choices  •  20-question pool (randomised)" RS);
    Terminal::boxRowC(BYL, "  " BCY "Score 8+ for Aviation Expert badge!" RS);
    Terminal::boxBot(BYL);
    printf("\n  Press ENTER to begin...");
    fflush(stdout);
    getchar();

    for (int i = 0; i < total; i++)
    {
        Presentation::printBanner();
        printf("\n");
        /* progress bar */
        {
            int bw = TW - 8, filled = bw * i / total;
            char hdr[60];
            snprintf(hdr, 60, "  Q%d/%d  Score:%d  ", i + 1, total, score);
            printf(BYL "  %-20s" RS "  [", hdr);
            for (int x = 0; x < bw; x++)
                printf("%s%s" RS, (x < filled) ? BGR : DIM, (x < filled) ? "█" : "░");
            printf("]\n\n");
        }
        Terminal::boxTop(BYL);
        {
            char b[80];
            snprintf(b, 80, "  " DIM "QUESTION %d of %d  " RS BCY "[Aviation Knowledge]" RS, i + 1, total);
            Terminal::boxRowC(BYL, b);
        }
        Terminal::boxSep(BYL);
        {
            char b[400];
            snprintf(b, 400, "  " BWH BOLD "%s" RS, pool[i].q);
            Terminal::boxRowC(BYL, b);
        }
        Terminal::boxSep(BYL);
        const char *opts[] = {"  " BCY "[1] " RS, "  " BGR "[2] " RS, "  " BYL "[3] " RS, "  " BMG "[4] " RS};
        for (int j = 0; j < 4; j++)
        {
            char b[200];
            snprintf(b, 200, "%s" BWH "%s" RS, opts[j], pool[i].a[j]);
            Terminal::boxRowC(BYL, b);
        }
        Terminal::boxBot(BYL);
        int ans = Input::integer("Your answer [1-4]:", 1, 4) - 1;
        bool correct = (ans == pool[i].ans);
        if (correct)
            score++;
        questionResult(correct, pool[i].a[pool[i].ans]);
        Utils::sleepMs(1100);
    }
    Presentation::printBanner();
    Terminal::sectionHeader("QUIZ COMPLETE!", BYL);
    animateScore(score, total);
    gradeBadge(score, total);
    Terminal::waitEnter();
}

/* ══════════════════════════════════════════════════════════════
   WORD SCRAMBLE — 5 random aviation words per game
══════════════════════════════════════════════════════════════*/
static void wordScrambleGame()
{
    struct Word
    {
        const char *word;
        const char *hint;
    };
    Word words[] = {
        {"RUNWAY", "Where planes take off and land"},
        {"COCKPIT", "Where pilots sit"},
        {"TURBINE", "Engine type in jet aircraft"},
        {"ALTITUDE", "Height above sea level"},
        {"BOARDING", "Passengers entering the plane"},
        {"TAXIWAY", "Path between runway and terminal"},
        {"TERMINAL", "Airport building for passengers"},
        {"FUSELAGE", "Main body of an aircraft"},
        {"AILERON", "Wing flap for roll control"},
        {"HANGAR", "Building for storing aircraft"},
    };
    int n = 10;
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        Word t = words[i];
        words[i] = words[j];
        words[j] = t;
    }
    int total = 5, score = 0;

    Presentation::printBanner();
    Terminal::sectionHeader("AVIATION WORD SCRAMBLE", BMG);
    printf("\n");
    Terminal::boxTop(BMG);
    Terminal::boxCenter(BMG, BMG BOLD "  🔤  UNSCRAMBLE THE AVIATION WORD!  🔤  " RS);
    Terminal::boxSep(BMG);
    Terminal::boxRowC(BMG, "  " DIM "Unscramble each word. 3 attempts per word." RS);
    Terminal::boxRowC(BMG, "  " BCY "Type in UPPERCASE and press Enter." RS);
    Terminal::boxBot(BMG);
    printf("\n  Press ENTER to start...");
    fflush(stdout);
    getchar();

    for (int i = 0; i < total; i++)
    {
        string w(words[i].word);
        string sc = w;
        do
        {
            for (int j = (int)sc.size() - 1; j > 0; j--)
            {
                int k = rand() % (j + 1);
                swap(sc[j], sc[k]);
            }
        } while (sc == w);

        Presentation::printBanner();
        printf("\n");
        Terminal::boxTop(BMG);
        {
            char b[80];
            snprintf(b, 80, "  " DIM "Word %d of %d  " RS "  Score: " BGR "%d" RS, i + 1, total, score);
            Terminal::boxRowC(BMG, b);
        }
        Terminal::boxSep(BMG);
        {
            char b[100];
            snprintf(b, 100, "  Scrambled : " BYL BOLD "%s" RS, sc.c_str());
            Terminal::boxRowC(BMG, b);
        }
        {
            char b[100];
            snprintf(b, 100, "  Hint      : " DIM "%s" RS, words[i].hint);
            Terminal::boxRowC(BMG, b);
        }
        {
            char b[60];
            snprintf(b, 60, "  Letters   : " BCY "%d" RS, (int)w.size());
            Terminal::boxRowC(BMG, b);
        }
        Terminal::boxBot(BMG);

        bool got = false;
        for (int attempt = 0; attempt < 3 && !got; attempt++)
        {
            if (attempt > 0)
            {
                char hint[80];
                snprintf(hint, 80, "  " BYL "Try %d/3 — starts with '" BGR "%c" BYL "'\n" RS, attempt + 1, w[0]);
                printf("%s", hint);
            }
            string ans = Input::field("Your answer (UPPERCASE):");
            for (char &c : ans)
                c = toupper(c);
            if (ans == w)
            {
                got = true;
                score++;
                printf(BGR BOLD "\n  ✅  CORRECT! '%s' is right! +1\n" RS, w.c_str());
                SoundEngine::ok();
            }
            else if (attempt < 2)
            {
                printf(BRD "\n  ❌  Not quite! Try again...\n" RS);
                SoundEngine::error();
            }
        }
        if (!got)
        {
            printf(BRD "\n  ❌  Answer was: " BGR BOLD "%s\n" RS, w.c_str());
            SoundEngine::error();
        }
        Utils::sleepMs(1000);
    }
    Presentation::printBanner();
    Terminal::sectionHeader("SCRAMBLE RESULTS", BMG);
    animateScore(score, total);
    gradeBadge(score, total);
    Terminal::waitEnter();
}

/* ══════════════════════════════════════════════════════════════
   MEMORY CHALLENGE — growing icon sequence
══════════════════════════════════════════════════════════════*/
static void flightMemoryGame()
{
    const char *names[] = {"Plane", "Takeoff", "Landing", "Globe", "Map",
                           "Cloud", "Mountain", "Ocean", "Moon", "Star"};
    const char *icons[] = {"[PL]", "[TO]", "[LD]", "[GB]", "[MP]",
                           "[CL]", "[MT]", "[OC]", "[MN]", "[ST]"};
    int n = 10;
    Presentation::printBanner();
    Terminal::sectionHeader("FLIGHT MEMORY CHALLENGE", BCY);
    printf("\n");
    Terminal::boxTop(BCY);
    Terminal::boxCenter(BCY, BCY BOLD "  🧠  NEXUS AIR MEMORY CHALLENGE  🧠  " RS);
    Terminal::boxSep(BCY);
    Terminal::boxRowC(BCY, "  " DIM "Watch the sequence then type the numbers back in order." RS);
    Terminal::boxRowC(BCY, "  " BCY "Sequence grows by 1 each round. Survive 8 rounds to win!" RS);
    Terminal::boxBot(BCY);
    printf("\n  Press ENTER to begin...");
    fflush(stdout);
    getchar();

    vector<int> seq;
    int round = 0;
    for (;;)
    {
        round++;
        seq.push_back(rand() % n);
        /* show sequence */
        Presentation::printBanner();
        printf("\n");
        Terminal::boxTop(BCY);
        {
            char b[60];
            snprintf(b, 60, "  " BCY "ROUND %d" RS "  — Memorise this sequence!", round);
            Terminal::boxRowC(BCY, b);
        }
        Terminal::boxSep(BCY);
        /* show numbered key */
        {
            string row = "  ";
            for (int i = 0; i < n; i++)
            {
                char t[20];
                snprintf(t, 20, "[%d]=%s ", i + 1, names[i]);
                row += t;
            }
            Terminal::boxRowC(BCY, row.c_str());
        }
        Terminal::boxSep(BCY);
        /* show sequence with icons */
        {
            string row = "  Sequence: ";
            for (int idx : seq)
            {
                char t[12];
                snprintf(t, 12, BCY "[%d]" RS " ", idx + 1);
                row += t;
            }
            Terminal::boxRowC(BCY, row.c_str());
        }
        Terminal::boxBot(BCY);
        fflush(stdout);
        Utils::sleepMs(1200 + round * 280);

        /* hide it */
        Presentation::printBanner();
        printf("\n");
        Terminal::boxTop(BCY);
        {
            char b[60];
            snprintf(b, 60, "  " BCY "ROUND %d" RS "  — Now type back the sequence!", round);
            Terminal::boxRowC(BCY, b);
        }
        Terminal::boxSep(BCY);
        {
            string row = "  ";
            for (int i = 0; i < n; i++)
            {
                char t[20];
                snprintf(t, 20, "[%d]=%s ", i + 1, names[i]);
                row += t;
            }
            Terminal::boxRowC(BCY, row.c_str());
        }
        Terminal::boxBot(BCY);

        bool ok = true;
        for (int s = 0; s < (int)seq.size(); s++)
        {
            char b[50];
            snprintf(b, 50, "Position %d of %d [1-%d]:", s + 1, (int)seq.size(), n);
            int ans = Input::integer(b, 1, n) - 1;
            if (ans != seq[s])
            {
                ok = false;
                break;
            }
        }
        if (ok)
        {
            printf(BGR BOLD "\n  ✅  Perfect! Round %d complete!\n" RS, round);
            SoundEngine::ok();
            Utils::sleepMs(700);
        }
        else
        {
            printf(BRD BOLD "\n  ❌  Wrong! You survived %d round(s)!\n" RS, round - 1);
            SoundEngine::error();
            printf("\n  " DIM "Correct sequence was: " RS);
            for (int idx : seq)
                printf(BGR "%s " RS, names[idx]);
            printf("\n");
            Utils::sleepMs(1500);
            Presentation::printBanner();
            Terminal::sectionHeader("MEMORY GAME OVER", BCY);
            animateScore(round - 1, max(1, round - 1 + 2));
            Terminal::boxTop(BCY);
            {
                char b[80];
                snprintf(b, 80, "  🧠  You completed " BGR BOLD "%d round(s)" RS "!", round - 1);
                Terminal::boxRowC(BCY, b);
            }
            Terminal::boxBot(BCY);
            Terminal::waitEnter();
            return;
        }
        if (round >= 8)
        {
            Presentation::printBanner();
            Terminal::sectionHeader("MEMORY MASTER!", BCY);
            Terminal::boxTop(BCY);
            Terminal::boxCenter(BCY, BCY BOLD "  🏆  FLAWLESS! 8/8 ROUNDS PERFECT!  🏆  " RS);
            Terminal::boxCenter(BCY, DIM "  You have the memory of a seasoned Captain!  " RS);
            Terminal::boxBot(BCY);
            SoundEngine::pay();
            Utils::sleepMs(400);
            SoundEngine::pay();
            Terminal::waitEnter();
            return;
        }
    }
}

/*──────────────────────────────────────────────────────────────────────────────*/
void PassengerDashboard::priceCalc()
{
    Presentation::printBanner();
    Terminal::sectionHeader("FLIGHT PRICE CALCULATOR", BMG);
    Terminal::boxTop(BMG);
    Terminal::boxCenter(BMG, BMG "  💰  NEXUS AIR FARE ESTIMATOR  " RS);
    Terminal::boxSep(BMG);
    Terminal::boxRowC(BMG, "  " DIM "Popular routes: Karachi→Dubai ~2hr | Lahore→London ~9hr | ISB→NYC ~14hr" RS);
    Terminal::boxBot(BMG);
    printf("\n");
    string org = Input::field("Origin city/airport:");
    string dst = Input::field("Destination city/airport:");
    int dist = Input::integer("Estimated distance (km):", 100, 20000);
    printf("  " BCY "[1]" RS " Economy  " BCY "[2]" RS " Premium-Eco  " BCY "[3]" RS " Business  " BCY "[4]" RS " First\n");
    int cc = Input::integer("Cabin class:", 1, 4);
    int pax = Input::integer("Number of passengers:", 1, 9);
    int bags = Input::integer("Checked bags per person (0-3):", 0, 3);
    double rates[] = {0.09, 0.16, 0.30, 0.48};
    const char *cabNames[] = {"Economy", "Premium Economy", "Business", "First Class"};
    double bagRate[] = {0, 30, 55, 75};
    double base = dist * rates[cc - 1] * pax, taxes = base * 0.167, svcFee = 15.0 * pax;
    double bagTotal = bagRate[bags] * pax, total = base + taxes + svcFee + bagTotal;
    Terminal::spinner("Searching best fares and calculating...", 900);
    printf("\n");
    Terminal::boxTop(BMG);
    Terminal::boxCenter(BMG, BMG BOLD "  💰  DETAILED FARE BREAKDOWN  " RS);
    Terminal::boxSep(BMG);
    {
        char b[200];
        snprintf(b, 200, "  Route       : " BWH "%s  →  %s" RS, org.c_str(), dst.c_str());
        Terminal::boxRow(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Distance    : " BWH "%d km" RS "  (~%.0f hours)", dist, dist / 850.0);
        Terminal::boxRow(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Class       : " BWH "%s" RS, cabNames[cc - 1]);
        Terminal::boxRow(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Passengers  : " BWH "%d" RS "   Bags/pax: " BWH "%d" RS, pax, bags);
        Terminal::boxRow(BMG, b);
    }
    Terminal::boxSep(BMG);
    {
        char b[200];
        snprintf(b, 200, "  Base Fares  : " BWH "$%.2f" RS, base);
        Terminal::boxRow(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Baggage Fees: " BWH "$%.2f" RS, bagTotal);
        Terminal::boxRow(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Taxes (16.7%%): " BWH "$%.2f" RS, taxes);
        Terminal::boxRow(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Service Fees: " BWH "$%.2f" RS, svcFee);
        Terminal::boxRow(BMG, b);
    }
    Terminal::boxSep(BMG);
    {
        char b[200];
        snprintf(b, 200, "  " BOLD "GRAND TOTAL : " BGR "$%.2f" RS "  (per person: " BCY "$%.2f" RS ")", total, total / pax);
        Terminal::boxRowC(BMG, b);
    }
    Terminal::boxSep(BMG);
    const char *tips[] = {"Book 6-8 weeks in advance for best fares.",
                          "Tue/Wed departures are often cheapest.",
                          "Avoid peak school holiday dates.",
                          "Sign up for fare alerts to catch price drops."};
    {
        char b[200];
        snprintf(b, 200, "  " YL "💡 Tip: " RS DIM "%s" RS, tips[rand() % 4]);
        Terminal::boxRowC(BMG, b);
    }
    Terminal::boxBot(BMG);
    Terminal::warnMsg("Estimate only. Book on nexusair.com for exact fares.");
    Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────*/
void PassengerDashboard::entertainment()
{
    int ch;
    do
    {
        Presentation::printBanner();
        printf("\n");
        Terminal::boxTop(BYL);
        Terminal::boxCenter(BYL, BYL BOLD "  🎮  NEXUS AIR ENTERTAINMENT LOUNGE  🎮  " RS);
        Terminal::boxSep(BYL);
        Terminal::boxRowC(BYL, "  " DIM "Pass the time with aviation-themed games and tools!" RS);
        Terminal::boxSep(BYL);
        Terminal::boxRowC(BYL, "  " BYL "[1] " RS "  ✈   Aviation Knowledge Quiz   " DIM "(10 Qs, 20-question random pool)" RS);
        Terminal::boxRowC(BYL, "  " BYL "[2] " RS "  🔤  Aviation Word Scramble    " DIM "(unscramble 5 aviation words)" RS);
        Terminal::boxRowC(BYL, "  " BYL "[3] " RS "  🧠  Flight Memory Challenge   " DIM "(remember the growing sequence)" RS);
        Terminal::boxRowC(BYL, "  " BYL "[4] " RS "  💰  Flight Price Calculator   " DIM "(detailed fare estimate)" RS);
        Terminal::boxSep(BYL);
        Terminal::boxRowC(BYL, "  " BRD "[5] " RS "  🔙  Back to Dashboard");
        Terminal::boxBot(BYL);
        ch = Input::integer("Choice:", 1, 5);
        switch (ch)
        {
        case 1:
            aviationQuiz();
            break;
        case 2:
            wordScrambleGame();
            break;
        case 3:
            flightMemoryGame();
            break;
        case 4:
            priceCalc();
            break;
        }
    } while (ch != 5);
}

/*──────────────────────────────────────────────────────────────────────────────
  PassengerDashboard::run — CONCEPT [5] polymorphic dispatch via virtual methods
──────────────────────────────────────────────────────────────────────────────*/
void PassengerDashboard::run()
{
    for (;;)
    {
        Passenger *p = db_.passAt(sess_.getIdx());
        p->updateTier();
        Presentation::printBanner();
        printf("\n  %s★ %s MEMBER" RS "  │  " DIM "Miles: " RS BWH "%d" RS "  │  " DIM "Points: " RS BWH "%d" RS "\n\n",
               Utils::tierColor(p->getLoyaltyTier().c_str()), p->getLoyaltyTier().c_str(),
               p->getTotalMiles(), p->getLoyaltyPts());
        Terminal::boxTop(BGR);
        {
            char b[200];
            snprintf(b, 200, "  " BGR "👤  PASSENGER DASHBOARD — %s" RS, p->getFullName().c_str());
            Terminal::boxRowC(BGR, b);
        }
        Terminal::boxMid(BGR);
        Terminal::boxRowC(BGR, "  " BGR "[1]" RS "  🔍  Search & Book Flights");
        Terminal::boxRowC(BGR, "  " BGR "[2]" RS "  📋  My Bookings");
        Terminal::boxRowC(BGR, "  " BGR "[3]" RS "  ✈   Online Check-In");
        Terminal::boxRowC(BGR, "  " BGR "[4]" RS "  ⭐  Nexus Miles Loyalty");
        Terminal::boxRowC(BGR, "  " BGR "[5]" RS "  👤  My Profile");
        Terminal::boxRowC(BGR, "  " BGR "[6]" RS "  🧳  Baggage Tracking");
        Terminal::boxRowC(BGR, "  " BGR "[7]" RS "  🎮  Entertainment");
        Terminal::boxSep(BGR);
        Terminal::boxRowC(BGR, "  " BRD "[8]" RS "  🚪  Logout");
        Terminal::boxBot(BGR);

        /* Upcoming flights summary */
        printf("\n");
        Terminal::boxSep(BCY);
        printf(BCY "  UPCOMING FLIGHTS" RS "\n");
        Terminal::boxSep(BCY);
        int uc = 0;
        for (int i = 0; i < db_.bookingCount(); i++)
        {
            Booking *bk = db_.bookingAt(i);
            if (bk->getPassengerId() == p->getId() && bk->getStatus() == "CONFIRMED")
            {
                printf("  " BCY "✈ %s" RS "   Seat: " BWH "%s" RS "   %s\n",
                       bk->getFlightNo().c_str(), bk->getSeatNo().c_str(),
                       bk->isCheckedIn() ? (BGR "✔ Checked-In" RS) : (BYL "⏳ Pending Check-in" RS));
                uc++;
            }
        }
        if (!uc)
            printf("  " DIM "No upcoming bookings.\n" RS);

        int ch = Input::integer("\nChoice [1-8]:", 1, 8);
        switch (ch)
        {
        case 1:
            bookFlight();
            break;
        case 2:
            viewBookings();
            break;
        case 3:
            onlineCheckIn();
            break;
        case 4:
            loyaltyProgram();
            break;
        case 5:
            myProfile();
            break;
        case 6:
            baggageTracking();
            break;
        case 7:
            entertainment();
            break;
        case 8:
            sess_.logout();
            Terminal::success("Logged out. Safe travels!");
            Utils::sleepMs(700);
            return;
        }
    }
}

/*══════════════════════════════════════════════════════════════════════════════
  StaffDashboard::run
══════════════════════════════════════════════════════════════════════════════*/
void StaffDashboard::run()
{
    for (;;)
    {
        Staff *s = db_.staffAt(sess_.getIdx());
        Presentation::printBanner();
        printf("\n");
        Terminal::boxTop(BMG);
        {
            char b[200];
            snprintf(b, 200, "  " BMG "🛂  %s — %s" RS, s->getRoleName(), s->getEmpId().c_str());
            Terminal::boxRowC(BMG, b);
        }
        {
            char b[200];
            snprintf(b, 200, "  " DIM "%s" RS, s->getRoleDesc());
            Terminal::boxRowC(BMG, b);
        }
        {
            char b[200];
            snprintf(b, 200, "  Terminal: " BWH "%s" RS "   Gates: " BWH "%s" RS "   Shift: " BWH "%s–%s" RS, s->getTerminal().c_str(), s->getGates().c_str(), s->getShiftStart().c_str(), s->getShiftEnd().c_str());
            Terminal::boxRowC(BMG, b);
        }
        Terminal::boxMid(BMG);
        Terminal::boxRowC(BMG, "  " BMG "[1]" RS "  Primary Role Duties");
        Terminal::boxRowC(BMG, "  " BMG "[2]" RS "  ✈   View/Update Flight Status");
        Terminal::boxRowC(BMG, "  " BMG "[3]" RS "  🔍  Search Flights");
        Terminal::boxRowC(BMG, "  " BMG "[4]" RS "  🧳  Baggage Operations");
        Terminal::boxSep(BMG);
        Terminal::boxRowC(BMG, "  " BRD "[5]" RS "  🚪  Logout");
        Terminal::boxBot(BMG);

        int ch = Input::integer("Choice:", 1, 5);
        if (ch == 5)
        {
            sess_.logout();
            Terminal::success("Logged out. Good shift!");
            Utils::sleepMs(600);
            return;
        }

        if (ch == 1)
        {
            /* CONCEPT [5] Polymorphic dispatch — factory creates correct subtype */
            IRolePanel *panel = createRolePanel(*s, db_, sess_);
            panel->execute();
            delete panel;
        }
        else if (ch == 2)
        {
            string fno = Input::field("Flight Number:");
            int fi = db_.findFlightByNo(fno);
            if (fi < 0)
                Terminal::errMsg("Not found.");
            else
            {
                Flight *fl = db_.flightAt(fi);
                printf("  " BCY "[1]" RS " ON_TIME  [2] DELAYED  [3] BOARDING  [4] DEPARTED  [5] LANDED\n");
                int sc = Input::integer("Status:", 1, 5);
                const char *sts[] = {"ON_TIME", "DELAYED", "BOARDING", "DEPARTED", "LANDED"};
                fl->setStatus(sts[sc - 1]);
                if (sc == 2)
                {
                    string r = Input::field("Delay reason:");
                    fl->setDelayReason(r);
                }
                db_.saveAll();
                Terminal::success("Status updated.");
                int cnt = 0;
                for (int i = 0; i < db_.bookingCount(); i++)
                    if (db_.bookingAt(i)->getFlightNo() == fno)
                        cnt++;
                char b[80];
                snprintf(b, 80, "%d passengers notified.", cnt);
                Terminal::infoMsg(b);
            }
            Terminal::waitEnter();
        }
        else if (ch == 3)
        {
            string org = Input::field("From (Enter=all):");
            string dst = Input::field("To (Enter=all):");
            Terminal::spinner("Searching flights...", 600);
            int found = 0;
            for (int i = 0; i < db_.flightCount(); i++)
            {
                Flight *fl = db_.flightAt(i);
                if (!fl->isActive())
                    continue;
                bool mo = org.empty() || fl->getOrigin().find(org) != string::npos || fl->getFlightNo().find(org) != string::npos;
                bool md = dst.empty() || fl->getDest().find(dst) != string::npos;
                if (mo && md)
                {
                    fl->displayCard(found);
                    found++;
                }
            }
            if (!found)
                Terminal::infoMsg("No flights match your search.");
            Terminal::waitEnter();
        }
        else if (ch == 4)
        {
            BaggagePanel bp(*s, db_, sess_);
            bp.execute();
        }
    }
}

/*══════════════════════════════════════════════════════════════════════════════
  AdminDashboard — full admin feature set
══════════════════════════════════════════════════════════════════════════════*/
void AdminDashboard::addFlight()
{
    Presentation::printBanner();
    Terminal::sectionHeader("ADD NEW FLIGHT — PAGE 1/3", BYL);
    string fno = Input::field("Flight Number (e.g. NX606):");
    if (db_.findFlightByNo(fno) >= 0)
    {
        Terminal::errMsg("Flight number already exists.");
        Terminal::waitEnter();
        return;
    }
    string air = Input::field("Airline:");
    string ac = Input::field("Aircraft Type:");

    Presentation::printBanner();
    Terminal::sectionHeader("ADD NEW FLIGHT — PAGE 2/3", BYL);
    string org = Input::field("Origin:");
    string dst = Input::field("Destination:");
    string dt = Input::date("Date:");
    string dep = Input::field("Departure (HH:MM):");
    string arr = Input::field("Arrival (HH:MM):");
    int dur = Input::integer("Duration (mins):", 30, 1500);
    int gate = Input::integer("Gate:", 1, 99);
    int td = Input::integer("Dep Terminal:", 1, 9);
    int ta = Input::integer("Arr Terminal:", 1, 9);

    Presentation::printBanner();
    Terminal::sectionHeader("ADD NEW FLIGHT — PAGE 3/3", BYL);
    double ff = Input::dbl("First Class Fare $:");
    if (!ff)
        ff = 2500;
    double fb = Input::dbl("Business Fare $:");
    if (!fb)
        fb = 1200;
    double fp = Input::dbl("Prem-Eco Fare $:");
    if (!fp)
        fp = 700;
    double fe = Input::dbl("Economy Fare $:");
    if (!fe)
        fe = 300;

    Flight fl(fno, air, org, dst, dt, dep, arr, ac, gate, td, ta, dur, ff, fb, fp, fe);
    db_.addFlight(fl);
    Terminal::spinner("Saving flight to database...", 700);
    db_.saveAll();
    Terminal::success(("Flight " + fno + " created!").c_str());
    Terminal::waitEnter();
}

void AdminDashboard::viewFlights()
{
    Presentation::printBanner();
    Terminal::sectionHeader("ALL FLIGHTS", BYL);
    printf("\n  " BOLD CY "%-4s %-8s %-20s %-8s %-12s %-5s" RS "\n", "#", "FLIGHT", "ROUTE", "DEP", "STATUS", "LOAD%");
    printf("  " DIM "─────────────────────────────────────────────────────────────────\n" RS);
    for (int i = 0; i < db_.flightCount(); i++)
    {
        Flight *fl = db_.flightAt(i);
        if (!fl->isActive())
            continue;
        int pct = fl->loadPct();
        const char *lc = (pct > 80) ? BRD : (pct > 60) ? BYL
                                                       : BGR;
        char route[24];
        snprintf(route, 24, "%.9s→%.9s", fl->getOrigin().c_str(), fl->getDest().c_str());
        printf("  " BWH "%-4d" BCY "%-8s" WH "%-20s " WH "%-8s %s%-12s" WH "%s%-5d%%" RS "\n",
               i + 1, fl->getFlightNo().c_str(), route, fl->getDepTime().c_str(),
               Utils::statusColor(fl->getStatus().c_str()), fl->getStatus().c_str(), lc, pct);
    }
    printf("\n  " DIM "Total: %d flights\n" RS, db_.flightCount());
    Terminal::waitEnter();
}

void AdminDashboard::editFlight()
{
    Presentation::printBanner();
    Terminal::sectionHeader("EDIT FLIGHT", BYL);
    string fno = Input::field("Flight Number:");
    int idx = db_.findFlightByNo(fno);
    if (idx < 0)
    {
        Terminal::errMsg("Flight not found.");
        Terminal::waitEnter();
        return;
    }
    Flight *fl = db_.flightAt(idx);
    printf("  " CY "[1]" RS " Status  [2] Gate  [3] Departure  [4] Economy Fare  [5] Delay Reason  [6] Deactivate\n\n");
    int ch = Input::integer("Field to edit:", 1, 6);
    if (ch == 1)
    {
        printf("  [1]ON_TIME [2]DELAYED [3]CANCELLED [4]BOARDING [5]DEPARTED [6]LANDED\n");
        int sc = Input::integer(":", 1, 6);
        const char *sts[] = {"ON_TIME", "DELAYED", "CANCELLED", "BOARDING", "DEPARTED", "LANDED"};
        fl->setStatus(sts[sc - 1]);
        db_.saveAll();
        Terminal::success("Status updated.");
    }
    else if (ch == 2)
    {
        fl->setGate(Input::integer("New Gate:", 1, 99));
        db_.saveAll();
        Terminal::success("Gate updated.");
    }
    else if (ch == 3)
    {
        fl->setDepTime(Input::field("New Dep Time (HH:MM):"));
        db_.saveAll();
        Terminal::success("Dep time updated.");
    }
    else if (ch == 4)
    {
        fl->setFareEco(Input::dbl("New Economy Fare:"));
        db_.saveAll();
        Terminal::success("Fare updated.");
    }
    else if (ch == 5)
    {
        fl->setDelayReason(Input::field("Delay reason:"));
        db_.saveAll();
        Terminal::success("Reason saved.");
    }
    else if (ch == 6)
    {
        if (Input::field("Type CONFIRM:") == "CONFIRM")
        {
            fl->setActive(false);
            db_.saveAll();
            Terminal::success("Flight deactivated.");
        }
    }
    Terminal::waitEnter();
}

void AdminDashboard::cancelFlight()
{
    Presentation::printBanner();
    Terminal::sectionHeader("CANCEL FLIGHT", BRD);
    string fno = Input::field("Flight Number:");
    int idx = db_.findFlightByNo(fno);
    if (idx < 0)
    {
        Terminal::errMsg("Not found.");
        Terminal::waitEnter();
        return;
    }
    Flight *fl = db_.flightAt(idx);
    printf("  " BRD "Flight %s — %d passengers affected.\n" RS, fl->getFlightNo().c_str(), fl->totalBooked());
    if (Input::field("Type CONFIRM to cancel:") != "CONFIRM")
    {
        Terminal::infoMsg("Cancelled.");
        Terminal::waitEnter();
        return;
    }
    fl->setStatus("CANCELLED");
    int ref = 0;
    for (int i = 0; i < db_.bookingCount(); i++)
    {
        Booking *b = db_.bookingAt(i);
        if (b->getFlightNo() == fno && b->getStatus() == "CONFIRMED")
        {
            b->setStatus("CANCELLED");
            ref++;
        }
    }
    db_.saveAll();
    char b[100];
    snprintf(b, 100, "Flight cancelled. %d bookings cancelled. Refunds initiated.", ref);
    Terminal::success(b);
    Terminal::waitEnter();
}

void AdminDashboard::viewPassengers()
{
    Presentation::printBanner();
    Terminal::sectionHeader("ALL PASSENGERS", BYL);
    printf("\n  " BOLD CY "%-6s %-20s %-25s %-10s %-6s" RS "\n", "ID", "NAME", "EMAIL", "TIER", "MILES");
    printf("  " DIM "─────────────────────────────────────────────────────────────────────────\n" RS);
    for (int i = 0; i < db_.passCount(); i++)
    {
        Passenger *p = db_.passAt(i);
        if (!p->isActive())
            continue;
        p->display();
    }
    printf("\n  " DIM "Total: %d passengers\n" RS, db_.passCount());
    Terminal::waitEnter();
}

void AdminDashboard::viewStaff()
{
    Presentation::printBanner();
    Terminal::sectionHeader("ALL STAFF MEMBERS", BYL);
    printf("\n  " BOLD CY "%-10s %-20s %-20s %-12s" RS "\n", "EMP ID", "NAME", "ROLE", "TERMINAL");
    printf("  " DIM "──────────────────────────────────────────────────────────────────\n" RS);
    for (int i = 0; i < db_.staffCount(); i++)
    {
        Staff *s = db_.staffAt(i);
        if (!s->isActive())
            continue;
        s->display();
    }
    printf("\n  " DIM "Total: %d staff\n" RS, db_.staffCount());
    Terminal::waitEnter();
}

void AdminDashboard::reports()
{
    Presentation::printBanner();
    Terminal::sectionHeader("REPORTS & ANALYTICS", BYL);
    printf("  " BCY "[1]" RS " Load Factor   " BCY "[2]" RS " Revenue   " BCY "[3]" RS " Passenger Stats   " BCY "[4]" RS " Booking Summary\n\n");
    int ch = Input::integer("Report:", 1, 4);
    if (ch == 1)
    {
        Presentation::printBanner();
        Terminal::sectionHeader("LOAD FACTOR REPORT", BYL);
        for (int i = 0; i < db_.flightCount(); i++)
        {
            Flight *fl = db_.flightAt(i);
            if (!fl->isActive())
                continue;
            int pct = fl->loadPct();
            printf("  " BCY "%-8s" RS " %s→%s  %s%d%%" RS "\n",
                   fl->getFlightNo().c_str(), fl->getOrigin().c_str(), fl->getDest().c_str(),
                   (pct > 80) ? BRD : (pct > 60) ? BYL
                                                 : BGR,
                   pct);
        }
    }
    else if (ch == 2)
    {
        /* CONCEPT [7] Template filterIf on bookings */
        auto confirmed = db_.bookings().filterIf([](Booking *b)
                                                 { return b->getStatus() == "CONFIRMED"; });
        double total = 0;
        for (auto *b : confirmed)
            total += b->getFare().grandTotal;
        printf("\n  " CY "Total Revenue (Confirmed): " RS BOLD BGR "$%.2f\n" RS, total);
        for (int i = 0; i < db_.flightCount(); i++)
        {
            double fr = 0;
            int cnt = 0;
            for (int j = 0; j < db_.bookingCount(); j++)
            {
                Booking *bk = db_.bookingAt(j);
                if (bk->getFlightNo() == db_.flightAt(i)->getFlightNo() && bk->getStatus() == "CONFIRMED")
                {
                    fr += bk->getFare().grandTotal;
                    cnt++;
                }
            }
            if (cnt > 0)
                printf("  " BCY "%-8s" RS "  $%.2f  (%d bookings)\n", db_.flightAt(i)->getFlightNo().c_str(), fr, cnt);
        }
    }
    else if (ch == 3)
    {
        int bz = 0, si = 0, g = 0, pl = 0;
        for (int i = 0; i < db_.passCount(); i++)
        {
            string t = db_.passAt(i)->getLoyaltyTier();
            if (t == "PLATINUM")
                pl++;
            else if (t == "GOLD")
                g++;
            else if (t == "SILVER")
                si++;
            else
                bz++;
        }
        printf("\n  " YL "BRONZE   : %d\n" BWH "SILVER   : %d\n" BYL "GOLD     : %d\n" BCY "PLATINUM : %d\n" RS, bz, si, g, pl);
        printf("  " DIM "Total    : %d\n" RS, db_.passCount());
    }
    else if (ch == 4)
    {
        int conf = 0, canc = 0;
        for (int i = 0; i < db_.bookingCount(); i++)
        {
            if (db_.bookingAt(i)->getStatus() == "CONFIRMED")
                conf++;
            else
                canc++;
        }
        printf("\n  " BGR "Confirmed : %d\n" BRD "Cancelled : %d\n" DIM "Total     : %d\n" RS, conf, canc, db_.bookingCount());
    }
    Terminal::waitEnter();
}

void AdminDashboard::viewBookings()
{
    Presentation::printBanner();
    Terminal::sectionHeader("ALL BOOKINGS", BYL);
    printf("\n  " BOLD CY "%-6s %-20s %-8s %-14s %-10s %-10s" RS "\n", "REF", "PASSENGER", "FLIGHT", "CABIN", "TOTAL", "STATUS");
    printf("  " DIM "─────────────────────────────────────────────────────────────────────────────\n" RS);
    for (int i = 0; i < db_.bookingCount(); i++)
    {
        Booking *b = db_.bookingAt(i);
        const char *sc = (b->getStatus() == "CONFIRMED") ? BGR : BRD;
        printf("  " BCY "%-6s" WH "%-20.19s " YL "%-8s " WH "%-14s " BGR "$%-9.2f" RS "%s%-10s" RS "\n",
               b->getBookingRef().c_str(), b->getPassengerName().c_str(), b->getFlightNo().c_str(),
               b->getCabin().c_str(), b->getFare().grandTotal, sc, b->getStatus().c_str());
    }
    printf("\n  " DIM "Total: %d bookings\n" RS, db_.bookingCount());
    Terminal::waitEnter();
}

void AdminDashboard::systemOverview()
{
    Presentation::printBanner();
    Terminal::sectionHeader("SYSTEM OVERVIEW", BYL);
    int on = 0, del = 0, can = 0;
    double rev = 0;
    for (int i = 0; i < db_.flightCount(); i++)
    {
        Flight *fl = db_.flightAt(i);
        if (!fl->isActive())
            continue;
        if (fl->getStatus() == "ON_TIME")
            on++;
        else if (fl->getStatus() == "DELAYED")
            del++;
        else if (fl->getStatus() == "CANCELLED")
            can++;
    }
    int allSeats = 0, allBkd = 0;
    for (int i = 0; i < db_.flightCount(); i++)
    {
        if (!db_.flightAt(i)->isActive())
            continue;
        allSeats += db_.flightAt(i)->totalSeats();
        allBkd += db_.flightAt(i)->totalBooked();
    }
    for (int i = 0; i < db_.bookingCount(); i++)
        if (db_.bookingAt(i)->getStatus() == "CONFIRMED")
            rev += db_.bookingAt(i)->getFare().grandTotal;
    int lpct = allSeats > 0 ? allBkd * 100 / allSeats : 0;
    printf("\n");
    Terminal::boxTop(BYL);
    Terminal::boxCenter(BYL, BYL "  📊  NEXUS AIR SYSTEM OVERVIEW  " RS);
    Terminal::boxSep(BYL);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Flights     : " RS BWH "%d" RS "   " BGR "On-Time:%d" RS "   " BYL "Delayed:%d" RS "   " BRD "Cancelled:%d" RS, db_.flightCount(), on, del, can);
        Terminal::boxRowC(BYL, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Passengers  : " RS BWH "%d" RS, db_.passCount());
        Terminal::boxRowC(BYL, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Staff       : " RS BWH "%d" RS, db_.staffCount());
        Terminal::boxRowC(BYL, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Bookings    : " RS BWH "%d" RS, db_.bookingCount());
        Terminal::boxRowC(BYL, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Baggage     : " RS BWH "%d bags" RS, db_.bagCount());
        Terminal::boxRowC(BYL, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Load Factor : " RS "%s" BWH "%d%%" RS, (lpct > 80 ? BRD : lpct > 60 ? BYL
                                                                                                       : BGR),
                 lpct);
        Terminal::boxRowC(BYL, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Revenue     : " RS BOLD BGR "$%.2f" RS, rev);
        Terminal::boxRowC(BYL, b);
    }
    Terminal::boxSep(BYL);
    Terminal::boxRowC(BYL, "  " DIM "Files: nx_flights.txt  nx_passengers.txt  nx_staff.txt  nx_bookings.txt" RS);
    Terminal::boxBot(BYL);
    Terminal::waitEnter();
}

void AdminDashboard::run()
{
    for (;;)
    {
        Admin *a = db_.adminAt(sess_.getIdx());
        Presentation::printBanner();
        printf("\n  " BG_YL "\033[30m" BOLD "  STATUS: OPERATIONAL  │  Flights:%d  │  Passengers:%d  │  Bookings:%d  " RS "\n\n",
               db_.flightCount(), db_.passCount(), db_.bookingCount());
        Terminal::boxTop(BYL);
        {
            char b[200];
            snprintf(b, 200, "  " BYL "👑  ADMIN DASHBOARD — %s" RS, a->getFullName().c_str());
            Terminal::boxRowC(BYL, b);
        }
        Terminal::boxMid(BYL);
        Terminal::boxRowC(BYL, CY "  ── ✈  FLIGHT MANAGEMENT ──────────────────────────────────" RS);
        Terminal::boxRowC(BYL, "  " BYL "[1]" RS "  View All Flights");
        Terminal::boxRowC(BYL, "  " BYL "[2]" RS "  Add New Flight");
        Terminal::boxRowC(BYL, "  " BYL "[3]" RS "  Edit Flight");
        Terminal::boxRowC(BYL, "  " BYL "[4]" RS "  Cancel Flight");
        Terminal::boxSep(BYL);
        Terminal::boxRowC(BYL, CY "  ── 👥  USER MANAGEMENT ─────────────────────────────────" RS);
        Terminal::boxRowC(BYL, "  " BYL "[5]" RS "  View Passengers");
        Terminal::boxRowC(BYL, "  " BYL "[6]" RS "  View Staff");
        Terminal::boxSep(BYL);
        Terminal::boxRowC(BYL, CY "  ── 📊  ANALYTICS & REPORTS ─────────────────────────────" RS);
        Terminal::boxRowC(BYL, "  " BYL "[7]" RS "  Reports & Analytics");
        Terminal::boxRowC(BYL, "  " BYL "[8]" RS "  View All Bookings");
        Terminal::boxRowC(BYL, "  " BYL "[9]" RS "  System Overview");
        Terminal::boxSep(BYL);
        Terminal::boxRowC(BYL, "  " BRD "[0]" RS "  Logout");
        Terminal::boxBot(BYL);

        int ch = Input::integer("Choice [0-9]:", 0, 9);
        switch (ch)
        {
        case 1:
            viewFlights();
            break;
        case 2:
            addFlight();
            break;
        case 3:
            editFlight();
            break;
        case 4:
            cancelFlight();
            break;
        case 5:
            viewPassengers();
            break;
        case 6:
            viewStaff();
            break;
        case 7:
            reports();
            break;
        case 8:
            viewBookings();
            break;
        case 9:
            systemOverview();
            break;
        case 0:
            sess_.logout();
            Terminal::success("Admin session ended.");
            Utils::sleepMs(600);
            return;
        default:
            Terminal::warnMsg("Invalid.");
            Utils::sleepMs(400);
        }
    }
}

/*══════════════════════════════════════════════════════════════════════════════
  AuthController — login / register orchestrator
  CONCEPT [4][5]: virtual authenticate() called on correct derived User subtype
══════════════════════════════════════════════════════════════════════════════*/
bool AuthController::loginPassenger()
{
    Presentation::printBanner();
    Terminal::sectionHeader("PASSENGER LOGIN", BGR);
    Terminal::boxTop(BGR);
    Terminal::boxCenter(BGR, BGR "  🔑  PASSENGER PORTAL  " RS);
    Terminal::boxSep(BGR);
    string un = Input::field("Username:");
    string pw = Input::password("Password:");
    Terminal::boxBot(BGR);
    Terminal::spinner("Authenticating...", 700);

    int idx = db_.findPassByUser(un);
    /* CONCEPT [4][5]: authenticate() is virtual on IAuthenticatable */
    if (idx < 0 || !db_.passAt(idx)->authenticate(pw))
    {
        Terminal::glitch("  ██ ACCESS DENIED ██");
        Terminal::errMsg("Invalid username or password.");
        Terminal::waitEnter();
        return false;
    }
    sess_.login(RPASS, idx, un);
    Passenger *p = db_.passAt(idx);
    p->updateTier();
    Presentation::printBanner();
    Terminal::sectionHeader("WELCOME BACK", BGR);
    printf("  " BOLD BGR "✔  Welcome back, %s!\n\n" RS, p->getFullName().c_str());
    printf("  " CY "Loyalty Status : " RS "%s" BOLD "%s MEMBER" RS "\n", Utils::tierColor(p->getLoyaltyTier().c_str()), p->getLoyaltyTier().c_str());
    printf("  " CY "Nexus Miles    : " RS BWH "%d" RS "\n", p->getTotalMiles());
    printf("  " CY "Points Balance : " RS BWH "%d pts" RS "\n", p->getLoyaltyPts());
    int uc = 0;
    for (int i = 0; i < db_.bookingCount(); i++)
        if (db_.bookingAt(i)->getPassengerId() == p->getId() && db_.bookingAt(i)->getStatus() == "CONFIRMED")
            uc++;
    printf("  " CY "Upcoming Trips : " RS BWH "%d" RS "\n", uc);
    SoundEngine::login();
    Terminal::waitEnter();
    return true;
}

void AuthController::registerPassenger()
{
    Presentation::printBanner();
    Terminal::sectionHeader("NEW PASSENGER REGISTRATION", BGR);
    Terminal::boxTop(BGR);
    Terminal::boxCenter(BGR, BGR "  📋  STEP 1 — PERSONAL DETAILS  " RS);
    Terminal::boxSep(BGR);
    string fn = Input::field("First Name:", "Legal name");
    string ln = Input::field("Last Name:", "Legal name");
    string dob = Input::date("Date of Birth:");
    string nat = Input::field("Nationality:");
    string pp = Input::passport("Passport Number:");
    Terminal::boxSep(BGR);
    Terminal::boxCenter(BGR, BGR "  📞  STEP 2 — CONTACT DETAILS  " RS);
    Terminal::boxSep(BGR);
    string em = Input::email("Email Address:");
    string ph = Input::phone("Phone Number:");
    string addr = Input::field("Home Address:");
    string city = Input::field("City:");
    string ctry = Input::field("Country:");
    Terminal::boxSep(BGR);
    Terminal::boxCenter(BGR, BGR "  🔐  STEP 3 — ACCOUNT SETUP  " RS);
    Terminal::boxSep(BGR);
    string un = Input::field("Username:", "Unique, no spaces");
    if (db_.findPassByUser(un) >= 0)
    {
        Terminal::errMsg("Username already taken!");
        Terminal::boxBot(BGR);
        Terminal::waitEnter();
        return;
    }
    string pw = Input::password("Password:", true);
    string pw2 = Input::password("Confirm Password:");
    if (pw != pw2)
    {
        Terminal::errMsg("Passwords do not match!");
        Terminal::boxBot(BGR);
        Terminal::waitEnter();
        return;
    }
    Terminal::boxBot(BGR);
    Terminal::spinner("Creating your Nexus Air account...", 1000);

    /* CONCEPT [15]: parameterised constructor builds complete Passenger */
    Passenger p(un, fn, ln, em, ph, pp, nat, dob, addr, city, ctry);
    p.setPassword(pw);
    db_.addPassenger(p);
    db_.saveAll();

    Presentation::printBanner();
    Terminal::sectionHeader("REGISTRATION SUCCESSFUL", BGR);
    Terminal::boxTop(BGR);
    {
        char b[200];
        snprintf(b, 200, "  🎉  " BOLD BGR "Welcome aboard, %s %s!" RS, fn.c_str(), ln.c_str());
        Terminal::boxRowC(BGR, b);
    }
    Terminal::boxSep(BGR);
    Terminal::boxRowC(BGR, "  " DIM "Welcome Bonus   : " RS BYL "500 pts credited!" RS);
    Terminal::boxRowC(BGR, "  " DIM "Loyalty Tier    : " RS YL "BRONZE MEMBER" RS);
    Terminal::boxBot(BGR);
    Terminal::waitEnter();
}

bool AuthController::loginStaff()
{
    Presentation::printBanner();
    Terminal::sectionHeader("STAFF LOGIN", BMG);
    Terminal::boxTop(BMG);
    Terminal::boxCenter(BMG, BMG "  🛂  STAFF PORTAL  " RS);
    Terminal::boxSep(BMG);
    string un = Input::field("Employee Username:");
    string pw = Input::password("Password:");
    Terminal::boxBot(BMG);
    Terminal::spinner("Authenticating staff...", 700);

    int idx = db_.findStaffByUser(un);
    if (idx < 0 || !db_.staffAt(idx)->authenticate(pw))
    {
        Terminal::glitch("  ██ ACCESS DENIED ██");
        Terminal::errMsg("Invalid credentials.");
        Terminal::waitEnter();
        return false;
    }
    sess_.login(RSTAFF, idx, un);
    Staff *s = db_.staffAt(idx);
    Presentation::printBanner();
    Terminal::sectionHeader("STAFF LOGIN SUCCESSFUL", BMG);
    printf("  " BOLD BMG "✔  Welcome, %s — %s" RS "\n\n", s->getFullName().c_str(), s->getRoleName());
    printf("  " CY "Employee ID : " RS BWH "%s" RS "\n", s->getEmpId().c_str());
    printf("  " CY "Role        : " RS "%s%s" RS "\n", SROLE_COL[s->getRoleCode()], s->getRoleName());
    printf("  " CY "Duties      : " RS DIM "%s" RS "\n", s->getRoleDesc());
    printf("  " CY "Terminal    : " RS BWH "%s" RS "   Gates: " BWH "%s" RS "\n", s->getTerminal().c_str(), s->getGates().c_str());
    SoundEngine::login();
    Terminal::waitEnter();
    return true;
}

void AuthController::registerStaff()
{
    Presentation::printBanner();
    Terminal::sectionHeader("STAFF REGISTRATION", BMG);
    Terminal::boxTop(BMG);
    Terminal::boxCenter(BMG, BMG "  🛡  STAFF CODE VERIFICATION  " RS);
    Terminal::boxMid(BMG);
    Terminal::boxRowC(BMG, "  " DIM "Staff access requires the authorisation code." RS);
    Terminal::boxBot(BMG);
    string code = Input::password("Staff Code:");
    if (code != STAFF_CODE)
    {
        Terminal::glitch("  ██ STAFF CODE REJECTED ██");
        Terminal::errMsg("Invalid staff code.");
        Terminal::waitEnter();
        return;
    }
    Terminal::success("Staff code verified.");

    string un = Input::field("Username:");
    if (db_.findStaffByUser(un) >= 0)
    {
        Terminal::errMsg("Username already taken.");
        Terminal::waitEnter();
        return;
    }
    string pw = Input::password("Password:", true);
    string pw2 = Input::password("Confirm Password:");
    if (pw != pw2)
    {
        Terminal::errMsg("Passwords do not match.");
        Terminal::waitEnter();
        return;
    }

    printf("\n");
    Terminal::boxTop(BMG);
    Terminal::boxCenter(BMG, BMG "  🛂  SELECT YOUR ROLE  " RS);
    Terminal::boxSep(BMG);
    for (int i = 0; i < SR_COUNT; i++)
    {
        char b[200];
        snprintf(b, 200, "  %s[%2d] %-20s" RS " " DIM "— %s" RS, SROLE_COL[i], i + 1, SROLES[i], SROLE_DESC[i]);
        Terminal::boxRowC(BMG, b);
    }
    Terminal::boxBot(BMG);
    int rc = Input::integer("Select Role:", 1, SR_COUNT) - 1;

    printf("\n");
    Terminal::boxTop(BMG);
    Terminal::boxCenter(BMG, BMG "  📋  PERSONAL DETAILS  " RS);
    Terminal::boxSep(BMG);
    string fn = Input::field("First Name:");
    string ln = Input::field("Last Name:");
    string em = Input::email("Email:");
    string ph = Input::phone("Phone:");
    string ter = Input::field("Terminal (e.g. T2):");
    string gts = Input::field("Assigned Gates (e.g. 10,11):");
    Terminal::boxBot(BMG);

    Staff s(un, fn, ln, em, ph, (StaffRole)rc, ter, gts);
    s.setPassword(pw);
    db_.addStaff(s);
    db_.saveAll();
    {
        string _m = "Account created! Employee ID: ";
        _m += db_.staffAt(db_.staffCount() - 1)->getEmpId();
        Terminal::success(_m.c_str());
    }
    Terminal::waitEnter();
}

bool AuthController::loginAdmin()
{
    Presentation::printBanner();
    Terminal::sectionHeader("ADMIN LOGIN", BYL);
    Terminal::boxTop(BYL);
    Terminal::boxCenter(BYL, BYL "  👑  ADMIN PORTAL  " RS);
    Terminal::boxSep(BYL);
    string un = Input::field("Username:");
    string pw = Input::password("Password:");
    string code = Input::password("2FA Admin Code:");
    Terminal::boxBot(BYL);

    if (code != ADMIN_CODE)
    {
        Terminal::glitch("  ██ ADMIN ACCESS DENIED ██");
        Terminal::errMsg("Invalid admin code.");
        Terminal::waitEnter();
        return false;
    }
    Terminal::spinner("Verifying admin credentials...", 900);
    int idx = db_.findAdminByUser(un);
    if (idx < 0 || !db_.adminAt(idx)->authenticate(pw))
    {
        Terminal::glitch("  ██ ACCESS DENIED ██");
        Terminal::errMsg("Invalid credentials.");
        Terminal::waitEnter();
        return false;
    }
    sess_.login(RADMIN, idx, un);
    Admin *a = db_.adminAt(idx);
    Presentation::printBanner();
    Terminal::sectionHeader("ADMIN LOGIN SUCCESSFUL", BYL);
    printf("  " BOLD BYL "✔  WELCOME, ADMINISTRATOR %s!\n\n" RS, a->getFullName().c_str());
    printf("  " CY "Access Level  : " RS BOLD BYL "FULL SYSTEM ACCESS" RS "\n");
    printf("  " CY "Flights       : " RS BWH "%d" RS "\n", db_.flightCount());
    printf("  " CY "Passengers    : " RS BWH "%d" RS "\n", db_.passCount());
    printf("  " CY "Staff Members : " RS BWH "%d" RS "\n", db_.staffCount());
    printf("  " CY "Total Bookings: " RS BWH "%d" RS "\n", db_.bookingCount());
    SoundEngine::login();
    Terminal::waitEnter();
    return true;
}

void AuthController::registerAdmin()
{
    Presentation::printBanner();
    Terminal::sectionHeader("ADMIN REGISTRATION", BYL);
    string code = Input::password("Admin Code:");
    if (code != ADMIN_CODE)
    {
        Terminal::glitch("  ██ ADMIN ACCESS DENIED ██");
        Terminal::errMsg("Invalid admin code.");
        Terminal::waitEnter();
        return;
    }
    Terminal::success("Admin code verified.");

    string un = Input::field("Username:");
    if (db_.findAdminByUser(un) >= 0)
    {
        Terminal::errMsg("Username taken.");
        Terminal::waitEnter();
        return;
    }
    string pw = Input::password("Password:", true);
    string pw2 = Input::password("Confirm:");
    if (pw != pw2)
    {
        Terminal::errMsg("Passwords do not match.");
        Terminal::waitEnter();
        return;
    }
    string fn = Input::field("First Name:");
    string ln = Input::field("Last Name:");
    string em = Input::email("Email:");

    Admin a(un, fn, ln, em);
    a.setPassword(pw);
    db_.addAdmin(a);
    db_.saveAll();
    Terminal::success("Admin account created.");
    Terminal::waitEnter();
}

void AuthController::searchFlights()
{
    Presentation::printBanner();
    Terminal::sectionHeader("FLIGHT SEARCH", BCY);
    string org = Input::field("From (city/code, Enter=all):");
    string dst = Input::field("To   (city/code, Enter=all):");
    Terminal::spinner("Searching flights...", 600);
    int found = 0;
    for (int i = 0; i < db_.flightCount(); i++)
    {
        Flight *fl = db_.flightAt(i);
        if (!fl->isActive())
            continue;
        bool mo = org.empty() || fl->getOrigin().find(org) != string::npos || fl->getFlightNo().find(org) != string::npos;
        bool md = dst.empty() || fl->getDest().find(dst) != string::npos;
        if (mo && md)
        {
            fl->displayCard(found);
            found++;
        }
    }
    if (!found)
        Terminal::infoMsg("No flights match your search.");
    else
    {
        char b[80];
        snprintf(b, 80, "Found %d flight(s).", found);
        Terminal::infoMsg(b);
    }
    Terminal::waitEnter();
}

void AuthController::flightStatus()
{
    Presentation::printBanner();
    Terminal::sectionHeader("FLIGHT STATUS LOOKUP", BCY);
    string fno = Input::field("Enter Flight Number:");
    Terminal::spinner("Fetching real-time data...", 700);
    int idx = db_.findFlightByNo(fno);
    if (idx < 0)
    {
        Terminal::errMsg("Flight not found.");
        Terminal::waitEnter();
        return;
    }
    Flight *fl = db_.flightAt(idx);
    {
        char b[60];
        snprintf(b, 60, "FLIGHT STATUS — %s", fl->getFlightNo().c_str());
        Presentation::printBanner();
        Terminal::sectionHeader(b, BCY);
    }
    const char *sc = Utils::statusColor(fl->getStatus().c_str());
    printf("\n");
    Terminal::boxTop(BCY);
    {
        char b[200];
        snprintf(b, 200, "  " BOLD BWH "✈ %s" RS "  │  " CY "%s" RS, fl->getFlightNo().c_str(), fl->getAirline().c_str());
        Terminal::boxRowC(BCY, b);
    }
    Terminal::boxSep(BCY);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Aircraft  : " RS BWH "%s" RS, fl->getAircraft().c_str());
        Terminal::boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "From      : " RS BWH "%s  (T%d)" RS, fl->getOrigin().c_str(), fl->getTermDep());
        Terminal::boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "To        : " RS BWH "%s  (T%d)" RS, fl->getDest().c_str(), fl->getTermArr());
        Terminal::boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Date      : " RS BWH "%s" RS, fl->getDate().c_str());
        Terminal::boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Departure : " RS BWH "%s" RS "   Arrival: " BWH "%s" RS, fl->getDepTime().c_str(), fl->getArrTime().c_str());
        Terminal::boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Gate      : " RS BWH "%d" RS "   Status: " RS "%s" BOLD "%s" RS, fl->getGate(), sc, fl->getStatus().c_str());
        Terminal::boxRowC(BCY, b);
    }
    if (fl->getStatus() == "DELAYED" && fl->getDelayReason() != "N/A")
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Reason    : " RS BYL "%s" RS, fl->getDelayReason().c_str());
        Terminal::boxRowC(BCY, b);
    }
    Terminal::boxSep(BCY);
    auto showCls = [&](const char *cls, int tot, int bk, const char *col)
    {
        int av = tot - bk;
        double pct = tot > 0 ? 100.0 * bk / tot : 0;
        char b[300];
        snprintf(b, 300, "  %s%-20s" RS " Total:" BWH "%3d" RS " Booked:" BWH "%3d" RS " " BGR "Avail:%3d" RS " [%.0f%%]", col, cls, tot, bk, av, pct);
        Terminal::boxRowC(BCY, b);
    };
    showCls("FIRST CLASS", fl->getSeatsFirst(), fl->getBkFirst(), BMG);
    showCls("BUSINESS CLASS", fl->getSeatsBiz(), fl->getBkBiz(), BYL);
    showCls("PREMIUM ECONOMY", fl->getSeatsPrem(), fl->getBkPrem(), BCY);
    showCls("ECONOMY", fl->getSeatsEco(), fl->getBkEco(), BGR);
    Terminal::boxBot(BCY);
    Terminal::waitEnter();
}

/*──────────────────────────────────────────────────────────────────────────────
  AuthController::run — main portal loop
──────────────────────────────────────────────────────────────────────────────*/
void AuthController::run()
{
    for (;;)
    {
        Presentation::printBanner();
        printf("\n");
        Terminal::boxTop(BCY);
        Terminal::boxCenter(BCY, BYL "⚡ NEXUS AIR" RS "  " BCY "ACCESS PORTAL" RS);
        Terminal::boxSep(BCY);
        Terminal::boxRowC(BCY, "  " BCY "── 👤  PASSENGER ───────────────────────────────────────" RS);
        Terminal::boxRowC(BCY, "  " BGR "[1]" RS "  🔑  Passenger Login");
        Terminal::boxRowC(BCY, "  " BGR "[2]" RS "  📝  Passenger Sign Up");
        Terminal::boxSep(BCY);
        Terminal::boxRowC(BCY, "  " BCY "── 🛂  STAFF  (code: 123a) ──────────────────────────────" RS);
        Terminal::boxRowC(BCY, "  " BMG "[3]" RS "  🔑  Staff Login");
        Terminal::boxRowC(BCY, "  " BMG "[4]" RS "  📝  Staff Sign Up  " DIM "(code required)" RS);
        Terminal::boxSep(BCY);
        Terminal::boxRowC(BCY, "  " BCY "── 👑  ADMIN  (code + 2FA: 123a) ────────────────────────" RS);
        Terminal::boxRowC(BCY, "  " BYL "[5]" RS "  🔑  Admin Login");
        Terminal::boxRowC(BCY, "  " BYL "[6]" RS "  📝  Admin Sign Up");
        Terminal::boxSep(BCY);
        Terminal::boxRowC(BCY, "  " BCY "── 🌐  PUBLIC ──────────────────────────────────────────" RS);
        Terminal::boxRowC(BCY, "  " BCY "[7]" RS "  🔍  Search Flights  (no login)");
        Terminal::boxRowC(BCY, "  " BCY "[8]" RS "  📡  Flight Status   (no login)");
        Terminal::boxSep(BCY);
        Terminal::boxRowC(BCY, "  " BRD "[9]" RS "  🚪  Exit NEXUS AIR");
        Terminal::boxBot(BCY);

        int ch = Input::integer("Choice [1-9]:", 1, 9);
        switch (ch)
        {
        case 1:
            if (loginPassenger())
            {
                PassengerDashboard pd(sess_, db_);
                pd.run();
            }
            break;
        case 2:
            registerPassenger();
            break;
        case 3:
            if (loginStaff())
            {
                StaffDashboard sd(sess_, db_);
                sd.run();
            }
            break;
        case 4:
            registerStaff();
            break;
        case 5:
            if (loginAdmin())
            {
                AdminDashboard ad(sess_, db_);
                ad.run();
            }
            break;
        case 6:
            registerAdmin();
            break;
        case 7:
            searchFlights();
            break;
        case 8:
            flightStatus();
            break;
        case 9:
            Presentation::exitAnim();
            return;
        default:
            Terminal::warnMsg("Invalid.");
            Utils::sleepMs(400);
        }
    }
}

/*══════════════════════════════════════════════════════════════════════════════
  main() — Application entry point
══════════════════════════════════════════════════════════════════════════════*/
int main()
{
    system("chcp 65001");
    /* Enable ANSI virtual terminal processing on Windows 10+ */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    system("title NEXUS AIR v5.0 - Airline Management System (OOP Edition)");
    setbuf(stdout, NULL);
    srand((unsigned)time(NULL));

    /* Boot sequence */
    Presentation::bootSeq();
    Presentation::aiLogs();
    Terminal::progressBar("Loading NEXUS AIR Airline Management System", 1300);
    Terminal::neuralPulse(8);
    Terminal::flyingPlane();
    Terminal::dataStream(4);

    /* CONCEPT [9] Singleton DataStore loaded once */
    printf("\n  " BYL "⚡ NEXUS AIR" RS "  loading saved data...\n");
    Terminal::neuralPulse(6);
    DataStore &db = DataStore::instance();

    /* CONCEPT [8] Exception handling around load */
    try
    {
        db.loadAll();
        db.seedData();
    }
    catch (const NexusException &e)
    {
        Terminal::errMsg(e.what());
    }
    catch (const exception &e)
    {
        Terminal::errMsg(e.what());
    }

    printf("\n  " BGR "✔  Data loaded:" RS
           "  Flights:" BWH "%d" RS
           "  Passengers:" BWH "%d" RS
           "  Staff:" BWH "%d" RS
           "  Bookings:" BWH "%d" RS "\n\n",
           db.flightCount(), db.passCount(), db.staffCount(), db.bookingCount());
    Terminal::waitEnter();

    /* Session object shared across the whole run */
    Session sess;
    AuthController auth(sess, db);

    /* Main loop */
    for (;;)
    {
        try
        {
            auth.run();
            break; /* run() exits only when user selects Exit (ch==9) */
        }
        catch (const NexusException &e)
        {
            Terminal::errMsg(e.what());
            Terminal::waitEnter();
        }
        catch (const exception &e)
        {
            Terminal::errMsg(e.what());
            Terminal::waitEnter();
        }
    }

    return 0;
}
