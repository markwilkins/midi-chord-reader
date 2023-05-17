
/*
*/

#pragma once

// #include <JuceHeader.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
// #include <juce_core/text/juce_Identifier.h>
using namespace juce;

/**
 * 
 */
class MidiStore 
{
public:
    //==============================================================================
    MidiStore();
    ~MidiStore();
    bool hasData();
    void setName(juce::String name);
    String getName();
    void addNoteAtTime(int64 time, int note);

    Array<int> getNotesAtTime(int64 time);

private:

    juce::ValueTree *trackData;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiStore)
};
