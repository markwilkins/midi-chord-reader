
#pragma once

// I am not able to figure out the CmakeLists.txt info to get both the app and test builds to be able to find JuceHeader.h
// So I am including the raw include files that are needed. I suppose that makes for a more efficient build. But I hate
// myself for not being able to understand the syntax to just be able to use JuceHeader.h. Grrrrrr. 
//#include <JuceHeader.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_extra/juce_gui_extra.h>

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

    // This should be a private method, but I don't have time to figure out how to mock the juce::Graphics
    // object so that I can just call paint(). I think HippoMocks would work for that, but that is an LGPL
    // license ... which maybe that is okay to use? I don't want to get tied into the "make it all free" stuff
    map<float, string> getChordsToDisplay();

private:
    MidiStore &midiState;
    string chordsToShow();
    pair<float, float> getViewWindowSize();
    bool isEventInWindow(pair<float, float> viewWindow, float eventSeconds, float &relativePosition);
    void drawChords(map<float, string> chords, juce::Graphics &g);

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordView)
};
