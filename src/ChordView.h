
#pragma once
#include <JuceHeader.h>
#include "MidiStore.h"

using namespace std;


//==============================================================================
class ChordView : public juce::AnimatedAppComponent
{
public:
    ChordView(MidiStore&);

    void update() override;
    void paint(juce::Graphics &g) override;


    void resized() override;

private:
    MidiStore &midiState;
    string chordsToShow();
    map<float, string> getChordsToDisplay();
    pair<float, float> getViewWindowSize();
    bool isEventInWindow(pair<float, float> viewWindow, float eventSeconds, float &relativePosition);
    void drawChords(map<float, string> chords, juce::Graphics &g);

    // The time (in seconds) of the most recently seen track time
    float mostRecentPlayPosition = 0.0;
    // This represents where we believe the playhead to be. Tracking this value instead of pestering the audio processor
    // for actual playhead position constantly
    atomic<float> estimatedPlayPosition = 0.0;

    float viewWidthInSeconds = 20.0;
    // Reference posiotion of "now" in the view port. This is where in the current position of the playhead resides.
    // In other words, if window width represents 20 seconds and this is value 5, then the currently playing note (chord)
    // will be at 25% from the left.
    float currentNotePosition = 5.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordView)
};
