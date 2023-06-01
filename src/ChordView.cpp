#include "ChordView.h"
#include "ChordName.h"

using namespace std;
using namespace juce;

ChordView::ChordView(MidiStore &ms) : midiState(ms)
{
    setFramesPerSecond(60);
}

void ChordView::update()
{
    // This function is called at the frequency specified by the setFramesPerSecond() call
    // in the constructor. You can use it to update counters, animate values, etc.
    //auto ms = getMillisecondsSinceLastUpdate();
    //DBG("ms since last update: " + to_string(ms));
    float lastSeenPosition = static_cast<float>(midiState.getLastEventTimeInSeconds());
    if (lastSeenPosition != mostRecentPlayPosition)
    {
        this->mostRecentPlayPosition = lastSeenPosition;
        this->estimatedPlayPosition = this->mostRecentPlayPosition;
    }
    else
    {
        auto ms = getMillisecondsSinceLastUpdate();
        // This is not an atomic add ... but this method is the only one updating this value and I *assume* (yeah yeah) that
        // update() would not be called concurrently on multiple threads. Worst case is the read and add would be out of
        // sync and basically reset it to an older value. Next time an actual playhead event occurs, it will be fixed.
        if (midiState.getIsPlaying())
            this->estimatedPlayPosition = this->estimatedPlayPosition + static_cast<float>(ms / 1000.0);
    }
}

void ChordView::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));


    // juce::Rectangle<float> textBox;
    // auto area = getLocalBounds();
    // textBox = {static_cast<float>(area.getX()), static_cast<float>(area.getY()), static_cast<float>(area.getWidth()), static_cast<float>(area.getHeight())};
    // string chords = chordsToShow();
    // g.setFont(25.0);
    // g.drawText(chords, textBox, juce::Justification::centred);
    int x = static_cast<int>(getWidth() * this->currentNotePosition / this->viewWidthInSeconds);
    g.setColour(juce::Colours::red);
    g.drawVerticalLine(x, 0, getHeight());

    g.setColour(getLookAndFeel().findColour(juce::Slider::thumbColourId));
    map<float, string> chords = this->getChordsToDisplay();
    this->drawChords(chords, g);
}


/**
 * @brief Draw the given set of chords onto the graphics area
 * 
 * @param chords   map of chords by time relative to the window
 * @param g 
 */
void ChordView::drawChords(map<float, string> chords, juce::Graphics &g)
{
    map<float, string>::iterator it;
    auto area = getLocalBounds();
    juce::Rectangle<float> textBox;
    textBox = area.toFloat();
    g.setFont(25.0);
    float ratio = textBox.getWidth() / this->viewWidthInSeconds;

    for (it = chords.begin(); it != chords.end(); ++it)
    {
        float leftPos = it->first * ratio;
        textBox.setLeft(leftPos);
        g.drawText(it->second, textBox, juce::Justification::centredLeft);

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
map<float, string> ChordView::getChordsToDisplay() 
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
pair<float, float> ChordView::getViewWindowSize()
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
bool ChordView::isEventInWindow(pair<float, float> viewWindow, float eventSeconds, float &relativePosition)
{
    // Include some buffer on each end to avoid having them pop in/out of view rather than slide in/off smoothly
    relativePosition = 0.0;
    float buffer = 25.0;
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

// Temp debugging function - will go away
string ChordView::chordsToShow()
{
    ChordName cn;
    string    chords = "";

    vector<int64> eventTimes = midiState.getEventTimes();
    for (vector<int64>::iterator i = eventTimes.begin(); i != eventTimes.end(); ++i) 
    {
        vector<int> itNotes = midiState.getAllNotesOnAtTime(0, *i);
        string lastChord = cn.nameChord(itNotes);
        if (lastChord != "") 
            chords += lastChord + "  ";
    }
    return chords;
}

void ChordView::resized()
{
    // auto area = getLocalBounds();
    // setBounds(area);
}