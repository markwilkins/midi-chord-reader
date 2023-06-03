
#include <catch2/catch_test_macros.hpp>
#include "ChordView.h"
#include <algorithm>
using namespace std;


TEST_CASE("chord view empty", "chordview")
{
    MidiStore ms;
    ms.setQuantizationValue(1);
    ChordClipper cp(ms);
    map<float, string> display;

    display = cp.getChordsToDisplay();
    REQUIRE(display.size() == 0);
}


TEST_CASE("chord view window", "chordview")
{
    MidiStore ms;
    ms.setQuantizationValue(1);

    // Note - The relative event times and times in seconds are not relative. The ones that
    // matter here are the seconds. The integer event times just need to be incrementing
    // C
    ms.addNoteEventAtTime(50, 12, true);
    ms.setEventTimeSeconds(50, 5.0);
    ms.addNoteEventAtTime(60, 12, false);

    // F
    ms.addNoteEventAtTime(100, 17, true);
    ms.setEventTimeSeconds(100, 15.0);
    ms.addNoteEventAtTime(110, 17, false);

    // G
    ms.addNoteEventAtTime(200, 19, true);
    ms.setEventTimeSeconds(200, 30.0);
    ms.addNoteEventAtTime(210, 19, false);

    ChordClipper cp(ms);
    map<float, string> chords;
    map<float, string> expected;

    chords = cp.getChordsToDisplay();
    // The expected chords are offset by "current note position" ... the default is to have the currently
    // playing note at position 5 seconds ... so the position in the window will be time+5
    expected = {{10, "C"}, {20, "F"}};
    REQUIRE(chords == expected);

    // move the playhead forward 1 second
    ms.setIsPlaying(true);
    cp.updateCurrentPosition(1000);
    chords = cp.getChordsToDisplay();
    expected = {{9, "C"}, {19, "F"}};
    REQUIRE(chords == expected);

    // explicitly set the position (as if playback had set it to be "this is where we are now")
    ms.setLastEventTimeInSeconds(20.0);
    // and update the position ... it should ignore the "seconds since last update" since we have a new last seen value
    cp.updateCurrentPosition(1500);
    chords = cp.getChordsToDisplay();
    expected = {{0, "F"}, {15, "G"}};
    REQUIRE(chords == expected);

}