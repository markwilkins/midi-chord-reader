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
    : AudioProcessorEditor (&p), juce::Timer(), audioProcessor (p), options(ms)
{
    setResizable(true, true);
    setSize (600, 400);


    currentTime.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
    currentTimestamp.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
    currentNote.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);

    addAndMakeVisible(&currentTime);
    addAndMakeVisible(&currentTimestamp);
    addAndMakeVisible(&currentNote);
    addAndMakeVisible(&options);

    Timer::startTimer(100);
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
    // currentNote.setText(text, juce::NotificationType::sendNotification);

    
    juce::String lastTime = std::to_string(audioProcessor.lastEventTime);
    juce::String lastTimestamp = std::to_string((int64)(audioProcessor.lastEventTimestamp));
    currentTime.setText("lasttime: " + lastTime, juce::NotificationType::sendNotification);
    currentTimestamp.setText("lts: " + lastTimestamp, juce::NotificationType::sendNotification);
    repaint();

    std::string info = "";
    MidiStore *ms = audioProcessor.getReferenceTrack();
    vector<int> currentNotes = ms->getAllNotesOnAtTime(0, audioProcessor.lastEventTimestamp);
    ChordName cn;
    string lastChord = cn.nameChord(currentNotes);
    // currentNote.setText(lastChord, juce::NotificationType::sendNotification);
    currentTimestamp.setText(lastChord, juce::NotificationType::sendNotification);
    std::vector<int64> eventTimes = ms->getEventTimes();
    for (std::vector<int64>::iterator i = eventTimes.begin(); i != eventTimes.end(); ++i) 
    {
        // info += std::to_string(*i) + ", ";
        vector<int> itNotes = ms->getAllNotesOnAtTime(0, *i);
        lastChord = cn.nameChord(itNotes);
        info += lastChord + ", ";
    }
    currentNote.setText(info, juce::NotificationType::sendNotification);


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
    currentTime.setBounds(10, 10, 100, 30);
    currentTimestamp.setBounds(10, 40, 100, 30);
    currentNote.setBounds(10, 50, getWidth() - 10, 100);
    options.setBounds(0, getHeight() - 100, getWidth(), 100);
}
