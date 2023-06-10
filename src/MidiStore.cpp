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
#include "ChordName.h"

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
    
    this->chordState = newState;
    this->isViewUpToDate = false;
    refreshSettingsFromState();
    return true;
}


/**
 * @brief Update the various plugin settings from the current state
 * 
 */
void MidiStore::refreshSettingsFromState()
{
    if (chordState.hasProperty(allowRecordingProp))
    {
        bool allow = chordState.getProperty(allowRecordingProp);
        this->allowDataRecording = allow;
    }

}

/**
 * @brief Are state changes (e.g., update of midi notes) allowed?
 * Update the prop in the state as well so it gets saved by the DAW
 * 
 * @param bool allow 
 */
void MidiStore::allowStateChange(bool allow) 
{
    allowDataRecording = allow;
    chordState.setProperty(allowRecordingProp, allow, nullptr);
}

/**
 * @brief Store the relative playhead position in the state tree. This is the 
 * line in the scrolling view that represents "now" 
 * 
 * @param float percentage A % value 1 to 99 (0 and 100 cause the line to be off screen)
 */
void MidiStore::setPlayHeadPosition(float percentage)
{
    chordState.setProperty(playHeadPositionProp, percentage, nullptr);
}

/**
 * @brief Retrieve the location of playhead in the view window as a percentage
 * 
 * @return float  percentage
 */
float MidiStore::getPlayHeadPosition()
{
    float position = 25.0;
    if (chordState.hasProperty(playHeadPositionProp))
    {
        float storedPosition = chordState.getProperty(playHeadPositionProp);
        if (storedPosition >= 0.0 && storedPosition <= 100.0)
            position = storedPosition;
    }
    return position;
}

/**
 * @brief Store the width of the scrolling view window in seconds 
 * 
 * @param width  
 */
void MidiStore::setTimeWidth(float width)
{
    chordState.setProperty(viewWidthProp, width, nullptr);
}

/**
 * @brief Retrieve the width (in seconds) of the view window
 * 
 * @return float 
 */
float MidiStore::getTimeWidth()
{
    // Until I start using this for real and playing live while watching the playback, I 
    // am not sure what good values are. Default to 20 seconds and limit to 100 for now.
    float width = 20.0;
    if (chordState.hasProperty(viewWidthProp))
    {
        float storedWidth = chordState.getProperty(viewWidthProp);
        if (storedWidth >= 1.0 && storedWidth <= 100.0)
            width = storedWidth;
    }
    return width;
}

/**
 * @brief Remove the midi events from the store
 */
void MidiStore::clear()
{
    const ScopedLock lock(storeLock);
    // Note - Intentionally ignoring the recordData state change flag on this
    chordState.removeAllChildren(nullptr);
    this->isViewUpToDate = false;
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
    // In order to know if modifications were made (so we know if static view is out in sync), check to see if value
    // will be changing
    juce::var varOn = isOn;
    if (!child.hasProperty(noteProp) || child.getProperty(noteProp) != varOn)
    {
        child.setProperty(noteProp, varOn, nullptr);
        this->isViewUpToDate = false;
    }
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
    if (!allowDataRecording) 
        return;
    time = this->quantizeEventTime(time);
    Identifier timeProp = notesAtIdent(time);
    const ScopedLock lock(storeLock);
    ValueTree eventTree = chordState.getChildWithName(timeProp);
    if (eventTree.isValid()) 
    {
        juce::var varSeconds = seconds;
        if (!eventTree.hasProperty(eventTimeInSecondsProp) || eventTree.getProperty(eventTimeInSecondsProp) != varSeconds)
        {
            eventTree.setProperty(eventTimeInSecondsProp, varSeconds, nullptr);
            this->isViewUpToDate = false;
        }
    }
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
    set<int> notes;

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

            updateCurrentlyOn(child, notes, i);
        }
    }

    vector<int> returnVal(notes.size());
    copy(notes.begin(), notes.end(), returnVal.begin());

    return returnVal;
}

/**
 * @brief Add/delete the a note from the chidl tree to the set of notes that is current "on"
 * 
 * @param ValueTree childEvents   value tree containing note on/off events
 * @param set<int>  notes         update this set accordingly 
 * @param int       propIndex     The index of the property of interest
 */
void MidiStore::updateCurrentlyOn(ValueTree &childEvents, set<int> &notes, int propIndex)
{
    Identifier noteIdent = childEvents.getPropertyName(propIndex);
    string eventStr = noteIdent.toString().toStdString();
    int note;
    // If this is not an integer, it is not a note event. Skip it in that case
    if (!noteIdentToInt(eventStr, &note))
        return;

    bool isOn = childEvents.getProperty(noteIdent);
    if (isOn)
        notes.insert(note);
    else
        notes.erase(note);

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


/**
 * @brief Create the simple (efficient) static view of the chords for repeated use
 * 
 * @return vector <pair<float, string>> 
 */
vector <pair<float, string>> MidiStore::createStaticView()
{
    // I *think* that this lock held here will be the most "expensive" of all the operations. May need to rethink
    // how this is done. This operation is O(n) without a lot of cost involved (perhaps the chord naming is most costly?)
    // It might be more efficient to get the lock and then make a copy of the chordState tree, unlock, build up the
    // chord set, then lock again and replace the static view?
    const ScopedLock lock(storeLock);
    set<int> notes;
    double prevTime = 0.0;
    string prevChord = "";
    ChordName cn;
    vector<pair<float, string>> newStaticView;

    for (ValueTree::Iterator events = chordState.begin(); events != chordState.end(); ++events)
    {
        ValueTree child = *events;
        double eventTimeInSeconds = child.getProperty(eventTimeInSecondsProp);

        // sanity check on the expected sortedness of the value tree events
        // FIXME: what to do about this... I think it happens when playing back over the
        // top of existing data. The slight shift I see in the int64 event times applies
        // to the floating point seconds as well. If I do a straight through recording
        // of the data (totally clean) then I do no thit this assert
        //if (eventTimeInSeconds < prevTime) 
        //    continue;
        jassert(eventTimeInSeconds >= prevTime);
        prevTime = eventTimeInSeconds;

        for (int i = 0; i < child.getNumProperties(); ++i)
        {
            updateCurrentlyOn(child, notes, i);
        }

        vector<int> currentChord(notes.size());
        copy(notes.begin(), notes.end(), currentChord.begin());
        string newChord = cn.nameChord(currentChord);
        if (newChord != prevChord && newChord != "")
        {
            newStaticView.push_back({eventTimeInSeconds, newChord});
            prevChord = newChord;
        }

    }

    return newStaticView;
}


/**
 * @brief Update the "efficient" static view of the set of chords represented by chordState
 * This rebuilds it from scratch each time. The assumption (possibly a poor one) is that this
 * rebuild will be relatively infrequent.
 * If this assumption proves to be majorly bad and updates are going to be frequent, then believe it
 * will be necessary to figure out how to update this static view selectively and efficiently. 
 * It is a bit if a tricky problem ... but certainly solvable. If, for example, a new midi note is
 * added at time T, then 
 */
void MidiStore::updateStaticView()
{
    vector<pair<float, string>> newStaticView;
    newStaticView = createStaticView();

    const ScopedLock lock(viewLock);
    this->staticView = newStaticView;
}


/**
 * @brief Update the efficient static view of the chords if it is out of date
 * This puts a limit on the frequency of updates. Currently hard coded at once per second.
 * Maybe need to make this controllable via the interface.
 */
void MidiStore::updateStaticViewIfOutOfDate()
{
    // if it appears to be up-to-date, don't do anything
    if (!this->isViewUpToDate) 
    {
        int64 curTime = juce::Time::currentTimeMillis();
        // otherwise do an update only at most once per second
        if (curTime - this->lastViewUpdateTime > 1000)
        {
            DBG("Updating static view at time " + to_string(curTime));
            this->updateStaticView();
            this->lastViewUpdateTime = curTime;  // yeah ... not entirely accurate but close enough
            this->isViewUpToDate = true;
        }
    }
}


/**
 * @brief Retrieve a vector of time,chord pairs that are in the given window of time (in seconds).
 * This retrieves the data from the static view, which is updated intermittently. It is not guaranteed
 * to match the chordState ... but it should always be very close
 * 
 * @param viewWindow 
 * @return vector<pair<float, string>> 
 */
vector<pair<float, string>> MidiStore::getChordsInWindow(pair<float, float> viewWindow)
{
    const ScopedLock lock(viewLock);

    // Find the lower end
    float viewStart = viewWindow.first;
    float viewEnd = viewWindow.second;

    pair<float, string> p(viewStart, "");
    vector<pair<float, string>> chords;
    auto curChord = std::lower_bound(this->staticView.begin(), this->staticView.end(), p);
    for (; curChord != this->staticView.end(); ++curChord)
    {
        if (curChord->first > viewEnd)
            // past end of chords that fit in the window
            break;
        chords.push_back({curChord->first, curChord->second});
    }

    this->viewWindowChordCount = static_cast<int>(chords.size());

    return chords;
}