
#include "MidiStore.h"

using namespace juce;

MidiStore::MidiStore()
{
    static juce::Identifier propertyName("name");
    trackData = new juce::ValueTree(propertyName);
}

MidiStore::~MidiStore()
{
    delete trackData;
}

/**
 * @brief Determine if any events are in this tree. Probably not useful.
 *
 * @return bool true if it has children (e.g., there are note events, false if not)
 */
bool MidiStore::hasData()
{
    if (trackData->getNumChildren() > 0)
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
    trackData->setProperty(propertyName, name, nullptr);
}

/**
 * @brief retrieve the name
 *
 * @return juce::String
 */
juce::String MidiStore::getName()
{
    static juce::Identifier propertyName("name");
    return trackData->getProperty(propertyName);
}

/**
 * @brief Remove the midi events from the store
 */
void MidiStore::clear()
{
    const ScopedLock lock(storeLock);
    // Note - Intentionally ignoring the recordData state change flag on this
    trackData->removeAllChildren(nullptr);
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
    const ScopedLock lock(storeLock);
    // Find the valuetree for this time (create it if it does not exist)
    ValueTree child = ensureNoteEventAtTime(time);

    // Store the note with the identifier (the label) being the note number and the value being bool on/off
    Identifier noteProp(std::to_string(note));
    child.setProperty(noteProp, isOn, nullptr);
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
    ValueTree existingChild = trackData->getChildWithName(timeProp);
    if (existingChild.isValid()) 
        return existingChild;
    
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
        for (ValueTree::Iterator events = trackData->begin(); events != trackData->end(); ++events)
        {
            ValueTree child = *events;
            int64 eventTime = child.getProperty("eventTime");
            jassert(eventTime != time);
            if (eventTime > time)
                break;
            pos++;
        }

    }

    ValueTree newChild(timeProp);
    // Make sure it has the time stored
    // tbd : need to convert this to an identifier
    newChild.setProperty("eventTime", time, nullptr);
    trackData->addChild(newChild, pos, nullptr);
    return newChild;
}


/**
 * @brief Retrieve a sorted array of note values that have ON events at the given time.
 * This is the list of notes that have on events at the time (not necessarily all notes that are on)
 *
 * @param int64 time
 * @return std::vector<int>
 */
std::vector<int> MidiStore::getNoteOnEventsAtTime(int64 time)
{
    Identifier timeProp = notesAtIdent(time);
    const ScopedLock lock(storeLock);
    ValueTree child = trackData->getChildWithName(timeProp);
    std::vector<int> notes;
    for (int i = 0; i < child.getNumProperties(); ++i)
    {
        Identifier noteIdent = child.getPropertyName(i);
        String eventStr = noteIdent.toString();
        int note;
        // If this is not an integer, it is not a note event. Skip it in that case
        if (!stringToInt(eventStr, &note))
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
    std::sort(notes.begin(), notes.end());
    return notes;
}

/**
 * @brief Retrieve the list of notes that are ON at the given moment
 * This assumes that the child nodes in the tree are sorted by time
 * Current algorithm is brute force. Start at the beginning and track on/off events
 *
 * @param int64 startTime     Ignore notes before this time
 * @param int64 endTime       Point in time of interest
 * @return std::vector<int>   Note values at that time
 */
std::vector<int> MidiStore::getAllNotesOnAtTime(int64 startTime, int64 endTime)
{
    const ScopedLock lock(storeLock);
    SortedSet<int> notes;

    // Brute force for now. Start at the beginning and work through
    for (ValueTree::Iterator events = trackData->begin(); events != trackData->end(); ++events)
    {
        ValueTree child = *events;
        for (int i = 0; i < child.getNumProperties(); ++i)
        {
            int64 eventTime = child.getProperty("eventTime");
            if (eventTime < startTime) 
                continue;
            if (eventTime > endTime)
                break;

            Identifier noteIdent = child.getPropertyName(i);
            std::string eventStr = noteIdent.toString().toStdString();
            int note;
            // If this is not an integer, it is not a note event. Skip it in that case
            if (!stringToInt(eventStr, &note))
                continue;

            bool isOn = child.getProperty(noteIdent);
            if (isOn)
                notes.add(note);
            else
                notes.removeValue(note);
        }
    }

    std::vector<int> returnVal;
    for (int i = 0; i < notes.size(); i++)
        returnVal.push_back(notes[i]);

    return returnVal;
}


/**
 * @brief Retrieve all times where event changes occur
 * 
 * @return std::vector<int64> 
 */
std::vector<int64> MidiStore::getEventTimes() {
    const ScopedLock lock(storeLock);
    std::vector<int64> eventTimes;

    for (ValueTree::Iterator events = trackData->begin(); events != trackData->end(); ++events)
    {
        ValueTree child = *events;
        int64 eventTime = child.getProperty("eventTime");
        eventTimes.push_back(eventTime);
    }

    return eventTimes;
}

/**
 * @brief Convert a juce string to an integer
 * Could just use the built-in getIntValue, but it returns 0 if not a string with no indicator 
 * that I can see if it wasn't an integer
 *
 * @param str
 * @param value
 * @return bool  True if converted, false if not (e.g., not an integer)
 */
bool MidiStore::stringToInt(String str, int *value)
{
    std::string stdstr = str.toStdString();
    if (stdstr.find_first_not_of("0123456789") != std::string::npos)
    {
        // Not an integer apparently
        return false;
    }
    *value = std::stoi(stdstr);
    return true;
}


/**
 * @brief Create a ValudTree identifier for notes at a given time
 *
 * @param int64 time
 * @return Identifier
 */
Identifier MidiStore::notesAtIdent(int64 time)
{
    std::ostringstream oss;
    oss << "notesat:" << time;
    Identifier timeProp(oss.str());
    return timeProp;
}
