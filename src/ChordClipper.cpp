/**
 * @file ChordClipper.cpp
 * @author Mark Wilkins
 * @brief Part of MidiChords project (plugin to display chord names from a MIDI track on playback)
 * @version 0.1
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "ChordName.h"
#include "ChordClipper.h"

using namespace std;

ChordClipper::ChordClipper(MidiStore &ms) : midiState(ms)
{
    // Extract state info
    // ms.getPlayHeadPosition ...
}

/**
 * @brief Keep the current logic playhead position up to date. 
 * This is expected to be called on a constant timer so it can update the info that the view window
 * uses for knowing which chords to display.
 * 
 * @param msSinceLastUpdate   milliseconds since the last time this was called
 */
void ChordClipper::updateCurrentPosition(int msSinceLastUpdate)
{
    // If we have an update that represents the "true" position, then use that. This gets set during
    // callbacks by the plugin processor. This can happen during playback when it sends the next
    // bit of playback info to us. But even outside of playback, a click in the track by the user to
    // move the playhead position results in a call ... sometimes. That is cool because it keeps
    // the window up to date with respect to what the user is looking at in the track. But I notice
    // that it does not always update if the track that this plugin is on is not the current one.
    float lastSeenPosition = static_cast<float>(midiState.getLastEventTimeInSeconds());
    if (lastSeenPosition != this->mostRecentPlayPosition)
    {
        // We have new position info from the DAW. Use it
        this->mostRecentPlayPosition = lastSeenPosition;
        this->estimatedPlayPosition = lastSeenPosition;
    }
    else
    {
        // We do not have new "official" position info, so just keep moving the window along based on
        // the amount of elapsed time. If we are not currently in playback, then don't change the position.
        if (midiState.getIsPlaying())
        {
            // This is not an atomic add ... but this method is the only one updating this value and I *assume* (yeah yeah) that
            // update() would not be called concurrently on multiple threads. Worst case is the read and add would be out of
            // sync and basically reset it to an older value. Next time an actual playhead event occurs, it would be fixed.
            this->estimatedPlayPosition = this->estimatedPlayPosition + static_cast<float>(msSinceLastUpdate / 1000.0);
        }
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
 * @return vector<pair<float, string>>
 */
vector<pair<float, string>> ChordClipper::getChordsToDisplay() 
{
    // Compute the view window for the chords of interest
    pair<float, float> viewWindow = getViewWindowSize();
    vector<pair<float, string>> chords;
    float offset = viewWindow.first;

    // TODO: efficiency change:
    // Maintain state info in this class of current chords being displayed. And then here, we should be able to
    // just get new chords once every second perhaps (since we have the 2 second buffer). That way, if we already
    // have chords for time T 20 to 30 and then when we are asking for chords from 21 to 31, we should need only
    // to get the chords for time T 30 to 31 and just update the existing set.

    viewWindow.first -= 2.0f;
    viewWindow.second += 2.0f;
    chords = midiState.getChordsInWindow(viewWindow);
    // need to offset the values to the window. If the window is 50 to 70, and we have events
    // at 55 and 56, then we want to change them to be 5 and 6 respectively (make their offsets
    // be relative to the window)
    for (auto &i : chords) 
        i.first -= offset;

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

    start = this->estimatedPlayPosition - this->getCurrentNotePosition();
    end = start + this->getViewWidthInSeconds();
    return {start, end};
}

// Reference position of "now" in the view port. This is where in the current position of the playhead resides.
// In other words, if window width represents 20 seconds and this is value 5, then the currently playing note (chord)
// will be at 25% from the left.
float ChordClipper::getCurrentNotePosition()
{
    // in percent
    float position = midiState.getPlayHeadPosition();
    float positionInSeconds = static_cast<float>(this->getViewWidthInSeconds() * (position / 100.0));
    return positionInSeconds;
}


/**
 * @brief Retrieve the stored value of the view window
 * 
 * @return float  
 */
float ChordClipper::getViewWidthInSeconds()
{
    return midiState.getTimeWidth();
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
