#include "scheduler.hpp"
#include <iomanip>
#include <algorithm>
#include <sstream>

namespace SmartScheduler {

constexpr int DAYS_IN_MONTH[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

bool Date::isValid() const {
    if (month < 1 || month > 12) return false;
    return (day >= 1 && day <= DAYS_IN_MONTH[month - 1]);
}

int Date::toDayOfYear() const {
    int idx = 0;
    for (int i = 0; i < month - 1; ++i) {
        idx += DAYS_IN_MONTH[i];
    }
    return idx + day - 1;
}

Date Date::fromDayOfYear(int dayOfYear) {
    int month = 0;
    int remaining = dayOfYear;
    while (month < 12 && remaining >= DAYS_IN_MONTH[month]) {
        remaining -= DAYS_IN_MONTH[month];
        month++;
    }
    return Date{month + 1, remaining + 1};
}

std::string Date::toString() const {
    std::ostringstream oss;
    oss << (month < 10 ? "0" : "") << month << "/"
        << (day < 10 ? "0" : "") << day;
    return oss.str();
}

Timetable::Timetable() : days(DAYS_IN_YEAR) {}

int Timetable::getFreeSlots(int startDay, int endDay) const {
    int freeCount = 0;
    for (int i = std::max(0, startDay); i <= std::min(DAYS_IN_YEAR - 1, endDay); ++i) {
        freeCount += days[i].freeSlots();
    }
    return freeCount;
}

bool Timetable::hasCapacity(const Assignment& asg) const {
    return getFreeSlots(asg.startDay, asg.endDay) >= asg.requiredSlots();
}

bool Timetable::slicefill(const Assignment& asg) {
    if (!hasCapacity(asg)) {
        return false;
    }

    int remainingSlots = asg.requiredSlots();
    int windowDays = asg.endDay - asg.startDay + 1;
    if (windowDays <= 0) return false;

    // Distribute slots round-robin across days with free slots
    while (remainingSlots > 0) {
        bool placedAny = false;
        for (int i = asg.startDay; i <= asg.endDay && remainingSlots > 0; ++i) {
            if (days[i].freeSlots() > 0) {
                days[i].slots.push_back(asg.name);
                remainingSlots--;
                placedAny = true;
            }
        }
        if (!placedAny) break; // Should not happen if hasCapacity is true
    }

    assignments.push_back(asg);
    return true;
}

bool Timetable::moveinsert(const Assignment& asg) {
    // Make a working copy for rollback if this strategy fails
    std::vector<DaySchedule> backupDays = days;
    std::vector<Assignment> backupAsgs = assignments;

    int slotsNeeded = asg.requiredSlots();
    int currentFree = getFreeSlots(asg.startDay, asg.endDay);
    int slotsToFree = slotsNeeded - currentFree;

    if (slotsToFree <= 0) {
        return slicefill(asg);
    }

    // Identify candidate assignments with deadline strictly greater than asg.endDay
    // and that currently occupy slots inside [asg.startDay, asg.endDay]
    for (auto& other : assignments) {
        if (other.endDay <= asg.endDay) continue;

        // Count how many slots 'other' occupies in [asg.startDay, asg.endDay]
        // and how many free slots exist in (asg.endDay, other.endDay]
        int movableCount = 0;
        for (int d = asg.startDay; d <= asg.endDay; ++d) {
            for (const auto& slotOwner : days[d].slots) {
                if (slotOwner == other.name) movableCount++;
            }
        }

        int availableLater = getFreeSlots(asg.endDay + 1, other.endDay);
        int shiftAmount = std::min({movableCount, availableLater, slotsToFree});

        if (shiftAmount > 0) {
            // Remove 'shiftAmount' slots of 'other' from [asg.startDay, asg.endDay]
            int removed = 0;
            for (int d = asg.endDay; d >= asg.startDay && removed < shiftAmount; --d) {
                auto it = std::find(days[d].slots.begin(), days[d].slots.end(), other.name);
                if (it != days[d].slots.end()) {
                    days[d].slots.erase(it);
                    removed++;
                }
            }

            // Re-allocate 'shiftAmount' slots into (asg.endDay, other.endDay]
            int placed = 0;
            for (int d = asg.endDay + 1; d <= other.endDay && placed < shiftAmount; ++d) {
                while (days[d].freeSlots() > 0 && placed < shiftAmount) {
                    days[d].slots.push_back(other.name);
                    placed++;
                }
            }

            slotsToFree -= shiftAmount;
            if (slotsToFree <= 0) break;
        }
    }

    if (slotsToFree <= 0 && slicefill(asg)) {
        return true;
    }

    // Rollback if unable to satisfy constraints
    days = backupDays;
    assignments = backupAsgs;
    return false;
}

std::optional<std::pair<int, int>> Timetable::findAlternativeWindow(const Assignment& asg) const {
    int duration = asg.endDay - asg.startDay;
    int needed = asg.requiredSlots();

    for (int s = asg.startDay + 1; s + duration < DAYS_IN_YEAR; ++s) {
        int e = s + duration;
        if (getFreeSlots(s, e) >= needed) {
            return std::make_pair(s, e);
        }
    }
    return std::nullopt;
}

void Timetable::forceOverfill(const Assignment& asg) {
    int remaining = asg.requiredSlots();
    int window = asg.endDay - asg.startDay + 1;
    while (remaining > 0) {
        for (int i = asg.startDay; i <= asg.endDay && remaining > 0; ++i) {
            days[i].slots.push_back(asg.name);
            remaining--;
        }
    }
    assignments.push_back(asg);
}

ScheduleStrategy Timetable::addAssignment(const Assignment& asg, bool allowOverfill) {
    if (slicefill(asg)) {
        return ScheduleStrategy::DirectFill;
    }
    if (moveinsert(asg)) {
        return ScheduleStrategy::MoveInsert;
    }
    if (allowOverfill) {
        forceOverfill(asg);
        return ScheduleStrategy::DirectFill;
    }
    return ScheduleStrategy::Suggestion;
}

bool Timetable::removeAssignment(const std::string& name) {
    auto it = std::find_if(assignments.begin(), assignments.end(),
                           [&name](const Assignment& a) { return a.name == name; });
    if (it == assignments.end()) {
        return false;
    }

    assignments.erase(it);
    for (auto& day : days) {
        day.slots.erase(std::remove(day.slots.begin(), day.slots.end(), name),
                        day.slots.end());
    }
    return true;
}

void Timetable::displaySchedule(std::ostream& os, int startDay, int endDay) const {
    startDay = std::max(0, startDay);
    endDay = std::min(DAYS_IN_YEAR - 1, endDay);

    os << "\n| Date  | Slot 1   | Slot 2   | Slot 3   | Slot 4   | Slot 5   | Slot 6   | Hours | Notes\n";
    os << "|-------|----------|----------|----------|----------|----------|----------|-------|-------\n";

    for (int d = startDay; d <= endDay; ++d) {
        if (days[d].slots.empty()) continue; // skip entirely empty days for readability

        Date dt = Date::fromDayOfYear(d);
        os << "| " << dt.toString() << " | ";

        for (int s = 0; s < SLOTS_PER_DAY; ++s) {
            if (s < static_cast<int>(days[d].slots.size())) {
                std::string label = days[d].slots[s];
                if (label.length() > 8) label = label.substr(0, 8);
                os << std::setw(8) << std::left << label << " | ";
            } else {
                os << "   -     | ";
            }
        }

        float hours = days[d].slots.size() * 0.5f;
        os << std::setw(5) << std::fixed << std::setprecision(1) << hours << " | ";

        if (days[d].isOverfilled()) {
            os << "OVERLOAD (" << days[d].slots.size() - SLOTS_PER_DAY << " extra slots)";
        }
        os << "\n";
    }
}

void Timetable::printSummary(std::ostream& os) const {
    os << "Total Assignments Scheduled: " << assignments.size() << "\n";
    float totalHours = 0.0f;
    for (const auto& a : assignments) {
        Date s = Date::fromDayOfYear(a.startDay);
        Date e = Date::fromDayOfYear(a.endDay);
        os << " - " << std::setw(15) << std::left << a.name << ": "
           << s.toString() << " to " << e.toString()
           << " (" << a.hours << " hrs / " << a.requiredSlots() << " slots)\n";
        totalHours += a.hours;
    }
    os << "Total Workload: " << totalHours << " hours across " << totalHours * 2 << " slots.\n";
}

} // namespace SmartScheduler
