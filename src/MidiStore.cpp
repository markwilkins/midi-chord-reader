
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
    {
        return true;
    }
    else
    {
        return false;
    }
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
 * @brief Add a midi note on/off event at the given time
 * 
 * @param int64 time   time info derived from AudioProcessor::processBlock callback
 * @param int   note   midi note number
 * @param bool  isOn   Is it an on or off event?
 */
void MidiStore::addNoteEventAtTime(int64 time, int note, bool isOn)
{
    Identifier timeProp = notesAtIdent(time);
    const ScopedLock lock(storeLock);
    // Find the valuetree for this time (create it if it does not exist)
    ValueTree child = trackData->getOrCreateChildWithName(timeProp, nullptr);

    // Store the note with the identifier (the label) being the note number and the value being bool on/off
    Identifier noteProp(std::to_string(note));
    child.setProperty(noteProp, isOn, nullptr);

    // Make sure it has the time stored
    // tbd : need to convert this to an identifier
    child.setProperty("eventTime", time, nullptr);
}

/**
 * @brief Retrieve a sorted array of note values that have ON events at the given time.
 * This is the list of notes that have on events at the time (not necessarily all notes that are on)
 * 
 * @param int64 time 
 * @return Array<int> 
 */
Array<int> MidiStore::getNoteOnEventsAtTime(int64 time) 
{
    Identifier timeProp = notesAtIdent(time);
    const ScopedLock lock(storeLock);
    ValueTree child = trackData->getChildWithName(timeProp);
    Array<int> notes;
    for (int i = 0; i < child.getNumProperties(); ++i)
    {
        Identifier noteIdent = child.getPropertyName(i);
        int note = std::stoi(noteIdent.toString().toStdString());
        bool isOn = child.getProperty(noteIdent);
        if (isOn) {
            notes.add(note);
        }
    }
    // Need to figure out the nuances of the scopedlock ... ideally should release the lock here before the 
    // sort operation. Or maybe move the extraction call/loop above into a simple method and use the scoped
    // lock there.
    
    DefaultElementComparator<int> sorter;
    notes.sort(sorter);
    return notes;
}

/**
 * @brief Retrieve the list of notes that are ON at the given moment
 * This assumes that the child nodes in the tree are sorted by time
 * Current algorithm is brute force. Start at the beginning and track on/off events
 * 
 * @param int64 time 
 * @return Array<int>   Note values at that time
 */
Array<int> MidiStore::getAllNotesOnAtTime(int64 time) 
{
    const ScopedLock lock(storeLock);
    SortedSet<int> notes;

    for (ValueTree::Iterator events = trackData->begin(); events != trackData->end(); ++events)
    {
        ValueTree child = *events;
        for (int i = 0; i < child.getNumProperties(); ++i)
        {
            int64 eventTime = child.getProperty("eventTime");
            if (eventTime > time)
            {
                break;
            }
            Identifier noteIdent = child.getPropertyName(i);
            int note = std::stoi(noteIdent.toString().toStdString());
            bool isOn = child.getProperty(noteIdent);
            if (isOn) 
            {
                notes.add(note);
            }
            else 
            {
                notes.remove(note);
            }
        }

    }

    Array<int> returnVal;
    for (int i = 0; i < notes.size(); i++) 
    {
        returnVal.add(notes[i]);
    }

    return returnVal;

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
