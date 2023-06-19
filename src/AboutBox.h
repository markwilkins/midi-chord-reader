
/**
 * @file AboutBox.h
 * @author Mark Wilkins
 * @brief Part of MidiChords project (plugin to display chord names from a MIDI track on playback)
 * @version 0.1
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#pragma once

#include <JuceHeader.h>

/**
 * @brief Provide a set of controls for affecting the behavior of the plugin
 */
class AboutBox : public juce::Component
{
public:
    AboutBox();

private:
    juce::Label title;
    juce::Label version;
    juce::Label myOwnName;
    juce::HyperlinkButton myGitLink;
    juce::Label juceAttribution;
    juce::HyperlinkButton juceLink;

    void paint(juce::Graphics &g) override;
    void resized() override;

};