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
    setSize (1000, 210);

    addAndMakeVisible(&options);
    addAndMakeVisible(&chordView);


    Timer::startTimer(500);
}

MidiChordsAudioProcessorEditor::~MidiChordsAudioProcessorEditor()
{
}

void MidiChordsAudioProcessorEditor::timerCallback()
{

    // The setDirty method for vst3 has this bit of code for setting dirty. I don't think it works, but leaving
    // it here for further testing when I feel like it. If I figure out that it does work, then I need to set
    // this when actual changes occur (e.g., when isViewUpToDate is set to false and for props like allowStateChange)
    // this->audioProcessor.updateHostDisplay(AudioPluginInstance::ChangeDetails{}.withNonParameterStateChanged(true));


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
    options.setBounds(0, getHeight() - 100, getWidth(), 100);
    chordView.setBounds(0, 10, getWidth(), getHeight() - 120);
}
