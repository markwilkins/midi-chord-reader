/**
 * @file ChordClipper.h
 * @author Mark Wilkins
 * @brief Part of MidiChords project (plugin to display chord names from a MIDI track on playback)
 * @version 0.1
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include "MidiStore.h"
#include "MidiChordsTypes.h"

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
    vector<pair<float, string>> getChordsToDisplay();
    void updateCurrentPosition(int msSinceLastUpdate);

    vector<float> getMeasuresToDisplay();

    float getViewWidthInSeconds();
    float getCurrentNotePosition();

private:

    pair<float, float> viewBufferSize;
    vector<pair<float, string>> viewBuffer;

    MidiStore &midiState;
    pair<float, float> getViewWindowSize();
    bool isEventInWindow(pair<float, float> viewWindow, float eventSeconds, float &relativePosition);
    pair<float, float> computeNewWindowSize(pair<float, float> neededWindow);
    bool hasForwardOverlap(ViewWindowType neededWindow);
    void constructDisplayedChords(ViewWindowType viewWindow, ChordVectorType newChords);

    // The time (in seconds) of the most recently seen track time
    float mostRecentPlayPosition = 0.0;
    // This represents where we believe the playhead to be. Tracking this value instead of pestering the audio processor
    // for actual playhead position constantly
    atomic<float> estimatedPlayPosition = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordClipper)
};