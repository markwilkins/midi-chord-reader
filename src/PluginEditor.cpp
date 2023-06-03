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
    setResizable(true, true);
    setSize (600, 400);


    lastTimeStamp.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
    lastChordSeen.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
    currentChords.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);

    addAndMakeVisible(&lastTimeStamp);
    addAndMakeVisible(&lastChordSeen);
    addAndMakeVisible(&currentChords);
    addAndMakeVisible(&options);
    addAndMakeVisible(&chordView);

    Timer::startTimer(1000);
}

MidiChordsAudioProcessorEditor::~MidiChordsAudioProcessorEditor()
{
}

void MidiChordsAudioProcessorEditor::timerCallback()
{
    juce::String text;
    std::unordered_set<juce::String>::iterator it;
    text = "8:26 last: " + audioProcessor.lastNote + ": ";
    for (it = audioProcessor.currentNotes.begin(); it != audioProcessor.currentNotes.end(); it++) {
        text = text + " " + *it;
    }
    
    juce::String lastTime = std::to_string(audioProcessor.lastEventTime);
    juce::String lastTimestampValue = std::to_string((int64)(audioProcessor.lastEventTimestamp));
    lastTimeStamp.setText("lasttime: " + lastTimestampValue, juce::NotificationType::sendNotification);
    repaint();

    std::string info = "";
    MidiStore *ms = audioProcessor.getReferenceTrack();
    vector<int> currentNotes = ms->getAllNotesOnAtTime(0, audioProcessor.lastEventTimestamp);
    ChordName cn;
    string lastChord = cn.nameChord(currentNotes);
    lastChordSeen.setText("latest Chord: " + lastChord, juce::NotificationType::sendNotification);
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
    lastChordSeen.setBounds(10, 40, 100, 30);
    currentChords.setBounds(10, 70, getWidth() - 10, 60);
    options.setBounds(0, getHeight() - 100, getWidth(), 100);
    chordView.setBounds(0, getHeight() - 200, getWidth(), 100);
}
