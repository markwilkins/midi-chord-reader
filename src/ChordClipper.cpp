#include "ChordName.h"
#include "ChordClipper.h"

using namespace std;

ChordClipper::ChordClipper(MidiStore &ms) : midiState(ms)
{
}

void ChordClipper::updateCurrentPosition(int msSinceLastUpdate)
{
    float lastSeenPosition = static_cast<float>(midiState.getLastEventTimeInSeconds());
    if (lastSeenPosition != this->mostRecentPlayPosition)
    {
        this->mostRecentPlayPosition = lastSeenPosition;
        this->estimatedPlayPosition = this->mostRecentPlayPosition;
    }
    else
    {
        // This is not an atomic add ... but this method is the only one updating this value and I *assume* (yeah yeah) that
        // update() would not be called concurrently on multiple threads. Worst case is the read and add would be out of
        // sync and basically reset it to an older value. Next time an actual playhead event occurs, it will be fixed.
        if (midiState.getIsPlaying())
            this->estimatedPlayPosition = this->estimatedPlayPosition + static_cast<float>(msSinceLastUpdate / 1000.0);
    }

}

/**
 * @brief Get the set of displayable chords.
 * @details
 * This retrieves the "viewport" of the current window based on the current playhead position.
 * For each of the chords that falls into that window, it puts into a map of chords where the
 * floating point time in seconds is the time relative to the window. (e.g., where 0 is the left-most
 * side of the window)
 * 
 * @return map<float, string> 
 */
map<float, string> ChordClipper::getChordsToDisplay() 
{
    // Compute the view window for the chords of interest
    pair<float, float> viewWindow = getViewWindowSize();
    map<float, string> chords;
    ChordName cn;

    vector<int64> eventTimes = midiState.getEventTimes();
    for (vector<int64>::iterator i = eventTimes.begin(); i != eventTimes.end(); ++i) 
    {
        float eventSeconds = static_cast<float>(midiState.getEventTimeInSeconds(*i));
        float relativePosition;
        if (isEventInWindow(viewWindow, eventSeconds, relativePosition))
        {
            vector<int> itNotes = midiState.getAllNotesOnAtTime(0, *i);
            if (itNotes.size() > 0)
            {
                string chord = cn.nameChord(itNotes);
                chords.insert({relativePosition, chord});
            }

        }
    }
    return chords;
}

/**
 * @brief Retrieve the start/end times (in seconds) of the current view port
 * 
 * @return pair<float, float> 
 */
pair<float, float> ChordClipper::getViewWindowSize()
{
    float start;
    float end;

    start = this->estimatedPlayPosition - this->currentNotePosition;
    end = start + this->viewWidthInSeconds;
    return {start, end};
}

/**
 * @brief Determine if the given event falls in the displayable window.
 * 
 * @param viewWindow         View window in "real time"
 * @param eventSeconds       Time in seconds of the chord (the event)
 * @param relativePosition   Return the time offset by the view window
 * @return bool              Return true if displayable, false if not
 */
bool ChordClipper::isEventInWindow(pair<float, float> viewWindow, float eventSeconds, float &relativePosition)
{
    // Include some buffer on each end to avoid having them pop in/out of view rather than slide in/off smoothly
    relativePosition = 0.0;
    float buffer = 2.0;  // seconds
    if (eventSeconds >= viewWindow.first - buffer && eventSeconds <= viewWindow.second + buffer)
    {
        relativePosition = eventSeconds - viewWindow.first;
        return true;
    }
    else
    {
        return false;
    }
}
