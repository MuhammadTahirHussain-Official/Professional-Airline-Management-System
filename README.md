# ✈ NEXUS AIR — Airline Management System v5.0

Welcome to **NEXUS AIR**: a full-featured, robust Airline Management System engineered in modern C++ (Object-Oriented Programming). Designed for real-world airline, airport, and airline staff simulation scenarios. Created by Muhammad Tahir Hussain.

---

## Table of Contents

- [About](#about)
- [Features](#features)
- [System Architecture](#system-architecture)
- [Object-Oriented Design & Concepts](#object-oriented-design--concepts)
- [File Structure](#file-structure)
- [Installation](#installation)
- [Usage](#usage)
- [Data Persistence](#data-persistence)
- [Extending the System](#extending-the-system)
- [Code Quality: Fixes & Best Practices](#code-quality-fixes--best-practices)
- [Screenshots](#screenshots)
- [Credits](#credits)
- [License](#license)

---

## About

NEXUS AIR is a comprehensive C++ terminal-based application that simulates airline management — including flight scheduling, bookings, staff/crew management, baggage tracking, payment/billing, check-in/boarding operations, and extensive OOP-based role handling. The system is written with SOLID object-oriented principles, proper exception handling, templates, composition, and polymorphic patterns for maximum code scalability and maintainability.

> **Project Status:** Production Stable · Last major OOP revision: v5.0

---

## Features

- **User Roles & Authentication**
  - Passenger, Staff (multiple roles), and Admin accounts
  - Secure password system with basic hashing
  - Role-based dashboard navigation and access control

- **Flight Management**
  - Add, view, edit, cancel, and search for flights (Admin)
  - Real-time status, delay reasons, and seat capacity tracking

- **Passenger & Booking System**
  - Flight search and booking by passengers
  - Interactive seat selection (with real-time visual maps)
  - Fare calculation and secure payment simulation (Card/PayPal/Bank/Miles)
  - Loyalty points and tiered customer rewards

- **Staff Operations & Polymorphic Panels**
  - Ten specialized staff roles (gate, check-in, baggage, security, lounge, etc.)
  - Each role receives a unique, role-specific panel for tasks
  - Efficient staff, on-duty, and shift management

- **Baggage Handling & Tracking**
  - Baggage check-in, tracking through multiple stages (registered → delivered)
  - Tag-based search, status update, and reporting lost/missing baggage

- **Comprehensive Data Management**
  - Template repositories for efficient in-memory data structuring and search
  - Data persistence (flights, staff, passengers, bookings, baggage, seats)

- **Admin Dashboard & Analytics**
  - Full user overviews, reporting, quick CRUD for flights, staff, and bookings
  - Overview of all real-time stats and summary dashboards

- **Advanced UI/UX**
  - Cross-platform color-coded ANSI terminal UI with animated banners, boxes, and progress bars
  - SFX feedback for all critical actions (using `Beep`)
  - Guided input prompts and error handling

- **Robust Error Handling**
  - Custom exception classes for authentication, capacity, validation, payments, and not found errors

---

## System Architecture

**Multi-file, large-scale C++ program:**

- **OOP Hierarchy:** Clear separation between interfaces and implementations
- **Role panels:** Polymorphic factory system for staff role-based UI
- **Repository pattern:** Templated repository for efficient CRUD
- **Singleton pattern:** Central `DataStore` class
- **Composition:** Booking contains FareBreakdown, Baggage is linked by booking ref, etc.
- **Smart separation:** User base → Passenger/Staff/Admin derived classes

### Key Class/Module Responsibilities

- `User` (abstract) – base for all passenger/staff/admin, authentication
- `Passenger`, `Staff`, `Admin` – specialized users, each with extra data and behaviors
- `Flight` – all flight details plus seat maps
- `Booking` – connects flight, passenger, fare breakdown, payment info
- `BaggageItem` – full baggage tracking and state machine
- `SeatMap` – interactive seat selection and visualization
- `Repository<T>` – smart container for indexed search/filter
- `Session`, `AuthController` – manages user login/session

See `class_airline.cpp` for the full class relationships and implementation details.

---

## Object-Oriented Design & Concepts

NEXUS AIR is a C++ OOP showcase, utilizing:

- **Abstraction:** via pure interfaces (`ISerializable`, `IDisplayable`, `IAuthenticatable`, `IRolePanel`)
- **Encapsulation:** clean data hiding with class private/protected fields
- **Inheritance:** User → [Passenger, Staff, Admin], RolePanel hierarchy
- **Polymorphism:** virtual methods and runtime dispatch (especially for staff dashboard panels)
- **Multiple Inheritance:** Staff panels implement shared base and interface
- **Templates:** Generic repository pattern for all key object sets
- **Composition:** Objects combine in a real-world model (e.g., Booking contains FareBreakdown, Baggage associated per booking)
- **Exception Hierarchy:** Custom exceptions for error handling (Auth, Validation, NotFound, Capacity, Payments)

**Code is fully modular** — each class is in its own file in a full project, but is presented in a single `.cpp` for demonstration.

---

## File Structure

```
[project-root]/
  class_airline.cpp         # Main application and all modules (single file version)
  nexus_impl[1-4].cpp       # (Optional in full) Module splits: utilities, OOP types, data, UI
  nx_flights.txt            # Persistent storage for flights
  nx_passengers.txt         # Persistent storage for passengers
  nx_staff.txt              # Persistent storage for staff
  nx_admins.txt             # Persistent storage for admins
  nx_bookings.txt           # Persistent storage for bookings
  nx_baggage.txt            # Persistent storage for baggage
  nx_seats.txt              # Seat maps persistence
  README.md                 # You are here!
  LICENSE                   # License file
```

> In classroom/portfolio use, all code may be kept in a single C++ file.

---

## Installation

> This project is for Windows platforms (uses `windows.h` and `Beep` API for sound; can be easily adapted for POSIX).

**Requirements:**

- C++17 or higher
- Windows OS and a modern terminal supporting ANSI colors (or use Git Bash / Windows Terminal)
- g++/MinGW, MSVC, or compatible compiler

**To Build:**

```sh
g++ class_airline.cpp -o NexusAir.exe -std=c++17 -Wall
```

Or, with MSVC:

```sh
cl /EHsc /std:c++17 class_airline.cpp
```

---

## Usage

After building, launch the application:

```
NexusAir.exe
```

On first run, the system will **seed demo flights and users** if data files are missing. 

**Dashboards:**

- **Passengers:** Book/search/view flights, select seats, track own bookings/baggage, manage profile, earn/redeem loyalty points.
- **Staff:** Each staff role gets an interactive panel (gate, check-in, baggage, etc.) with context-aware actions and visual feedback.
- **Admin:** Add/edit/remove flights, view passengers/staff/bookings, system analytics, issue reports.
  
**CLI Controls:**  
- Follow on-screen menus and prompts  
- Use color cues to spot info/errors/warnings (see legends throughout UI)

**Data files** will be created and persist on next application run.

---

## Data Persistence

- All data is stored in plain text files: flights (`nx_flights.txt`), users (`nx_passengers.txt`, `nx_staff.txt`, `nx_admins.txt`), seat maps, bookings, and baggage.
- Text files can be viewed and edited with any text editor.
- Data is **auto-loaded and auto-saved** with each operation. Consistency is enforced by robust file parsing and exception management.

---

## Extending the System

- **Add new staff roles:** Extend enum/list, implement a new `StaffPanelBase`-derived panel with custom logic, register it in the factory.
- **Support for additional payment types:** Extend `BillingEngine::payWizard()` — UI will update automatically.
- **Add analytic dashboards:** Insert custom reporting to AdminDashboard, using repository filter/template methods.
- **Port to cross-platform:** Replace Windows-specific code (`Beep`, `Sleep`, `system("cls")`) with portable equivalents.

---

## Code Quality: Fixes & Best Practices

- **ODR Safe:** All statics defined exactly once to fix multi-definition errors
- **Getter/Setter Consistency:** Only a single, unambiguous `getUsername() const` override (see [FIX-1] in code)
- **String vs string_view:** All user-readable text getter use `string_view` for speed/compatibility
- **Comprehensive Documentation:** Heavily commented source with design notes

---

## Screenshots

<img width="863" height="840" alt="image" src="https://github.com/user-attachments/assets/a2cd75db-a064-49ae-957f-9d3aa48f9e72" />
<img width="990" height="800" alt="image" src="https://github.com/user-attachments/assets/dca2d08a-d534-4c09-9ffa-379935a1495f" />
<img width="1011" height="911" alt="image" src="https://github.com/user-attachments/assets/ef6048e3-b41c-4c89-9bcd-ea1ccd170772" />



## Credits

**Author:**  
[Muhammad Tahir Hussain](https://github.com/MuhammadTahirHussain-Official) — Original Author, OOP C++ System Design

---

## License

[MIT License](LICENSE)

---

*For contributions, bug reports, and feature requests, please open an Issue or Submit a Pull Request!*
