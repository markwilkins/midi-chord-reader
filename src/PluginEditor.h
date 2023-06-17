/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "OptionsComponent.h"
#include "ChordView.h"

//==============================================================================
/**
*/
class MidiChordsAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                            public juce::Timer
{
public:
    MidiChordsAudioProcessorEditor (MidiChordsAudioProcessor&, MidiStore&);
    ~MidiChordsAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MidiChordsAudioProcessor& audioProcessor;

    // some debug info placeholders
    juce::Label currentChords;
    juce::Label lastTimeStamp;
    juce::Label debugInfo1;

    OptionsComponent options;
    ChordView chordView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiChordsAudioProcessorEditor)
};
