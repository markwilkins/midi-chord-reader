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
MidiChordsAudioProcessorEditor::MidiChordsAudioProcessorEditor (MidiChordsAudioProcessor& p)
    : AudioProcessorEditor (&p), juce::Timer(), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setResizable(true, true);
    setSize (400, 400);


    //juce::String noteText = "7:49 the note goes here";
    //currentNote.setText(noteText, juce::NotificationType::sendNotification);
    currentTime.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
    currentTimestamp.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
    currentNote.setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);

    // this function adds the slider to the editor
    addAndMakeVisible(&currentTime);
    addAndMakeVisible(&currentTimestamp);
    addAndMakeVisible(&currentNote);

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
    currentNote.setText(lastChord, juce::NotificationType::sendNotification);
    std::vector<int64> eventTimes = ms->getEventTimes();
    for (std::vector<int64>::iterator i = eventTimes.begin(); i != eventTimes.end(); ++i) 
    {
        info += std::to_string(*i) + ", ";
    }
    // currentNote.setText(info, juce::NotificationType::sendNotification);


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
}