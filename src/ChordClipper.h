
#pragma once
#include "MidiStore.h"

using namespace std;

/**
 * @brief This is a helper class for ChordView to determine which chords are in view
 * This does the math calculation offsets of the current position of the viewable window
 * with respect to the currently played notes. The window might be displaying from
 * seconds 50 to 70, for example. It will return the chord names that fall in that window
 * (with some buffer on each side to allow for smoother transition of the chords in/out of view).
 * 
 */
class ChordClipper
{
public:
    ChordClipper(MidiStore&);
    map<float, string> getChordsToDisplay();
    void updateCurrentPosition(int msSinceLastUpdate);

    float getViewWidthInSeconds() {return viewWidthInSeconds;}
    float getCurrentNotePosition() {return currentNotePosition;}

private:

    MidiStore &midiState;
    pair<float, float> getViewWindowSize();
    bool isEventInWindow(pair<float, float> viewWindow, float eventSeconds, float &relativePosition);

    // The time (in seconds) of the most recently seen track time
    float mostRecentPlayPosition = 0.0;
    // This represents where we believe the playhead to be. Tracking this value instead of pestering the audio processor
    // for actual playhead position constantly
    atomic<float> estimatedPlayPosition = 0.0;

    float viewWidthInSeconds = 20.0;
    // Reference position of "now" in the view port. This is where in the current position of the playhead resides.
    // In other words, if window width represents 20 seconds and this is value 5, then the currently playing note (chord)
    // will be at 25% from the left.
    float currentNotePosition = 5.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordClipper)
};