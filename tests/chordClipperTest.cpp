
#include <catch2/catch_test_macros.hpp>
#include "ChordView.h"
#include <algorithm>
using namespace std;


TEST_CASE("chord view empty", "chordview")
{
    MidiStore ms;
    ms.setQuantizationValue(1);
    ChordClipper cp(ms);
    vector<pair<float, string>> display;

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
    ms.setEventTimeSeconds(60, 5.5);

    // F
    ms.addNoteEventAtTime(100, 17, true);
    ms.setEventTimeSeconds(100, 15.0);
    ms.addNoteEventAtTime(110, 17, false);
    ms.setEventTimeSeconds(110, 15.5);

    // G
    ms.addNoteEventAtTime(200, 19, true);
    ms.setEventTimeSeconds(200, 30.0);
    ms.addNoteEventAtTime(210, 19, false);
    ms.setEventTimeSeconds(210, 32.0);

    ms.updateStaticViewIfOutOfDate();

    ChordClipper cp(ms);
    vector<pair<float, string>> chords;
    vector<pair<float, string>> expected;

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

// verify scrolling forward
TEST_CASE("chord view forward", "chordview")
{
    MidiStore ms;
    ChordClipper cp(ms);
    ms.setQuantizationValue(1);
    ChordVectorType chords;
    ChordVectorType expected;

    ms.setTimeWidth(3.0);
    // Set the "now" position at the left side of the view window (math is easier 
    // for writing this test)
    ms.setPlayHeadPosition(0.0);

    addNote(ms, 1.0, 1.0, 12);  // C
    addNote(ms, 5.0, 1.0, 17);  // F
    addNote(ms, 9.0, 1.0, 19);  // G
    addNote(ms, 13.0, 1.0, 16);  // E
    addNote(ms, 17.0, 1.0, 20);  // Ab

    ms.setLastEventTimeInSeconds(1.0);
    cp.updateCurrentPosition(0);
    chords = cp.getChordsToDisplay();
    expected = {{0, "C"}, {4, "F"}};
    REQUIRE(chords == expected);

    ms.setLastEventTimeInSeconds(3.0);
    cp.updateCurrentPosition(0);
    chords = cp.getChordsToDisplay();
    expected = {{-2, "C"}, {2, "F"}};
    REQUIRE(chords == expected);

    ms.setLastEventTimeInSeconds(4.0);
    cp.updateCurrentPosition(0);
    chords = cp.getChordsToDisplay();
    expected = {{1, "F"}, {5, "G"}};
    REQUIRE(chords == expected);

    ms.setLastEventTimeInSeconds(12.0);
    cp.updateCurrentPosition(0);
    chords = cp.getChordsToDisplay();
    expected = {{1, "E"}, {5, "Ab"}};
    REQUIRE(chords == expected);

    ms.setLastEventTimeInSeconds(8.0);
    cp.updateCurrentPosition(0);
    chords = cp.getChordsToDisplay();
    expected = {{1, "G"}, {5, "E"}};
    REQUIRE(chords == expected);
}

// Utility to add measure numbers to measure position vector
static MeasurePositionType addMeasureNumbers(int start, vector<float> measures)
{
    MeasurePositionType m;

    for (vector<float>::iterator it = measures.begin(); it != measures.end(); ++it)
    {
        m.push_back({start, *it});
        start++;
    }
    return m;
}

TEST_CASE("measure bars", "chordview")
{
    MidiStore ms;
    ChordClipper cp(ms);
    MeasurePositionType bars;
    MeasurePositionType expected;
    ms.setQuantizationValue(1);

    ms.setTimeWidth(20.0);
    ms.setPlayHeadPosition(50.0);


    // verify that if values are not set, it is empty
    bars = cp.getMeasuresToDisplay();
    expected = {};
    REQUIRE(bars == expected);

    ms.setBPMeasure(4);
    ms.setBPMinute(60.0);

    ms.setLastEventTimeInSeconds(24.0);
    cp.updateCurrentPosition(0);
    bars = cp.getMeasuresToDisplay();
    expected = addMeasureNumbers(5, {2, 6, 10, 14, 18});
    REQUIRE(bars == expected);

    ms.setLastEventTimeInSeconds(25.0);
    cp.updateCurrentPosition(0);
    bars = cp.getMeasuresToDisplay();
    expected = addMeasureNumbers(5, {1, 5, 9, 13, 17});
    REQUIRE(bars == expected);

    ms.setLastEventTimeInSeconds(26.0);
    cp.updateCurrentPosition(0);
    bars = cp.getMeasuresToDisplay();
    expected = addMeasureNumbers(6, {4, 8, 12, 16});
    REQUIRE(bars == expected);

    ms.setLastEventTimeInSeconds(0.0);
    cp.updateCurrentPosition(0);
    bars = cp.getMeasuresToDisplay();
    expected = addMeasureNumbers(-1, {2, 6, 10, 14, 18});
    REQUIRE(bars == expected);

    ms.setLastEventTimeInSeconds(1.0);
    cp.updateCurrentPosition(0);
    bars = cp.getMeasuresToDisplay();
    expected = addMeasureNumbers(-1, {1, 5, 9, 13, 17});
    REQUIRE(bars == expected);

    // Try something slightly more mathematically challenging (but still with nice round numbers)
    ms.setBPMeasure(6);
    ms.setBPMinute(90.0);
    ms.setLastEventTimeInSeconds(15.0);
    cp.updateCurrentPosition(0);
    bars = cp.getMeasuresToDisplay();
    expected = addMeasureNumbers(3, {3, 7, 11, 15, 19});
    REQUIRE(bars == expected);

    ms.setBPMeasure(4);
    ms.setBPMinute(80.0);
    ms.setLastEventTimeInSeconds(14.0);
    cp.updateCurrentPosition(0);
    bars = cp.getMeasuresToDisplay();
    expected = addMeasureNumbers(3, {2, 5, 8, 11, 14, 17});
    REQUIRE(bars == expected);
}


TEST_CASE("mouse nudge", "chordview")
{
    MidiStore ms;
    ChordClipper cp(ms);
    ChordVectorType chords;
    ChordVectorType expected;

    ms.setTimeWidth(10.0);
    ms.setPlayHeadPosition(0.0);
    
    addNote(ms, 1.0, 1.0, 12);  // C
    addNote(ms, 13.0, 1.0, 17);  // F
    ms.setLastEventTimeInSeconds(1.0);
    
    chords = cp.getChordsToDisplay();
    expected = {{1, "C"}};
    REQUIRE(chords == expected);

    // A nudge while playing should not move it
    ms.setIsPlaying(true);
    cp.scrollWheelNudge(0.5);
    chords = cp.getChordsToDisplay();
    REQUIRE(chords == expected);

    // If not playing, a nudge of 0.5 should move it half of the width (5 seconds since we set width to 10.0 above)
    ms.setIsPlaying(false);
    cp.scrollWheelNudge(0.5);
    chords = cp.getChordsToDisplay();
    expected = {{8, "F"}};
    REQUIRE(chords == expected);

    // negative nudge
    cp.scrollWheelNudge(-0.5);
    chords = cp.getChordsToDisplay();
    expected = {{1, "C"}};
    REQUIRE(chords == expected);
}