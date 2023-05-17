
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
    std::ostringstream oss;
    oss << "notesat:" << time;
    Identifier timeProp(oss.str());
    const ScopedLock lock(storeLock);
    // Find the valuetree for this time (create it if it does not exist)
    ValueTree child = trackData->getOrCreateChildWithName(timeProp, nullptr);

    // Store the note with the identifier (the label) being the note number and the value being bool on/off
    Identifier noteProp(std::to_string(note));
    child.setProperty(noteProp, isOn, nullptr);
}

Array<int> MidiStore::getNotesAtTime(int64 time) 
{
    std::ostringstream oss;
    oss << "notesat:" << time;
    Identifier timeProp(oss.str());
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

