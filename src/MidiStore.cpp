/**
 * @file MidiStore.cpp
 * @author Mark Wilkins
 * @brief Part of MidiChords project (plugin to display chord names from a MIDI track on playback)
 * @version 0.1
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#include "MidiStore.h"

using namespace juce;
using namespace std;

MidiStore::MidiStore() : chordState("name")
{
    // Start out with our current version; a load of older version might change it
    chordState.setProperty(this->midiChordsVersionProp, this->currentVersion, nullptr);
}

MidiStore::~MidiStore()
{
    //delete chordState;
}

/**
 * @brief Determine if any events are in this tree. Probably not useful.
 *
 * @return bool true if it has children (e.g., there are note events, false if not)
 */
bool MidiStore::hasData()
{
    if (chordState.getNumChildren() > 0)
        return true;
    else
        return false;
}

/**
 * @brief Set the name of the object ... probably not useful
 *
 * @param juce::String name
 */
void MidiStore::setName(juce::String name)
{
    static juce::Identifier propertyName("name");
    chordState.setProperty(propertyName, name, nullptr);
}

/**
 * @brief retrieve the name
 *
 * @return juce::String
 */
juce::String MidiStore::getName()
{
    static juce::Identifier propertyName("name");
    return chordState.getProperty(propertyName);
}


/**
 * @brief Replace the state info with the new value tree. This is intended for loading saved state
 * 
 * @param ValueTree newState 
 */
bool MidiStore::replaceState(ValueTree &newState)
{
    // check the version of the state and make sure we can work with it
    if (!newState.hasProperty(this->midiChordsVersionProp)) 
    {
        DBG("Cannot load saved state. It does not appear to be valid");
        return false;
    }

    int version = newState.getProperty(this->midiChordsVersionProp);
    if (version > this->currentVersion)
    {
        DBG("Cannot load saved state. It is from a newer version of the plugin. Version: " + to_string(version));
        return false;
    }
    chordState = newState;
    return true;
}

/**
 * @brief Remove the midi events from the store
 */
void MidiStore::clear()
{
    const ScopedLock lock(storeLock);
    // Note - Intentionally ignoring the recordData state change flag on this
    chordState.removeAllChildren(nullptr);
}

/**
 * @brief Add a midi note on/off event at the given time
 *
 * @param int64 time   time info derived from AudioProcessor::processBlock callback
 * @param int   note   midi note number
 * @param bool  isOn   Is it an on or off event?
 */
void MidiStore::addNoteEventAtTime(int64 time, int note, bool isOn)
{
    if (!allowDataRecording) 
        return;
    time = quantizeEventTime(time);

    const ScopedLock lock(storeLock);
    // Find the valuetree for this time (create it if it does not exist)
    ValueTree child = ensureNoteEventAtTime(time);

    // Store the note with the identifier (the label) being the note number and the value being bool on/off
    Identifier noteProp = noteIdentFromInt(note);
    child.setProperty(noteProp, isOn, nullptr);
}


/**
 * @brief I'm not sure if it is my interpretation of the incoming data being slightly off or if that's simply the nature
 * but on playbacks, the int64 even time is not always exactly the same. This rounds the value to the nearest Nth.
 * The N defaults to 1000 because that seems a good value for my use case. If this doesn't work for other DAWs, then
 * I will need to make this controllable.
 * 
 * @param int64 time 
 * @return int64      Return value rounded to the nearest quantization value. If q=1000, then 12345 will be 12000
 *                    and 12555 will be 13000
 */
int64 MidiStore::quantizeEventTime(int64 time) 
{
    int qValue = this->getQuantizationValue();
    double fraction = round(static_cast<double>(time) / qValue);
    int64 rounded = static_cast<int64>(fraction);
    return rounded * qValue;
}

/**
 * @brief Store the time in seconds for an event
 * 
 * @param time      the native integer event time
 * @param seconds   Number of seconds associated with the event
 */
void MidiStore::setEventTimeSeconds(int64 time, double seconds) 
{
    time = this->quantizeEventTime(time);
    Identifier timeProp = notesAtIdent(time);
    const ScopedLock lock(storeLock);
    ValueTree eventTree = chordState.getChildWithName(timeProp);
    if (eventTree.isValid()) 
        eventTree.setProperty(eventTimeInSecondsProp, seconds, nullptr);
}

/**
 * @brief Make sure a child exists at the specified time. If not add it in sorted order
 * 
 * @param time 
 * @return ValueTree 
 */
ValueTree MidiStore::ensureNoteEventAtTime(int64 time) 
{
    Identifier timeProp = notesAtIdent(time);
    const ScopedLock lock(storeLock);
    ValueTree existingChild = chordState.getChildWithName(timeProp);
    if (existingChild.isValid()) 
        return existingChild;
    
    if (maxTimeStored == 0) 
        // On a load of a saved state, this won't be set; make it be set now
        maxTimeStored = findMaxTime();

    int pos;
    if (time > maxTimeStored) 
    {
        // We can just add it at the end
        pos = -1;
        maxTimeStored = time;
    }
    else 
    {
        // super bad cheesy insertion sort ... assumption is that this is relatively uncommon
        pos = 0;
        for (ValueTree::Iterator events = chordState.begin(); events != chordState.end(); ++events)
        {
            ValueTree child = *events;
            int64 eventTime = child.getProperty(eventTimeProp);
            jassert(eventTime != time);
            if (eventTime > time)
                break;
            pos++;
        }

    }

    ValueTree newChild(timeProp);
    // Make sure it has the time stored
    // tbd : need to convert this to an identifier
    newChild.setProperty(eventTimeProp, time, nullptr);
    chordState.addChild(newChild, pos, nullptr);
    return newChild;
}

/**
 * @brief Retrieve the maximum event time of all the children in the value tree. 
 * 
 * @return int64 
 */
int64 MidiStore::findMaxTime()
{
    int64 max = 0;
    for (ValueTree::Iterator events = chordState.begin(); events != chordState.end(); ++events)
    {
        ValueTree child = *events;
        int64 eventTime = child.getProperty(eventTimeProp);
        max = std::max(max, eventTime);
    }
    return max;
}


/**
 * @brief Retrieve a sorted array of note values that have ON events at the given time.
 * This is the list of notes that have on events at the time (not necessarily all notes that are on)
 *
 * @param int64 time
 * @return vector<int>
 */
vector<int> MidiStore::getNoteOnEventsAtTime(int64 time)
{
    time = this->quantizeEventTime(time);
    Identifier timeProp = notesAtIdent(time);
    const ScopedLock lock(storeLock);
    ValueTree child = chordState.getChildWithName(timeProp);
    vector<int> notes;
    for (int i = 0; i < child.getNumProperties(); ++i)
    {
        Identifier noteIdent = child.getPropertyName(i);
        String eventStr = noteIdent.toString();
        int note;
        // If this is not an integer, it is not a note event. Skip it in that case
        if (!noteIdentToInt(eventStr, &note))
        {
            continue;
        }
        bool isOn = child.getProperty(noteIdent);
        if (isOn)
        {
            notes.push_back(note);
        }
    }

    // Need to figure out the nuances of the scopedlock ... ideally should release the lock here before the
    // sort operation. Or maybe move the extraction call/loop above into a simple method and use the scoped
    // lock there.
    sort(notes.begin(), notes.end());
    return notes;
}

/**
 * @brief Retrieve the list of notes that are ON at the given moment
 * This assumes that the child nodes in the tree are sorted by time
 * Current algorithm is brute force. Start at the beginning and track on/off events
 *
 * @param int64 startTime     Ignore notes before this time
 * @param int64 endTime       Point in time of interest
 * @return vector<int>   Note values at that time
 */
vector<int> MidiStore::getAllNotesOnAtTime(int64 startTime, int64 endTime)
{
    const ScopedLock lock(storeLock);
    SortedSet<int> notes;

    startTime = this->quantizeEventTime(startTime);
    endTime = this->quantizeEventTime(endTime);

    // Brute force for now. Start at the beginning and work through
    for (ValueTree::Iterator events = chordState.begin(); events != chordState.end(); ++events)
    {
        ValueTree child = *events;
        for (int i = 0; i < child.getNumProperties(); ++i)
        {
            int64 eventTime = child.getProperty(eventTimeProp);
            if (eventTime < startTime) 
                continue;
            if (eventTime > endTime)
                break;

            Identifier noteIdent = child.getPropertyName(i);
            string eventStr = noteIdent.toString().toStdString();
            int note;
            // If this is not an integer, it is not a note event. Skip it in that case
            if (!noteIdentToInt(eventStr, &note))
                continue;

            bool isOn = child.getProperty(noteIdent);
            if (isOn)
                notes.add(note);
            else
                notes.removeValue(note);
        }
    }

    vector<int> returnVal;
    for (int i = 0; i < notes.size(); i++)
        returnVal.push_back(notes[i]);

    return returnVal;
}

/**
 * @brief Retrieve the time (in seconds) associated with this event
 * 
 * @param time 
 * @return double 
 */
double MidiStore::getEventTimeInSeconds(int64 time) 
{
    double seconds = 0.0;
    time = this->quantizeEventTime(time);
    Identifier timeProp = notesAtIdent(time);
    const ScopedLock lock(storeLock);
    ValueTree eventTree = chordState.getChildWithName(timeProp);
    if (eventTree.isValid()) 
    {
        seconds = eventTree.getProperty(eventTimeInSecondsProp);
    }

    return seconds;
}


/**
 * @brief Retrieve all times where event changes occur
 * 
 * @return vector<int64> 
 */
vector<int64> MidiStore::getEventTimes() {
    const ScopedLock lock(storeLock);
    vector<int64> eventTimes;

    for (ValueTree::Iterator events = chordState.begin(); events != chordState.end(); ++events)
    {
        ValueTree child = *events;
        int64 eventTime = child.getProperty(eventTimeProp);
        eventTimes.push_back(eventTime);
    }

    return eventTimes;
}


/**
 * @brief Store the given quantization value. 
 * 
 * maybe todo: If this gets set to a different value, then it is likely necessary to update
 * the existing event times in the tree accordingly. But that is tricky if the quantization
 * value is reduced. Unless we saved the original event times, then we can't refine the
 * quantization (rounding). 
 * 
 * @param int q   new quantization value
 */
void MidiStore::setQuantizationValue(int q) 
{
    chordState.setProperty(quantizationValueProp, q, nullptr);
}

/**
 * @brief Retrieve the integer quantization (rounding) value
 * 
 * @return int 
 */
int MidiStore::getQuantizationValue()
{
    int q = 0;
    if (chordState.hasProperty(quantizationValueProp))
        q = chordState.getProperty(quantizationValueProp);

    // default to 1000 ... for no good reason other than that it works well for Logic Pro X
    return q > 0 ? q : 1000;
}



/**
 * @brief Convert an ident of the form ":<int>" to the integer. Returns false if not in that form
 *
 * @param String str
 * @param int    *value
 * @return bool  True if converted, false if not (e.g., not an integer, or not of right form)
 */
bool MidiStore::noteIdentToInt(String str, int *value)
{

    if (!str.startsWithChar(':')) 
        return false;

    string stdstr = str.substring(1).toStdString();
    if (stdstr.find_first_not_of("0123456789") != string::npos)
    {
        // Not an integer apparently
        return false;
    }
    *value = stoi(stdstr);
    return true;
}

/**
 * @brief Create a ValueTree identifier for an integer midi note value
 * I was using the integer value as the label, but I also want to use the xml conversion logic built 
 * into the ValueTree for save/restore. And a simple integer is not a valid XML name. So use a : prefix
 *
 * @param int64 time
 * @return Identifier
 */
Identifier MidiStore::noteIdentFromInt(int note)
{
    ostringstream oss;
    oss << ":" << note;
    Identifier noteProp(oss.str());
    return noteProp;
}

/**
 * @brief Create a ValueTree identifier for notes at a given time
 *
 * @param int64 time
 * @return Identifier
 */
Identifier MidiStore::notesAtIdent(int64 time)
{
    ostringstream oss;
    oss << "notesat:" << time;
    Identifier timeProp(oss.str());
    return timeProp;
}
