/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║          NEXUS AIR  ✈  Professional Airline Management System v5.0           ║
 ║                   Created by Muhammad Tahir Hussain                          ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <stdarg.h>
using namespace std;

#define CLEAR "cls"
static void sleepMs(int ms) { Sleep(ms); }

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

/* ─── Limits ──────────────────────────────────────────────── */
#define MAX_FL 200
#define MAX_PA 500
#define MAX_ST 80
#define MAX_AD 20
#define MAX_BK 800
#define MAX_BG 800
#define SZ 64

/* ─── Seat geometry ───────────────────────────────────────── */
#define ROWS_FIRST 2 /* rows 0-1   : 4 seats/row A B C D */
#define ROWS_BIZ 6   /* rows 2-7   : 4 seats/row A B C D */
#define ROWS_PREM 4  /* rows 8-11  : 6 seats/row A-F     */
#define ROWS_ECO 18  /* rows 12-29 : 6 seats/row A-F     */
#define TOTAL_ROWS 30

/* ─── Staff role codes ────────────────────────────────────── */
#define SR_GATE 0
#define SR_CHECKIN 1
#define SR_BAGGAGE 2
#define SR_SUPERVISOR 3
#define SR_TICKET 4
#define SR_SECURITY 5
#define SR_LOUNGE 6
#define SR_GROUND 7
#define SR_DISPATCH 8
#define SR_CUSTOMS 9
#define SR_COUNT 10

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

/* ═══════════════════════════════════════════════════════════
   DATA STRUCTURES
   ═══════════════════════════════════════════════════════════ */
struct SeatMap
{
    char flightNo[16];
    int st[TOTAL_ROWS][6];       /* 0=avail 1=booked 2=blocked */
    char who[TOTAL_ROWS][6][SZ]; /* passenger name */
};

struct Flight
{
    int id;
    char flightNo[16], airline[48], origin[48], dest[48];
    char date[16], depTime[8], arrTime[8], aircraft[40], status[20];
    int gate, termDep, termArr, duration;
    int seatsFirst, bkFirst, seatsBiz, bkBiz, seatsPrem, bkPrem, seatsEco, bkEco;
    double fareFirst, fareBiz, farePrem, fareEco, taxRate;
    char delayReason[80];
    int active;
};

struct Passenger
{
    int id;
    char username[SZ], passHash[24];
    char firstName[SZ], lastName[SZ];
    char email[SZ], phone[SZ];
    char passport[SZ], nationality[SZ], dob[16];
    char address[80], city[40], country[40];
    int loyaltyPts, totalMiles;
    char loyaltyTier[12], created[20];
    int active;
};

struct Staff
{
    int id;
    char empId[16], username[SZ], passHash[24];
    char firstName[SZ], lastName[SZ];
    char email[SZ], phone[SZ];
    int roleCode;
    char terminal[20], gates[40];
    char shiftStart[8], shiftEnd[8], created[20];
    int active;
};

struct Admin
{
    int id;
    char username[SZ], passHash[24];
    char firstName[SZ], lastName[SZ];
    char email[SZ], created[20];
    int active;
};

#define BAG_STAGES 8
static const char *BAG_ST[BAG_STAGES] = {
    "REGISTERED", "CHECK-IN DONE", "SECURITY CLEARED",
    "SORTED", "LOADED", "IN TRANSIT", "ARRIVED", "DELIVERED"};
static const char *BAG_COL[BAG_STAGES] = {DIM, BCY, BGR, BYL, BMG, BCY, BGR, BOLD BGR};

struct BagItem
{
    int id, bookingId;
    char bookingRef[20], passengerName[80], flightNo[16], tagNo[20];
    double weightKg;
    int stageIdx, fragile, special;
    char location[48], carousel[8], created[20];
};

struct Booking
{
    int id, passengerId, flightId;
    char flightNo[16], passengerName[80];
    char cabin[24], seatNo[8], meal[40];
    double baseFare, seatFee, baggageFee, mealFee, serviceFee, taxes, discount, grandTotal;
    char bookingRef[20], payMethod[24], cardLast4[8], status[16];
    int checkedIn, bagsCount;
    char boardingGate[8], boardingGroup[4], created[20], specialReq[60];
};

/* ─── Globals ──────────────────────────────────────────────── */
Flight flights[MAX_FL];
int flightCount = 0;
Passenger passengers[MAX_PA];
int passCount = 0;
Staff staffArr[MAX_ST];
int staffCount = 0;
Admin admins[MAX_AD];
int adminCount = 0;
Booking bookings[MAX_BK];
int bookingCount = 0;
BagItem bags[MAX_BG];
int bagCount = 0;
SeatMap seatMaps[MAX_FL];
int seatMapCount = 0;

int nextFl = 1001, nextPa = 2001, nextSt = 3001, nextAd = 4001, nextBk = 5001, nextBg = 6001;

enum Role
{
    RNONE = 0,
    RPASS,
    RSTAFF,
    RADMIN
};
struct Session
{
    Role role;
    int idx;
    char username[SZ];
    int loggedIn;
} sess = {};

/* ═══════════════════════════════════════════════════════════
   SOUNDS  (Windows Beep stubs work cross-platform)
   ═══════════════════════════════════════════════════════════ */
void sndBoot()
{
    int m[] = {400, 500, 600, 700, 900, 1100};
    for (int i = 0; i < 6; i++)
    {
        Beep(m[i], 80);
        Sleep(20);
    }
}
void sndOK()
{
    Beep(800, 70);
    Sleep(25);
    Beep(1050, 70);
    Sleep(25);
    Beep(1300, 160);
}
void sndErr()
{
    Beep(300, 180);
    Sleep(50);
    Beep(230, 180);
    Sleep(50);
    Beep(190, 300);
}
void sndClick() { Beep(900, 30); }
void sndTick() { Beep(1100, 18); }
void sndWarn()
{
    Beep(700, 130);
    Sleep(70);
    Beep(580, 130);
}
void sndLogin()
{
    Beep(880, 65);
    Sleep(25);
    Beep(1100, 65);
    Sleep(25);
    Beep(1320, 140);
}
void sndSel()
{
    Beep(950, 38);
    Sleep(12);
    Beep(1060, 38);
}
void sndPay()
{
    int m[] = {523, 659, 784, 1047, 1319};
    for (int i = 0; i < 5; i++)
    {
        Beep(m[i], 85);
        Sleep(28);
    }
}
void sndThink()
{
    Beep(600, 45);
    Sleep(90);
    Beep(650, 45);
    Sleep(90);
    Beep(600, 45);
    Sleep(90);
    Beep(700, 45);
}
void sndScan()
{
    Beep(1200, 55);
    Sleep(18);
    Beep(1450, 55);
}
void sndFly()
{
    for (int f = 350; f <= 1150; f += 55)
    {
        Beep(f, 18);
    }
}
void sndAlert()
{
    Beep(1000, 100);
    Beep(800, 100);
    Sleep(60);
    Beep(1000, 100);
}
void sndExit()
{
    int m[] = {1047, 988, 880, 784, 698, 659, 587, 523};
    for (int i = 0; i < 8; i++)
    {
        Beep(m[i], 95);
        Sleep(35);
    }
}

/* ═══════════════════════════════════════════════════════════
   BOX DRAWING ENGINE
   ═══════════════════════════════════════════════════════════ */
static int ansiLen(const char *s)
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
static void spc(int n)
{
    while (n-- > 0)
        putchar(' ');
}

/* UTF-8 box chars */
#define BT "\xe2\x95\x94"  /* ╔ */
#define BB "\xe2\x95\x9a"  /* ╚ */
#define BH "\xe2\x95\x90"  /* ═ */
#define BR "\xe2\x95\x97"  /* ╗ */
#define BL2 "\xe2\x95\x9d" /* ╝ */
#define BV "\xe2\x95\x91"  /* ║ */
#define BMT "\xe2\x95\xa0" /* ╠ */
#define BMB "\xe2\x95\xa3" /* ╣ */
#define BST "\xe2\x95\x9f" /* ╟ */
#define BSB "\xe2\x95\xa2" /* ╢ */
#define SH "\xe2\x94\x80"  /* ─ */

void boxTop(const char *c = BCY)
{
    printf("%s" BT, c);
    for (int i = 0; i < TW; i++)
        printf(BH);
    printf(BR RS "\n");
}
void boxBot(const char *c = BCY)
{
    printf("%s" BB, c);
    for (int i = 0; i < TW; i++)
        printf(BH);
    printf(BL2 RS "\n");
}
void boxMid(const char *c = BCY)
{
    printf("%s" BMT, c);
    for (int i = 0; i < TW; i++)
        printf(BH);
    printf(BMB RS "\n");
}
void boxSep(const char *c = BCY)
{
    printf("%s" BST, c);
    for (int i = 0; i < TW; i++)
        printf(SH);
    printf(BSB RS "\n");
}
void boxEmpty(const char *c = BCY)
{
    printf("%s" BV RS, c);
    spc(TW);
    printf("%s" BV RS "\n", c);
}

static void _row(const char *bc, const char *t, int pw)
{
    int p = TW - pw;
    if (p < 0)
        p = 0;
    printf("%s" BV RS "%s", bc, t);
    spc(p);
    printf("%s" BV RS "\n", bc);
}
void boxRow(const char *bc, const char *t) { _row(bc, t, (int)strlen(t)); }
void boxRowC(const char *bc, const char *t) { _row(bc, t, ansiLen(t)); }
void boxRowF(const char *bc, const char *fmt, ...)
{
    char buf[600];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, 600, fmt, ap);
    va_end(ap);
    boxRow(bc, buf);
}
static void boxCen(const char *bc, const char *t)
{
    int tw = ansiLen(t), lp = (TW - tw) / 2;
    if (lp < 0)
        lp = 0;
    int rp = TW - tw - lp;
    if (rp < 0)
        rp = 0;
    printf("%s" BV RS, bc);
    spc(lp);
    printf("%s", t);
    spc(rp);
    printf("%s" BV RS "\n", bc);
}
static void boxLR(const char *bc, const char *l, const char *r2)
{
    int lw = ansiLen(l), rw = ansiLen(r2), g = TW - lw - rw;
    if (g < 1)
        g = 1;
    printf("%s" BV RS "%s", bc, l);
    spc(g);
    printf("%s", r2);
    printf("%s" BV RS "\n", bc);
}

/* ═══════════════════════════════════════════════════════════
   ANIMATIONS
   ═══════════════════════════════════════════════════════════ */
void typeWrite(const char *t, int d = 16)
{
    for (int i = 0; t[i]; i++)
    {
        printf("%c", t[i]);
        fflush(stdout);
        sleepMs(d);
    }
}

void progressBar(const char *lbl, int ms = 1100)
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
        sndTick();
        sleepMs(ms / bw);
    }
    printf(RS "]  " BGR "✔" RS " Done");
    spc(TW - (4 + 1 + bw + 1 + 9));
    printf(BYL BV RS "\n");
    boxBot(BYL);
    fflush(stdout);
}

void spinner(const char *msg, int ms = 800)
{
    const char *fr[] = {"◐", "◓", "◑", "◒"};
    int steps = ms / 80;
    for (int i = 0; i < steps; i++)
    {
        printf("\r  " BCY "%s" RS "  " YL "%s" RS "   ", fr[i % 4], msg);
        fflush(stdout);
        sleepMs(80);
    }
    printf("\r  " BGR "✔" RS "  " GR "%s — Done!" RS "   \n", msg);
    sndOK();
}

void neuralPulse(int n)
{
    const char *d[] = {"●○○○○", "○●○○○", "○○●○○", "○○○●○", "○○○○●"};
    int lp = (TW + 2 - 10) / 2;
    if (lp < 0)
        lp = 0;
    for (int i = 0; i < n; i++)
    {
        printf("\r");
        spc(lp);
        printf(BCY "%s" RS " ", d[i % 5]);
        fflush(stdout);
        sleepMs(110);
    }
    printf("\r");
    spc(lp + 12);
    printf("\r");
    fflush(stdout);
}

void glitch(const char *text)
{
    const char *gc = "@#$%&*!?~^";
    int len = (int)strlen(text), gl = (int)strlen(gc);
    int lp = (TW + 2 - len) / 2;
    if (lp < 0)
        lp = 0;
    for (int p = 0; p < 3; p++)
    {
        printf("\r");
        spc(lp);
        printf(BCY);
        for (int i = 0; i < len; i++)
            printf("%c", text[i] == ' ' ? ' ' : gc[rand() % gl]);
        printf(RS);
        fflush(stdout);
        sleepMs(80);
    }
    printf("\r");
    spc(lp);
    printf(BRD BOLD "%s" RS "\n", text);
    fflush(stdout);
}

void dataStream(int lines)
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
        spc(lp);
        printf(DIM "  %s" RS "\n", s[i % 6]);
        fflush(stdout);
        sleepMs(85);
    }
}

void flyingPlane()
{
    printf("\n");
    for (int p = 0; p < TW + 2; p++)
    {
        printf("\r");
        spc(p);
        printf(BYL "✈" RS);
        fflush(stdout);
        sleepMs(11);
    }
    printf("\n");
    sndFly();
}

void starBurst(int lines)
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
        sleepMs(155);
    }
}

/* ═══════════════════════════════════════════════════════════
   MESSAGES
   ═══════════════════════════════════════════════════════════ */
void success(const char *m)
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
    sndOK();
}
void errMsg(const char *m)
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
    sndErr();
}
void infoMsg(const char *m)
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
void warnMsg(const char *m)
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
    sndWarn();
}

/* ═══════════════════════════════════════════════════════════
   UTILITIES
   ═══════════════════════════════════════════════════════════ */
void clearScreen() { system(CLEAR); }
void clearInput()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}
void waitEnter()
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
    sndClick();
    getchar();
}

static string getTS()
{
    time_t n = time(NULL);
    struct tm *t = localtime(&n);
    char b[20];
    strftime(b, 20, "%Y-%m-%d %H:%M", t);
    return string(b);
}
static string genRef()
{
    static int s = 1000;
    char b[20];
    snprintf(b, 20, "NX%04d", s++);
    return string(b);
}
static string genTag(const char *f)
{
    static int s = 100;
    char b[20];
    snprintf(b, 20, "%s-B%03d", f, s++);
    return string(b);
}
static string tier(int m)
{
    if (m >= 50000)
        return "PLATINUM";
    if (m >= 20000)
        return "GOLD";
    if (m >= 5000)
        return "SILVER";
    return "BRONZE";
}
static const char *tierCol(const char *t)
{
    if (!strcmp(t, "PLATINUM"))
        return BCY;
    if (!strcmp(t, "GOLD"))
        return BYL;
    if (!strcmp(t, "SILVER"))
        return BWH;
    return YL;
}
static const char *stCol(const char *s)
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

static string hashPW(const string &pw)
{
    unsigned long h = 5381;
    for (char c : pw)
        h = ((h << 5) + h) ^ (unsigned char)c;
    char b[24];
    snprintf(b, 24, "%012lu", h % 1000000000000UL);
    return string(b);
}

void readPW(char *buf, int mx)
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

void sectionHeader(const char *title, const char *col = BCY)
{
    printf("\n");
    boxTop(col);
    char b[256];
    snprintf(b, 256, "%s✈  %s  ✈" RS, col, title);
    boxCen(col, b);
    boxBot(col);
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════
   FORMATTED INPUT SYSTEM  — professional, validated
   ═══════════════════════════════════════════════════════════ */
string iField(const char *lbl, const char *hint = "")
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
    sndTick();
    return string(buf + s);
}

int iInt(const char *lbl, int lo = 0, int hi = 9999)
{
    for (;;)
    {
        printf("  " BYL " ▶ " RS BOLD BWH "%-24s" RS " " BCY, lbl);
        fflush(stdout);
        int v;
        if (scanf("%d", &v) != 1)
        {
            clearInput();
            continue;
        }
        clearInput();
        printf(RS);
        sndTick();
        if (v >= lo && v <= hi)
            return v;
        printf("  " BRD " ✘ Enter %d-%d\n" RS, lo, hi);
        sndErr();
    }
}

double iDbl(const char *lbl)
{
    printf("  " BYL " ▶ " RS BOLD BWH "%-24s" RS " " BCY, lbl);
    fflush(stdout);
    double v;
    if (scanf("%lf", &v) != 1)
        v = 0;
    clearInput();
    printf(RS);
    sndTick();
    return v;
}

string iPhone(const char *lbl)
{
    for (;;)
    {
        string s = iField(lbl, "+Code-Number");
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
        printf("  " BRD " ✘ Invalid phone (7-15 digits, may include +/-)\n" RS);
        sndErr();
    }
}

string iEmail(const char *lbl)
{
    for (;;)
    {
        string s = iField(lbl, "user@domain.com");
        if (s.find('@') != string::npos && s.find('.') != string::npos && s.size() > 5)
            return s;
        printf("  " BRD " ✘ Invalid email format.\n" RS);
        sndErr();
    }
}

string iDate(const char *lbl)
{
    for (;;)
    {
        string s = iField(lbl, "DD-MMM-YYYY e.g. 15-MAR-1990");
        if (s.size() >= 9)
            return s;
        printf("  " BRD " ✘ Use format: DD-MMM-YYYY\n" RS);
        sndErr();
    }
}

string iPass(const char *lbl, bool chk = false)
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
            sndErr();
            continue;
        }
        if (chk && (int)strlen(buf) < 8)
        {
            printf("  " BYL " ⚠ Weak password. Continue? [Y/N]: " RS " " BCY);
            char c;
            scanf(" %c", &c);
            clearInput();
            printf(RS);
            if (tolower(c) != 'y')
                continue;
        }
        return string(buf);
    }
}

string iPassport(const char *lbl)
{
    for (;;)
    {
        string s = iField(lbl, "e.g. AB1234567");
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
        printf("  " BRD " ✘ Passport: 6-12 alphanumeric chars.\n" RS);
        sndErr();
    }
}

string iCard16()
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
            sndTick();
            return raw;
        }
        printf("  " BRD " ✘ Exactly 16 digits required.\n" RS);
        sndErr();
    }
}

string iCVV()
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
        sndErr();
    }
}

string iExpiry()
{
    for (;;)
    {
        string s = iField("Expiry (MM/YY):", "e.g. 09/27");
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
        sndErr();
    }
}

void fieldSep()
{
    printf("  " DIM);
    for (int i = 0; i < TW - 2; i++)
        printf("─");
    printf(RS "\n");
}

/* ═══════════════════════════════════════════════════════════
   NEXUS AIR ASCII BANNER
   ═══════════════════════════════════════════════════════════ */
static int _sl = 0;
static const char *SL[][2] = {
    {"⚡  NEXUS AIR — CONNECTING THE WORLD, ONE FLIGHT AT A TIME  ⚡",
     "🌏 Precision. Innovation. Excellence in every journey."},
    {"🚀  BEYOND THE HORIZON WITH NEXUS AIR.",
     "💺 From booking to landing — a seamless premium experience"},
    {"🌟  WHERE EVERY DESTINATION IS A PROMISE.",
     "✈  Nexus Air — your trusted partner at 35,000 feet"},
    {"🎯  TECHNOLOGY MEETS ALTITUDE. NEXUS AIR LEADS.",
     "🛫 Innovative airline management for the modern world"},
    {"🏆  THE GOLD STANDARD IN AIRLINE EXCELLENCE.",
     "🌊 Smooth operations, happy passengers, on-time every time"},
};

void printBanner()
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
    int ns = 5;
    const char *s1 = SL[_sl % ns][0], *s2 = SL[_sl % ns][1];
    _sl++;
    system(CLEAR);
    printf("\n");
    printf(BCY BT);
    for (int i = 0; i < TW; i++)
        printf(BH);
    printf(BR RS "\n");
    {
        const char *lt = "  ✈ NEXUS AIR", *rt = "v5.0  ";
        int lw = 14, rw = (int)strlen(rt), g = TW - lw - rw;
        if (g < 1)
            g = 1;
        printf(BCY BV RS BCY "  ✈ NEXUS AIR" RS);
        spc(g);
        printf(BYL "%s" RS BCY BV RS "\n", rt);
    }
    printf(BCY BST);
    for (int i = 0; i < TW; i++)
        printf(SH);
    printf(BSB RS "\n");
    printf(BCY BV RS);
    spc(TW);
    printf(BCY BV RS "\n");
    for (int r = 0; r < 6; r++)
    {
        printf(BCY BV RS);
        spc(lp);
        printf(BYL "%s" RS, art[r]);
        spc(rp);
        printf(BCY BV RS "\n");
    }
    printf(BCY BV RS);
    spc(TW);
    printf(BCY BV RS "\n");
    printf(BCY BST);
    for (int i = 0; i < TW; i++)
        printf(SH);
    printf(BSB RS "\n");
    {
        char b[256];
        snprintf(b, 256, BMG "%s" RS, s1);
        boxCen(BCY, b);
    }
    {
        char b[256];
        snprintf(b, 256, DIM "%s" RS, s2);
        boxCen(BCY, b);
    }
    boxCen(BCY, DIM "  ✦  Created by  " BWH "Muhammad Tahir Hussain" DIM "  ✦  " RS);
    printf(BCY BV RS);
    spc(TW);
    printf(BCY BV RS "\n");
    static int _tk = 0;
    static const char *tks[] = {"  🌌 CONNECTING THE WORLD ONE NEXUS FLIGHT AT A TIME",
                                "  🔭 WHERE INNOVATION MEETS THE OPEN SKY",
                                "  🎨 PRECISION MANAGEMENT FOR EVERY ROUTE",
                                "  🦋 YOUR PASSENGERS TAKE WING WITH NEXUS AIR",
                                "  🌊 SMOOTH SKIES AHEAD — POWERED BY NEXUS",
                                "  🏛  BUILT ON TRUST. ENGINEERED FOR EXCELLENCE.",
                                "  🎭 EVERY GREAT JOURNEY BEGINS AT NEXUS AIR",
                                "  🎵 YOUR OPERATIONS, ORCHESTRATED TO PERFECTION"};
    static const char *rks[] = {"Fly. Manage. Excel.  ",
                                "Every flight, on time  ", "Nexus: precision altitude  ",
                                "The sky is no limit  ", "Your airline, reimagined  ",
                                "Fly bold. Fly Nexus.  ", "Where ideas take off  ",
                                "The future flies Nexus  "};
    int nt = 8;
    const char *lt = tks[_tk % nt], *rt2 = rks[_tk % nt];
    _tk++;
    int lw = (int)strlen(lt), rw = (int)strlen(rt2), g = TW - lw - rw;
    if (g < 1)
        g = 1;
    printf(BCY BV RS BMG "%s" RS, lt);
    spc(g);
    printf(BCY "%s" RS BCY BV RS "\n", rt2);
    printf(BCY BB);
    for (int i = 0; i < TW; i++)
        printf(BH);
    printf(BL2 RS "\n\n");
}

void bootSeq()
{
    system(CLEAR);
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
    spc(TW);
    printf(BCY BV RS "\n");
    for (int r = 0; r < 6; r++)
    {
        printf(BCY BV RS);
        spc(lp);
        printf(BYL "%s" RS, art[r]);
        spc(rp);
        printf(BCY BV RS "\n");
    }
    printf(BCY BV RS);
    spc(TW);
    printf(BCY BV RS "\n");
    printf(BCY BB);
    for (int i = 0; i < TW; i++)
        printf(BH);
    printf(BL2 RS "\n");
    fflush(stdout);
    sleepMs(280);
    printf("\n");
    int slp = (TW + 2 - 44) / 2;
    if (slp < 0)
        slp = 0;
    spc(slp);
    printf(BYL "  ⚡  Initialising NEXUS AIR Engine  " BGR);
    for (int i = 0; i < 6; i++)
    {
        printf(".");
        fflush(stdout);
        sndBoot();
        sleepMs(290);
    }
    printf("  " BWH "[ ONLINE ]" RS "\n\n");
    sleepMs(190);
}

void aiLogs()
{
    const char *ll[] = {
        "  🔧  Loading NEXUS AIR flight database modules",
        "  ⚡  Starting booking & payment engine",
        "  📖  Mounting passenger records & seat maps",
        "  🔐  Loading authentication system",
        "  🛡   Setting up role-based access control",
        "  ✈   Synchronising real-time flight & baggage status",
        "  ✅  NEXUS AIR v5.0 is ONLINE and READY",
    };
    const char *cc[] = {DIM, DIM, DIM, DIM, DIM, DIM, BGR};
    boxTop(DIM);
    for (int i = 0; i < 7; i++)
    {
        char b[128];
        snprintf(b, 128, "%s%s" RS, cc[i], ll[i]);
        boxRowC(DIM, b);
        fflush(stdout);
        sleepMs(75);
    }
    boxBot(DIM);
    printf("\n");
}

void exitAnim()
{
    system(CLEAR);
    sleepMs(75);
    starBurst(3);
    printf("\n");
    flyingPlane();
    printf("\n");
    boxTop(BCY);
    boxCen(BCY, BYL "⚡  THANK YOU FOR FLYING NEXUS AIR  ⚡" RS);
    boxMid(BCY);
    const char *fw[] = {
        "  ✨  Thank you for using NEXUS AIR Management System!",
        "  💡  Every flight you managed made our skies better.",
        "  🌊  Your data is safe — NEXUS AIR never forgets.",
        "  🎯  Come back anytime. The skies await.",
        "",
        "  ✦  Created by  Muhammad Tahir Hussain  ✦",
        "",
        "  🚀  NEXUS AIR v5.0  —  Until next time...",
    };
    for (int i = 0; i < 8; i++)
    {
        int tl = (int)strlen(fw[i]), pd = TW - tl;
        if (pd < 0)
            pd = 0;
        printf(BMG BV RS);
        printf(BWH);
        for (int c = 0; fw[i][c]; c++)
        {
            printf("%c", fw[i][c]);
            fflush(stdout);
            sleepMs(17);
        }
        printf(RS);
        spc(pd);
        printf(BMG BV RS "\n");
        sleepMs(95);
    }
    boxMid(BCY);
    {
        int bw = TW - 8;
        printf(BCY BV RS "    [" BGR);
        for (int i = 0; i < bw; i++)
            printf("░");
        printf(RS "]    " BCY BV RS);
        printf("\r" BCY BV RS "    [" BGR);
        fflush(stdout);
        for (int i = 0; i < bw; i++)
        {
            printf("█");
            fflush(stdout);
            sleepMs(10);
        }
        printf(RS "]  " BGR "✔" RS);
        spc(TW - (4 + 1 + bw + 1 + 4));
        printf(BCY BV RS "\n");
    }
    boxBot(BCY);
    sndExit();
    sleepMs(600);
    system(CLEAR);
}

/* ═══════════════════════════════════════════════════════════
   SEAT MAP ENGINE
   ═══════════════════════════════════════════════════════════ */
SeatMap *getSM(const char *fno)
{
    for (int i = 0; i < seatMapCount; i++)
        if (!strcmp(seatMaps[i].flightNo, fno))
            return &seatMaps[i];
    if (seatMapCount >= MAX_FL)
        return nullptr;
    SeatMap &sm = seatMaps[seatMapCount++];
    strncpy(sm.flightNo, fno, 15);
    memset(sm.st, 0, sizeof(sm.st));
    memset(sm.who, 0, sizeof(sm.who));
    return &sm;
}

static const char *rowCabin(int r)
{
    if (r < ROWS_FIRST)
        return "FIRST";
    if (r < ROWS_FIRST + ROWS_BIZ)
        return "BUSINESS";
    if (r < ROWS_FIRST + ROWS_BIZ + ROWS_PREM)
        return "PREMIUM_ECO";
    return "ECONOMY";
}

static double rowFare(int r, const Flight &fl)
{
    const char *c = rowCabin(r);
    if (!strcmp(c, "FIRST"))
        return fl.fareFirst;
    if (!strcmp(c, "BUSINESS"))
        return fl.fareBiz;
    if (!strcmp(c, "PREMIUM_ECO"))
        return fl.farePrem;
    return fl.fareEco;
}

static int rowCols(int r) { return (r < ROWS_FIRST + ROWS_BIZ) ? 4 : 6; }

static char colLet(int c, int cols)
{
    if (cols == 4)
    {
        const char *l = "ABCD";
        return l[c];
    }
    const char *l = "ABCDEF";
    return l[c];
}

/* ── Seat map display ─────────────────────────────────────── */
void showSeatMap(SeatMap *sm, const Flight &fl)
{
    printBanner();
    printf("\n");
    boxTop(BCY);
    {
        char b[200];
        snprintf(b, 200, "  ✈  " BOLD BCY "NEXUS AIR — INTERACTIVE SEAT MAP" RS "  Flight " BYL "%s" RS, fl.flightNo);
        boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Aircraft: " RS BWH "%s" RS "   Date: " BWH "%s" RS "   Route: " BWH "%s → %s" RS, fl.aircraft, fl.date, fl.origin, fl.dest);
        boxRowC(BCY, b);
    }
    boxSep(BCY);
    boxRowC(BCY, "  Legend:  " BGR "[◉]" RS " Available   " BRD "[X]" RS " Taken   " BYL "[★]" RS " Selected   " DIM "[▪]" RS " Blocked");
    boxSep(BCY);
    /* fare summary */
    {
        char b[300];
        snprintf(b, 300, "  " BMG "FIRST $%.0f" RS "  " BYL "BUSINESS $%.0f" RS "  " BCY "PREM-ECO $%.0f" RS "  " BGR "ECONOMY $%.0f" RS,
                 fl.fareFirst, fl.fareBiz, fl.farePrem, fl.fareEco);
        boxRowC(BCY, b);
    }
    boxSep(BCY);
    /* Header */
    boxRowC(BCY, "  " DIM "Class             Row  |  A    B    C  |  D    E    F  | Fare/seat" RS);
    boxSep(BCY);

    const char *lastCab = "";
    int dispRow = 10;
    for (int r = 0; r < TOTAL_ROWS; r++)
    {
        int cols = rowCols(r);
        const char *cab = rowCabin(r);
        double fare = rowFare(r, fl);
        const char *cabCol = (!strcmp(cab, "FIRST")) ? BMG : (!strcmp(cab, "BUSINESS"))  ? BYL
                                                         : (!strcmp(cab, "PREMIUM_ECO")) ? BCY
                                                                                         : BGR;

        if (strcmp(cab, lastCab))
        {
            if (*lastCab)
                boxSep(BCY);
            char cls[60];
            if (!strcmp(cab, "FIRST"))
                strcpy(cls, "  ✦ FIRST CLASS");
            else if (!strcmp(cab, "BUSINESS"))
                strcpy(cls, "  ◆ BUSINESS CLASS");
            else if (!strcmp(cab, "PREMIUM_ECO"))
                strcpy(cls, "  ▲ PREMIUM ECONOMY");
            else
                strcpy(cls, "  ● ECONOMY CLASS");
            char b2[120];
            snprintf(b2, 120, "%s%s" RS "  " DIM "Base fare: " RS BWH "$%.0f" RS, cabCol, cls, fare);
            boxRowC(BCY, b2);
            boxSep(BCY);
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
            int st = sm->st[r][c];
            const char *sym = (st == 0) ? (BGR "[◉]" RS) : (st == 1) ? (BRD "[X]" RS)
                                                                     : (DIM "[▪]" RS);
            strncat(line, sym, sizeof(line) - strlen(line) - 1);
            strncat(line, "  ", sizeof(line) - strlen(line) - 1);
        }
        /* fare */
        {
            char fs[24];
            snprintf(fs, 24, " | $%.0f", fare);
            strncat(line, fs, sizeof(line) - strlen(line) - 1);
        }
        boxRowC(BCY, line);
        dispRow++;
    }
    boxBot(BCY);
}

/* ── Seat selection ───────────────────────────────────────── */
bool selectSeat(SeatMap *sm, const Flight &fl,
                int &outRow, int &outCol, double &outFare, char outSeat[8])
{
    showSeatMap(sm, fl);
    printf("\n");
    boxTop(BYL);
    boxRowC(BYL, "  ✈  SEAT SELECTION  — Enter seat e.g. " BWH "12A" RS "  or  " BRD "CANCEL" RS " to go back");
    boxRowC(BYL, "  " DIM "Row 10-11: First   12-17: Business   18-21: Prem-Eco   22-39: Economy" RS);
    boxBot(BYL);

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
            warnMsg("Invalid. Use row+col e.g. 12A");
            continue;
        }

        char colCh = toupper(inp[strlen(inp) - 1]);
        char rowStr[8] = {0};
        strncpy(rowStr, inp, strlen(inp) - 1);
        int dispRow = atoi(rowStr);
        int r = dispRow - 10;
        if (r < 0 || r >= TOTAL_ROWS)
        {
            warnMsg("Row out of range (10-39)");
            sndErr();
            continue;
        }

        int cols = rowCols(r);
        int c = -1;
        for (int i = 0; i < cols; i++)
            if (colLet(i, cols) == colCh)
            {
                c = i;
                break;
            }
        if (c < 0)
        {
            warnMsg("Invalid column for this class");
            sndErr();
            continue;
        }
        if (sm->st[r][c] == 1)
        {
            errMsg("Seat already booked!");
            continue;
        }
        if (sm->st[r][c] == 2)
        {
            errMsg("Seat is blocked.");
            continue;
        }

        const char *cab = rowCabin(r);
        double fare = rowFare(r, fl);
        const char *cabCol = (!strcmp(cab, "FIRST")) ? BMG : (!strcmp(cab, "BUSINESS"))  ? BYL
                                                         : (!strcmp(cab, "PREMIUM_ECO")) ? BCY
                                                                                         : BGR;
        char seatLbl[8];
        snprintf(seatLbl, 8, "%d%c", dispRow, colLet(c, cols));

        printf("\n");
        boxTop(BGR);
        {
            char b[200];
            snprintf(b, 200, "  ☆  Seat " BOLD BYL "%s" RS "  selected!  Class: %s%s" RS "  Fare: " BGR "$%.2f" RS, seatLbl, cabCol, cab, fare);
        }
        {
            char b[200];
            snprintf(b, 200, "  ☆  Seat " BYL "%s" RS "   Class: %s%s" RS "   Seat Fee: " BGR "$%.2f" RS, seatLbl, cabCol, cab, fare);
            boxRowC(BGR, b);
        }
        boxRowC(BGR, "  " DIM "Note: seat fare is charged on top of the class base fare." RS);
        boxBot(BGR);
        sndSel();

        printf("  " BYL " ▶ " RS BWH "Confirm seat " BYL "%s" RS "? [Y/N]: " RS " " BCY);
        fflush(stdout);
        char conf;
        scanf(" %c", &conf);
        clearInput();
        printf(RS);
        if (tolower(conf) == 'y')
        {
            outRow = r;
            outCol = c;
            outFare = fare;
            strncpy(outSeat, seatLbl, 7);
            sndOK();
            return true;
        }
    }
}

/* Seat fee calculator (extra on top of base, based on position) */
double seatFeeForRow(int r, const Flight &fl)
{
    const char *c = rowCabin(r);
    if (!strcmp(c, "FIRST"))
        return 0; /* included */
    if (!strcmp(c, "BUSINESS"))
        return 50; /* premium position */
    if (!strcmp(c, "PREMIUM_ECO"))
        return 35;
    /* economy: exit rows extra */
    if (r == 14 || r == 15)
        return 25;
    if (r <= 13)
        return 15; /* front eco */
    return 0;
} /* standard */

/* ═══════════════════════════════════════════════════════════
   BAGGAGE SYSTEM
   ═══════════════════════════════════════════════════════════ */
void showBagStatus(const BagItem &bg)
{
    printf("\n");
    boxTop(BCY);
    {
        char b[200];
        snprintf(b, 200, "  🟢  Tag: " BWH "%s" RS "   Flight: " BWH "%s" RS "   Pax: " BWH "%s" RS, bg.tagNo, bg.flightNo, bg.passengerName);
        boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Weight: " BWH "%.1fkg" RS "   Fragile: %s   Special: %s",
                 bg.weightKg, bg.fragile ? (BRD "YES" RS) : (BGR "NO" RS), bg.special ? (BYL "YES" RS) : (DIM "NO" RS));
        boxRowC(BCY, b);
    }
    boxSep(BCY);
    /* Progress chain */
    for (int i = 0; i < BAG_STAGES; i++)
    {
        char b[100];
        if (i <= bg.stageIdx)
            snprintf(b, 100, "  %s[%d] %s" RS " %s", BAG_COL[i], i + 1, BAG_ST[i], (i == bg.stageIdx) ? " ← CURRENT" : "");
        else
            snprintf(b, 100, "  " DIM "[%d] %s" RS, i + 1, BAG_ST[i]);
        boxRowC(BCY, b);
    }
    boxSep(BCY);
    {
        char b[200];
        snprintf(b, 200, "  Location: " BWH "%s" RS, bg.location);
        boxRowC(BCY, b);
    }
    if (bg.stageIdx >= 6 && *bg.carousel)
    {
        char b[80];
        snprintf(b, 80, "  Carousel: " BYL "%s" RS, bg.carousel);
        boxRowC(BCY, b);
    }
    boxBot(BCY);
}

/* ═══════════════════════════════════════════════════════════
   FILE I/O
   ═══════════════════════════════════════════════════════════ */
void saveFlights()
{
    FILE *f = fopen(FILE_FLIGHTS, "w");
    if (!f)
    {
        errMsg("Cannot write flights.txt");
        return;
    }
    fprintf(f, "COUNT:%d\n", flightCount);
    for (int i = 0; i < flightCount; i++)
    {
        Flight &fl = flights[i];
        fprintf(f, "%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%.2f|%.2f|%.2f|%.2f|%.2f|%s|%d\n",
                fl.id, fl.flightNo, fl.airline, fl.origin, fl.dest, fl.date, fl.depTime, fl.arrTime,
                fl.aircraft, fl.status, fl.gate, fl.termDep, fl.termArr, fl.duration,
                fl.seatsFirst, fl.bkFirst, fl.seatsBiz, fl.bkBiz, fl.seatsPrem, fl.bkPrem, fl.seatsEco, fl.bkEco,
                fl.fareFirst, fl.fareBiz, fl.farePrem, fl.fareEco, fl.taxRate, fl.delayReason, fl.active);
    }
    fclose(f);
}
void loadFlights()
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
    sscanf(line, "COUNT:%d", &flightCount);
    if (flightCount > MAX_FL)
        flightCount = MAX_FL;
    for (int i = 0; i < flightCount; i++)
    {
        if (!fgets(line, 512, f))
            break;
        line[strcspn(line, "\n")] = 0;
        Flight &fl = flights[i];
        sscanf(line, "%d|%15[^|]|%47[^|]|%47[^|]|%47[^|]|%15[^|]|%7[^|]|%7[^|]|%39[^|]|%19[^|]|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%lf|%lf|%lf|%lf|%lf|%79[^|]|%d",
               &fl.id, fl.flightNo, fl.airline, fl.origin, fl.dest, fl.date, fl.depTime, fl.arrTime,
               fl.aircraft, fl.status, &fl.gate, &fl.termDep, &fl.termArr, &fl.duration,
               &fl.seatsFirst, &fl.bkFirst, &fl.seatsBiz, &fl.bkBiz, &fl.seatsPrem, &fl.bkPrem, &fl.seatsEco, &fl.bkEco,
               &fl.fareFirst, &fl.fareBiz, &fl.farePrem, &fl.fareEco, &fl.taxRate, fl.delayReason, &fl.active);
        if (fl.id >= nextFl)
            nextFl = fl.id + 1;
    }
    fclose(f);
}

void savePass()
{
    FILE *f = fopen(FILE_PASS, "w");
    if (!f)
        return;
    fprintf(f, "COUNT:%d\n", passCount);
    for (int i = 0; i < passCount; i++)
    {
        Passenger &p = passengers[i];
        fprintf(f, "%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%d|%d|%s|%s|%d\n",
                p.id, p.username, p.passHash, p.firstName, p.lastName, p.email, p.phone,
                p.passport, p.nationality, p.dob, p.address, p.city, p.country,
                p.loyaltyPts, p.totalMiles, p.loyaltyTier, p.created, p.active);
    }
    fclose(f);
}
void loadPass()
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
    sscanf(line, "COUNT:%d", &passCount);
    if (passCount > MAX_PA)
        passCount = MAX_PA;
    for (int i = 0; i < passCount; i++)
    {
        if (!fgets(line, 600, f))
            break;
        line[strcspn(line, "\n")] = 0;
        Passenger &p = passengers[i];
        sscanf(line, "%d|%63[^|]|%23[^|]|%63[^|]|%63[^|]|%63[^|]|%63[^|]|%63[^|]|%63[^|]|%15[^|]|%79[^|]|%39[^|]|%39[^|]|%d|%d|%11[^|]|%19[^|]|%d",
               &p.id, p.username, p.passHash, p.firstName, p.lastName, p.email, p.phone,
               p.passport, p.nationality, p.dob, p.address, p.city, p.country,
               &p.loyaltyPts, &p.totalMiles, p.loyaltyTier, p.created, &p.active);
        if (p.id >= nextPa)
            nextPa = p.id + 1;
    }
    fclose(f);
}

void saveStaff()
{
    FILE *f = fopen(FILE_STAFF, "w");
    if (!f)
        return;
    fprintf(f, "COUNT:%d\n", staffCount);
    for (int i = 0; i < staffCount; i++)
    {
        Staff &s = staffArr[i];
        fprintf(f, "%d|%s|%s|%s|%s|%s|%s|%s|%d|%s|%s|%s|%s|%s|%d\n",
                s.id, s.empId, s.username, s.passHash, s.firstName, s.lastName,
                s.email, s.phone, s.roleCode, s.terminal, s.gates, s.shiftStart, s.shiftEnd, s.created, s.active);
    }
    fclose(f);
}
void loadStaff()
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
    sscanf(line, "COUNT:%d", &staffCount);
    if (staffCount > MAX_ST)
        staffCount = MAX_ST;
    for (int i = 0; i < staffCount; i++)
    {
        if (!fgets(line, 512, f))
            break;
        line[strcspn(line, "\n")] = 0;
        Staff &s = staffArr[i];
        sscanf(line, "%d|%15[^|]|%63[^|]|%23[^|]|%63[^|]|%63[^|]|%63[^|]|%63[^|]|%d|%19[^|]|%39[^|]|%7[^|]|%7[^|]|%19[^|]|%d",
               &s.id, s.empId, s.username, s.passHash, s.firstName, s.lastName,
               s.email, s.phone, &s.roleCode, s.terminal, s.gates, s.shiftStart, s.shiftEnd, s.created, &s.active);
        if (s.id >= nextSt)
            nextSt = s.id + 1;
    }
    fclose(f);
}

void saveAdmins()
{
    FILE *f = fopen(FILE_ADMINS, "w");
    if (!f)
        return;
    fprintf(f, "COUNT:%d\n", adminCount);
    for (int i = 0; i < adminCount; i++)
    {
        Admin &a = admins[i];
        fprintf(f, "%d|%s|%s|%s|%s|%s|%s|%d\n", a.id, a.username, a.passHash, a.firstName, a.lastName, a.email, a.created, a.active);
    }
    fclose(f);
}
void loadAdmins()
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
    sscanf(line, "COUNT:%d", &adminCount);
    if (adminCount > MAX_AD)
        adminCount = MAX_AD;
    for (int i = 0; i < adminCount; i++)
    {
        if (!fgets(line, 400, f))
            break;
        line[strcspn(line, "\n")] = 0;
        Admin &a = admins[i];
        sscanf(line, "%d|%63[^|]|%23[^|]|%63[^|]|%63[^|]|%63[^|]|%19[^|]|%d",
               &a.id, a.username, a.passHash, a.firstName, a.lastName, a.email, a.created, &a.active);
        if (a.id >= nextAd)
            nextAd = a.id + 1;
    }
    fclose(f);
}

void saveBookings()
{
    FILE *f = fopen(FILE_BOOKINGS, "w");
    if (!f)
        return;
    fprintf(f, "COUNT:%d\n", bookingCount);
    for (int i = 0; i < bookingCount; i++)
    {
        Booking &b = bookings[i];
        fprintf(f, "%d|%d|%d|%s|%s|%s|%s|%s|%.2f|%.2f|%.2f|%.2f|%.2f|%.2f|%.2f|%.2f|%s|%s|%s|%s|%d|%d|%s|%s|%s|%s\n",
                b.id, b.passengerId, b.flightId, b.flightNo, b.passengerName, b.cabin, b.seatNo, b.meal,
                b.baseFare, b.seatFee, b.baggageFee, b.mealFee, b.serviceFee, b.taxes, b.discount, b.grandTotal,
                b.bookingRef, b.payMethod, b.cardLast4, b.status, b.checkedIn, b.bagsCount,
                b.boardingGate, b.boardingGroup, b.created, b.specialReq);
    }
    fclose(f);
}
void loadBookings()
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
    sscanf(line, "COUNT:%d", &bookingCount);
    if (bookingCount > MAX_BK)
        bookingCount = MAX_BK;
    for (int i = 0; i < bookingCount; i++)
    {
        if (!fgets(line, 800, f))
            break;
        line[strcspn(line, "\n")] = 0;
        Booking &b = bookings[i];
        sscanf(line, "%d|%d|%d|%15[^|]|%79[^|]|%23[^|]|%7[^|]|%39[^|]|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%19[^|]|%23[^|]|%7[^|]|%15[^|]|%d|%d|%7[^|]|%3[^|]|%19[^|]|%59[^\n]",
               &b.id, &b.passengerId, &b.flightId, b.flightNo, b.passengerName, b.cabin, b.seatNo, b.meal,
               &b.baseFare, &b.seatFee, &b.baggageFee, &b.mealFee, &b.serviceFee, &b.taxes, &b.discount, &b.grandTotal,
               b.bookingRef, b.payMethod, b.cardLast4, b.status, &b.checkedIn, &b.bagsCount,
               b.boardingGate, b.boardingGroup, b.created, b.specialReq);
        if (b.id >= nextBk)
            nextBk = b.id + 1;
    }
    fclose(f);
}

void saveBags()
{
    FILE *f = fopen(FILE_BAGGAGE, "w");
    if (!f)
        return;
    fprintf(f, "COUNT:%d\n", bagCount);
    for (int i = 0; i < bagCount; i++)
    {
        BagItem &bg = bags[i];
        fprintf(f, "%d|%d|%s|%s|%s|%s|%.2f|%d|%s|%s|%s|%d|%d\n",
                bg.id, bg.bookingId, bg.bookingRef, bg.passengerName, bg.flightNo,
                bg.tagNo, bg.weightKg, bg.stageIdx, bg.location, bg.carousel, bg.created, bg.fragile, bg.special);
    }
    fclose(f);
}
void loadBags()
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
    sscanf(line, "COUNT:%d", &bagCount);
    if (bagCount > MAX_BG)
        bagCount = MAX_BG;
    for (int i = 0; i < bagCount; i++)
    {
        if (!fgets(line, 512, f))
            break;
        line[strcspn(line, "\n")] = 0;
        BagItem &bg = bags[i];
        sscanf(line, "%d|%d|%19[^|]|%79[^|]|%15[^|]|%19[^|]|%lf|%d|%47[^|]|%7[^|]|%19[^|]|%d|%d",
               &bg.id, &bg.bookingId, bg.bookingRef, bg.passengerName, bg.flightNo,
               bg.tagNo, &bg.weightKg, &bg.stageIdx, bg.location, bg.carousel, bg.created, &bg.fragile, &bg.special);
        if (bg.id >= nextBg)
            nextBg = bg.id + 1;
    }
    fclose(f);
}

void saveSeatMaps()
{
    FILE *f = fopen(FILE_SEATS, "w");
    if (!f)
        return;
    fprintf(f, "COUNT:%d\n", seatMapCount);
    for (int i = 0; i < seatMapCount; i++)
    {
        SeatMap &sm = seatMaps[i];
        fprintf(f, "%s\n", sm.flightNo);
        for (int r = 0; r < TOTAL_ROWS; r++)
        {
            for (int c = 0; c < 6; c++)
                fprintf(f, "%d|%s|", sm.st[r][c], sm.who[r][c]);
            fprintf(f, "\n");
        }
    }
    fclose(f);
}
void loadSeatMaps()
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
    sscanf(line, "COUNT:%d", &seatMapCount);
    for (int i = 0; i < seatMapCount && i < MAX_FL; i++)
    {
        SeatMap &sm = seatMaps[i];
        if (!fgets(line, 1024, f))
            break;
        line[strcspn(line, "\n")] = 0;
        strncpy(sm.flightNo, line, 15);
        for (int r = 0; r < TOTAL_ROWS; r++)
        {
            if (!fgets(line, 1024, f))
                break;
            line[strcspn(line, "\n")] = 0;
            char *p = line;
            for (int c = 0; c < 6; c++)
            {
                sm.st[r][c] = atoi(p);
                p = strchr(p, '|');
                if (!p)
                    break;
                p++;
                char *end = strchr(p, '|');
                if (end)
                {
                    strncpy(sm.who[r][c], p, SZ - 1);
                    sm.who[r][c][SZ - 1] = 0;
                    p = end + 1;
                }
            }
        }
    }
    fclose(f);
}

void loadAll()
{
    loadFlights();
    loadPass();
    loadStaff();
    loadAdmins();
    loadBookings();
    loadBags();
    loadSeatMaps();
}
void saveAll()
{
    saveFlights();
    savePass();
    saveStaff();
    saveAdmins();
    saveBookings();
    saveBags();
    saveSeatMaps();
}

/* ═══════════════════════════════════════════════════════════
   SEED DATA
   ═══════════════════════════════════════════════════════════ */
void seedData()
{
    if (flightCount > 0)
        return;
    struct
    {
        const char *no, *org, *dst, *date, *dep, *arr, *ac;
        int gate, td, ta, dur;
        double f1, f2, f3, f4;
    } fd[] = {
        {"NX101", "Karachi (KHI)", "Dubai (DXB)", "20-MAR-2026", "09:00", "11:00", "Airbus A320-Neo", 10, 1, 3, 120, 2500, 1200, 700, 280},
        {"NX202", "Lahore (LHE)", "London (LHR)", "21-MAR-2026", "14:00", "18:45", "Boeing 777-300ER", 22, 2, 5, 480, 3200, 1800, 1100, 650},
        {"NX303", "Islamabad (ISB)", "New York (JFK)", "21-MAR-2026", "18:30", "06:45+1", "Boeing 787-9 Dreamliner", 42, 3, 8, 810, 4500, 2400, 1600, 890},
        {"NX404", "Karachi (KHI)", "Kuala Lumpur (KUL)", "22-MAR-2026", "23:00", "09:30+1", "Airbus A330-900Neo", 15, 1, 4, 540, 3100, 1700, 950, 420},
        {"NX505", "Dubai (DXB)", "London (LHR)", "22-MAR-2026", "10:30", "14:15", "Airbus A380-800", 30, 3, 5, 405, 3800, 2100, 1300, 720},
    };
    for (int i = 0; i < 5; i++)
    {
        Flight &fl = flights[flightCount++];
        fl.id = nextFl++;
        strncpy(fl.flightNo, fd[i].no, 15);
        strcpy(fl.airline, "Nexus Air");
        strncpy(fl.origin, fd[i].org, 47);
        strncpy(fl.dest, fd[i].dst, 47);
        strncpy(fl.date, fd[i].date, 15);
        strncpy(fl.depTime, fd[i].dep, 7);
        strncpy(fl.arrTime, fd[i].arr, 7);
        strncpy(fl.aircraft, fd[i].ac, 39);
        strcpy(fl.status, "ON_TIME");
        fl.gate = fd[i].gate;
        fl.termDep = fd[i].td;
        fl.termArr = fd[i].ta;
        fl.duration = fd[i].dur;
        fl.seatsFirst = ROWS_FIRST * 4;
        fl.seatsBiz = ROWS_BIZ * 4;
        fl.seatsPrem = ROWS_PREM * 6;
        fl.seatsEco = ROWS_ECO * 6;
        fl.fareFirst = fd[i].f1;
        fl.fareBiz = fd[i].f2;
        fl.farePrem = fd[i].f3;
        fl.fareEco = fd[i].f4;
        fl.taxRate = 16.72;
        strcpy(fl.delayReason, "N/A");
        fl.active = 1;
        getSM(fl.flightNo);
    }
    saveFlights();
    saveSeatMaps();
}

/* ─── Finders ──────────────────────────────────────────────── */
int findPass(const char *u)
{
    for (int i = 0; i < passCount; i++)
        if (!strcmp(passengers[i].username, u) && passengers[i].active)
            return i;
    return -1;
}
int findStaff(const char *u)
{
    for (int i = 0; i < staffCount; i++)
        if (!strcmp(staffArr[i].username, u) && staffArr[i].active)
            return i;
    return -1;
}
int findAdmin(const char *u)
{
    for (int i = 0; i < adminCount; i++)
        if (!strcmp(admins[i].username, u) && admins[i].active)
            return i;
    return -1;
}
int findFlight(const char *no)
{
    for (int i = 0; i < flightCount; i++)
        if (!strcmp(flights[i].flightNo, no) && flights[i].active)
            return i;
    return -1;
}
int findBooking(const char *ref)
{
    for (int i = 0; i < bookingCount; i++)
        if (!strcmp(bookings[i].bookingRef, ref))
            return i;
    return -1;
}

/* ═══════════════════════════════════════════════════════════
   PROFESSIONAL BILLING ENGINE
   ═══════════════════════════════════════════════════════════ */
void printBill(const Booking &b, const Flight &fl)
{
    printf("\n");
    boxTop(BYL);
    boxCen(BYL, BYL BOLD "  NEXUS AIR — OFFICIAL BILLING STATEMENT  " RS);
    boxSep(BYL);
    {
        char buf[200];
        snprintf(buf, 200, "  " DIM "Booking Ref  : " RS BWH "%s" RS "   Issued: " BWH "%s" RS, b.bookingRef, b.created);
        boxRowC(BYL, buf);
    }
    {
        char buf[200];
        snprintf(buf, 200, "  " DIM "Passenger    : " RS BWH "%s" RS, b.passengerName);
        boxRowC(BYL, buf);
    }
    {
        char buf[200];
        snprintf(buf, 200, "  " DIM "Flight       : " RS BWH "%s" RS "  " BWH "%s" RS " → " BWH "%s" RS, fl.flightNo, fl.origin, fl.dest);
        boxRowC(BYL, buf);
    }
    {
        char buf[200];
        snprintf(buf, 200, "  " DIM "Date/Time    : " RS BWH "%s  %s" RS "   Seat: " BYL "%s" RS "   Class: " BCY "%s" RS, fl.date, fl.depTime, b.seatNo, b.cabin);
        boxRowC(BYL, buf);
    }
    boxMid(BYL);
    /* Itemized */
    auto line = [&](const char *desc, double amt, const char *col = DIM)
    {
        char ld[50],rd[20];snprintf(ld,50,"  %-36s",desc);snprintf(rd,20,"$%10.2f",amt);
        char full[100];snprintf(full,100,"%s%s" RS "%s%s" RS,col,ld,DIM,rd);boxRowC(BYL,full); };
    line(("Base Fare (" + string(b.cabin) + ")").c_str(), b.baseFare);
    if (b.seatFee > 0)
        line("Seat Selection Fee", b.seatFee);
    if (b.baggageFee > 0)
        line(("Checked Baggage (" + to_string(b.bagsCount) + " bag(s))").c_str(), b.baggageFee);
    if (b.mealFee > 0)
        line("Meal Preference Fee", b.mealFee);
    line("Service & Handling Fee", b.serviceFee);
    line("Government Taxes (16.72%)", b.taxes);
    if (b.discount > 0)
    {
        char ld[50], rd[20];
        snprintf(ld, 50, "  %-36s", "Nexus Miles Discount");
        snprintf(rd, 20, "-$%9.2f", b.discount);
        char full[100];
        snprintf(full, 100, BGR "  %-36s" RS DIM "%s" RS, ld, rd);
        boxRowC(BYL, full);
    }
    boxSep(BYL);
    {
        char ld[50], rd[20];
        snprintf(ld, 50, "  %-36s", "GRAND TOTAL");
        snprintf(rd, 20, "$%10.2f", b.grandTotal);
        char full[100];
        snprintf(full, 100, BOLD "%-36s" RS BGR BOLD "%s" RS, ld, rd);
        boxRowC(BYL, full);
    }
    boxSep(BYL);
    {
        char buf[100];
        snprintf(buf, 100, "  Payment: " BWH "%s" RS "   Card: " BWH "****%s" RS, b.payMethod, b.cardLast4);
        boxRowC(BYL, buf);
    }
    boxBot(BYL);
}

/* ─── Payment wizard ──────────────────────────────────────── */
bool payWizard(double amount, string &outMethod, char outCard[8])
{
    printBanner();
    sectionHeader("NEXUS AIR — SECURE PAYMENT GATEWAY", BCY);
    printf("\n");
    boxTop(BCY);
    boxCen(BCY, BCY BOLD "  🔒  SECURE PAYMENT PORTAL  " RS);
    boxSep(BCY);
    {
        char b[100];
        snprintf(b, 100, "  " DIM "Amount Due: " RS BGR BOLD "  $%.2f  " RS, amount);
        boxRowC(BCY, b);
    }
    boxSep(BCY);
    boxRowC(BCY, "  " BCY "[1]" RS "  💳  Credit / Debit Card   " DIM "(Visa · MasterCard · Amex)" RS);
    boxRowC(BCY, "  " BCY "[2]" RS "  🌐  PayPal");
    boxRowC(BCY, "  " BCY "[3]" RS "  🏦  Bank Transfer  " DIM "(IBAN/SWIFT)" RS);
    boxRowC(BCY, "  " BCY "[4]" RS "  ⭐  Nexus Miles  " DIM "(Points Redemption)" RS);
    boxRowC(BCY, "  " BRD "[0]" RS "  Cancel");
    boxBot(BCY);

    int ch = iInt("Select Payment:", 0, 4);
    if (ch == 0)
        return false;
    sndClick();

    if (ch == 1)
    { /* Card */
        printBanner();
        sectionHeader("CARD PAYMENT — STEP 1 of 4", BCY);
        printf("\n");
        boxTop(BCY);
        boxCen(BCY, BCY "  💳  ENTER CARD DETAILS  " RS);
        boxSep(BCY);
        boxRowC(BCY, "  " DIM "All data is encrypted with 256-bit SSL" RS);
        boxSep(BCY);
        printf("  " BYL "[1]" RS " Visa   " BYL "[2]" RS " MasterCard   " BYL "[3]" RS " Amex   " BYL "[4]" RS " Discover\n");
        int ct = iInt("Card Type:", 1, 4);
        const char *ct_names[] = {"Visa", "MasterCard", "American Express", "Discover"};
        string ctype = ct_names[ct - 1];
        boxBot(BCY);

        printBanner();
        sectionHeader("CARD PAYMENT — STEP 2 of 4", BCY);
        printf("\n");
        boxTop(BCY);
        boxCen(BCY, BCY "  💳  CARD INFORMATION  " RS);
        boxSep(BCY);
        string cardNo = iCard16();
        string holder = iField("Cardholder Name:", "As printed on card");
        string expiry = iExpiry();
        string cvv = iCVV();
        boxBot(BCY);

        printBanner();
        sectionHeader("CARD PAYMENT — STEP 3 of 4", BCY);
        printf("\n");
        boxTop(BCY);
        boxCen(BCY, BCY "  🏠  BILLING ADDRESS  " RS);
        boxSep(BCY);
        string addr = iField("Billing Address:");
        string city2 = iField("City:");
        string country2 = iField("Country:");
        string zip = iField("Postal Code:");
        boxBot(BCY);

        printBanner();
        sectionHeader("CARD PAYMENT — STEP 4 of 4", BCY);
        progressBar("Securely processing payment", 1800);

        /* Animated processing steps */
        printf("\n");
        boxTop(BGR);
        boxCen(BGR, BGR BOLD "  PAYMENT AUTHORISATION  " RS);
        boxMid(BGR);
        const char *steps[] = {"Contacting issuing bank...", "Verifying card details...", "Checking available funds...", "Authorising transaction...", "Encrypting & finalising..."};
        for (int i = 0; i < 5; i++)
        {
            char b[100];
            snprintf(b, 100, "  " DIM "[%d/5]  " RS BCY "%s" RS, i + 1, steps[i]);
            boxRowC(BGR, b);
            fflush(stdout);
            sleepMs(500);
        }
        boxBot(BGR);
        sndPay();
        outMethod = ctype;
        strncpy(outCard, cardNo.substr(12, 4).c_str(), 7);
        return true;
    }

    if (ch == 2)
    { /* PayPal */
        printBanner();
        sectionHeader("PAYPAL PAYMENT", BCY);
        printf("\n");
        boxTop(BCY);
        boxCen(BCY, BCY "  🌐  PAYPAL  " RS);
        boxSep(BCY);
        string ppemail = iEmail("PayPal Email:");
        string pppw = iPass("PayPal Password:");
        boxBot(BCY);
        spinner("Connecting to PayPal...", 1200);
        sndPay();
        outMethod = "PayPal";
        strcpy(outCard, "PPAY");
        return true;
    }

    if (ch == 3)
    { /* Bank */
        printBanner();
        sectionHeader("BANK TRANSFER", BCY);
        printf("\n");
        boxTop(BCY);
        boxCen(BCY, BCY "  🏦  NEXUS AIR BANK DETAILS  " RS);
        boxSep(BCY);
        boxRowC(BCY, "  " DIM "Bank   : " RS BWH "Nexus Air Financial Services" RS);
        boxRowC(BCY, "  " DIM "IBAN   : " RS BWH "NX00 0000 0000 1234 5678 9012" RS);
        boxRowC(BCY, "  " DIM "SWIFT  : " RS BWH "NXAIUSXX" RS);
        {
            char b[100];
            snprintf(b, 100, "  " DIM "Amount : " RS BGR "$%.2f" RS, amount);
            boxRowC(BCY, b);
        }
        boxBot(BCY);
        string ref = iField("Your Transfer Reference:");
        spinner("Verifying bank transfer...", 1400);
        sndPay();
        outMethod = "Bank Transfer";
        strcpy(outCard, "BANK");
        return true;
    }

    /* Miles */
    if (sess.idx >= 0 && sess.role == RPASS)
    {
        Passenger &p = passengers[sess.idx];
        int needed = (int)(amount * 10);
        printf("\n  " CY "Your Nexus Miles: " RS BWH "%d" RS "   Required: " BYL "%d" RS "\n", p.loyaltyPts, needed);
        if (p.loyaltyPts >= needed)
        {
            p.loyaltyPts -= needed;
            savePass();
            sndPay();
            outMethod = "Nexus Miles";
            strcpy(outCard, "MILE");
            return true;
        }
        errMsg("Insufficient Nexus Miles for this booking.");
    }
    else
        errMsg("Miles redemption requires passenger login.");
    return false;
}

/* ═══════════════════════════════════════════════════════════
   FLIGHT CARD
   ═══════════════════════════════════════════════════════════ */
void showFlightCard(const Flight &fl, int idx)
{
    const char *sc = stCol(fl.status);
    int tot = fl.seatsFirst + fl.seatsBiz + fl.seatsPrem + fl.seatsEco;
    int bkd = fl.bkFirst + fl.bkBiz + fl.bkPrem + fl.bkEco;
    int pct = tot > 0 ? bkd * 100 / tot : 0;
    printf("\n");
    printf("  " BCY "[%d]" RS "  " BOLD BWH "✈ %s" RS "  │  " CY "%s" RS "\n", idx + 1, fl.flightNo, fl.airline);
    printf("      " YL "%s" RS "  →  " BYL "%s" RS "\n", fl.origin, fl.dest);
    printf("      " DIM "Date: " RS BWH "%s  Dep:%s  Arr:%s  Duration:%dh%02dm" RS "\n", fl.date, fl.depTime, fl.arrTime, fl.duration / 60, fl.duration % 60);
    printf("      " DIM "Status: " RS "%s" BOLD "%s" RS "   Gate: " BWH "%d" RS "   Aircraft: " BWH "%s" RS "\n", sc, fl.status, fl.gate, fl.aircraft);
    printf("      " BMG "First:$%.0f" RS "  " BYL "Biz:$%.0f" RS "  " BCY "PrmEco:$%.0f" RS "  " BGR "Eco:$%.0f" RS "   Load: %d%%\n",
           fl.fareFirst, fl.fareBiz, fl.farePrem, fl.fareEco, pct);
    printf("      " DIM "──────────────────────────────────────────────────────────────" RS "\n");
}

/* ═══════════════════════════════════════════════════════════
   SEARCH FLIGHTS (public)
   ═══════════════════════════════════════════════════════════ */
void searchFlights()
{
    printBanner();
    sectionHeader("FLIGHT SEARCH", BCY);
    string org = iField("From (city/code, Enter=all):");
    string dst = iField("To   (city/code, Enter=all):");
    spinner("Searching flights...", 600);
    int found = 0;
    for (int i = 0; i < flightCount; i++)
    {
        Flight &fl = flights[i];
        if (!fl.active)
            continue;
        bool mo = org.empty() || string(fl.origin).find(org) != string::npos || string(fl.flightNo).find(org) != string::npos;
        bool md = dst.empty() || string(fl.dest).find(dst) != string::npos;
        if (mo && md)
        {
            showFlightCard(fl, found);
            found++;
        }
    }
    if (!found)
        infoMsg("No flights match your search.");
    else
    {
        char b[80];
        snprintf(b, 80, "Found %d flight(s).", found);
        infoMsg(b);
    }
    waitEnter();
}

/* ─── Flight Status ───────────────────────────────────────── */
void flightStatus()
{
    printBanner();
    sectionHeader("FLIGHT STATUS LOOKUP", BCY);
    string fno = iField("Enter Flight Number:");
    spinner("Fetching real-time data...", 700);
    int idx = findFlight(fno.c_str());
    if (idx < 0)
    {
        errMsg("Flight not found.");
        waitEnter();
        return;
    }
    Flight &fl = flights[idx];
    printBanner();
    {
        char b[60];
        snprintf(b, 60, "FLIGHT STATUS — %s", fl.flightNo);
        sectionHeader(b, BCY);
    }
    const char *sc = stCol(fl.status);
    printf("\n");
    boxTop(BCY);
    {
        char b[200];
        snprintf(b, 200, "  " BOLD BWH "✈ %s" RS "  │  " CY "%s" RS, fl.flightNo, fl.airline);
        boxRowC(BCY, b);
    }
    boxSep(BCY);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Aircraft  : " RS BWH "%s" RS, fl.aircraft);
        boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "From      : " RS BWH "%s  (T%d)" RS, fl.origin, fl.termDep);
        boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "To        : " RS BWH "%s  (T%d)" RS, fl.dest, fl.termArr);
        boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Date      : " RS BWH "%s" RS, fl.date);
        boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Departure : " RS BWH "%s" RS "   Arrival: " BWH "%s" RS, fl.depTime, fl.arrTime);
        boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Gate      : " RS BWH "%d" RS "   Status: " RS "%s" BOLD "%s" RS, fl.gate, sc, fl.status);
        boxRowC(BCY, b);
    }
    if (strcmp(fl.status, "DELAYED") == 0 && strcmp(fl.delayReason, "N/A"))
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Reason    : " RS BYL "%s" RS, fl.delayReason);
        boxRowC(BCY, b);
    }
    boxSep(BCY);
    auto showCls = [&](const char *cls, int tot, int bk, const char *col)
    {
        int av=tot-bk;double pct2=tot>0?100.0*bk/tot:0;
        char b[300];snprintf(b,300,"  %s%-20s" RS " Total:" BWH "%3d" RS " Booked:" BWH "%3d" RS " " BGR "Avail:%3d" RS " [%.0f%%]",col,cls,tot,bk,av,pct2);
        boxRowC(BCY,b); };
    showCls("FIRST CLASS", fl.seatsFirst, fl.bkFirst, BMG);
    showCls("BUSINESS CLASS", fl.seatsBiz, fl.bkBiz, BYL);
    showCls("PREMIUM ECONOMY", fl.seatsPrem, fl.bkPrem, BCY);
    showCls("ECONOMY", fl.seatsEco, fl.bkEco, BGR);
    boxBot(BCY);
    waitEnter();
}

/* ═══════════════════════════════════════════════════════════
   PASSENGER REGISTER / LOGIN
   ═══════════════════════════════════════════════════════════ */
void registerPass()
{
    printBanner();
    sectionHeader("NEW PASSENGER REGISTRATION", BGR);
    if (passCount >= MAX_PA)
    {
        errMsg("Passenger capacity reached.");
        waitEnter();
        return;
    }
    printf("\n");
    boxTop(BGR);
    boxCen(BGR, BGR "  📋  STEP 1 — PERSONAL DETAILS  " RS);
    boxSep(BGR);
    string fn = iField("First Name:", "Legal name");
    string ln = iField("Last Name:", "Legal name");
    string dob = iDate("Date of Birth:");
    string nat = iField("Nationality:");
    string pp = iPassport("Passport Number:");
    boxSep(BGR);
    boxCen(BGR, BGR "  📞  STEP 2 — CONTACT DETAILS  " RS);
    boxSep(BGR);
    string em = iEmail("Email Address:");
    string ph = iPhone("Phone Number:");
    string addr = iField("Home Address:");
    string city2 = iField("City:");
    string country2 = iField("Country:");
    boxSep(BGR);
    boxCen(BGR, BGR "  🔐  STEP 3 — ACCOUNT SETUP  " RS);
    boxSep(BGR);
    string un = iField("Username:", "Unique, no spaces");
    if (findPass(un.c_str()) >= 0)
    {
        errMsg("Username already taken!");
        boxBot(BGR);
        waitEnter();
        return;
    }
    string pw = iPass("Password:", true);
    string pw2 = iPass("Confirm Password:");
    if (pw != pw2)
    {
        errMsg("Passwords do not match!");
        boxBot(BGR);
        waitEnter();
        return;
    }
    boxBot(BGR);
    spinner("Creating your Nexus Air account...", 1000);

    Passenger &p = passengers[passCount++];
    p.id = nextPa++;
    strncpy(p.username, un.c_str(), SZ - 1);
    string h = hashPW(pw);
    strncpy(p.passHash, h.c_str(), 23);
    strncpy(p.firstName, fn.c_str(), SZ - 1);
    strncpy(p.lastName, ln.c_str(), SZ - 1);
    strncpy(p.email, em.c_str(), SZ - 1);
    strncpy(p.phone, ph.c_str(), SZ - 1);
    strncpy(p.passport, pp.c_str(), SZ - 1);
    strncpy(p.nationality, nat.c_str(), SZ - 1);
    strncpy(p.dob, dob.c_str(), 15);
    strncpy(p.address, addr.c_str(), 79);
    strncpy(p.city, city2.c_str(), 39);
    strncpy(p.country, country2.c_str(), 39);
    p.loyaltyPts = 500;
    p.totalMiles = 0;
    strcpy(p.loyaltyTier, "BRONZE");
    string ts = getTS();
    strncpy(p.created, ts.c_str(), 19);
    p.active = 1;
    savePass();

    printBanner();
    sectionHeader("REGISTRATION SUCCESSFUL", BGR);
    boxTop(BGR);
    {
        char b[200];
        snprintf(b, 200, "  🎉  " BOLD BGR "Welcome aboard, %s %s!" RS, fn.c_str(), ln.c_str());
        boxRowC(BGR, b);
    }
    boxSep(BGR);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Passenger ID    : " RS BGR "P%d" RS, p.id);
        boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Nexus Miles     : " RS BWH "0" RS);
        boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Welcome Bonus   : " RS BYL "500 pts credited!" RS);
        boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Loyalty Tier    : " RS YL "BRONZE MEMBER" RS);
        boxRowC(BGR, b);
    }
    boxBot(BGR);
    waitEnter();
}

bool loginPass()
{
    printBanner();
    sectionHeader("PASSENGER LOGIN", BGR);
    printf("\n");
    boxTop(BGR);
    boxCen(BGR, BGR "  🔑  PASSENGER PORTAL  " RS);
    boxSep(BGR);
    string un = iField("Username:");
    string pw = iPass("Password:");
    boxBot(BGR);
    spinner("Authenticating...", 700);
    int idx = findPass(un.c_str());
    if (idx < 0 || hashPW(pw) != string(passengers[idx].passHash))
    {
        glitch("  ██ ACCESS DENIED ██");
        errMsg("Invalid username or password.");
        waitEnter();
        return false;
    }
    sess.role = RPASS;
    sess.idx = idx;
    sess.loggedIn = 1;
    strncpy(sess.username, un.c_str(), SZ - 1);
    Passenger &p = passengers[idx];
    string t = tier(p.totalMiles);
    strncpy(p.loyaltyTier, t.c_str(), 11);
    printBanner();
    sectionHeader("WELCOME BACK", BGR);
    printf("  " BOLD BGR "✔  Welcome back, %s %s!" RS "\n\n", p.firstName, p.lastName);
    printf("  " CY "Loyalty Status : " RS "%s" BOLD "%s MEMBER" RS "\n", tierCol(p.loyaltyTier), p.loyaltyTier);
    printf("  " CY "Nexus Miles    : " RS BWH "%d" RS "\n", p.totalMiles);
    printf("  " CY "Points Balance : " RS BWH "%d pts" RS "\n", p.loyaltyPts);
    int uc = 0;
    for (int i = 0; i < bookingCount; i++)
        if (bookings[i].passengerId == p.id && !strcmp(bookings[i].status, "CONFIRMED"))
            uc++;
    printf("  " CY "Upcoming Trips : " RS BWH "%d" RS "\n", uc);
    sndLogin();
    waitEnter();
    return true;
}

/* ─── Book Flight ─────────────────────────────────────────── */
void bookFlight()
{
    if (sess.idx < 0)
        return;
    Passenger &p = passengers[sess.idx];
    printBanner();
    sectionHeader("BOOK A FLIGHT", BMG);
    /* Show flights */
    int shown = 0;
    for (int i = 0; i < flightCount; i++)
        if (flights[i].active)
        {
            showFlightCard(flights[i], shown);
            shown++;
        }
    if (!shown)
    {
        infoMsg("No active flights available.");
        waitEnter();
        return;
    }
    int ch = iInt("Select flight [1-N] or 0 to cancel:", 0, shown);
    if (ch < 1)
    {
        infoMsg("Booking cancelled.");
        waitEnter();
        return;
    }
    int fIdx = -1, cnt = 0;
    for (int i = 0; i < flightCount; i++)
        if (flights[i].active)
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
        errMsg("Invalid.");
        waitEnter();
        return;
    }
    Flight &fl = flights[fIdx];

    /* STEP 1 — Seat Selection */
    SeatMap *sm = getSM(fl.flightNo);
    if (!sm)
    {
        errMsg("No seat map for this flight.");
        waitEnter();
        return;
    }
    int outRow = -1, outCol = -1;
    double seatBaseFare = 0;
    char seatNo[8] = "";
    if (!selectSeat(sm, fl, outRow, outCol, seatBaseFare, seatNo))
    {
        infoMsg("Booking cancelled.");
        waitEnter();
        return;
    }
    const char *cabin = rowCabin(outRow);
    double seatExtraFee = seatFeeForRow(outRow, fl);

    /* STEP 2 — Baggage */
    printBanner();
    sectionHeader("BAGGAGE SELECTION — STEP 2", BMG);
    printf("\n");
    boxTop(BMG);
    boxCen(BMG, BMG "  🧳  CHECKED BAGGAGE OPTIONS  " RS);
    boxSep(BMG);
    boxRowC(BMG, "  " BCY "[0]" RS "  Hand luggage only  (free)  " DIM "7kg cabin bag included" RS);
    boxRowC(BMG, "  " BCY "[1]" RS "  1 bag  " BGR "+$30" RS "        " DIM "up to 23kg" RS);
    boxRowC(BMG, "  " BCY "[2]" RS "  2 bags " BGR "+$55" RS "        " DIM "up to 23kg each" RS);
    boxRowC(BMG, "  " BCY "[3]" RS "  3 bags " BGR "+$75" RS "        " DIM "up to 23kg each" RS);
    boxBot(BMG);
    int bags2 = iInt("Bags (0-3):", 0, 3);
    double baggageFees[] = {0, 30, 55, 75};
    double bagFee = baggageFees[bags2];

    /* Fragile/special */
    int fragile = 0, special = 0;
    if (bags2 > 0)
    {
        printf("  " BYL " ▶ " RS BWH "Any fragile items? [Y/N]: " RS " " BCY);
        fflush(stdout);
        char c;
        scanf(" %c", &c);
        clearInput();
        printf(RS);
        fragile = (tolower(c) == 'y');
        sndTick();
        printf("  " BYL " ▶ " RS BWH "Oversized/sports equipment? [Y/N]: " RS " " BCY);
        fflush(stdout);
        scanf(" %c", &c);
        clearInput();
        printf(RS);
        special = (tolower(c) == 'y');
        sndTick();
        if (special)
            bagFee += 30;
    }

    /* STEP 3 — Meal */
    printBanner();
    sectionHeader("MEAL SELECTION — STEP 3", BMG);
    const char *meals[] = {"No Preference (Standard)", "Vegetarian (VGML)", "Vegan (VOML)",
                           "Halal (MOML)", "Kosher (KSML)", "Gluten-Free (GFML)", "Diabetic (DBML)", "Child Meal (CHML)"};
    double mealFees[] = {0, 0, 0, 5, 8, 10, 10, 0};
    printf("\n");
    boxTop(BMG);
    boxCen(BMG, BMG "  🍽  MEAL PREFERENCE  " RS);
    boxSep(BMG);
    for (int i = 0; i < 8; i++)
    {
        char b[100];
        snprintf(b, 100, "  " BCY "[%d] " RS BWH "%s" RS "  " DIM "%s" RS, i + 1, meals[i], mealFees[i] > 0 ? "(+fee)" : "(free)");
        boxRowC(BMG, b);
    }
    boxBot(BMG);
    int mc = iInt("Select meal [1-8]:", 1, 8);
    mc--;
    string meal = meals[mc];
    double mealFee = mealFees[mc];

    /* STEP 4 — Special requests */
    printBanner();
    sectionHeader("SPECIAL REQUESTS — STEP 4", BMG);
    string specialReq = iField("Special Request (wheelchair/medical/other)", "or press Enter to skip");

    /* Fare Calculation */
    double baseFare = seatBaseFare;
    double seatFee = seatExtraFee;
    double serviceFee = 15.0;
    double subtotal = baseFare + seatFee + bagFee + mealFee + serviceFee;
    double taxes = subtotal * (fl.taxRate / 100.0);
    /* Loyalty discount */
    double discount = 0;
    if (!strcmp(p.loyaltyTier, "GOLD"))
        discount = subtotal * 0.05;
    if (!strcmp(p.loyaltyTier, "PLATINUM"))
        discount = subtotal * 0.10;
    double grandTotal = subtotal + taxes - discount;

    /* STEP 5 — Review */
    printBanner();
    sectionHeader("BOOKING REVIEW — STEP 5", BMG);
    printf("\n");
    boxTop(BMG);
    boxCen(BMG, BMG BOLD "  📋  BOOKING SUMMARY  " RS);
    boxSep(BMG);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Flight   : " RS BWH "%s  %s → %s" RS, fl.flightNo, fl.origin, fl.dest);
        boxRowC(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Date     : " RS BWH "%s  Dep:%s" RS, fl.date, fl.depTime);
        boxRowC(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Seat     : " RS BYL "%s" RS "   Class: " BCY "%s" RS, seatNo, cabin);
        boxRowC(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Meal     : " RS BWH "%s" RS, meal.c_str());
        boxRowC(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Bags     : " RS BWH "%d checked bag(s)" RS, bags2);
        boxRowC(BMG, b);
    }
    boxSep(BMG);
    {
        char b[100];
        snprintf(b, 100, "  %-34s $%.2f", "Base Fare:", baseFare);
        boxRow(BMG, b);
    }
    {
        char b[100];
        snprintf(b, 100, "  %-34s $%.2f", "Seat Fee:", seatFee);
        boxRow(BMG, b);
    }
    if (bagFee > 0)
    {
        char b[100];
        snprintf(b, 100, "  %-34s $%.2f", "Baggage:", bagFee);
        boxRow(BMG, b);
    }
    if (mealFee > 0)
    {
        char b[100];
        snprintf(b, 100, "  %-34s $%.2f", "Meal:", mealFee);
        boxRow(BMG, b);
    }
    {
        char b[100];
        snprintf(b, 100, "  %-34s $%.2f", "Service Fee:", serviceFee);
        boxRow(BMG, b);
    }
    {
        char b[100];
        snprintf(b, 100, "  %-34s $%.2f", "Taxes (16.72%):", taxes);
        boxRow(BMG, b);
    }
    if (discount > 0)
    {
        char b[100];
        snprintf(b, 100, "  %-34s -$%.2f", "Loyalty Discount:", discount);
        boxRow(BMG, b);
    }
    boxSep(BMG);
    {
        char b[100];
        snprintf(b, 100, "  " BOLD "%-34s " BGR "$%.2f" RS, "GRAND TOTAL:", grandTotal);
        boxRowC(BMG, b);
    }
    boxBot(BMG);

    string conf = iField("Type CONFIRM to proceed to payment:");
    if (conf != "CONFIRM")
    {
        infoMsg("Booking cancelled.");
        waitEnter();
        return;
    }

    /* STEP 6 — Payment */
    string payMethod;
    char cardLast4[8] = "";
    if (!payWizard(grandTotal, payMethod, cardLast4))
    {
        infoMsg("Payment not completed. Booking cancelled.");
        waitEnter();
        return;
    }

    /* Finalise booking */
    progressBar("Finalising your booking", 1000);
    spinner("Issuing E-Ticket...", 600);

    /* Mark seat */
    sm->st[outRow][outCol] = 1;
    char pname[80];
    snprintf(pname, 80, "%s %s", p.firstName, p.lastName);
    strncpy(sm->who[outRow][outCol], pname, SZ - 1);

    /* Update cabin counts */
    if (!strcmp(cabin, "FIRST"))
        fl.bkFirst++;
    else if (!strcmp(cabin, "BUSINESS"))
        fl.bkBiz++;
    else if (!strcmp(cabin, "PREMIUM_ECO"))
        fl.bkPrem++;
    else
        fl.bkEco++;

    /* Miles */
    int milesEarned = fl.duration * 8;
    p.totalMiles += milesEarned;
    p.loyaltyPts += milesEarned / 10;
    string nt2 = tier(p.totalMiles);
    strncpy(p.loyaltyTier, nt2.c_str(), 11);

    /* Create booking */
    Booking &bk = bookings[bookingCount++];
    bk.id = nextBk++;
    bk.passengerId = p.id;
    bk.flightId = fl.id;
    strncpy(bk.flightNo, fl.flightNo, 15);
    strncpy(bk.passengerName, pname, 79);
    strncpy(bk.cabin, cabin, 23);
    strncpy(bk.seatNo, seatNo, 7);
    strncpy(bk.meal, meal.c_str(), 39);
    bk.baseFare = baseFare;
    bk.seatFee = seatFee;
    bk.baggageFee = bagFee;
    bk.mealFee = mealFee;
    bk.serviceFee = serviceFee;
    bk.taxes = taxes;
    bk.discount = discount;
    bk.grandTotal = grandTotal;
    string bref = genRef();
    strncpy(bk.bookingRef, bref.c_str(), 19);
    strncpy(bk.payMethod, payMethod.c_str(), 23);
    strncpy(bk.cardLast4, cardLast4, 7);
    strcpy(bk.status, "CONFIRMED");
    bk.checkedIn = 0;
    bk.bagsCount = bags2;
    snprintf(bk.boardingGate, 8, "%d", fl.gate);
    strcpy(bk.boardingGroup, "2");
    string ts = getTS();
    strncpy(bk.created, ts.c_str(), 19);
    strncpy(bk.specialReq, specialReq.c_str(), 59);

    /* Create bag records */
    for (int nb = 0; nb < bags2 && bagCount < MAX_BG; nb++)
    {
        BagItem &bg = bags[bagCount++];
        bg.id = nextBg++;
        bg.bookingId = bk.id;
        strncpy(bg.bookingRef, bref.c_str(), 19);
        strncpy(bg.passengerName, pname, 79);
        strncpy(bg.flightNo, fl.flightNo, 15);
        string tag = genTag(fl.flightNo);
        strncpy(bg.tagNo, tag.c_str(), 19);
        bg.weightKg = 0;
        bg.stageIdx = 0;
        bg.fragile = fragile;
        bg.special = special;
        strcpy(bg.location, "Awaiting Check-in");
        strcpy(bg.carousel, "TBA");
        strncpy(bg.created, ts.c_str(), 19);
    }

    saveAll();

    /* E-Ticket display */
    printBanner();
    sectionHeader("BOOKING CONFIRMED!", BGR);
    printf("\n");
    boxTop(BGR);
    boxCen(BGR, BOLD BGR "✔  E-TICKET ISSUED SUCCESSFULLY  ✔" RS);
    boxSep(BGR);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Booking Ref : " RS BOLD BWH "%s" RS, bk.bookingRef);
        boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Ticket No   : " RS BWH "TKT-%d" RS, bk.id);
        boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Passenger   : " RS BWH "%s" RS, bk.passengerName);
        boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Frequent Flyer: " RS "%s" BOLD "%s" RS, tierCol(p.loyaltyTier), p.loyaltyTier);
        boxRowC(BGR, b);
    }
    boxSep(BGR);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Flight      : " RS BWH "%s  %s → %s" RS, fl.flightNo, fl.origin, fl.dest);
        boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Date/Time   : " RS BWH "%s  %s" RS, fl.date, fl.depTime);
        boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Seat        : " RS BYL "%s  %s" RS, seatNo, cabin);
        boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Gate/Grp    : " RS BWH "%s  Group %s" RS, bk.boardingGate, bk.boardingGroup);
        boxRowC(BGR, b);
    }
    boxSep(BGR);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Grand Total : " RS BGR BOLD "$%.2f" RS "  via %s", grandTotal, payMethod.c_str());
        boxRowC(BGR, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Miles Earned: " RS BYL "+%d Nexus Miles" RS, milesEarned);
        boxRowC(BGR, b);
    }
    boxSep(BGR);
    boxRowC(BGR, "  " YL "⚠  Check-in opens 24 hrs before departure" RS);
    boxRowC(BGR, "  " YL "⚠  Arrive at airport 3 hours before departure" RS);
    boxBot(BGR);

    /* Full bill */
    printBill(bk, fl);
    waitEnter();
}

/* ─── View My Bookings ────────────────────────────────────── */
void viewBookings()
{
    if (sess.idx < 0)
        return;
    Passenger &p = passengers[sess.idx];
    printBanner();
    {
        char b[80];
        snprintf(b, 80, "MY BOOKINGS — %s %s", p.firstName, p.lastName);
        sectionHeader(b, BCY);
    }
    int found = 0;
    for (int i = 0; i < bookingCount; i++)
    {
        Booking &bk = bookings[i];
        if (bk.passengerId != p.id)
            continue;
        const char *sc = (!strcmp(bk.status, "CONFIRMED")) ? BGR : (!strcmp(bk.status, "CANCELLED")) ? BRD
                                                                                                     : DIM;
        printf("\n  " BCY "[%d] " RS BOLD BWH "✈ %s" RS "\n", ++found, bk.flightNo);
        printf("      " DIM "Ref: " RS BWH "%s" RS "   Seat: " BWH "%s" RS "   Class: " BWH "%s" RS "\n", bk.bookingRef, bk.seatNo, bk.cabin);
        printf("      " DIM "Total: " RS BGR "$%.2f" RS "   Status: %s" BOLD "%s" RS "   CheckIn: %s\n",
               bk.grandTotal, sc, bk.status, bk.checkedIn ? (BGR "✔ Done" RS) : (BYL "Pending" RS));
        printf("      " DIM "───────────────────────────────────────────────────────────" RS "\n");
    }
    if (!found)
        infoMsg("No bookings found.");
    printf("\n  " BCY "[C]" RS " Cancel booking   " BCY "[B]" RS " View bill   " BCY "[ENTER]" RS " Back\n");
    string ch = iField("Choice (C/B/Enter):");
    if (ch == "C" || ch == "c")
    {
        string ref = iField("Booking Reference:");
        int bi = findBooking(ref.c_str());
        if (bi < 0 || bookings[bi].passengerId != p.id)
        {
            errMsg("Booking not found.");
        }
        else if (!strcmp(bookings[bi].status, "CANCELLED"))
        {
            infoMsg("Already cancelled.");
        }
        else
        {
            strcpy(bookings[bi].status, "CANCELLED");
            /* restore seat */
            SeatMap *sm2 = getSM(bookings[bi].flightNo);
            if (sm2)
            {
                for (int r = 0; r < TOTAL_ROWS; r++)
                    for (int c = 0; c < 6; c++)
                        if (!strcmp(sm2->who[r][c], bookings[bi].passengerName))
                        {
                            sm2->st[r][c] = 0;
                            strcpy(sm2->who[r][c], "");
                        }
            }
            saveAll();
            success(("Booking " + ref + " cancelled. Refund initiated.").c_str());
        }
    }
    else if (ch == "B" || ch == "b")
    {
        string ref = iField("Booking Reference for bill:");
        int bi = findBooking(ref.c_str());
        if (bi < 0 || bookings[bi].passengerId != p.id)
            errMsg("Booking not found.");
        else
        {
            int fi = findFlight(bookings[bi].flightNo);
            if (fi >= 0)
                printBill(bookings[bi], flights[fi]);
        }
    }
    waitEnter();
}

/* ─── Online Check-in ─────────────────────────────────────── */
void onlineCheckin()
{
    if (sess.idx < 0)
        return;
    Passenger &p = passengers[sess.idx];
    printBanner();
    sectionHeader("ONLINE CHECK-IN", BYL);
    int elig[MAX_BK], ec = 0;
    for (int i = 0; i < bookingCount; i++)
        if (bookings[i].passengerId == p.id && !strcmp(bookings[i].status, "CONFIRMED") && !bookings[i].checkedIn)
            elig[ec++] = i;
    if (!ec)
    {
        infoMsg("No flights available for check-in.");
        waitEnter();
        return;
    }
    for (int e = 0; e < ec; e++)
    {
        Booking &bk = bookings[elig[e]];
        printf("  " BCY "[%d] " RS BOLD "✈ %s" RS "   Ref: " BWH "%s" RS "   Seat: " BWH "%s" RS "\n\n", e + 1, bk.flightNo, bk.bookingRef, bk.seatNo);
    }
    int ch = iInt("Select flight to check in:", 1, ec);
    Booking &bk = bookings[elig[ch - 1]];

    /* Step 1 */
    sectionHeader("STEP 1 — IDENTITY VERIFICATION", BYL);
    printf("  Passenger : " BWH "%s %s" RS "\n", p.firstName, p.lastName);
    printf("  Passport  : " BWH "%s" RS "\n", p.passport);
    printf("  DOB       : " BWH "%s" RS "\n\n", p.dob);
    string conf = iField("Confirm details are correct [YES/NO]:");
    if (conf != "YES" && conf != "yes")
    {
        infoMsg("Check-in cancelled.");
        waitEnter();
        return;
    }

    /* Step 2 — Bag drop */
    sectionHeader("STEP 2 — BAGGAGE DROP", BYL);
    if (bk.bagsCount > 0)
    {
        printf("  " BWH "You have %d checked bag(s) on this booking.\n" RS, bk.bagsCount);
        printf("  " BCY "[1]" RS " Drop all bags now   " BCY "[2]" RS " Skip bag drop (airport only)\n");
        int bch = iInt("Choice:", 1, 2);
        if (bch == 1)
        {
            /* Update bag statuses */
            double totalW = 0;
            for (int i = 0; i < bagCount; i++)
            {
                if (!strcmp(bags[i].bookingRef, bk.bookingRef))
                {
                    bags[i].stageIdx = 1; /* CHECK-IN DONE */
                    strcpy(bags[i].location, "Check-in Counter");
                    printf("  " BYL " ▶ " RS BWH "Weight for bag %s (kg): " RS " " BCY, bags[i].tagNo);
                    fflush(stdout);
                    double w;
                    scanf("%lf", &w);
                    clearInput();
                    printf(RS);
                    if (w < 0.1 || w > 32)
                    {
                        warnMsg("Weight out of range. Using 20kg.");
                        w = 20;
                    }
                    bags[i].weightKg = w;
                    totalW += w;
                    sndScan();
                }
            }
            saveBags();
            if (totalW > 0)
            {
                char b[80];
                snprintf(b, 80, "%.1fkg total bag weight recorded.", totalW);
                infoMsg(b);
            }
        }
    }
    else
        infoMsg("No checked bags on this booking.");

    /* Step 3 — Travel docs */
    sectionHeader("STEP 3 — TRAVEL DOCUMENTS", BYL);
    printf("  " BWH "Destination Requirements:\n" RS);
    printf("  • Valid passport required  • Check visa requirements for destination\n\n");
    string emName = iField("Emergency Contact Name:");
    string emPhone = iPhone("Emergency Contact Phone:");
    string finalConf = iField("Type CONFIRM to complete check-in:");
    if (finalConf != "CONFIRM")
    {
        infoMsg("Check-in cancelled.");
        waitEnter();
        return;
    }

    spinner("Processing check-in...", 900);
    bk.checkedIn = 1;
    saveBookings();

    /* Boarding pass */
    int gate = 0;
    for (int i = 0; i < flightCount; i++)
        if (!strcmp(flights[i].flightNo, bk.flightNo))
        {
            gate = flights[i].gate;
            break;
        }
    printBanner();
    sectionHeader("BOARDING PASS", BGR);
    boxTop(BCY);
    boxCen(BCY, BOLD BGR "✔  CHECK-IN COMPLETE — BOARDING PASS READY  ✔" RS);
    boxSep(BCY);
    {
        char b[200];
        snprintf(b, 200, "  " BWH "NEXUS AIR" RS "              Flight: " BYL "%s" RS "   Gate: " BYL "%d" RS, bk.flightNo, gate);
        boxRowC(BCY, b);
    }
    boxSep(BCY);
    {
        char b[200];
        snprintf(b, 200, "  " CY "PASSENGER   : " RS BOLD BWH "%s" RS, bk.passengerName);
        boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " CY "SEAT        : " RS BOLD BYL "%s" RS "   CLASS: " BWH "%s" RS, bk.seatNo, bk.cabin);
        boxRowC(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " CY "BOARDING    : " RS BWH "Group %s" RS "   Seq: 0%d" RS, bk.boardingGroup, bk.id % 99);
        boxRowC(BCY, b);
    }
    boxSep(BCY);
    /* Simple QR box */
    printf(BCY BV RS "  ");
    printf(BG_WH "\033[30m"
                 "  +----------+  " RS);
    spc(TW - 18);
    printf(BCY BV RS "\n");
    printf(BCY BV RS "  ");
    printf(BG_WH "\033[30m"
                 "  | QR CODE |  " RS BCY "  [ SCAN AT GATE ]" RS);
    spc(TW - 33);
    printf(BCY BV RS "\n");
    printf(BCY BV RS "  ");
    printf(BG_WH "\033[30m"
                 "  +----------+  " RS);
    spc(TW - 18);
    printf(BCY BV RS "\n");
    boxSep(BCY);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "BP-%s-%s-%04d" RS, bk.flightNo, bk.seatNo, bk.id);
        boxRowC(BCY, b);
    }
    boxBot(BCY);
    warnMsg("Please arrive at gate 45 minutes before departure.");
    waitEnter();
}

/* ─── Loyalty Program ─────────────────────────────────────── */
void loyaltyProgram()
{
    if (sess.idx < 0)
        return;
    Passenger &p = passengers[sess.idx];
    string t = tier(p.totalMiles);
    strncpy(p.loyaltyTier, t.c_str(), 11);
    printBanner();
    sectionHeader("NEXUS MILES LOYALTY PROGRAM", BYL);
    printf("  " DIM "Member: " RS BWH "%s %s" RS "   ID: " BWH "NM%d" RS "\n\n", p.firstName, p.lastName, p.id * 7 + 654321);
    printf("  " DIM "Current Tier : " RS "%s" BOLD "%s" RS "\n", tierCol(p.loyaltyTier), p.loyaltyTier);
    int target = (strcmp(p.loyaltyTier, "SILVER") == 0) ? 20000 : (strcmp(p.loyaltyTier, "GOLD") == 0) ? 50000
                                                                                                       : 50000;
    int needed = (int)fmax(0, target - p.totalMiles);
    printf("  " DIM "Miles to Next: " RS BYL "%d" RS "\n", needed);
    int bLen = 40, filled = target > 0 ? (int)fmin(bLen, 1.0 * p.totalMiles / target * bLen) : bLen;
    printf("  [");
    for (int i = 0; i < bLen; i++)
        printf("%s", i < filled ? (BGR "█" RS) : (DIM "░" RS));
    printf("]\n\n");
    printf("  " BYL "Tier Benefits:" RS "\n");
    printf("  " YL "BRONZE" RS " → 0 miles   • 1x points\n");
    printf("  " BWH "SILVER" RS " → 5,000     • 1.25x points  • Priority check-in\n");
    printf("  " BYL "GOLD  " RS " → 20,000    • 1.5x points   • Lounge access  • Extra bag\n");
    printf("  " BCY "PLAT  " RS " → 50,000    • 2x points     • Suite upgrade  • Chauffeur\n\n");
    printf("  " BCY "[1] " RS "Flight Discount  (10,000 pts = $100)\n");
    printf("  " BCY "[2] " RS "Seat Upgrade     (8,000  pts)\n");
    printf("  " BCY "[3] " RS "Extra Baggage    (2,500  pts)\n");
    printf("  " BCY "[0] " RS "Back\n\n");
    int ch = iInt("Select:", 0, 3);
    if (ch == 1 && p.loyaltyPts >= 10000)
    {
        p.loyaltyPts -= 10000;
        savePass();
        success("10,000 pts redeemed! $100 discount on next booking.");
    }
    else if (ch != 0)
        warnMsg("Insufficient points or invalid option.");
    waitEnter();
}

/* ─── My Profile ──────────────────────────────────────────── */
void myProfile()
{
    if (sess.idx < 0)
        return;
    Passenger &p = passengers[sess.idx];
    printBanner();
    sectionHeader("MY PROFILE", BMG);
    printf("  " CY "Full Name   : " RS BWH "%s %s" RS "\n", p.firstName, p.lastName);
    printf("  " CY "Email       : " RS BWH "%s" RS "\n", p.email);
    printf("  " CY "Phone       : " RS BWH "%s" RS "\n", p.phone);
    printf("  " CY "Passport    : " RS BWH "%s" RS "\n", p.passport);
    printf("  " CY "Nationality : " RS BWH "%s" RS "\n", p.nationality);
    printf("  " CY "DOB         : " RS BWH "%s" RS "\n", p.dob);
    printf("  " CY "Address     : " RS BWH "%s, %s, %s" RS "\n", p.address, p.city, p.country);
    printf("  " CY "Username    : " RS BWH "%s" RS "\n\n", p.username);
    printf("  " BCY "[E]" RS " Edit  " BCY "[P]" RS " Change Password  " BCY "[B]" RS " Back\n");
    string ch = iField("Choice:");
    if (ch == "E" || ch == "e")
    {
        string fn = iField(("First Name [" + string(p.firstName) + "]:").c_str());
        string ln = iField(("Last Name  [" + string(p.lastName) + "]:").c_str());
        string em = iEmail("New Email:");
        string ph = iPhone("New Phone:");
        if (!fn.empty())
            strncpy(p.firstName, fn.c_str(), SZ - 1);
        if (!ln.empty())
            strncpy(p.lastName, ln.c_str(), SZ - 1);
        strncpy(p.email, em.c_str(), SZ - 1);
        strncpy(p.phone, ph.c_str(), SZ - 1);
        savePass();
        success("Profile updated!");
    }
    else if (ch == "P" || ch == "p")
    {
        string op = iPass("Current Password:");
        if (hashPW(op) != string(p.passHash))
        {
            errMsg("Wrong current password.");
        }
        else
        {
            string np = iPass("New Password:", true);
            string nc = iPass("Confirm New Password:");
            if (np != nc)
                errMsg("Passwords do not match.");
            else
            {
                string h = hashPW(np);
                strncpy(p.passHash, h.c_str(), 23);
                savePass();
                success("Password changed!");
            }
        }
    }
    waitEnter();
}

/* ─── Baggage Tracking ────────────────────────────────────── */
void baggageTracking()
{
    printBanner();
    sectionHeader("BAGGAGE TRACKING", BCY);
    string ref = iField("Enter Booking Reference:");
    spinner("Locating your baggage...", 700);
    int found = 0;
    for (int i = 0; i < bagCount; i++)
    {
        BagItem &bg = bags[i];
        if (strcmp(bg.bookingRef, ref.c_str()))
            continue;
        showBagStatus(bg);
        found++;
    }
    if (!found)
        infoMsg("No baggage found for this booking reference.");
    waitEnter();
}

/* ─── Entertainment ────────────────────────────────────────── */
void aviationQuiz()
{
    printBanner();
    sectionHeader("NEXUS AIR AVIATION QUIZ", BYL);
    struct Q
    {
        const char *q, *a[4];
        int ans;
    } qs[] = {
        {"What does 'KHI' stand for?", {"Karachi Airport", "Khi International", "Kolkata Hub Index", "Kuala Hub Int"}, 0},
        {"What is Mach 1?", {"Speed of Light", "Speed of Sound", "Max aircraft speed", "Runway speed"}, 1},
        {"What does 'ETA' mean?", {"Extra Travel Allowance", "Estimated Time of Arrival", "Engine Thrust Angle", "Exit Terminal Access"}, 1},
        {"Which is the world's longest flight route?", {"Dubai-New York", "Sydney-Dallas", "Singapore-New York (SQ21)", "London-Sydney"}, 2},
        {"What colour is the flight data recorder?", {"Black", "Red", "Orange", "Yellow"}, 2},
        {"What does 'IATA' stand for?", {"International Air Transport Association", "Intl Airline Traffic Assoc", "Intl Airport Trade Auth", "Intl ATC Association"}, 0},
        {"The Airbus A380 has how many decks?", {"1", "2", "3", "4"}, 1},
        {"What is a 'red-eye' flight?", {"Emergency flight", "Overnight flight", "First class only", "Cancelled flight"}, 1},
    };
    int total = 8, score = 0;
    for (int i = 0; i < total; i++)
    {
        printBanner();
        printf("\n");
        boxTop(BYL);
        {
            char b[200];
            snprintf(b, 200, "  " BYL "QUESTION %d/%d" RS "   Score: " BGR "%d" RS, i + 1, total, score);
            boxRowC(BYL, b);
        }
        boxSep(BYL);
        {
            char b[300];
            snprintf(b, 300, "  " BWH "%s" RS, qs[i].q);
            boxRowC(BYL, b);
        }
        boxSep(BYL);
        for (int j = 0; j < 4; j++)
        {
            char b[200];
            snprintf(b, 200, "  " BCY "[%d] " RS BWH "%s" RS, j + 1, qs[i].a[j]);
            boxRowC(BYL, b);
        }
        boxBot(BYL);
        int ans = iInt("Answer [1-4]:", 1, 4) - 1;
        if (ans == qs[i].ans)
        {
            score++;
            printf(BGR "\n  ✔  CORRECT! +1\n" RS);
            sndOK();
        }
        else
        {
            printf(BRD "\n  ✘  Wrong! Correct: " BGR "%s\n" RS, qs[i].a[qs[i].ans]);
            sndErr();
        }
        sleepMs(900);
    }
    printBanner();
    sectionHeader("QUIZ RESULTS", BYL);
    boxTop(BYL);
    {
        char b[200];
        snprintf(b, 200, "  🏅  " BYL "Your Score: " BGR "%d / %d" RS, score, total);
        boxRowC(BYL, b);
    }
    boxSep(BYL);
    const char *grade = (score >= 7) ? (BGR BOLD "🌟 AVIATION EXPERT!" RS) : (score >= 5) ? (BYL BOLD "🔥 EXCELLENT!" RS)
                                                                         : (score >= 3)   ? (BCY BOLD "✅ GOOD EFFORT!" RS)
                                                                                          : (DIM BOLD "💪 KEEP LEARNING!" RS);
    {
        char b[200];
        snprintf(b, 200, "  ⭐  Rating: %s", grade);
        boxRowC(BYL, b);
    }
    boxBot(BYL);
    waitEnter();
}

void priceCalc()
{
    printBanner();
    sectionHeader("FLIGHT PRICE CALCULATOR", BMG);
    string org = iField("Origin city:");
    string dst = iField("Destination city:");
    int dist = iInt("Estimated distance (km):", 100, 20000);
    printf("  " BCY "[1]" RS " Economy  " BCY "[2]" RS " Premium-Eco  " BCY "[3]" RS " Business  " BCY "[4]" RS " First\n");
    int cc = iInt("Cabin:", 1, 4);
    int pax = iInt("Passengers:", 1, 9);
    double rates[] = {0.09, 0.16, 0.30, 0.48};
    if (cc < 1 || cc > 4)
        cc = 1;
    double base = dist * rates[cc - 1] * pax;
    double taxes = base * 0.167;
    double fees = 15.0 * pax;
    double total = base + taxes + fees;
    spinner("Calculating best fares...", 600);
    printf("\n");
    boxTop(BMG);
    boxCen(BMG, BMG "  💰  FARE ESTIMATE  " RS);
    boxSep(BMG);
    {
        char b[200];
        snprintf(b, 200, "  Route     : " BWH "%s → %s" RS, org.c_str(), dst.c_str());
        boxRow(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Distance  : " BWH "%d km" RS, dist);
        boxRow(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Passengers: " BWH "%d" RS, pax);
        boxRow(BMG, b);
    }
    boxSep(BMG);
    {
        char b[200];
        snprintf(b, 200, "  Base Fare : " BWH "$%.2f" RS, base);
        boxRow(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Taxes     : " BWH "$%.2f" RS, taxes);
        boxRow(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Fees      : " BWH "$%.2f" RS, fees);
        boxRow(BMG, b);
    }
    boxSep(BMG);
    {
        char b[200];
        snprintf(b, 200, "  " BOLD "TOTAL     : " BGR "$%.2f" RS " (estimate)", total);
        boxRowC(BMG, b);
    }
    boxBot(BMG);
    warnMsg("This is an estimate only. Actual fares vary.");
    waitEnter();
}

void entertainment()
{
    int ch;
    do
    {
        printBanner();
        printf("\n");
        boxTop(BYL);
        boxCen(BYL, BYL "  🎮  NEXUS AIR ENTERTAINMENT  " RS);
        boxSep(BYL);
        boxRowC(BYL, "  " BYL "1. " RS "  ✈  Aviation Quiz");
        boxRowC(BYL, "  " BYL "2. " RS "  💰  Flight Price Calculator");
        boxSep(BYL);
        boxRowC(BYL, "  " BRD "3. " RS "  🔙  Back");
        boxBot(BYL);
        ch = iInt("Choice:", 1, 3);
        if (ch == 1)
            aviationQuiz();
        else if (ch == 2)
            priceCalc();
    } while (ch != 3);
}

/* ═══════════════════════════════════════════════════════════
   PASSENGER DASHBOARD
   ═══════════════════════════════════════════════════════════ */
void passengerDash()
{
    for (;;)
    {
        Passenger &p = passengers[sess.idx];
        string t2 = tier(p.totalMiles);
        strncpy(p.loyaltyTier, t2.c_str(), 11);
        printBanner();
        printf("\n  %s★ %s MEMBER" RS "  │  " DIM "Miles: " RS BWH "%d" RS "  │  " DIM "Points: " RS BWH "%d" RS "\n\n",
               tierCol(p.loyaltyTier), p.loyaltyTier, p.totalMiles, p.loyaltyPts);
        boxTop(BGR);
        {
            char b[200];
            snprintf(b, 200, "  " BGR "👤  PASSENGER DASHBOARD — %s %s" RS, p.firstName, p.lastName);
            boxRowC(BGR, b);
        }
        boxMid(BGR);
        boxRowC(BGR, "  " BGR "[1]" RS "  🔍  Search & Book Flights");
        boxRowC(BGR, "  " BGR "[2]" RS "  📋  My Bookings");
        boxRowC(BGR, "  " BGR "[3]" RS "  ✈   Online Check-In");
        boxRowC(BGR, "  " BGR "[4]" RS "  ⭐  Nexus Miles Loyalty");
        boxRowC(BGR, "  " BGR "[5]" RS "  👤  My Profile");
        boxRowC(BGR, "  " BGR "[6]" RS "  🧳  Baggage Tracking");
        boxRowC(BGR, "  " BGR "[7]" RS "  🎮  Entertainment");
        boxSep(BGR);
        boxRowC(BGR, "  " BRD "[8]" RS "  🚪  Logout");
        boxBot(BGR);
        /* upcoming */
        printf("\n");
        boxSep(BCY);
        printf(BCY "  UPCOMING FLIGHTS" RS "\n");
        boxSep(BCY);
        int uc = 0;
        for (int i = 0; i < bookingCount; i++)
            if (bookings[i].passengerId == p.id && !strcmp(bookings[i].status, "CONFIRMED"))
            {
                printf("  " BCY "✈ %s" RS "   Seat: " BWH "%s" RS "   %s\n",
                       bookings[i].flightNo, bookings[i].seatNo,
                       bookings[i].checkedIn ? (BGR "✔ Checked-In" RS) : (BYL "⏳ Pending Check-in" RS));
                uc++;
            }
        if (!uc)
            printf("  " DIM "No upcoming bookings.\n" RS);
        int ch = iInt("\nChoice [1-8]:", 1, 8);
        switch (ch)
        {
        case 1:
            bookFlight();
            break;
        case 2:
            viewBookings();
            break;
        case 3:
            onlineCheckin();
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
            sess.loggedIn = 0;
            success("Logged out. Safe travels!");
            sleepMs(700);
            return;
        default:
            warnMsg("Invalid.");
            sleepMs(400);
        }
    }
}

/* ═══════════════════════════════════════════════════════════
   STAFF FUNCTIONS — 10 ROLES
   ═══════════════════════════════════════════════════════════ */
void staffSignup()
{
    printBanner();
    sectionHeader("STAFF REGISTRATION", BMG);
    printf("\n");
    boxTop(BMG);
    boxCen(BMG, BMG "  🛡  STAFF CODE VERIFICATION  " RS);
    boxMid(BMG);
    boxRowC(BMG, "  " DIM "Staff access requires the authorisation code." RS);
    boxBot(BMG);
    string code = iPass("Staff Code:");
    if (code != STAFF_CODE)
    {
        glitch("  ██ STAFF CODE REJECTED ██");
        errMsg("Invalid staff code.");
        waitEnter();
        return;
    }
    success("Staff code verified.");
    if (staffCount >= MAX_ST)
    {
        errMsg("Staff capacity reached.");
        waitEnter();
        return;
    }
    string un = iField("Username:");
    if (findStaff(un.c_str()) >= 0)
    {
        errMsg("Username already taken.");
        waitEnter();
        return;
    }
    string pw = iPass("Password:", true);
    string pw2 = iPass("Confirm Password:");
    if (pw != pw2)
    {
        errMsg("Passwords do not match.");
        waitEnter();
        return;
    }

    printf("\n");
    boxTop(BMG);
    boxCen(BMG, BMG "  🛂  SELECT YOUR ROLE  " RS);
    boxSep(BMG);
    for (int i = 0; i < SR_COUNT; i++)
    {
        char b[200];
        snprintf(b, 200, "  %s[%2d] %-20s" RS " " DIM "— %s" RS, SROLE_COL[i], i + 1, SROLES[i], SROLE_DESC[i]);
        boxRowC(BMG, b);
    }
    boxBot(BMG);
    int rc = iInt("Select Role:", 1, SR_COUNT);
    rc--;

    printf("\n");
    boxTop(BMG);
    boxCen(BMG, BMG "  📋  PERSONAL DETAILS  " RS);
    boxSep(BMG);
    string fn = iField("First Name:");
    string ln = iField("Last Name:");
    string em = iEmail("Email:");
    string ph = iPhone("Phone:");
    string ter = iField("Terminal (e.g. T2):");
    string gates = iField("Assigned Gates (e.g. 10,11):");
    boxBot(BMG);

    Staff &s = staffArr[staffCount++];
    s.id = nextSt++;
    char empId[16];
    snprintf(empId, 16, "NX-EMP-%04d", s.id);
    strncpy(s.empId, empId, 15);
    strncpy(s.username, un.c_str(), SZ - 1);
    string h = hashPW(pw);
    strncpy(s.passHash, h.c_str(), 23);
    strncpy(s.firstName, fn.c_str(), SZ - 1);
    strncpy(s.lastName, ln.c_str(), SZ - 1);
    strncpy(s.email, em.c_str(), SZ - 1);
    strncpy(s.phone, ph.c_str(), SZ - 1);
    s.roleCode = rc;
    strncpy(s.terminal, ter.c_str(), 19);
    strncpy(s.gates, gates.c_str(), 39);
    strcpy(s.shiftStart, "08:00");
    strcpy(s.shiftEnd, "16:00");
    string ts = getTS();
    strncpy(s.created, ts.c_str(), 19);
    s.active = 1;
    saveStaff();
    success(("Account created! Employee ID: " + string(s.empId)).c_str());
    waitEnter();
}

bool staffLogin()
{
    printBanner();
    sectionHeader("STAFF LOGIN", BMG);
    printf("\n");
    boxTop(BMG);
    boxCen(BMG, BMG "  🛂  STAFF PORTAL  " RS);
    boxSep(BMG);
    string un = iField("Employee Username:");
    string pw = iPass("Password:");
    boxBot(BMG);
    spinner("Authenticating staff...", 700);
    int idx = findStaff(un.c_str());
    if (idx < 0 || hashPW(pw) != string(staffArr[idx].passHash))
    {
        glitch("  ██ ACCESS DENIED ██");
        errMsg("Invalid credentials.");
        waitEnter();
        return false;
    }
    sess.role = RSTAFF;
    sess.idx = idx;
    sess.loggedIn = 1;
    strncpy(sess.username, un.c_str(), SZ - 1);
    Staff &s = staffArr[idx];
    printBanner();
    sectionHeader("STAFF LOGIN SUCCESSFUL", BMG);
    printf("  " BOLD BMG "✔  Welcome, %s %s — %s" RS "\n\n", s.firstName, s.lastName, SROLES[s.roleCode]);
    printf("  " CY "Employee ID : " RS BWH "%s" RS "\n", s.empId);
    printf("  " CY "Role        : " RS "%s%s" RS "\n", SROLE_COL[s.roleCode], SROLES[s.roleCode]);
    printf("  " CY "Duties      : " RS DIM "%s" RS "\n", SROLE_DESC[s.roleCode]);
    printf("  " CY "Terminal    : " RS BWH "%s" RS "   Gates: " BWH "%s" RS "\n", s.terminal, s.gates);
    sndLogin();
    waitEnter();
    return true;
}

/* ── Role-specific staff panels ──────────────────────────── */

/* Gate Agent / Ticket Checker — boarding management */
void staffBoardingPanel(Staff &s)
{
    printBanner();
    sectionHeader("BOARDING MANAGEMENT", BMG);
    string fno = iField("Flight Number:");
    int fi = findFlight(fno.c_str());
    if (fi < 0)
    {
        errMsg("Flight not found.");
        waitEnter();
        return;
    }
    Flight &fl = flights[fi];
    printf("\n");
    boxTop(BMG);
    {
        char b[200];
        snprintf(b, 200, "  " BOLD BMG "✈  %s — %s → %s" RS, fl.flightNo, fl.origin, fl.dest);
        boxRowC(BMG, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Gate: " RS BWH "%d" RS "   Status: " RS "%s" BOLD "%s" RS, fl.gate, stCol(fl.status), fl.status);
        boxRowC(BMG, b);
    }
    boxSep(BMG);
    int total = 0, ci = 0;
    for (int i = 0; i < bookingCount; i++)
        if (!strcmp(bookings[i].flightNo, fno.c_str()) && !strcmp(bookings[i].status, "CONFIRMED"))
        {
            total++;
            if (bookings[i].checkedIn)
                ci++;
            const char *chk = bookings[i].checkedIn ? (BGR "✔" RS) : (BRD "✘" RS);
            char b[200];
            snprintf(b, 200, "  %s %-22s Seat:%-5s  Class:%-16s",
                     chk, bookings[i].passengerName, bookings[i].seatNo, bookings[i].cabin);
            boxRow(BMG, b);
        }
    boxSep(BMG);
    {
        char b[100];
        snprintf(b, 100, "  " DIM "Checked-In: " RS BGR "%d" RS "/" BWH "%d" RS, ci, total);
        boxRowC(BMG, b);
    }
    boxBot(BMG);
    printf("\n  " BCY "[S]" RS " Scan ticket   " BCY "[B]" RS " Start boarding   " BCY "[D]" RS " Mark departed\n");
    string ch = iField("Action:");
    if (ch == "S" || ch == "s")
    {
        string ref = iField("Boarding pass ref or seat:");
        sndScan();
        /* Try find by ref */
        int bi = findBooking(ref.c_str());
        if (bi >= 0 && !strcmp(bookings[bi].flightNo, fno.c_str()))
        {
            if (!bookings[bi].checkedIn)
            {
                warnMsg("Passenger NOT checked in!");
                return;
            }
            else
            {
                char b[80];
                snprintf(b, 80, "Passenger %s — BOARDING APPROVED", bookings[bi].passengerName);
                success(b);
            }
        }
        else
            errMsg("Booking not found.");
    }
    else if (ch == "B" || ch == "b")
    {
        strcpy(fl.status, "BOARDING");
        saveFlights();
        success(("Flight " + fno + " — BOARDING STARTED").c_str());
    }
    else if (ch == "D" || ch == "d")
    {
        strcpy(fl.status, "DEPARTED");
        saveFlights();
        success(("Flight " + fno + " — MARKED DEPARTED").c_str());
    }
    waitEnter();
}

/* Check-in Agent */
void staffCheckInPanel(Staff &s)
{
    printBanner();
    sectionHeader("CHECK-IN OPERATIONS", BMG);
    string fno = iField("Flight Number:");
    int fi = findFlight(fno.c_str());
    if (fi < 0)
    {
        errMsg("Flight not found.");
        waitEnter();
        return;
    }
    Flight &fl = flights[fi];
    printf("\n  Flight: " BWH "%s" RS "  %s → %s  Gate %d\n\n", fl.flightNo, fl.origin, fl.dest, fl.gate);
    int ci = 0, total = 0;
    for (int i = 0; i < bookingCount; i++)
        if (!strcmp(bookings[i].flightNo, fno.c_str()) && !strcmp(bookings[i].status, "CONFIRMED"))
        {
            total++;
            if (bookings[i].checkedIn)
                ci++;
        }
    printf("  Check-in: " BGR "%d" RS " / " BWH "%d" RS " passengers\n\n", ci, total);
    printf("  " BCY "[F]" RS " Force check-in a passenger   " BCY "[V]" RS " View all   " BCY "[B]" RS " Back\n");
    string ch = iField("Action:");
    if (ch == "F" || ch == "f")
    {
        string ref = iField("Booking Reference:");
        int bi = findBooking(ref.c_str());
        if (bi < 0)
            errMsg("Booking not found.");
        else if (!strcmp(bookings[bi].flightNo, fno.c_str()))
        {
            bookings[bi].checkedIn = 1;
            saveBookings();
            success("Passenger checked in manually.");
            sndScan();
        }
        else
            errMsg("Booking not on this flight.");
    }
    else if (ch == "V" || ch == "v")
    {
        for (int i = 0; i < bookingCount; i++)
            if (!strcmp(bookings[i].flightNo, fno.c_str()) && !strcmp(bookings[i].status, "CONFIRMED"))
            {
                const char *sc2 = bookings[i].checkedIn ? (BGR "✔ In" RS) : (BYL "⏳ Out" RS);
                printf("  %s %-22s Seat:%-5s\n", sc2, bookings[i].passengerName, bookings[i].seatNo);
            }
    }
    waitEnter();
}

/* Baggage Handler */
void staffBaggagePanel(Staff &s)
{
    printBanner();
    sectionHeader("BAGGAGE OPERATIONS", BMG);
    printf("  " BCY "[1]" RS " View bags by flight\n");
    printf("  " BCY "[2]" RS " Update bag status\n");
    printf("  " BCY "[3]" RS " Report missing bag\n");
    printf("  " BCY "[4]" RS " Weight check\n");
    printf("  " BCY "[0]" RS " Back\n\n");
    int ch = iInt("Choice:", 0, 4);
    if (ch == 1)
    {
        string fno = iField("Flight Number:");
        bool found = false;
        for (int i = 0; i < bagCount; i++)
        {
            BagItem &bg = bags[i];
            if (!strcmp(bg.flightNo, fno.c_str()))
            {
                showBagStatus(bg);
                found = true;
            }
        }
        if (!found)
            infoMsg("No bags on record for this flight.");
    }
    else if (ch == 2)
    {
        string tag = iField("Bag Tag:");
        bool found = false;
        for (int i = 0; i < bagCount; i++)
            if (!strcmp(bags[i].tagNo, tag.c_str()))
            {
                printf("  Current: %s%s" RS "\n", BAG_COL[bags[i].stageIdx], BAG_ST[bags[i].stageIdx]);
                for (int j = 0; j < BAG_STAGES; j++)
                {
                    printf("  " BCY "[%d] " RS "%s\n", j + 1, BAG_ST[j]);
                }
                int sc = iInt("New stage:", 1, BAG_STAGES);
                bags[i].stageIdx = sc - 1;
                if (sc == 7)
                    strncpy(bags[i].location, "Arrival Hall", 47);
                else if (sc == 5)
                    strncpy(bags[i].location, "Cargo Hold", 47);
                else if (sc == 4)
                    strncpy(bags[i].location, "Sorting Belt", 47);
                saveBags();
                success("Bag status updated.");
                sndScan();
                found = true;
                break;
            }
        if (!found)
            errMsg("Tag not found.");
    }
    else if (ch == 3)
    {
        string tag = iField("Bag Tag:");
        for (int i = 0; i < bagCount; i++)
            if (!strcmp(bags[i].tagNo, tag.c_str()))
            {
                bags[i].stageIdx = 0;
                strcpy(bags[i].location, "MISSING — Investigation Open");
                saveBags();
                warnMsg("Bag reported MISSING. Passenger will be notified.");
                break;
            }
    }
    else if (ch == 4)
    {
        string fno = iField("Flight Number:");
        double total = 0;
        int count = 0;
        for (int i = 0; i < bagCount; i++)
            if (!strcmp(bags[i].flightNo, fno.c_str()))
            {
                total += bags[i].weightKg;
                count++;
                if (bags[i].weightKg > 23)
                    warnMsg(("OVERWEIGHT bag: " + string(bags[i].tagNo)).c_str());
            }
        char b[100];
        snprintf(b, 100, "%d bags, total %.1fkg for flight %s.", count, total, fno.c_str());
        infoMsg(b);
    }
    waitEnter();
}

/* Security Officer */
void staffSecurityPanel(Staff &s)
{
    printBanner();
    sectionHeader("SECURITY OPERATIONS", BMG);
    printf("  " BCY "[1]" RS " Verify passenger ticket & ID\n");
    printf("  " BCY "[2]" RS " Random screening log\n");
    printf("  " BCY "[3]" RS " Prohibited items report\n");
    printf("  " BCY "[0]" RS " Back\n\n");
    int ch = iInt("Choice:", 0, 3);
    if (ch == 1)
    {
        string ref = iField("Booking Reference:");
        int bi = findBooking(ref.c_str());
        if (bi < 0)
        {
            errMsg("Booking not found.");
        }
        else
        {
            Booking &bk = bookings[bi];
            printf("\n");
            boxTop(BGR);
            {
                char b[200];
                snprintf(b, 200, "  " BWH "Passenger : %s" RS, bk.passengerName);
                boxRowC(BGR, b);
            }
            {
                char b[200];
                snprintf(b, 200, "  Flight   : %s   Seat: %s", bk.flightNo, bk.seatNo);
                boxRow(BGR, b);
            }
            {
                char b[200];
                snprintf(b, 200, "  Status   : %s%s" RS, (!strcmp(bk.status, "CONFIRMED")) ? BGR : BRD, bk.status);
                boxRowC(BGR, b);
            }
            {
                char b[200];
                snprintf(b, 200, "  CheckIn  : %s", bk.checkedIn ? "YES" : "NO");
                boxRow(BGR, b);
            }
            boxBot(BGR);
            sndScan();
        }
    }
    else if (ch == 2)
    {
        success("Screening log entry recorded.");
    }
    else if (ch == 3)
    {
        string desc = iField("Prohibited item found (description):");
        char b[100];
        snprintf(b, 100, "Item reported: %s. Security notified.", desc.c_str());
        warnMsg(b);
        sndAlert();
    }
    if (ch != 0)
        waitEnter();
}

/* Ticket Checker */
void staffTicketPanel(Staff &s)
{
    printBanner();
    sectionHeader("TICKET VERIFICATION", BMG);
    printf("  " DIM "Scan or enter boarding pass reference to verify.\n\n" RS);
    string ref = iField("Boarding Pass Ref:");
    sndScan();
    int bi = findBooking(ref.c_str());
    if (bi < 0)
    {
        errMsg("Booking NOT FOUND — deny boarding!");
        waitEnter();
        return;
    }
    Booking &bk = bookings[bi];
    printf("\n");
    boxTop(!strcmp(bk.status, "CONFIRMED") ? BGR : BRD);
    {
        char b[200];
        snprintf(b, 200, "  PASSENGER : " BWH "%s" RS, bk.passengerName);
        boxRow(!strcmp(bk.status, "CONFIRMED") ? BGR : BRD, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  FLIGHT    : " BWH "%s" RS "   SEAT: " BWH "%s" RS, bk.flightNo, bk.seatNo);
        boxRowC(!strcmp(bk.status, "CONFIRMED") ? BGR : BRD, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  STATUS    : %s%s" RS, (!strcmp(bk.status, "CONFIRMED")) ? BGR BOLD : BRD BOLD, bk.status);
        boxRowC(!strcmp(bk.status, "CONFIRMED") ? BGR : BRD, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  CHECKED-IN: %s", bk.checkedIn ? (BGR "YES — ALLOW BOARDING" RS) : (BRD "NO — SEND TO CHECK-IN" RS));
        boxRowC(!strcmp(bk.status, "CONFIRMED") ? BGR : BRD, b);
    }
    boxBot(!strcmp(bk.status, "CONFIRMED") ? BGR : BRD);
    if (!strcmp(bk.status, "CONFIRMED") && bk.checkedIn)
    {
        sndOK();
    }
    else
    {
        sndErr();
    }
    waitEnter();
}

/* Flight Dispatcher */
void staffDispatchPanel(Staff &s)
{
    printBanner();
    sectionHeader("FLIGHT DISPATCH", BMG);
    string fno = iField("Flight Number:");
    int fi = findFlight(fno.c_str());
    if (fi < 0)
    {
        errMsg("Flight not found.");
        waitEnter();
        return;
    }
    Flight &fl = flights[fi];
    printf("\n  Flight: " BWH "%s" RS "  Status: %s" BOLD "%s" RS "\n\n", fl.flightNo, stCol(fl.status), fl.status);
    printf("  " BCY "[1]" RS " Update status   " BCY "[2]" RS " Set delay   " BCY "[3]" RS " Clear for departure\n");
    int ch = iInt("Action:", 1, 3);
    if (ch == 1)
    {
        printf("  " BCY "[1]" RS " ON_TIME  [2] DELAYED  [3] BOARDING  [4] DEPARTED  [5] LANDED\n");
        int sc = iInt("Status:", 1, 5);
        const char *sts[] = {"ON_TIME", "DELAYED", "BOARDING", "DEPARTED", "LANDED"};
        strncpy(fl.status, sts[sc - 1], 19);
        saveFlights();
        success(("Flight " + string(fno) + " status → " + string(sts[sc - 1])).c_str());
    }
    else if (ch == 2)
    {
        string reason = iField("Delay reason:");
        strncpy(fl.delayReason, reason.c_str(), 79);
        strcpy(fl.status, "DELAYED");
        saveFlights();
        warnMsg(("Flight " + string(fno) + " delayed: " + reason).c_str());
    }
    else if (ch == 3)
    {
        int bkd = fl.bkFirst + fl.bkBiz + fl.bkPrem + fl.bkEco;
        printf("  " BWH "%d passengers on board. Clear for departure?\n" RS, bkd);
        string conf = iField("[YES/NO]:");
        if (conf == "YES")
        {
            strcpy(fl.status, "DEPARTED");
            saveFlights();
            success("Flight cleared for departure.");
            sndFly();
        }
    }
    waitEnter();
}

/* Customs Officer */
void staffCustomsPanel(Staff &s)
{
    printBanner();
    sectionHeader("CUSTOMS & IMMIGRATION", BMG);
    string ref = iField("Passport / Booking Ref:");
    int bi = findBooking(ref.c_str());
    if (bi < 0)
    { /* Try by ref directly */
        errMsg("Booking not found.");
        waitEnter();
        return;
    }
    Booking &bk = bookings[bi];
    /* Find passenger */
    int pi = -1;
    for (int i = 0; i < passCount; i++)
        if (passengers[i].id == bk.passengerId)
        {
            pi = i;
            break;
        }
    if (pi < 0)
    {
        errMsg("Passenger record not found.");
        waitEnter();
        return;
    }
    Passenger &p = passengers[pi];
    printf("\n");
    boxTop(BCY);
    {
        char b[200];
        snprintf(b, 200, "  Name        : " BWH "%s %s" RS, p.firstName, p.lastName);
        boxRow(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Nationality : " BWH "%s" RS, p.nationality);
        boxRow(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Passport    : " BWH "%s" RS, p.passport);
        boxRow(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  DOB         : " BWH "%s" RS, p.dob);
        boxRow(BCY, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  Destination : " BWH "%s" RS, bk.flightNo);
        boxRow(BCY, b);
    }
    boxBot(BCY);
    sndScan();
    printf("  " BCY "[A]" RS " Admit   " BCY "[F]" RS " Flag for inspection   " BCY "[D]" RS " Deny entry\n");
    string dec = iField("Decision:");
    if (dec == "A" || dec == "a")
        success("Passenger admitted. Immigration stamp issued.");
    else if (dec == "F" || dec == "f")
    {
        warnMsg("Passenger flagged. Escort to secondary inspection.");
        sndWarn();
    }
    else if (dec == "D" || dec == "d")
    {
        errMsg("Entry DENIED. Deportation process initiated.");
        sndErr();
    }
    waitEnter();
}

/* Lounge Attendant */
void staffLoungePanel(Staff &s)
{
    printBanner();
    sectionHeader("EXECUTIVE LOUNGE SERVICES", BMG);
    printf("  " BCY "[1]" RS " Verify lounge access\n");
    printf("  " BCY "[2]" RS " Log complimentary service\n");
    printf("  " BCY "[0]" RS " Back\n\n");
    int ch = iInt("Choice:", 0, 2);
    if (ch == 1)
    {
        string ref = iField("Booking Ref:");
        int bi = findBooking(ref.c_str());
        if (bi < 0)
        {
            errMsg("Booking not found.");
        }
        else
        {
            const char *c = bookings[bi].cabin;
            bool ok = (!strcmp(c, "FIRST") || !strcmp(c, "BUSINESS"));
            if (ok)
            {
                success("Lounge access GRANTED — First or Business class.");
                sndOK();
            }
            else
            { /* Check tier */
                int pi = -1;
                for (int i = 0; i < passCount; i++)
                    if (passengers[i].id == bookings[bi].passengerId)
                    {
                        pi = i;
                        break;
                    }
                bool tierOk = (pi >= 0 && (!strcmp(passengers[pi].loyaltyTier, "GOLD") || !strcmp(passengers[pi].loyaltyTier, "PLATINUM")));
                if (tierOk)
                    success("Lounge access GRANTED — Elite tier member.");
                else
                    errMsg("Lounge access DENIED — Economy class / Bronze/Silver tier.");
            }
        }
    }
    else if (ch == 2)
    {
        string svc = iField("Service rendered:");
        success(("Service logged: " + svc).c_str());
    }
    if (ch != 0)
        waitEnter();
}

/* Ground Crew */
void staffGroundPanel(Staff &s)
{
    printBanner();
    sectionHeader("GROUND OPERATIONS", BMG);
    string fno = iField("Flight Number:");
    int fi = findFlight(fno.c_str());
    if (fi < 0)
    {
        errMsg("Flight not found.");
        waitEnter();
        return;
    }
    Flight &fl = flights[fi];
    printf("\n  " BWH "Aircraft: %s" RS "   Gate: %d   Status: %s" BOLD "%s" RS "\n\n", fl.aircraft, fl.gate, stCol(fl.status), fl.status);
    printf("  " BCY "[1]" RS " Log aircraft ready   " BCY "[2]" RS " Fueling complete   " BCY "[3]" RS " Pushback authorised\n");
    int ch = iInt("Action:", 1, 3);
    if (ch == 1)
        success("Aircraft ready for boarding logged.");
    else if (ch == 2)
    {
        sndScan();
        success("Fueling complete. Quantity logged.");
    }
    else if (ch == 3)
    {
        strcpy(fl.status, "DEPARTED");
        saveFlights();
        sndFly();
        success("Pushback authorised. Flight departed.");
    }
    waitEnter();
}

/* Supervisor */
void staffSupervisorPanel(Staff &s)
{
    for (;;)
    {
        printBanner();
        sectionHeader("SUPERVISOR PANEL", BMG);
        printf("  " BCY "[1]" RS " View all flights\n");
        printf("  " BCY "[2]" RS " Update flight status\n");
        printf("  " BCY "[3]" RS " View staff on duty\n");
        printf("  " BCY "[4]" RS " View baggage overview\n");
        printf("  " BCY "[0]" RS " Back\n\n");
        int ch = iInt("Choice:", 0, 4);
        if (ch == 0)
            return;
        if (ch == 1)
        {
            for (int i = 0; i < flightCount; i++)
                if (flights[i].active)
                    showFlightCard(flights[i], i);
            waitEnter();
        }
        else if (ch == 2)
        {
            string fno = iField("Flight:");
            int fi = findFlight(fno.c_str());
            if (fi < 0)
                errMsg("Not found.");
            else
            {
                printf("  " BCY "[1]" RS " ON_TIME  [2] DELAYED  [3] BOARDING  [4] DEPARTED  [5] CANCELLED\n");
                int sc = iInt("Status:", 1, 5);
                const char *sts[] = {"ON_TIME", "DELAYED", "BOARDING", "DEPARTED", "CANCELLED"};
                strncpy(flights[fi].status, sts[sc - 1], 19);
                saveFlights();
                success("Status updated.");
            }
            waitEnter();
        }
        else if (ch == 3)
        {
            printf("\n  " BOLD CY "%-10s %-20s %-20s %-14s\n" RS, "EMP ID", "NAME", "ROLE", "TERMINAL");
            printf("  " DIM "───────────────────────────────────────────────────────────────────\n" RS);
            for (int i = 0; i < staffCount; i++)
                if (staffArr[i].active)
                {
                    Staff &st = staffArr[i];
                    printf("  " BCY "%-10s" RS WH "%-20s " BMG "%-20s" WH "%-14s\n" RS,
                           st.empId, st.firstName, SROLES[st.roleCode], st.terminal);
                }
            waitEnter();
        }
        else if (ch == 4)
        {
            int stageCounts[BAG_STAGES] = {};
            for (int i = 0; i < bagCount; i++)
                stageCounts[bags[i].stageIdx]++;
            printf("\n");
            boxTop(BCY);
            for (int i = 0; i < BAG_STAGES; i++)
            {
                char b[100];
                snprintf(b, 100, "  %s%-20s" RS " : " BWH "%d bags" RS, BAG_COL[i], BAG_ST[i], stageCounts[i]);
                boxRowC(BCY, b);
            }
            boxBot(BCY);
            waitEnter();
        }
    }
}

/* ─── Staff Dashboard ─────────────────────────────────────── */
void staffDash()
{
    for (;;)
    {
        Staff &s = staffArr[sess.idx];
        printBanner();
        {
            char h[80];
            snprintf(h, 80, "STAFF DASHBOARD — %s %s", s.firstName, s.lastName);
            printf("\n");
        }
        printf("\n");
        boxTop(BMG);
        {
            char b[200];
            snprintf(b, 200, "  " BMG "🛂  %s — %s" RS, SROLES[s.roleCode], s.empId);
            boxRowC(BMG, b);
        }
        {
            char b[200];
            snprintf(b, 200, "  " DIM "%s" RS, SROLE_DESC[s.roleCode]);
            boxRowC(BMG, b);
        }
        {
            char b[200];
            snprintf(b, 200, "  Terminal: " BWH "%s" RS "   Gates: " BWH "%s" RS "   Shift: " BWH "%s–%s" RS, s.terminal, s.gates, s.shiftStart, s.shiftEnd);
            boxRowC(BMG, b);
        }
        boxMid(BMG);
        /* Show role-appropriate menu items */
        boxRowC(BMG, "  " BMG "[1]" RS "  Primary Role Duties");
        boxRowC(BMG, "  " BMG "[2]" RS "  ✈   View/Update Flight Status");
        boxRowC(BMG, "  " BMG "[3]" RS "  🔍  Search Flights");
        boxRowC(BMG, "  " BMG "[4]" RS "  🧳  Baggage Operations");
        boxSep(BMG);
        boxRowC(BMG, "  " BRD "[5]" RS "  🚪  Logout");
        boxBot(BMG);
        int ch = iInt("Choice:", 1, 5);
        if (ch == 5)
        {
            sess.loggedIn = 0;
            success("Logged out. Good shift!");
            sleepMs(600);
            return;
        }
        if (ch == 1)
        {
            switch (s.roleCode)
            {
            case SR_GATE:
                staffBoardingPanel(s);
                break;
            case SR_CHECKIN:
                staffCheckInPanel(s);
                break;
            case SR_BAGGAGE:
                staffBaggagePanel(s);
                break;
            case SR_SUPERVISOR:
                staffSupervisorPanel(s);
                break;
            case SR_TICKET:
                staffTicketPanel(s);
                break;
            case SR_SECURITY:
                staffSecurityPanel(s);
                break;
            case SR_LOUNGE:
                staffLoungePanel(s);
                break;
            case SR_GROUND:
                staffGroundPanel(s);
                break;
            case SR_DISPATCH:
                staffDispatchPanel(s);
                break;
            case SR_CUSTOMS:
                staffCustomsPanel(s);
                break;
            default:
                infoMsg("No specific panel for this role.");
                waitEnter();
            }
        }
        else if (ch == 2)
        {
            string fno = iField("Flight Number:");
            int fi = findFlight(fno.c_str());
            if (fi < 0)
                errMsg("Not found.");
            else
            {
                Flight &fl = flights[fi];
                printf("  " BCY "[1]" RS " ON_TIME  [2] DELAYED  [3] BOARDING  [4] DEPARTED  [5] LANDED\n");
                int sc2 = iInt("Status:", 1, 5);
                const char *sts[] = {"ON_TIME", "DELAYED", "BOARDING", "DEPARTED", "LANDED"};
                strncpy(fl.status, sts[sc2 - 1], 19);
                if (sc2 == 2)
                {
                    string r = iField("Delay reason:");
                    strncpy(fl.delayReason, r.c_str(), 79);
                }
                saveFlights();
                success("Status updated.");
                int cnt = 0;
                for (int i = 0; i < bookingCount; i++)
                    if (!strcmp(bookings[i].flightNo, fno.c_str()))
                        cnt++;
                char b[80];
                snprintf(b, 80, "%d passengers notified.", cnt);
                infoMsg(b);
            }
            waitEnter();
        }
        else if (ch == 3)
            searchFlights();
        else if (ch == 4)
            staffBaggagePanel(s);
    }
}

/* ═══════════════════════════════════════════════════════════
   ADMIN FUNCTIONS
   ═══════════════════════════════════════════════════════════ */
void adminSignup()
{
    printBanner();
    sectionHeader("ADMIN REGISTRATION", BYL);
    string code = iPass("Admin Code:");
    if (code != ADMIN_CODE)
    {
        glitch("  ██ ADMIN ACCESS DENIED ██");
        errMsg("Invalid admin code.");
        waitEnter();
        return;
    }
    success("Admin code verified.");
    if (adminCount >= MAX_AD)
    {
        errMsg("Admin capacity reached.");
        waitEnter();
        return;
    }
    string un = iField("Username:");
    if (findAdmin(un.c_str()) >= 0)
    {
        errMsg("Username taken.");
        waitEnter();
        return;
    }
    string pw = iPass("Password:", true);
    string pw2 = iPass("Confirm:");
    if (pw != pw2)
    {
        errMsg("Passwords do not match.");
        waitEnter();
        return;
    }
    string fn = iField("First Name:");
    string ln = iField("Last Name:");
    string em = iEmail("Email:");
    Admin &a = admins[adminCount++];
    a.id = nextAd++;
    strncpy(a.username, un.c_str(), SZ - 1);
    string h = hashPW(pw);
    strncpy(a.passHash, h.c_str(), 23);
    strncpy(a.firstName, fn.c_str(), SZ - 1);
    strncpy(a.lastName, ln.c_str(), SZ - 1);
    strncpy(a.email, em.c_str(), SZ - 1);
    string ts = getTS();
    strncpy(a.created, ts.c_str(), 19);
    a.active = 1;
    saveAdmins();
    success("Admin account created.");
    waitEnter();
}

bool adminLogin()
{
    printBanner();
    sectionHeader("ADMIN LOGIN", BYL);
    printf("\n");
    boxTop(BYL);
    boxCen(BYL, BYL "  👑  ADMIN PORTAL  " RS);
    boxSep(BYL);
    string un = iField("Username:");
    string pw = iPass("Password:");
    string code = iPass("2FA Admin Code:");
    boxBot(BYL);
    if (code != ADMIN_CODE)
    {
        glitch("  ██ ADMIN ACCESS DENIED ██");
        errMsg("Invalid admin code.");
        waitEnter();
        return false;
    }
    spinner("Verifying admin credentials...", 900);
    int idx = findAdmin(un.c_str());
    if (idx < 0 || hashPW(pw) != string(admins[idx].passHash))
    {
        glitch("  ██ ACCESS DENIED ██");
        errMsg("Invalid credentials.");
        waitEnter();
        return false;
    }
    sess.role = RADMIN;
    sess.idx = idx;
    sess.loggedIn = 1;
    strncpy(sess.username, un.c_str(), SZ - 1);
    Admin &a = admins[idx];
    printBanner();
    sectionHeader("ADMIN LOGIN SUCCESSFUL", BYL);
    printf("  " BOLD BYL "✔  WELCOME, ADMINISTRATOR %s %s!" RS "\n\n", a.firstName, a.lastName);
    printf("  " CY "Access Level  : " RS BOLD BYL "FULL SYSTEM ACCESS" RS "\n");
    printf("  " CY "Flights       : " RS BWH "%d" RS "\n", flightCount);
    printf("  " CY "Passengers    : " RS BWH "%d" RS "\n", passCount);
    printf("  " CY "Staff Members : " RS BWH "%d" RS "\n", staffCount);
    printf("  " CY "Total Bookings: " RS BWH "%d" RS "\n", bookingCount);
    sndLogin();
    waitEnter();
    return true;
}

/* ── Admin flight management ──────────────────────────────── */
void adminAddFlight()
{
    printBanner();
    sectionHeader("ADD NEW FLIGHT — PAGE 1/3", BYL);
    if (flightCount >= MAX_FL)
    {
        errMsg("Flight capacity reached.");
        waitEnter();
        return;
    }
    Flight &fl = flights[flightCount];
    string fno = iField("Flight Number (e.g. NX606):");
    strncpy(fl.flightNo, fno.c_str(), 15);
    if (findFlight(fno.c_str()) >= 0)
    {
        errMsg("Flight number already exists.");
        waitEnter();
        return;
    }
    string air = iField("Airline:");
    strncpy(fl.airline, air.c_str(), 47);
    string ac = iField("Aircraft Type:");
    strncpy(fl.aircraft, ac.c_str(), 39);

    printBanner();
    sectionHeader("ADD NEW FLIGHT — PAGE 2/3", BYL);
    string org = iField("Origin:");
    strncpy(fl.origin, org.c_str(), 47);
    string dst = iField("Destination:");
    strncpy(fl.dest, dst.c_str(), 47);
    string dt = iDate("Date:");
    strncpy(fl.date, dt.c_str(), 15);
    string dep = iField("Departure (HH:MM):");
    strncpy(fl.depTime, dep.c_str(), 7);
    string arr = iField("Arrival (HH:MM):");
    strncpy(fl.arrTime, arr.c_str(), 7);
    fl.duration = iInt("Duration (mins):", 30, 1500);
    fl.gate = iInt("Gate:", 1, 99);
    fl.termDep = iInt("Dep Terminal:", 1, 9);
    fl.termArr = iInt("Arr Terminal:", 1, 9);

    printBanner();
    sectionHeader("ADD NEW FLIGHT — PAGE 3/3", BYL);
    fl.fareFirst = iDbl("First Class Fare $:");
    if (!fl.fareFirst)
        fl.fareFirst = 2500;
    fl.fareBiz = iDbl("Business Fare $:");
    if (!fl.fareBiz)
        fl.fareBiz = 1200;
    fl.farePrem = iDbl("Prem-Eco Fare $:");
    if (!fl.farePrem)
        fl.farePrem = 700;
    fl.fareEco = iDbl("Economy Fare $:");
    if (!fl.fareEco)
        fl.fareEco = 300;
    fl.taxRate = 16.72;
    fl.seatsFirst = ROWS_FIRST * 4;
    fl.seatsBiz = ROWS_BIZ * 4;
    fl.seatsPrem = ROWS_PREM * 6;
    fl.seatsEco = ROWS_ECO * 6;
    strcpy(fl.status, "ON_TIME");
    strcpy(fl.delayReason, "N/A");
    fl.id = nextFl++;
    fl.active = 1;
    flightCount++;
    getSM(fl.flightNo);
    spinner("Saving flight to database...", 700);
    saveFlights();
    saveSeatMaps();
    success(("Flight " + fno + " created!").c_str());
    waitEnter();
}

void adminViewFlights()
{
    printBanner();
    sectionHeader("ALL FLIGHTS", BYL);
    printf("\n  " BOLD CY "%-4s %-8s %-20s %-8s %-12s %-5s" RS "\n", "#", "FLIGHT", "ROUTE", "DEP", "STATUS", "LOAD%");
    printf("  " DIM "─────────────────────────────────────────────────────────────────\n" RS);
    for (int i = 0; i < flightCount; i++)
    {
        Flight &fl = flights[i];
        if (!fl.active)
            continue;
        int tot = fl.seatsFirst + fl.seatsBiz + fl.seatsPrem + fl.seatsEco;
        int bkd = fl.bkFirst + fl.bkBiz + fl.bkPrem + fl.bkEco;
        int pct = tot > 0 ? bkd * 100 / tot : 0;
        char route[24];
        snprintf(route, 24, "%.9s→%.9s", fl.origin, fl.dest);
        const char *lc = (pct > 80) ? BRD : (pct > 60) ? BYL
                                                       : BGR;
        printf("  " BWH "%-4d" BCY "%-8s" WH "%-20s " WH "%-8s %s%-12s" WH "%s%-5d%%" RS "\n",
               i + 1, fl.flightNo, route, fl.depTime, stCol(fl.status), fl.status, lc, pct);
    }
    printf("\n  " DIM "Total: %d flights\n" RS, flightCount);
    waitEnter();
}

void adminEditFlight()
{
    printBanner();
    sectionHeader("EDIT FLIGHT", BYL);
    string fno = iField("Flight Number:");
    int idx = findFlight(fno.c_str());
    if (idx < 0)
    {
        errMsg("Flight not found.");
        waitEnter();
        return;
    }
    Flight &fl = flights[idx];
    printf("  " CY "[1]" RS " Status  [2] Gate  [3] Departure  [4] Economy Fare  [5] Delay Reason  [6] Deactivate\n\n");
    int ch = iInt("Field to edit:", 1, 6);
    if (ch == 1)
    {
        printf("  [1]ON_TIME [2]DELAYED [3]CANCELLED [4]BOARDING [5]DEPARTED [6]LANDED\n");
        int sc = iInt(":", 1, 6);
        const char *sts[] = {"ON_TIME", "DELAYED", "CANCELLED", "BOARDING", "DEPARTED", "LANDED"};
        strncpy(fl.status, sts[sc - 1], 19);
        saveFlights();
        success("Status updated.");
    }
    else if (ch == 2)
    {
        fl.gate = iInt("New Gate:", 1, 99);
        saveFlights();
        success("Gate updated.");
    }
    else if (ch == 3)
    {
        string t = iField("New Dep Time (HH:MM):");
        strncpy(fl.depTime, t.c_str(), 7);
        saveFlights();
        success("Dep time updated.");
    }
    else if (ch == 4)
    {
        fl.fareEco = iDbl("New Economy Fare:");
        saveFlights();
        success("Fare updated.");
    }
    else if (ch == 5)
    {
        string r = iField("Delay reason:");
        strncpy(fl.delayReason, r.c_str(), 79);
        saveFlights();
        success("Reason saved.");
    }
    else if (ch == 6)
    {
        string c = iField("Type CONFIRM:");
        if (c == "CONFIRM")
        {
            fl.active = 0;
            saveFlights();
            success("Flight deactivated.");
        }
    }
    waitEnter();
}

void adminCancelFlight()
{
    printBanner();
    sectionHeader("CANCEL FLIGHT", BRD);
    string fno = iField("Flight Number:");
    int idx = findFlight(fno.c_str());
    if (idx < 0)
    {
        errMsg("Not found.");
        waitEnter();
        return;
    }
    Flight &fl = flights[idx];
    int bkd = fl.bkFirst + fl.bkBiz + fl.bkPrem + fl.bkEco;
    printf("  " BRD "Flight %s — %d passengers affected.\n" RS, fl.flightNo, bkd);
    string conf = iField("Type CONFIRM to cancel:");
    if (conf != "CONFIRM")
    {
        infoMsg("Cancelled.");
        waitEnter();
        return;
    }
    strcpy(fl.status, "CANCELLED");
    int ref = 0;
    for (int i = 0; i < bookingCount; i++)
        if (!strcmp(bookings[i].flightNo, fno.c_str()) && !strcmp(bookings[i].status, "CONFIRMED"))
        {
            strcpy(bookings[i].status, "CANCELLED");
            ref++;
        }
    saveFlights();
    saveBookings();
    char b[100];
    snprintf(b, 100, "Flight cancelled. %d bookings cancelled. Refunds initiated.", ref);
    success(b);
    waitEnter();
}

/* ── Admin user management ────────────────────────────────── */
void adminViewPassengers()
{
    printBanner();
    sectionHeader("ALL PASSENGERS", BYL);
    printf("\n  " BOLD CY "%-6s %-20s %-25s %-10s %-6s" RS "\n", "ID", "NAME", "EMAIL", "TIER", "MILES");
    printf("  " DIM "─────────────────────────────────────────────────────────────────────────\n" RS);
    for (int i = 0; i < passCount; i++)
    {
        Passenger &p = passengers[i];
        if (!p.active)
            continue;
        char name[80];
        snprintf(name, 80, "%s %s", p.firstName, p.lastName);
        printf("  " BWH "P%-5d" WH "%-20.19s " DIM "%-25.24s " RS "%s%-10s" RS WH "%-6d" RS "\n",
               p.id, name, p.email, tierCol(p.loyaltyTier), p.loyaltyTier, p.totalMiles);
    }
    printf("\n  " DIM "Total: %d passengers\n" RS, passCount);
    waitEnter();
}

void adminViewStaff()
{
    printBanner();
    sectionHeader("ALL STAFF MEMBERS", BYL);
    printf("\n  " BOLD CY "%-10s %-20s %-20s %-12s" RS "\n", "EMP ID", "NAME", "ROLE", "TERMINAL");
    printf("  " DIM "──────────────────────────────────────────────────────────────────\n" RS);
    for (int i = 0; i < staffCount; i++)
    {
        Staff &s = staffArr[i];
        if (!s.active)
            continue;
        char name[80];
        snprintf(name, 80, "%s %s", s.firstName, s.lastName);
        printf("  " BCY "%-10s" RS WH "%-20s %s%-20s" RS WH "%-12s" RS "\n",
               s.empId, name, SROLE_COL[s.roleCode], SROLES[s.roleCode], s.terminal);
    }
    printf("\n  " DIM "Total: %d staff\n" RS, staffCount);
    waitEnter();
}

void adminReports()
{
    printBanner();
    sectionHeader("REPORTS & ANALYTICS", BYL);
    printf("  " BCY "[1]" RS " Load Factor   " BCY "[2]" RS " Revenue   " BCY "[3]" RS " Passenger Stats   " BCY "[4]" RS " Booking Summary\n\n");
    int ch = iInt("Report:", 1, 4);
    if (ch == 1)
    {
        printBanner();
        sectionHeader("LOAD FACTOR REPORT", BYL);
        for (int i = 0; i < flightCount; i++)
        {
            Flight &fl = flights[i];
            if (!fl.active)
                continue;
            int tot = fl.seatsFirst + fl.seatsBiz + fl.seatsPrem + fl.seatsEco;
            int bkd = fl.bkFirst + fl.bkBiz + fl.bkPrem + fl.bkEco;
            int pct = tot > 0 ? bkd * 100 / tot : 0;
            const char *lc = (pct > 80) ? BRD : (pct > 60) ? BYL
                                                           : BGR;
            printf("  " BCY "%-8s" RS " %s→%s  %s%d%%" RS "\n", fl.flightNo, fl.origin, fl.dest, lc, pct);
        }
    }
    else if (ch == 2)
    {
        double total = 0;
        for (int i = 0; i < bookingCount; i++)
            if (!strcmp(bookings[i].status, "CONFIRMED"))
                total += bookings[i].grandTotal;
        printf("\n  " CY "Total Revenue (Confirmed): " RS BOLD BGR "$%.2f\n" RS, total);
        for (int i = 0; i < flightCount; i++)
        {
            double fr = 0;
            int cnt = 0;
            for (int j = 0; j < bookingCount; j++)
                if (!strcmp(bookings[j].flightNo, flights[i].flightNo) && !strcmp(bookings[j].status, "CONFIRMED"))
                {
                    fr += bookings[j].grandTotal;
                    cnt++;
                }
            if (cnt > 0)
                printf("  " BCY "%-8s" RS "  $%.2f  (%d bookings)\n", flights[i].flightNo, fr, cnt);
        }
    }
    else if (ch == 3)
    {
        int b = 0, s = 0, g = 0, pl = 0;
        for (int i = 0; i < passCount; i++)
        {
            string t2 = tier(passengers[i].totalMiles);
            if (t2 == "PLATINUM")
                pl++;
            else if (t2 == "GOLD")
                g++;
            else if (t2 == "SILVER")
                s++;
            else
                b++;
        }
        printf("\n  " YL "BRONZE   : %d\n" BWH "SILVER   : %d\n" BYL "GOLD     : %d\n" BCY "PLATINUM : %d\n" RS, b, s, g, pl);
        printf("  " DIM "Total    : %d\n" RS, passCount);
    }
    else if (ch == 4)
    {
        int conf = 0, canc = 0;
        for (int i = 0; i < bookingCount; i++)
        {
            if (!strcmp(bookings[i].status, "CONFIRMED"))
                conf++;
            else
                canc++;
        }
        printf("\n  " BGR "Confirmed : %d\n" BRD "Cancelled : %d\n" DIM "Total     : %d\n" RS, conf, canc, bookingCount);
    }
    waitEnter();
}

void adminSystemOverview()
{
    printBanner();
    sectionHeader("SYSTEM OVERVIEW", BYL);
    int on = 0, del = 0, can = 0, tot = 0, bkd = 0;
    double rev = 0;
    for (int i = 0; i < flightCount; i++)
    {
        if (!flights[i].active)
            continue;
        if (!strcmp(flights[i].status, "ON_TIME"))
            on++;
        else if (!strcmp(flights[i].status, "DELAYED"))
            del++;
        else if (!strcmp(flights[i].status, "CANCELLED"))
            can++;
        tot += flights[i].seatsFirst + flights[i].seatsBiz + flights[i].seatsPrem + flights[i].seatsEco;
        bkd += flights[i].bkFirst + flights[i].bkBiz + flights[i].bkPrem + flights[i].bkEco;
    }
    for (int i = 0; i < bookingCount; i++)
        if (!strcmp(bookings[i].status, "CONFIRMED"))
            rev += bookings[i].grandTotal;
    int lpct = tot > 0 ? bkd * 100 / tot : 0;
    printf("\n");
    boxTop(BYL);
    boxCen(BYL, BYL "  📊  NEXUS AIR SYSTEM OVERVIEW  " RS);
    boxSep(BYL);
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Flights     : " RS BWH "%d" RS "   " BGR "On-Time:%d" RS "   " BYL "Delayed:%d" RS "   " BRD "Cancelled:%d" RS, flightCount, on, del, can);
        boxRowC(BYL, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Passengers  : " RS BWH "%d" RS, passCount);
        boxRowC(BYL, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Staff       : " RS BWH "%d" RS, staffCount);
        boxRowC(BYL, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Bookings    : " RS BWH "%d" RS, bookingCount);
        boxRowC(BYL, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Baggage     : " RS BWH "%d bags" RS, bagCount);
        boxRowC(BYL, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Load Factor : " RS "%s" BWH "%d%%" RS, (lpct > 80 ? BRD : lpct > 60 ? BYL
                                                                                                       : BGR),
                 lpct);
        boxRowC(BYL, b);
    }
    {
        char b[200];
        snprintf(b, 200, "  " DIM "Revenue     : " RS BOLD BGR "$%.2f" RS, rev);
        boxRowC(BYL, b);
    }
    boxSep(BYL);
    boxRowC(BYL, "  " DIM "Files: nx_flights.txt  nx_passengers.txt  nx_staff.txt  nx_bookings.txt" RS);
    boxBot(BYL);
    waitEnter();
}

void adminViewBookings()
{
    printBanner();
    sectionHeader("ALL BOOKINGS", BYL);
    printf("\n  " BOLD CY "%-6s %-20s %-8s %-14s %-10s %-10s" RS "\n", "REF", "PASSENGER", "FLIGHT", "CABIN", "TOTAL", "STATUS");
    printf("  " DIM "─────────────────────────────────────────────────────────────────────────────\n" RS);
    for (int i = 0; i < bookingCount; i++)
    {
        Booking &b = bookings[i];
        const char *sc = (!strcmp(b.status, "CONFIRMED")) ? BGR : BRD;
        printf("  " BCY "%-6s" WH "%-20.19s " YL "%-8s " WH "%-14s " BGR "$%-9.2f" RS "%s%-10s" RS "\n",
               b.bookingRef, b.passengerName, b.flightNo, b.cabin, b.grandTotal, sc, b.status);
    }
    printf("\n  " DIM "Total: %d bookings\n" RS, bookingCount);
    waitEnter();
}

/* ─── Admin Dashboard ─────────────────────────────────────── */
void adminDash()
{
    for (;;)
    {
        Admin &a = admins[sess.idx];
        printBanner();
        printf("\n  " BG_YL "\033[30m" BOLD "  STATUS: OPERATIONAL  │  Flights:%d  │  Passengers:%d  │  Bookings:%d  " RS "\n\n",
               flightCount, passCount, bookingCount);
        boxTop(BYL);
        {
            char b[200];
            snprintf(b, 200, "  " BYL "👑  ADMIN DASHBOARD — %s %s" RS, a.firstName, a.lastName);
            boxRowC(BYL, b);
        }
        boxMid(BYL);
        boxRowC(BYL, CY "  ── ✈  FLIGHT MANAGEMENT ──────────────────────────────────" RS);
        boxRowC(BYL, "  " BYL "[1]" RS "  View All Flights");
        boxRowC(BYL, "  " BYL "[2]" RS "  Add New Flight");
        boxRowC(BYL, "  " BYL "[3]" RS "  Edit Flight");
        boxRowC(BYL, "  " BYL "[4]" RS "  Cancel Flight");
        boxSep(BYL);
        boxRowC(BYL, CY "  ── 👥  USER MANAGEMENT ─────────────────────────────────" RS);
        boxRowC(BYL, "  " BYL "[5]" RS "  View Passengers");
        boxRowC(BYL, "  " BYL "[6]" RS "  View Staff");
        boxSep(BYL);
        boxRowC(BYL, CY "  ── 📊  ANALYTICS & REPORTS ─────────────────────────────" RS);
        boxRowC(BYL, "  " BYL "[7]" RS "  Reports & Analytics");
        boxRowC(BYL, "  " BYL "[8]" RS "  View All Bookings");
        boxRowC(BYL, "  " BYL "[9]" RS "  System Overview");
        boxSep(BYL);
        boxRowC(BYL, "  " BRD "[0]" RS "  Logout");
        boxBot(BYL);
        int ch = iInt("Choice [0-9]:", 0, 9);
        switch (ch)
        {
        case 1:
            adminViewFlights();
            break;
        case 2:
            adminAddFlight();
            break;
        case 3:
            adminEditFlight();
            break;
        case 4:
            adminCancelFlight();
            break;
        case 5:
            adminViewPassengers();
            break;
        case 6:
            adminViewStaff();
            break;
        case 7:
            adminReports();
            break;
        case 8:
            adminViewBookings();
            break;
        case 9:
            adminSystemOverview();
            break;
        case 0:
            sess.loggedIn = 0;
            success("Admin session ended.");
            sleepMs(600);
            return;
        default:
            warnMsg("Invalid.");
            sleepMs(400);
        }
    }
}

/* ═══════════════════════════════════════════════════════════
   MAIN AUTH SCREEN
   ═══════════════════════════════════════════════════════════ */
void authScreen()
{
    sess.loggedIn = 0;
    for (;;)
    {
        printBanner();
        printf("\n");
        boxTop(BCY);
        boxCen(BCY, BYL "⚡ NEXUS AIR" RS "  " BCY "ACCESS PORTAL" RS);
        boxSep(BCY);
        boxRowC(BCY, "  " BCY "── 👤  PASSENGER ───────────────────────────────────────" RS);
        boxRowC(BCY, "  " BGR "[1]" RS "  🔑  Passenger Login");
        boxRowC(BCY, "  " BGR "[2]" RS "  📝  Passenger Sign Up");
        boxSep(BCY);
        boxRowC(BCY, "  " BCY "── 🛂  STAFF  (code: 123a) ──────────────────────────────" RS);
        boxRowC(BCY, "  " BMG "[3]" RS "  🔑  Staff Login");
        boxRowC(BCY, "  " BMG "[4]" RS "  📝  Staff Sign Up  " DIM "(code required)" RS);
        boxSep(BCY);
        boxRowC(BCY, "  " BCY "── 👑  ADMIN  (code + 2FA: 123a) ────────────────────────" RS);
        boxRowC(BCY, "  " BYL "[5]" RS "  🔑  Admin Login");
        boxRowC(BCY, "  " BYL "[6]" RS "  📝  Admin Sign Up");
        boxSep(BCY);
        boxRowC(BCY, "  " BCY "── 🌐  PUBLIC ──────────────────────────────────────────" RS);
        boxRowC(BCY, "  " BCY "[7]" RS "  🔍  Search Flights  (no login)");
        boxRowC(BCY, "  " BCY "[8]" RS "  📡  Flight Status   (no login)");
        boxSep(BCY);
        boxRowC(BCY, "  " BRD "[9]" RS "  🚪  Exit NEXUS AIR");
        boxBot(BCY);
        int ch = iInt("Choice [1-9]:", 1, 9);
        switch (ch)
        {
        case 1:
            if (loginPass())
                passengerDash();
            break;
        case 2:
            registerPass();
            break;
        case 3:
            if (staffLogin())
                staffDash();
            break;
        case 4:
            staffSignup();
            break;
        case 5:
            if (adminLogin())
                adminDash();
            break;
        case 6:
            adminSignup();
            break;
        case 7:
            searchFlights();
            break;
        case 8:
            flightStatus();
            break;
        case 9:
            exitAnim();
            exit(0);
            break;
        default:
            warnMsg("Invalid.");
            sleepMs(400);
        }
    }
}

/* ═══════════════════════════════════════════════════════════
   MAIN
   ═══════════════════════════════════════════════════════════ */
int main()
{
    system("chcp 65001");
    /* Enable ANSI on Windows 10+ */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    system("title NEXUS AIR v5.0 - Airline Management System");
    setbuf(stdout, NULL);
    srand((unsigned)time(NULL));

    /* Boot */
    bootSeq();
    aiLogs();
    progressBar("Loading NEXUS AIR Airline Management System", 1300);
    neuralPulse(8);
    flyingPlane();
    dataStream(4);
    printf("\n  " BYL "⚡ NEXUS AIR" RS "  loading saved data...\n");
    neuralPulse(6);
    loadAll();
    seedData();
    printf("\n  " BGR "✔  Data loaded:" RS "  Flights:" BWH "%d" RS "  Passengers:" BWH "%d" RS
           "  Staff:" BWH "%d" RS "  Bookings:" BWH "%d" RS "\n\n",
           flightCount, passCount, staffCount, bookingCount);
    waitEnter();

    for (;;)
    {
        authScreen();
        sess.loggedIn = 0;
    }
    return 0;
}
