
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
    void addNoteEventAtTime(int64 time, int note, bool isOn);

    Array<int> getNoteOnEventsAtTime(int64 time);

    Array<int> getAllNotesOnAtTime(int64 time);

private:
    // Critical section for concurrent access. The editor will be reading it. processor updates it
    CriticalSection storeLock;

    Identifier notesAtIdent(int64 time);

    juce::ValueTree *trackData;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiStore)
};
