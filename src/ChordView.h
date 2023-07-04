/**
 * @file ChordView.h
 * @author Mark Wilkins
 * @brief Part of MidiChords project (plugin to display chord names from a MIDI track on playback)
 * @version 0.8.1
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

// I am not able to figure out the CmakeLists.txt info to get both the app and test builds to be able to find JuceHeader.h
// So I am including the raw include files that are needed. I suppose that makes for a more efficient build. But I hate
// myself for not being able to understand the syntax to just be able to use JuceHeader.h. hmmmmph 
//#include <JuceHeader.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "MidiStore.h"
#include "ChordClipper.h"

using namespace std;


/**
 * @brief Handle the display of the "marquee" window of the chords being scrolled by
 */
class ChordView : public juce::AnimatedAppComponent
{
public:
    ChordView(MidiStore&);

    void update() override;
    void paint(juce::Graphics &g) override;


    void resized() override;

private:
    // flag that indicates if the bravura font available (for flat/sharp symbols)
    bool symbolFontAvailable = false;
    ChordClipper chordClipper;
    MidiStore &midiState;
    void drawChords(ChordVectorType chords, juce::Graphics &g);
    void drawMeasures(MeasurePositionType bars, juce::Graphics &g);
    bool nameHasSymbols(string chord);
    bool checkForBravura();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordView)
};
