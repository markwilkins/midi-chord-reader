/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ChordName.h"
#include <iostream>
#include <sstream>
using namespace std;

using std::unordered_set;

//==============================================================================
MidiChordsAudioProcessorEditor::MidiChordsAudioProcessorEditor (MidiChordsAudioProcessor& p, MidiStore& ms)
    : AudioProcessorEditor (&p), juce::Timer(), audioProcessor (p), options(ms), chordView(ms)
{
    getLookAndFeel().setDefaultLookAndFeel(&lookAndFeel);
    setResizable(true, true);
    // To see the debug controls, make the height 400 here
    setSize (1000, 210);


    lastTimeStamp.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
    debugInfo1.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
    currentChords.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);

    addAndMakeVisible(&lastTimeStamp);
    addAndMakeVisible(&debugInfo1);
    addAndMakeVisible(&currentChords);
    addAndMakeVisible(&options);
    addAndMakeVisible(&chordView);


    Timer::startTimer(500);
}

MidiChordsAudioProcessorEditor::~MidiChordsAudioProcessorEditor()
{
}

void MidiChordsAudioProcessorEditor::timerCallback()
{
    // some debug stuff that is hidden unless debugInfoSize is adjusted below
    juce::String text;
    std::unordered_set<juce::String>::iterator it;
    text = "8:26 last: " + audioProcessor.lastNote + ": ";
    for (it = audioProcessor.currentNotes.begin(); it != audioProcessor.currentNotes.end(); it++) {
        text = text + " " + *it;
    }

    // The setDirty method for vst3 has this bit of code for setting dirty. I don't think it works, but leaving
    // it here for further testing when I feel like it. If I figure out that it does work, then I need to set
    // this when actual changes occur (e.g., when isViewUpToDate is set to false and for props like allowStateChange)
    // this->audioProcessor.updateHostDisplay(AudioPluginInstance::ChangeDetails{}.withNonParameterStateChanged(true));

    juce::String lastTime = std::to_string(audioProcessor.lastEventTime);
    juce::String lastTimestampValue = std::to_string((int64)(audioProcessor.lastEventTimestamp));
    lastTimeStamp.setText("lasttime: " + lastTimestampValue, juce::NotificationType::sendNotification);

    // This is very cheesy - refresh the control settings in case the state has changed. I need to figure
    // out the listener stuff better. But there are very few controls, so not expensive
    // Current thinking for better solution:
    //  - Add a new child valuetree in the main state tree that stores prop values (e.g., scroll view window width, playhead position)
    //  - Put a broadcaster on that tree; then it should be efficient and not trigger for any of the other tree changes (e.g., new notes)
    //  - Add a listener for those state changes and call the refreshControlState
    //  - The tricky bit is that I blow away the entire state tree on the load of the state (in PluginProcessor.cpp). I would
    //    need to somehow reestablish that listener. Or maybe just move that props child tree to the new tree. A valuetree can
    //    have only one parent, so I suspect that is a simple operation.
    options.refreshControlState();


    MidiStore *ms = audioProcessor.getMidiState();
    ms->updateStaticViewIfOutOfDate();

    debugInfo1.setText("visible chord count: " + std::to_string(ms->getViewWindowChordCount()), juce::NotificationType::sendNotification);

    /*
    std::string info = "";
    std::vector<int64> eventTimes = ms->getEventTimes();
    for (std::vector<int64>::iterator i = eventTimes.begin(); i != eventTimes.end(); ++i) 
    {
        // info += std::to_string(*i) + ", ";
        vector<int> itNotes = ms->getAllNotesOnAtTime(0, *i);
        lastChord = cn.nameChord(itNotes);
        if (lastChord != "") 
        {
            double seconds = ms->getEventTimeInSeconds(*i);
            ostringstream oss;
            oss << info << lastChord << " (" << *i << ", " << seconds << "), ";
            info = oss.str();
        }
    }
    currentChords.setText("all chords: " + info, juce::NotificationType::sendNotification);
    */


}



//==============================================================================
void MidiChordsAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll(juce::Colours::white);

    g.setColour (juce::Colours::black);
    g.setFont (15.0f);
}

void MidiChordsAudioProcessorEditor::resized()
{
    // sets the position and size of the slider with arguments (x, y, width, height)
    lastTimeStamp.setBounds(10, 10, 100, 30);
    debugInfo1.setBounds(10, 40, getWidth() - 10, 30);
    currentChords.setBounds(10, 70, getWidth() - 10, 60);
    // Make this bigger to be able to see the debug info. Probably will remove this
    // stuff at some point
    int debugInfoSize = 0;

    options.setBounds(0, getHeight() - 100, getWidth(), 100);
    chordView.setBounds(0, debugInfoSize + 10, getWidth(), getHeight() - (debugInfoSize + 120));
}
