#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <string>
#include <vector>
#include <optional>
#include <iostream>

namespace SmartScheduler {

constexpr int DAYS_IN_YEAR = 365;
constexpr int SLOTS_PER_DAY = 6;  // 6 slots of 30 mins = 3 study hours/day

// Calendar Date representation
struct Date {
    int month;
    int day;

    bool isValid() const;
    int toDayOfYear() const;
    static Date fromDayOfYear(int dayOfYear);
    std::string toString() const;
};

// Represents an assignment task to be scheduled
struct Assignment {
    std::string name;
    int startDay;  // 0-indexed day of year (inclusive)
    int endDay;    // 0-indexed day of year (inclusive)
    float hours;   // required workload in hours

    Assignment() : name(""), startDay(0), endDay(0), hours(0.0f) {}
    Assignment(std::string n, int s, int e, float h)
        : name(std::move(n)), startDay(s), endDay(e), hours(h) {}

    int requiredSlots() const {
        return static_cast<int>(hours * 2.0f + 0.5f);
    }
};

// Represents a day's schedule slots
class DaySchedule {
public:
    std::vector<std::string> slots;  // names of assignments in each slot

    int freeSlots() const {
        return std::max(0, SLOTS_PER_DAY - static_cast<int>(slots.size()));
    }

    bool isOverfilled() const {
        return static_cast<int>(slots.size()) > SLOTS_PER_DAY;
    }
};

enum class ScheduleStrategy {
    DirectFill,   // slicefill: round-robin over available slots
    MoveInsert,   // move later-deadline tasks out of window, then slicefill
    Suggestion,   // suggest alternative nearest window
    Failed
};

// Core Timetable Scheduler engine
class Timetable {
private:
    std::vector<DaySchedule> days;
    std::vector<Assignment> assignments;

public:
    Timetable();

    // Query methods
    int getFreeSlots(int startDay, int endDay) const;
    bool hasCapacity(const Assignment& asg) const;
    const std::vector<DaySchedule>& getDays() const { return days; }
    const std::vector<Assignment>& getAssignments() const { return assignments; }

    // Core Scheduling Algorithms
    bool slicefill(const Assignment& asg);
    bool moveinsert(const Assignment& asg);
    std::optional<std::pair<int, int>> findAlternativeWindow(const Assignment& asg) const;
    void forceOverfill(const Assignment& asg);

    // High-level dispatch
    ScheduleStrategy addAssignment(const Assignment& asg, bool allowOverfill = false);
    bool removeAssignment(const std::string& name);

    // Formatted outputs
    void displaySchedule(std::ostream& os = std::cout, int startDay = 0, int endDay = DAYS_IN_YEAR - 1) const;
    void printSummary(std::ostream& os = std::cout) const;
};

} // namespace SmartScheduler

#endif // SCHEDULER_HPP
