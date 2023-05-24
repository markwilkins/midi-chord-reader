

#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
using namespace juce;

/**
 * This is the "data store" for the midi notes from a track. 
 * It is constructed as a juce::ValueTree that contains one level of child ValueTree objects
 * Each child ValueTree contains the midi note events for a given time. 
 * - The identifier for each child tree is a string of the form "notesat:<int64>" where
 *   the int value is the event time
 * - Each child tree has a property with the identifier "eventTime" where the value is the event time
 * - The child trees are kept in sorted order by event time
 * - The midi note numbers are stored as properties (in no particular order) in the child tree 
 *   with the identifier being the midi note number (string version of the int value) and the 
 *   value as a bool (true/false) representing if it is a note on or off event
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


    std::vector<int> getNoteOnEventsAtTime(int64 time);
    std::vector<int> getAllNotesOnAtTime(int64 startTime, int64 endTime);
    std::vector<int64> getEventTimes();


private:
    // Critical section for concurrent access. The editor will be reading it. processor updates it
    CriticalSection storeLock;

    // simple optimization tool. The normal process of adding values to the midistore will
    // be "in order". For example, playing a track from the start (or any position) will mean
    // time is increasing. So a typical operation of adding an event an a previously unseen
    // time will be to add it to the end and that will be the right place. But playback
    // operations of modified (new) notes will likely add new events into the middle.
    // If we know the max stored, then this helps us decide if we need to insert in tne middle.
    int64 maxTimeStored = 0;

    Identifier notesAtIdent(int64 time);
    bool stringToInt(String str, int *value);
    ValueTree ensureNoteEventAtTime(int64 time);

    juce::ValueTree *trackData;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiStore)
};
