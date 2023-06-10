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
    // propsPanel.addChildComponent(&resetChordsButton);
    addAndMakeVisible(propsPanel);
    recordingOnToggle.setButtonText("Record Notes");
    propsPanel.addAndMakeVisible(&recordingOnToggle);
    resetChordsButton.setButtonText("Clear Notes!");
    propsPanel.addAndMakeVisible(&resetChordsButton);

    // juce tutorial of interest:  https://docs.juce.com/master/tutorial_slider_values.html
    propsPanel.addAndMakeVisible(&positionOfPlayheadSlider);
    positionOfPlayheadSlider.setRange(1.0, 99.0, 1.0);
    positionOfPlayheadSlider.setTextValueSuffix(" %");

    // The number of seconds represented by the window
    propsPanel.addAndMakeVisible(&timeWidthSlider);
    timeWidthSlider.setRange(4.0, 30.0, 1.0);
    timeWidthSlider.setTextValueSuffix(" sec");

    // click handler lambdas
    resetChordsButton.onClick = [this] { resetClick(); };
    recordingOnToggle.onStateChange = [this] { recordingClick(recordingOnToggle.getToggleState()); };
    recordingOnToggle.setToggleState(ms.getRecordingState(), juce::sendNotification);

    positionOfPlayheadSlider.onValueChange = [this] { adjustPositionPlayhead(positionOfPlayheadSlider.getValue()); };
    positionOfPlayheadSlider.setValue(ms.getPlayHeadPosition(), juce::sendNotification);
    playheadLabel.attachToComponent(&positionOfPlayheadSlider, true);
    addAndMakeVisible(playheadLabel);
    playheadLabel.setText("Playhead", juce::dontSendNotification);

    timeWidthSlider.onValueChange = [this] { adjustTimeWidth(timeWidthSlider.getValue()); };
    timeWidthSlider.setValue(ms.getTimeWidth(), juce::sendNotification);
    timeWidthLabel.attachToComponent(&timeWidthSlider, true);
    addAndMakeVisible(timeWidthLabel);
    timeWidthLabel.setText("View Seconds", juce::dontSendNotification);
}

/**
 * @brief Remove the midi events on the midi state object
 */
void OptionsComponent::resetClick()
{
    DBG("Reset click happened");
    midiState.clear();
}

/**
 * @brief Update the controls to the current settings in the state tre
 */
void OptionsComponent::refreshControlState()
{
    recordingOnToggle.setToggleState(midiState.getRecordingState(), juce::sendNotification);
    positionOfPlayheadSlider.setValue(midiState.getPlayHeadPosition(), juce::sendNotification);
    timeWidthSlider.setValue(midiState.getTimeWidth(), juce::sendNotification);
}

// Handlers for changes in the settings (update the state tree with the info)
void OptionsComponent::adjustPositionPlayhead(double value)
{
    midiState.setPlayHeadPosition(static_cast<float>(value));
}

void OptionsComponent::adjustTimeWidth(double value)
{
    midiState.setTimeWidth(static_cast<float>(value));
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
    int buttonHeight = 40;
    int column;

    propsPanel.setBounds(area);
    // recordingOnToggle.setBounds(resetChordsButton.getBounds().getWidth() + 40, area.getHeight() / 2 - buttonHeight / 2, 100, buttonHeight);
    recordingOnToggle.setBounds(20, area.getHeight() / 3 - buttonHeight / 2, 100, buttonHeight);
    resetChordsButton.setBounds(20, area.getHeight() * 2 / 3 - buttonHeight / 2, 100, buttonHeight);

    column = resetChordsButton.getBounds().getWidth() + 150;
    positionOfPlayheadSlider.setBounds(column, area.getHeight() / 3 - buttonHeight / 2, 200, buttonHeight);
    timeWidthSlider.setBounds(column, area.getHeight() * 2 / 3 - buttonHeight / 2, 200, buttonHeight);
}