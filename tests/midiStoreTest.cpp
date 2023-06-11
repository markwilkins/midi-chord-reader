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

// Some basic testing associated with save/restore work
TEST_CASE("tree replacement", "storage")
{
    MidiStore ms;
    bool replaced;

    ms.setName("test 2");
    ms.setQuantizationValue(1);
    ms.addNoteEventAtTime(100, 12, true);
    juce::ValueTree vto = ms.getState();

    // do some swaparoo things and see if it works as expected 
    juce::ValueTree vtn("restored");
    vtn.setProperty("randokey", "randovalue", nullptr);

    // Should fail with no version info
    replaced = ms.replaceState(vtn);
    REQUIRE(replaced == false);
    // should fail with unknown version
    vtn.setProperty(ms.midiChordsVersionProp, ms.currentVersion + 1, nullptr);
    replaced = ms.replaceState(vtn);
    REQUIRE(replaced == false);

    vtn.setProperty(ms.midiChordsVersionProp, ms.currentVersion, nullptr);
    replaced = ms.replaceState(vtn);
    REQUIRE(replaced == true);
    vector<int> notes = ms.getNoteOnEventsAtTime(100);
    REQUIRE(notes.size() == 0);

    juce::ValueTree vtc = ms.getState();
    REQUIRE(vtc.getProperty("randokey") == "randovalue");

    replaced = ms.replaceState(vto);
    REQUIRE(replaced == true);
    notes = ms.getNoteOnEventsAtTime(100);
    REQUIRE(notes.size() == 1);
}

TEST_CASE("refresh from state", "storage")
{
    MidiStore ms;
    juce::ValueTree vtn("restored");
    bool replaced;

    ms.setQuantizationValue(1);
    vtn.setProperty(ms.midiChordsVersionProp, ms.currentVersion, nullptr);
    replaced = ms.replaceState(vtn);
    REQUIRE(replaced == true);

    ms.addNoteEventAtTime(100, 12, true);
    vector<int> notes = ms.getNoteOnEventsAtTime(100);
    REQUIRE(notes.size() == 1);

    vtn.setProperty(ms.allowRecordingProp, true, nullptr);
    replaced = ms.replaceState(vtn);
    REQUIRE(replaced == true);

    ms.addNoteEventAtTime(100, 14, true);
    notes = ms.getNoteOnEventsAtTime(100);
    REQUIRE(notes.size() == 2);

    vtn.setProperty(ms.allowRecordingProp, false, nullptr);
    replaced = ms.replaceState(vtn);
    REQUIRE(replaced == true);

    // This one shouldn't get added (the "newly loaded" state has that flag turned off)
    ms.addNoteEventAtTime(100, 16, true);
    notes = ms.getNoteOnEventsAtTime(100);
    // so still 2 
    REQUIRE(notes.size() == 2);
}

TEST_CASE("note storage basics", "storage")
{
    MidiStore ms;

    ms.setQuantizationValue(1);
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

    ms.setQuantizationValue(1);
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

TEST_CASE("test time width prop", "storage")
{
    MidiStore ms;
    float width;

    // default value
    width = ms.getTimeWidth();
    REQUIRE(width == 20.0);

    ms.setTimeWidth(35.0);
    width = ms.getTimeWidth();
    REQUIRE(width == 35.0);

    // Verify it does the range checking
    ms.setTimeWidth(135.0);
    width = ms.getTimeWidth();
    REQUIRE(width == 20.0);
    ms.setTimeWidth(0.0);
    width = ms.getTimeWidth();
    REQUIRE(width == 20.0);

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

    ms.setQuantizationValue(1);
    ms.addNoteEventAtTime(50, 120, false);
    ms.addNoteEventAtTime(50, 127, false);
    std::vector<int> notes = ms.getNoteOnEventsAtTime(50);
    REQUIRE(notes.size() == 0);
}

TEST_CASE("notes on and off", "storage")
{
    MidiStore ms;

    ms.setQuantizationValue(1);
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

    ms.setQuantizationValue(1);
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

    ms.setQuantizationValue(1);
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

TEST_CASE("quantization", "storage")
{
    MidiStore ms;
    std::vector<int> notes;
    std::vector<int> expected;
    ms.setQuantizationValue(1000);

    ms.addNoteEventAtTime(5123, 12, true);
    ms.addNoteEventAtTime(4811, 13, true);
    notes = ms.getAllNotesOnAtTime(0, 4900);
    expected = {12, 13};
    REQUIRE(notes == expected);
    notes = ms.getNoteOnEventsAtTime(5000);
    expected = {12, 13};
    REQUIRE(notes == expected);
    notes = ms.getNoteOnEventsAtTime(4500);
    REQUIRE(notes == expected);
    notes = ms.getNoteOnEventsAtTime(5499);
    REQUIRE(notes == expected);
    // round up - this ends up at 6000
    notes = ms.getNoteOnEventsAtTime(5500);
    REQUIRE(notes.size() == 0);
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

    ms.setQuantizationValue(1);
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

TEST_CASE("view window", "storage")
{
    struct event
    {
        int64 time;
        double seconds;
        int note;
        bool onoff;
    };
    MidiStore ms;
    std::vector<event> events;
    vector<pair<float, string>> chords;
    vector<pair<float, string>> expected;

    ms.setQuantizationValue(1);
    // the seconds vs event time values do not correlate to reality, but keeping them close here makes test understanding easier
    events = {
        // C
        {10, 10.1, 60, true},
        {15, 15.2, 64, true},
        {20, 20.3, 67, true},
        // {30, 30.4, 60, false},
        {30, 30.4, 64, false},
        {30, 30.4, 67, false},
        // F
        {30, 30.4, 60, true},
        {30, 30.4, 65, true},
        {30, 30.4, 69, true},
        // change it to Dm
        {50, 50.5, 62, true},
        {50, 50.5, 60, false}};

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
    {
        ms.addNoteEventAtTime(ev->time, ev->note, ev->onoff);
        ms.setEventTimeSeconds(ev->time, ev->seconds);
    }

    ms.updateStaticView();

    chords = ms.getChordsInWindow({7.1, 8.1});
    REQUIRE(chords.size() == 0);

    chords = ms.getChordsInWindow({10.1, 10.1});
    expected = {{10.1, "C"}};
    REQUIRE(chords == expected);

    // The chord at the end of the arpeggio should still be C ... only one total should be in the set
    chords = ms.getChordsInWindow({8.0, 20.3});
    expected = {{10.1, "C"}};
    REQUIRE(chords == expected);

    chords = ms.getChordsInWindow({29.0, 32.3});
    expected = {{30.4, "F/C"}};
    REQUIRE(chords == expected);

    chords = ms.getChordsInWindow({50.5, 51.3});
    expected = {{50.5, "Dmin"}};
    REQUIRE(chords == expected);

    chords = ms.getChordsInWindow({28.1, 51.3});
    expected = {{30.4, "F/C"}, {50.5, "Dmin"}};
    REQUIRE(chords == expected);
    chords = ms.getChordsInWindow({10.1, 51.3});
    expected = {{10.1, "C"}, {30.4, "F/C"}, {50.5, "Dmin"}};
    REQUIRE(chords == expected);

    chords = ms.getChordsInWindow({50.6, 100.1});
    REQUIRE(chords.size() == 0);

}

// verify that the view only gets updated "as needed" and not terribly often
TEST_CASE("view update", "storage")
{
    MidiStore ms;
    vector<pair<float, string>> chords;
    vector<pair<float, string>> expected;
    ms.setQuantizationValue(1);

    ms.addNoteEventAtTime(1000, 12, true); 
    ms.setEventTimeSeconds(1000, 10.0);
    // This should always update since the initial value of "last update" is 0
    ms.updateStaticViewIfOutOfDate();
    chords = ms.getChordsInWindow({10.0, 10.0});
    expected = {{10.0, "C"}};
    REQUIRE(chords == expected);

    ms.addNoteEventAtTime(1000, 15, true); 
    // Make sure the "last update" is in the future so this should not update anything
    int64 curTime = juce::Time::currentTimeMillis();
    // If you step through this in the debugger and take longer than 30 seconds, this will fail
    ms.setLastViewUpdateTime(curTime + 30000);
    ms.updateStaticViewIfOutOfDate();
    chords = ms.getChordsInWindow({10.0, 10.0});
    expected = {{10.0, "C"}};
    REQUIRE(chords == expected);

    // make the last update look like it was at least 1 second in the past
    ms.setLastViewUpdateTime(curTime - 1001);
    ms.updateStaticViewIfOutOfDate();
    chords = ms.getChordsInWindow({10.0, 10.0});
    expected = {{10.0, "Cmin"}};
    REQUIRE(chords == expected);
}

// Helper function for setting up a series of notes. This uses given floating point time
// in seconds and computes (arbitrarily) the int64 time in milliseconds
static void addNote(MidiStore &ms, double time, double duration, int note)
{
    int64 onTime = static_cast<int64>(time * 1000);
    int64 offTime = static_cast<int64>((time + duration) * 1000);
    ms.addNoteEventAtTime(onTime, note, true);
    ms.setEventTimeSeconds(onTime, static_cast<float>(time));
    ms.addNoteEventAtTime(offTime, note, false);
    ms.setEventTimeSeconds(offTime, static_cast<float>(time + duration));
    ms.updateStaticView();
}


TEST_CASE("short chord removal basic", "storage")
{
    MidiStore ms;
    vector<pair<float, string>> chords;
    vector<pair<float, string>> expected;
    ms.setQuantizationValue(1);

    ms.setShortChordThreshold(0.0);

    addNote(ms, 1.0, 1.0, 12);
    addNote(ms, 2.0, 1.0, 17);
    chords = ms.getChordsInWindow({1.0, 10.0});
    expected = {{1.0, "C"}, {2.0, "F"}};
    REQUIRE(chords == expected);

    // add a slight blip of a chord at the end of the C
    addNote(ms, 1.9, 0.1, 21);
    chords = ms.getChordsInWindow({1.0, 10.0});
    expected = {{1.0, "C"}, {1.9, "C6"}, {2.0, "F"}};
    REQUIRE(chords == expected);

    ms.setShortChordThreshold(0.25);
    ms.updateStaticViewIfOutOfDate();
    chords = ms.getChordsInWindow({1.0, 10.0});
    expected = {{1.0, "C"}, {2.0, "F"}};
    REQUIRE(chords == expected);
}

// verify that removal of chord between two chords with same name combines them
TEST_CASE("short chord removal connect", "storage")
{
    MidiStore ms;
    vector<pair<float, string>> chords;
    vector<pair<float, string>> expected;
    ms.setQuantizationValue(1);

    addNote(ms, 1.0, 1.0, 12);
    addNote(ms, 2.0, 1.0, 12);
    chords = ms.getChordsInWindow({1.0, 10.0});
    expected = {{1.0, "C"}};
    REQUIRE(chords == expected);

    // Add a short chord in the middle
    addNote(ms, 1.9, 0.1, 21);
    chords = ms.getChordsInWindow({1.0, 10.0});
    expected = {{1.0, "C"}};
    REQUIRE(chords == expected);
}

// Test case to verify that short last chord behaves okay
TEST_CASE("short chord removal last", "storage")
{
    MidiStore ms;
    vector<pair<float, string>> chords;
    vector<pair<float, string>> expected;
    ms.setQuantizationValue(1);

    addNote(ms, 1.0, 1.0, 12);
    addNote(ms, 2.0, 1.0, 17);
    addNote(ms, 2.9, 0.1, 12);
    addNote(ms, 2.9, 0.1, 21);
    chords = ms.getChordsInWindow({1.0, 10.0});
    expected = {{1.0, "C"}, {2.0, "F"}, {2.9, "F/C"}};
    REQUIRE(chords == expected);
}

// test first short chord
TEST_CASE("short chord removal first", "storage")
{
    MidiStore ms;
    vector<pair<float, string>> chords;
    vector<pair<float, string>> expected;
    ms.setQuantizationValue(1);

    addNote(ms, 1.0, 0.1, 12);
    addNote(ms, 1.1, 0.9, 17);
    chords = ms.getChordsInWindow({1.0, 10.0});
    expected = {{1.0, "C"}, {1.1, "F"}};
    REQUIRE(chords == expected);
}


TEST_CASE("get event times", "storage") 
{
    MidiStore ms;
    std::vector<int64> times;
    std::vector<int64> expected;
    ms.setQuantizationValue(10);

    times = ms.getEventTimes();
    REQUIRE(times.size() == 0);

    ms.addNoteEventAtTime(248, 1, true);
    ms.addNoteEventAtTime(252, 2, true);
    ms.setEventTimeSeconds(251, 2.34);
    times = ms.getEventTimes();
    expected = {250};
    REQUIRE(times == expected);
    double seconds = ms.getEventTimeInSeconds(240);
    REQUIRE(seconds == 0.0);
    seconds = ms.getEventTimeInSeconds(251);
    REQUIRE(seconds == 2.34);

    ms.addNoteEventAtTime(301, 2, false);
    ms.setEventTimeSeconds(299, 3.34);
    // This one should be a no-op since time 31 does not exist
    ms.setEventTimeSeconds(310, 3.35);
    ms.addNoteEventAtTime(381, 88, true);
    ms.addNoteEventAtTime(270, 42, true);
    times = ms.getEventTimes();
    expected = {250, 270, 300, 380};
    REQUIRE(times == expected);

    ms.clear();
    times = ms.getEventTimes();
    REQUIRE(times.size() == 0);
    ms.addNoteEventAtTime(570, 42, true);
    times = ms.getEventTimes();
    expected = {570};
    REQUIRE(times == expected);

}


// Test temp stuff ... figuring out how sorted vector of pairs works
TEST_CASE("tmp", "storage")
{
    vector<pair<float, string>> v;
    pair<float, string> p;
    v.push_back({4.5, "a"});
    v.push_back({3.3, "b"});
    v.push_back({2.3, "c"});
    v.push_back({1.3, "d"});
    std::sort(v.begin(), v.end());
    for (auto i: v) 
    {
        DBG("val: " + to_string(i.first) + " " + i.second);
    }

    p = {2.5, ""};
    auto it = std::lower_bound(
        v.begin(),
        v.end(),
        p);

    for (; it != v.end(); ++it)
    {
        DBG("val: " + to_string(it->first) + " " + it->second);
    }

    juce::ValueTree v1("one");
    juce::ValueTree v2("two");
    v1.setProperty("p1", "v1value", nullptr);
    v2 = v1.createCopy();
    v2.setProperty("p1", "v2value", nullptr);
    REQUIRE(v2.getProperty("p1") == "v2value");
    REQUIRE(v1.getProperty("p1") == "v1value");
}