
#include <catch2/catch_test_macros.hpp>
#include "ChordView.h"
#include <algorithm>
using namespace std;

// running in debugger produces errors for this test ... and ... I really don't have time to understand
// what is going on.
// #define JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

TEST_CASE("chord view empty", "chordview")
{
    MidiStore ms;
    ChordView cv(ms);
    map<float, string> display;

    display = cv.getChordsToDisplay();
    REQUIRE(display.size() == 0);
}


TEST_CASE("chord view window", "chordview")
{
    MidiStore ms;

    // C
    ms.addNoteEventAtTime(50, 12, true);
    ms.setEventTimeSeconds(50, 5.0);
    ms.addNoteEventAtTime(60, 12, false);

    // F
    ms.addNoteEventAtTime(100, 17, true);
    ms.setEventTimeSeconds(100, 10.0);
    ms.addNoteEventAtTime(110, 17, false);

    // G
    ms.addNoteEventAtTime(300, 19, true);
    ms.setEventTimeSeconds(300, 30.0);
    ms.addNoteEventAtTime(310, 19, false);

    ChordView cv(ms);
    map<float, string> chords;

    chords = cv.getChordsToDisplay();

}