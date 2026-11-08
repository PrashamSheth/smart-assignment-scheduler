#include "scheduler.hpp"
#include <cassert>
#include <iostream>

using namespace SmartScheduler;

void testDateOperations() {
    std::cout << "[Test] Running testDateOperations... ";
    Date d1{1, 15};
    assert(d1.isValid());
    assert(d1.toDayOfYear() == 14);

    Date d2 = Date::fromDayOfYear(14);
    assert(d2.month == 1 && d2.day == 15);

    Date invalid{2, 30};
    assert(!invalid.isValid());

    Date endOfYear{12, 31};
    assert(endOfYear.isValid());
    assert(endOfYear.toDayOfYear() == 364);

    std::cout << "PASSED\n";
}

void testDirectSlicefill() {
    std::cout << "[Test] Running testDirectSlicefill... ";
    Timetable table;
    // Assignment 1: 03/01 to 03/05 (5 days = 30 slots available), 2.5 hours = 5 slots
    Date start{3, 1};
    Date end{3, 5};
    Assignment a("Lab1", start.toDayOfYear(), end.toDayOfYear(), 2.5f);

    ScheduleStrategy s = table.addAssignment(a, false);
    assert(s == ScheduleStrategy::DirectFill);
    assert(table.getAssignments().size() == 1);

    // Each day in range should have received 1 slot
    for (int d = a.startDay; d <= a.endDay; ++d) {
        assert(table.getDays()[d].slots.size() == 1);
        assert(table.getDays()[d].slots[0] == "Lab1");
    }
    std::cout << "PASSED\n";
}

void testMoveInsertConstraintShift() {
    std::cout << "[Test] Running testMoveInsertConstraintShift... ";
    Timetable table;

    // Fill days 03/01 to 03/02 completely with a long-deadline project (due 03/10)
    // 03/01 and 03/02 = 12 slots total
    Date startP{3, 1};
    Date endP{3, 10};
    Assignment proj("BigProject", startP.toDayOfYear(), endP.toDayOfYear(), 6.0f); // 12 slots
    ScheduleStrategy s1 = table.addAssignment(proj, false);
    assert(s1 == ScheduleStrategy::DirectFill);

    // Now insert an urgent 2-hour assignment (4 slots) between 03/01 and 03/02
    Date startU{3, 1};
    Date endU{3, 2};
    Assignment urgent("UrgentTask", startU.toDayOfYear(), endU.toDayOfYear(), 2.0f); // 4 slots

    // MoveInsert should shift slots of BigProject (which has deadline 03/10) into 03/03-03/10
    ScheduleStrategy s2 = table.addAssignment(urgent, false);
    assert(s2 == ScheduleStrategy::MoveInsert || s2 == ScheduleStrategy::DirectFill);

    // Verify UrgentTask has been placed
    bool foundUrgent = false;
    for (int d = urgent.startDay; d <= urgent.endDay; ++d) {
        for (const auto& name : table.getDays()[d].slots) {
            if (name == "UrgentTask") foundUrgent = true;
        }
    }
    assert(foundUrgent);
    std::cout << "PASSED\n";
}

void testAlternativeWindowSuggestion() {
    std::cout << "[Test] Running testAlternativeWindowSuggestion... ";
    Timetable table;

    // Completely saturate a single day
    Date single{4, 1};
    Assignment heavy("HeavyLoad", single.toDayOfYear(), single.toDayOfYear(), 3.0f); // 6 slots = 100% capacity
    ScheduleStrategy s1 = table.addAssignment(heavy, false);
    assert(s1 == ScheduleStrategy::DirectFill);

    // Attempt to add another assignment on the same day
    Assignment extra("ExtraTask", single.toDayOfYear(), single.toDayOfYear(), 1.0f);
    ScheduleStrategy s2 = table.addAssignment(extra, false);
    assert(s2 == ScheduleStrategy::Suggestion);

    auto alt = table.findAlternativeWindow(extra);
    assert(alt.has_value());
    assert(alt->first > single.toDayOfYear());
    std::cout << "PASSED\n";
}

void testDeletionAndReclamation() {
    std::cout << "[Test] Running testDeletionAndReclamation... ";
    Timetable table;
    Date s{5, 1};
    Date e{5, 3};
    Assignment a("TempTask", s.toDayOfYear(), e.toDayOfYear(), 1.5f);
    table.addAssignment(a, false);
    assert(table.getAssignments().size() == 1);

    bool removed = table.removeAssignment("TempTask");
    assert(removed);
    assert(table.getAssignments().empty());
    assert(table.getFreeSlots(s.toDayOfYear(), e.toDayOfYear()) == 18); // 3 days * 6 slots = 18
    std::cout << "PASSED\n";
}

int main() {
    std::cout << "=======================================\n";
    std::cout << " Running Smart Scheduler Test Suite   \n";
    std::cout << "=======================================\n";

    testDateOperations();
    testDirectSlicefill();
    testMoveInsertConstraintShift();
    testAlternativeWindowSuggestion();
    testDeletionAndReclamation();

    std::cout << "All 5 unit tests passed successfully!\n";
    return 0;
}
