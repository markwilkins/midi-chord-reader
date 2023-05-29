#include <catch2/catch_test_macros.hpp>
#include "MidiStore.h"
#include <algorithm>
#include <random>
using namespace std;

TEST_CASE("midi store basics", "storage")
{
    MidiStore ms;
    REQUIRE(ms.hasData() == false);
    ms.setName("my new name");
    // Still no children - so hasData is false
    REQUIRE(ms.hasData() == false);
    juce::String name = ms.getName();
    REQUIRE(name == "my new name");
    ms.addNoteEventAtTime(50, 123, true);
    REQUIRE(ms.hasData() == true);
}

TEST_CASE("note storage basics", "storage")
{
    MidiStore ms;

    ms.addNoteEventAtTime(50, 123, true);
    std::vector<int> notes = ms.getNoteOnEventsAtTime(50);
    REQUIRE(notes.size() == 1);
    REQUIRE(notes[0] == 123);
    ms.addNoteEventAtTime(50, 15, true);
    notes = ms.getNoteOnEventsAtTime(50);
    REQUIRE(notes.size() == 2);
    // This verifies that the results are sorted
    REQUIRE(notes[0] == 15);
    REQUIRE(notes[1] == 123);

    // adding duplicate note should not add a new one
    ms.addNoteEventAtTime(50, 15, true);
    notes = ms.getNoteOnEventsAtTime(50);
    REQUIRE(notes.size() == 2);
    REQUIRE(notes[0] == 15);
    REQUIRE(notes[1] == 123);

    ms.clear();
    notes = ms.getNoteOnEventsAtTime(50);
    REQUIRE(notes.size() == 0);

}

TEST_CASE("test state change flag", "storage")
{
    MidiStore ms;
    vector<int> notes;
    vector<int> expected;

    // verify that the state change flag is obeyed
    ms.allowStateChange(false);
    ms.addNoteEventAtTime(50, 15, true);
    notes = ms.getAllNotesOnAtTime(0, 200);
    REQUIRE(notes.size() == 0);
    ms.allowStateChange(true);
    ms.addNoteEventAtTime(50, 15, true);
    notes = ms.getAllNotesOnAtTime(0, 200);
    REQUIRE(notes.size() == 1);

}

TEST_CASE("empty note slot", "storage")
{
    MidiStore ms;
    std::vector<int> notes = ms.getNoteOnEventsAtTime(3333);
    REQUIRE(notes.size() == 0);
}

TEST_CASE("notes off", "storage")
{
    MidiStore ms;

    ms.addNoteEventAtTime(50, 120, false);
    ms.addNoteEventAtTime(50, 127, false);
    std::vector<int> notes = ms.getNoteOnEventsAtTime(50);
    REQUIRE(notes.size() == 0);
}

TEST_CASE("notes on and off", "storage")
{
    MidiStore ms;

    // two note off events and one on
    ms.addNoteEventAtTime(50, 120, false);
    ms.addNoteEventAtTime(50, 123, true);
    ms.addNoteEventAtTime(50, 127, false);
    std::vector<int> notes = ms.getNoteOnEventsAtTime(50);
    REQUIRE(notes.size() == 1);
    REQUIRE(notes[0] == 123);
}

TEST_CASE("notes time case 2", "storage")
{
    MidiStore ms;
    std::vector<int> notes;

    // Corner case where the off is added out of order. This essentially
    // verifies that events are kept in order in the MidiStore
    ms.addNoteEventAtTime(15, 100, false);
    ms.addNoteEventAtTime(10, 100, true);
    ms.addNoteEventAtTime(20, 103, true);
    notes = ms.getAllNotesOnAtTime(0, 200);
    std::vector<int> expected = {103};
    REQUIRE(notes == expected);
}

TEST_CASE("notes on simple", "storage")
{
    MidiStore ms;

    std::vector<int> notes = {0};

    notes = ms.getAllNotesOnAtTime(0, 25);
    REQUIRE(notes.size() == 0);

    ms.addNoteEventAtTime(50, 120, true);
    notes = ms.getAllNotesOnAtTime(0, 25);
    REQUIRE(notes.size() == 0);

    notes = ms.getAllNotesOnAtTime(0, 50);
    REQUIRE(notes.size() == 1);
    REQUIRE(notes[0] == 120);
    notes = ms.getAllNotesOnAtTime(0, 51);
    REQUIRE(notes.size() == 1);
    REQUIRE(notes[0] == 120);
}

// This is a simple chord sequence that provides a more "end to end" test of the class
TEST_CASE("notes on random", "storage")
{
    struct event
    {
        int64 time;
        int note;
        bool onoff;
    };
    MidiStore ms;
    std::vector<event> events;
    std::vector<int> notes;

    events = {
        // C
        {10, 60, true},
        {15, 64, true},
        {20, 67, true},
        // {30, 60, false},
        {30, 64, false},
        {30, 67, false},
        // F
        {30, 60, true},
        {30, 65, true},
        {30, 69, true},
        // change it to Dm
        {50, 62, true},
        {50, 60, false}};

    // Just to make it more interesting, shuffle it before adding to ensure that
    // it is treated consistently. Note that the note off for 60 at time 20 is commented out
    // above. If that is left in and it is added AFTER the the "subsequent" on event, then
    // the MidiStore code treats it as "off" since last event wins. I'm not entirely sure
    // what happens in the ProcessAudio callback for situations like that. I'm not sure
    // it is actually possible in a midi track to have a note end and the same note start
    // at exactly the same time. If so ... then ... well I'm still not sure what the result is
    // supposed to be.
    std::shuffle(events.begin(), events.end(), std::random_device());

    for (std::vector<event>::iterator ev = events.begin(); ev != events.end(); ++ev)
        ms.addNoteEventAtTime(ev->time, ev->note, ev->onoff);

    notes = ms.getAllNotesOnAtTime(0, 15);
    std::vector<int> expected = {60, 64};
    REQUIRE(notes == expected);
    notes = ms.getAllNotesOnAtTime(0, 20);
    expected = {60, 64, 67};
    REQUIRE(notes == expected);
    notes = ms.getAllNotesOnAtTime(0, 30);
    expected = {60, 65, 69};
    REQUIRE(notes == expected);
    notes = ms.getAllNotesOnAtTime(0, 50);
    expected = {62, 65, 69};
    REQUIRE(notes == expected);

    notes = ms.getAllNotesOnAtTime(50, 50);
    expected = {62};
    REQUIRE(notes == expected);
}

TEST_CASE("get event times", "storage") 
{
    MidiStore ms;
    std::vector<int64> times;
    std::vector<int64> expected;

    times = ms.getEventTimes();
    REQUIRE(times.size() == 0);

    ms.addNoteEventAtTime(25, 1, true);
    ms.addNoteEventAtTime(25, 2, true);
    ms.setEventTimeSeconds(25, 2.34);
    times = ms.getEventTimes();
    expected = {25};
    REQUIRE(times == expected);
    double seconds = ms.getEventTimeInSeconds(24);
    REQUIRE(seconds == 0.0);
    seconds = ms.getEventTimeInSeconds(25);
    REQUIRE(seconds == 2.34);

    ms.addNoteEventAtTime(30, 2, false);
    ms.setEventTimeSeconds(30, 3.34);
    // This one should be a no-op since time 31 does not exist
    ms.setEventTimeSeconds(31, 3.35);
    ms.addNoteEventAtTime(38, 88, true);
    ms.addNoteEventAtTime(27, 42, true);
    times = ms.getEventTimes();
    expected = {25, 27, 30, 38};
    REQUIRE(times == expected);

    ms.clear();
    times = ms.getEventTimes();
    REQUIRE(times.size() == 0);
    ms.addNoteEventAtTime(57, 42, true);
    times = ms.getEventTimes();
    expected = {57};
    REQUIRE(times == expected);

}