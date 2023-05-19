#include <catch2/catch_test_macros.hpp>
#include "MidiStore.h"


TEST_CASE( "midi store basics") {
    MidiStore ms;
    REQUIRE(ms.hasData() == false);
    ms.setName("my new name");
    // Still no children - so hasData is false
    REQUIRE(ms.hasData() == false);
    juce::String name = ms.getName();
    REQUIRE(name == "my new name");

}

TEST_CASE("note storage basics", "storage")
{
    MidiStore ms;

    ms.addNoteEventAtTime(50, 123, true);
    Array<int> notes = ms.getNoteOnEventsAtTime(50);
    REQUIRE(notes.size() == 1);
    REQUIRE(notes[0] == 123);
    ms.addNoteEventAtTime(50, 15, true);
    notes = ms.getNoteOnEventsAtTime(50);
    REQUIRE(notes.size() == 2);
    REQUIRE(notes[0] == 15);
    REQUIRE(notes[1] == 123);

    // adding duplicate note should not add a new one
    ms.addNoteEventAtTime(50, 15, true);
    REQUIRE(notes.size() == 2);
    REQUIRE(notes[0] == 15);
    REQUIRE(notes[1] == 123);
}

TEST_CASE("empty note slot", "storage") 
{
    MidiStore ms;
    Array<int> notes = ms.getNoteOnEventsAtTime(3333);
    REQUIRE(notes.size() == 0);
}

TEST_CASE("notes off", "storage") 
{
    MidiStore ms;

    ms.addNoteEventAtTime(50, 120, false);
    ms.addNoteEventAtTime(50, 127, false);
    Array<int> notes = ms.getNoteOnEventsAtTime(50);
    REQUIRE(notes.size() == 0);
}

TEST_CASE("notes on and off", "storage") 
{
    MidiStore ms;

    // two note off events and one on
    ms.addNoteEventAtTime(50, 120, false);
    ms.addNoteEventAtTime(50, 123, true);
    ms.addNoteEventAtTime(50, 127, false);
    Array<int> notes = ms.getNoteOnEventsAtTime(50);
    REQUIRE(notes.size() == 1);
    REQUIRE(notes[0] == 123);
}