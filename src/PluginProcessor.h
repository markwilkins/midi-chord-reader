/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MidiStore.h"

using std::unordered_set;

//==============================================================================
/**
*/
class MidiChordsAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    MidiChordsAudioProcessor();
    ~MidiChordsAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;


    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    juce::String lastNote;
    int lastEventTime = 0;
    int64 lastEventTimestamp = 0;
    std::unordered_set<juce::String> currentNotes;

    MidiStore* getReferenceTrack() { return &referenceTrack; }

private:
    MidiStore referenceTrack;
    pair<int64, double> currentPlayheadPosition();

    // variables for some of the pluginprocessor things I don't need yet
    int currentProgram = 0;
    juce::String programName = "";
    double currentSampleRate = 0.0;
    int currentSamplesPerBlock;
    juce::MemoryBlock *stateInfo = nullptr;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiChordsAudioProcessor)
};
