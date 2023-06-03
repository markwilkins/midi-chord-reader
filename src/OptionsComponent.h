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

/**
 * @brief Provide a set of controls for affecting the behavior of the plugin
 */
class OptionsComponent : public juce::Component
{
public:
    OptionsComponent(MidiStore&);

    void resetClick();

    void recordingClick(bool state);

private:
    juce::GroupComponent propsPanel;
    juce::TextButton resetChords;
    juce::ToggleButton recordingOn;
    void paint(juce::Graphics &g) override;
    MidiStore &midiState;

    void resized() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsComponent)
};
