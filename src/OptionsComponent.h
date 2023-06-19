/**
 * @file OptionsComponent.h
 * @author Mark Wilkins
 * @brief Part of MidiChords project (plugin to display chord names from a MIDI track on playback)
 * @version 0.1
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#pragma once

#include <JuceHeader.h>
#include "MidiStore.h"
#include "AboutBox.h"

/**
 * @brief Provide a set of controls for affecting the behavior of the plugin
 */
class OptionsComponent : public juce::Component
{
public:
    OptionsComponent(MidiStore&);

    void resetClick();

    void adjustPositionPlayhead(double value);
    void adjustTimeWidth(double value);
    void adjustShortChordThreshold(double value);
    void adjustChordFontSize(double value);
    void showAboutBox();

    void recordingClick(bool state);


    void refreshControlState();

private:
    MidiStore &midiState;
    juce::GroupComponent propsPanel;
    juce::TextButton resetChordsButton;
    juce::ToggleButton recordingOnToggle;
    juce::Label playheadLabel;
    juce::Slider positionOfPlayheadSlider;
    juce::Label timeWidthLabel;
    juce::Slider timeWidthSlider;
    juce::Label shortChordLabel;
    juce::Slider shortChordSlider;
    juce::Label chordFontSizeLabel;
    juce::Slider chordFontSizeSlider;
    juce::TextButton aboutBoxButton;
    AboutBox aboutBox;

    void paint(juce::Graphics &g) override;
    void resized() override;
    void setSliderColors(juce::Slider &slider);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsComponent)
};
