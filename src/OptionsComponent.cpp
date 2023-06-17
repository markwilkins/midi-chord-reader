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

    addAndMakeVisible(propsPanel);

    // juce tutorial of interest:  https://docs.juce.com/master/tutorial_slider_values.html
    propsPanel.addAndMakeVisible(&positionOfPlayheadSlider);
    positionOfPlayheadSlider.setRange(1.0, 99.0, 1.0);
    positionOfPlayheadSlider.setTextValueSuffix(" %");
    playheadLabel.attachToComponent(&positionOfPlayheadSlider, true);
    propsPanel.addAndMakeVisible(playheadLabel);
    playheadLabel.setText("Playhead", juce::dontSendNotification);

    // The number of seconds represented by the window
    propsPanel.addAndMakeVisible(&timeWidthSlider);
    timeWidthSlider.setRange(4.0, 30.0, 1.0);
    timeWidthSlider.setTextValueSuffix(" sec");
    timeWidthLabel.attachToComponent(&timeWidthSlider, true);
    propsPanel.addAndMakeVisible(timeWidthLabel);
    timeWidthLabel.setText("View Seconds", juce::dontSendNotification);

    // Minimum width (in seconds) for chords to display
    propsPanel.addAndMakeVisible(&shortChordSlider);
    shortChordSlider.setRange(0.0, 2.0, 0.1);
    shortChordSlider.setTextValueSuffix(" sec");
    shortChordLabel.attachToComponent(&shortChordSlider, true);
    propsPanel.addAndMakeVisible(shortChordLabel);
    shortChordLabel.setText("Minimum chord", juce::dontSendNotification);

    // Size of the chord name font
    propsPanel.addAndMakeVisible(&chordFontSizeSlider);
    chordFontSizeSlider.setRange(5.0, 50.0, 1.0);
    chordFontSizeSlider.setTextValueSuffix(" font");
    chordFontSizeLabel.attachToComponent(&chordFontSizeSlider, true);
    propsPanel.addAndMakeVisible(chordFontSizeLabel);
    chordFontSizeLabel.setText("Font size", juce::dontSendNotification);

    recordingOnToggle.setButtonText("Record Notes");
    propsPanel.addAndMakeVisible(&recordingOnToggle);
    resetChordsButton.setButtonText("Clear Notes!");
    propsPanel.addAndMakeVisible(&resetChordsButton);

    // click handler lambdas
    resetChordsButton.onClick = [this] { resetClick(); };
    recordingOnToggle.onStateChange = [this] { recordingClick(recordingOnToggle.getToggleState()); };
    recordingOnToggle.setToggleState(ms.getRecordingState(), juce::sendNotification);

    positionOfPlayheadSlider.onValueChange = [this] { adjustPositionPlayhead(positionOfPlayheadSlider.getValue()); };
    positionOfPlayheadSlider.setValue(ms.getPlayHeadPosition(), juce::sendNotification);

    timeWidthSlider.onValueChange = [this] { adjustTimeWidth(timeWidthSlider.getValue()); };
    timeWidthSlider.setValue(ms.getTimeWidth(), juce::sendNotification);

    shortChordSlider.onValueChange = [this] { adjustShortChordThreshold(shortChordSlider.getValue()); };
    shortChordSlider.setValue(ms.getShortChordThreshold(), juce::sendNotification);

    chordFontSizeSlider.onValueChange = [this] { adjustChordFontSize(chordFontSizeSlider.getValue()); };
    chordFontSizeSlider.setValue(ms.getChordNameSize(), juce::sendNotification);

    this->resized();
}

/**
 * @brief Remove the midi events on the midi state object
 */
void OptionsComponent::resetClick()
{
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

void OptionsComponent::adjustShortChordThreshold(double value)
{
    midiState.setShortChordThreshold(static_cast<float>(value));
}

void OptionsComponent::adjustChordFontSize(double value)
{
    midiState.setChordNameSize(static_cast<float>(value));
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
    column = timeWidthLabel.getWidth() + 60;
    positionOfPlayheadSlider.setBounds(column, area.getHeight() / 3 - buttonHeight / 2, 200, buttonHeight);
    timeWidthSlider.setBounds(column, area.getHeight() * 2 / 3 - buttonHeight / 2, 200, buttonHeight);

    column += positionOfPlayheadSlider.getBounds().getWidth() + 150;
    shortChordSlider.setBounds(column, area.getHeight() / 3 - buttonHeight / 2, 200, buttonHeight);
    chordFontSizeSlider.setBounds(column, area.getHeight() * 2 / 3 - buttonHeight / 2, 200, buttonHeight);

    // put these on the right hand side
    column = area.getWidth() - recordingOnToggle.getWidth() - 40;
    recordingOnToggle.setBounds(column, area.getHeight() / 3 - buttonHeight / 2, 100, buttonHeight);
    resetChordsButton.setBounds(column, area.getHeight() * 2 / 3 - buttonHeight / 2, 100, buttonHeight);
}