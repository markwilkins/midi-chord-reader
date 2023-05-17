
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

void MidiStore::addNoteAtTime(int64 time, int note)
{
    std::ostringstream oss;
    oss << "notesat:" << time;
    Identifier timeProp(oss.str());
    ValueTree child = trackData->getOrCreateChildWithName(timeProp, nullptr);
    oss << "notenum:" << note;
    Identifier noteProp(oss.str());
    child.setProperty(noteProp, note, nullptr);
}

Array<int> MidiStore::getNotesAtTime(int64 time) 
{
    std::ostringstream oss;
    oss << "notesat:" << time;
    Identifier timeProp(oss.str());
    ValueTree child = trackData->getChildWithName(timeProp);
    Array<int> notes;
    for (int i = 0; i < child.getNumProperties(); ++i)
    {
        int note = child.getProperty(child.getPropertyName(i));
        notes.add(note);
    }
    DefaultElementComparator<int> sorter;
    notes.sort(sorter);
    return notes;
}

