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
#include "AboutBox.h"

using namespace juce;

OptionsComponent::OptionsComponent(MidiStore &ms) : midiState(ms)
{
    propsPanel.setColour(juce::GroupComponent::ColourIds::outlineColourId, juce::Colours::darkgrey);

    addAndMakeVisible(propsPanel);
    getLookAndFeel().setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);

    // juce tutorial of interest:  https://docs.juce.com/master/tutorial_slider_values.html
    propsPanel.addAndMakeVisible(&positionOfPlayheadSlider);
    setSliderColors(positionOfPlayheadSlider);
    positionOfPlayheadSlider.setHelpText("This is my help text. I hope it helps");
    positionOfPlayheadSlider.setRange(1.0, 99.0, 1.0);
    positionOfPlayheadSlider.setTextValueSuffix(" %");
    playheadLabel.attachToComponent(&positionOfPlayheadSlider, true);
    propsPanel.addAndMakeVisible(playheadLabel);
    playheadLabel.setText("Playhead", juce::dontSendNotification);
    

    // The number of seconds represented by the window
    propsPanel.addAndMakeVisible(&timeWidthSlider);
    setSliderColors(timeWidthSlider);
    timeWidthSlider.setRange(4.0, 30.0, 1.0);
    timeWidthSlider.setTextValueSuffix(" sec");
    timeWidthLabel.attachToComponent(&timeWidthSlider, true);
    propsPanel.addAndMakeVisible(timeWidthLabel);
    timeWidthLabel.setText("View Seconds", juce::dontSendNotification);

    // Minimum width (in seconds) for chords to display
    propsPanel.addAndMakeVisible(&shortChordSlider);
    setSliderColors(shortChordSlider);
    shortChordSlider.setRange(0.0, 2.0, 0.01);
    shortChordSlider.setTextValueSuffix(" sec");
    shortChordLabel.attachToComponent(&shortChordSlider, true);
    propsPanel.addAndMakeVisible(shortChordLabel);
    shortChordLabel.setText("Minimum chord", juce::dontSendNotification);

    // Size of the chord name font
    propsPanel.addAndMakeVisible(&chordFontSizeSlider);
    setSliderColors(chordFontSizeSlider);
    chordFontSizeSlider.setRange(5.0, 50.0, 1.0);
    chordFontSizeSlider.setTextValueSuffix(" pt");
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

    propsPanel.addAndMakeVisible(&aboutBoxButton);
    aboutBoxButton.setButtonText("About...");
    aboutBoxButton.onClick = [this] { showAboutBox(); };

    this->resized();
}

/**
 * @brief Set the text color of the slider to black and background to white
 * I have something messed up with the looknfeel or something. If I don't do this, the slider text is white, but if I
 * open a second plugin instance (the editor) within the app, then they suddenly change to black on white. This makes
 * them black on white from the get go. dunno ¯\_(ツ)_/¯
 *
 * @param slider
 */
void OptionsComponent::setSliderColors(juce::Slider &slider) 
{
    slider.setColour(Slider::textBoxTextColourId, Colours::black);
    slider.setColour(Slider::textBoxBackgroundColourId, Colours::white);
}

/**
 * @brief Remove the midi events on the midi state object
 */
void OptionsComponent::resetClick()
{
    midiState.clear();
}

/**
 * @brief Update the controls to the current settings in the state tree
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

void OptionsComponent::showAboutBox()
{
    DialogWindow::showDialog("", &aboutBox, nullptr, Colours::white, true, false, false);
}

void OptionsComponent::paint(juce::Graphics &g) // override
{
    propsPanel.setColour(juce::GroupComponent::ColourIds::outlineColourId, juce::Colours::lightgrey);
    getLookAndFeel().setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    g.fillAll(juce::Colours::lightgrey);
    g.setColour(juce::Colours::black);
}

void OptionsComponent::resized() // override
{
    auto area = getLocalBounds();
    int controlHeight = 40;
    int buttonHeight = 30;
    int column;

    propsPanel.setBounds(area);
    column = timeWidthLabel.getWidth() + 60;
    positionOfPlayheadSlider.setBounds(column, area.getHeight() / 3 - controlHeight / 2, 200, controlHeight);
    timeWidthSlider.setBounds(column, area.getHeight() * 2 / 3 - controlHeight / 2, 200, controlHeight);

    column += positionOfPlayheadSlider.getBounds().getWidth() + 125;
    shortChordSlider.setBounds(column, area.getHeight() / 3 - controlHeight / 2, 200, controlHeight);
    chordFontSizeSlider.setBounds(column, area.getHeight() * 2 / 3 - controlHeight / 2, 200, controlHeight);

    column = shortChordSlider.getBounds().getTopLeft().getX() + shortChordSlider.getBounds().getWidth() + 30;
    recordingOnToggle.setBounds(column, area.getHeight() / 3 - controlHeight / 2, 100, controlHeight);
    resetChordsButton.setBounds(column, area.getHeight() * 2 / 3 - buttonHeight / 2, 100, buttonHeight);

    // this "sticks" the about... button to the right hand side
    column = area.getWidth() - 125;
    aboutBoxButton.setBounds(column, area.getHeight() * 2 / 3 - buttonHeight / 2, 100, buttonHeight);
}