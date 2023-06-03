/**
 * @file OptionsComponent.cpp
 * @author Mark Wilkins
 * @brief Part of MidiChords project (plugin to display chord names from a MIDI track on playback)
 * @version 0.1
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#include "OptionsComponent.h"

using namespace juce;

OptionsComponent::OptionsComponent(MidiStore &ms) : midiState(ms)
{
    propsPanel.setColour(juce::GroupComponent::ColourIds::outlineColourId, juce::Colours::darkgrey);
    propsPanel.addChildComponent(&resetChords);
    addAndMakeVisible(propsPanel);
    resetChords.setButtonText("Clear Notes!");
    propsPanel.addAndMakeVisible(&resetChords);
    recordingOn.setButtonText("Record Notes");
    propsPanel.addAndMakeVisible(&recordingOn);

    // click handler lambdas
    resetChords.onClick = [this] { resetClick(); };
    recordingOn.onStateChange = [this] {recordingClick(recordingOn.getToggleState()); };
    recordingOn.setToggleState(ms.getRecordingState(), juce::sendNotification);
}

/**
 * @brief Remove the midi events on the midi state object
 */
void OptionsComponent::resetClick()
{
    DBG("Reset click happened");
    midiState.clear();
}

void OptionsComponent::recordingClick(bool state)
{
    midiState.allowStateChange(state);
}

void OptionsComponent::paint(juce::Graphics &g) // override
{
    g.fillAll(juce::Colours::lightgrey);
}

void OptionsComponent::resized() // override
{
    auto area = getLocalBounds();
    int  buttonHeight = 40;

    propsPanel.setBounds(area);
    resetChords.setBounds(20, area.getHeight() / 2 - buttonHeight / 2, 100, buttonHeight);
    recordingOn.setBounds(resetChords.getBounds().getWidth() + 40, area.getHeight() / 2 - buttonHeight / 2, 100, buttonHeight);

}