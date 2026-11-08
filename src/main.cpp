#include "scheduler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace SmartScheduler;

void printMenu() {
    std::cout << "\n============================================\n"
              << "       Smart Assignment Scheduler CLI       \n"
              << "============================================\n"
              << " 1. Add Assignment\n"
              << " 2. Delete Assignment\n"
              << " 3. Display Schedule\n"
              << " 4. Display Summary\n"
              << " 0. Exit\n"
              << "Choice: ";
}

int main(int argc, char* argv[]) {
    Timetable table;
    std::istream* inputSource = &std::cin;
    std::ifstream fileStream;

    bool isScripted = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--script" || arg == "-s") {
            isScripted = true;
        } else if (arg == "--file" && i + 1 < argc) {
            fileStream.open(argv[++i]);
            if (!fileStream.is_open()) {
                std::cerr << "Error: Could not open file " << argv[i] << "\n";
                return 1;
            }
            inputSource = &fileStream;
            isScripted = true;
        }
    }

    int choice;
    while (true) {
        if (!isScripted) printMenu();
        if (!(*inputSource >> choice)) break;

        if (choice == 0) {
            if (!isScripted) std::cout << "Exiting Smart Scheduler. Goodbye!\n";
            break;
        } else if (choice == 1) {
            std::string name;
            int sm, sd, em, ed;
            float hours;

            if (!isScripted) std::cout << "Enter Assignment Name: ";
            *inputSource >> name;

            if (!isScripted) std::cout << "Enter Start Date (MM DD): ";
            *inputSource >> sm >> sd;

            if (!isScripted) std::cout << "Enter Deadline (MM DD): ";
            *inputSource >> em >> ed;

            if (!isScripted) std::cout << "Enter Workload Hours: ";
            *inputSource >> hours;

            Date start{sm, sd};
            Date end{em, ed};

            if (!start.isValid() || !end.isValid()) {
                std::cout << "[Error] Invalid calendar date provided.\n";
                continue;
            }
            if (start.toDayOfYear() > end.toDayOfYear()) {
                std::cout << "[Error] Start date cannot be after deadline.\n";
                continue;
            }
            if (hours <= 0) {
                std::cout << "[Error] Workload hours must be positive.\n";
                continue;
            }

            Assignment asg(name, start.toDayOfYear(), end.toDayOfYear(), hours);
            ScheduleStrategy strat = table.addAssignment(asg, false);

            if (strat == ScheduleStrategy::DirectFill) {
                std::cout << "[Success] Scheduled via Direct Round-Robin (slicefill).\n";
            } else if (strat == ScheduleStrategy::MoveInsert) {
                std::cout << "[Success] Conflict resolved: lower-priority tasks shifted, scheduled via moveinsert.\n";
            } else if (strat == ScheduleStrategy::Suggestion) {
                auto alt = table.findAlternativeWindow(asg);
                std::cout << "[Alert] Workload exceeds capacity in requested window.\n";
                if (alt.has_value()) {
                    Date altStart = Date::fromDayOfYear(alt->first);
                    Date altEnd = Date::fromDayOfYear(alt->second);
                    std::cout << "Proposed Alternative Window: " << altStart.toString()
                              << " to " << altEnd.toString() << "\n";
                }
                std::cout << "Options: 1) Accept alternative (if available), 2) Skip, 3) Force overfill\nChoice: ";
                int opt = 2;
                *inputSource >> opt;
                if (opt == 1 && alt.has_value()) {
                    asg.startDay = alt->first;
                    asg.endDay = alt->second;
                    table.addAssignment(asg, false);
                    std::cout << "[Success] Scheduled in alternative window.\n";
                } else if (opt == 3) {
                    table.forceOverfill(asg);
                    std::cout << "[Warning] Force scheduled with overtime/overfill.\n";
                } else {
                    std::cout << "[Info] Assignment skipped.\n";
                }
            }
        } else if (choice == 2) {
            std::string name;
            if (!isScripted) std::cout << "Enter Assignment Name to delete: ";
            *inputSource >> name;
            if (table.removeAssignment(name)) {
                std::cout << "[Success] Assignment '" << name << "' removed.\n";
            } else {
                std::cout << "[Error] Assignment not found.\n";
            }
        } else if (choice == 3) {
            table.displaySchedule(std::cout);
        } else if (choice == 4) {
            table.printSummary(std::cout);
        } else {
            std::cout << "[Error] Invalid option selected.\n";
        }
    }

    return 0;
}
