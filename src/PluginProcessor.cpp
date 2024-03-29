/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace std;
using std::unordered_set;

//==============================================================================
MidiChordsAudioProcessor::MidiChordsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

MidiChordsAudioProcessor::~MidiChordsAudioProcessor()
{
}

//==============================================================================
const juce::String MidiChordsAudioProcessor::getName() const
{
    return "Midi Chord Reader";
}

bool MidiChordsAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MidiChordsAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MidiChordsAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MidiChordsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MidiChordsAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MidiChordsAudioProcessor::getCurrentProgram()
{
    return this->currentProgram;;
}

void MidiChordsAudioProcessor::setCurrentProgram (int index)
{
    this->currentProgram = index;
}

const juce::String MidiChordsAudioProcessor::getProgramName (int index)
{
    if (index == 0) 
        return this->programName;
    else
        return {};
}

void MidiChordsAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    if (index == 0) 
        this->programName = newName;
}

//==============================================================================
void MidiChordsAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
      
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    DBG("prepareToPlay called");
    this->currentSampleRate = sampleRate;
    this->currentSamplesPerBlock = samplesPerBlock;
}

void MidiChordsAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MidiChordsAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MidiChordsAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    pair<int64, double> posOfBlock = currentPlayheadPosition();
    AudioPlayHead::CurrentPositionInfo info;

    if (auto positionInfo = getPlayHead()->getPosition()) 
    {
        this->midiState.setIsPlaying(positionInfo->getIsPlaying());

        if (auto bpMeasure = positionInfo->getTimeSignature())
            this->midiState.setBPMeasure(bpMeasure->numerator);
        if (auto bpMinute = positionInfo->getBpm())
            this->midiState.setBPMinute(*bpMinute);
    }

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        
        int noteNumber = message.getNoteNumber();
        auto messageEventTime = (int64)message.getTimeStamp() + posOfBlock.first;
        if (message.isNoteOn())
        {
            midiState.addNoteEventAtTime(messageEventTime, noteNumber, true);
            // mlwtbd - Store the current time in seconds that "might be" associated with this event.
            // However this value is the current position of the playhead ... and we are offsetting the
            // actual event time by the metadata.samplePosition. There is the concept of:
            // MidiFile::convertTimestampTicksToSeconds; but this isn't a midi file so I don't have that
            // context. Observation shows that the behavior is as desired. But not sure it will always
            // work that way. Need to keep an eye on it.
            // Maybe the samplesPerBlock from PrepareToPlay would give me that info?
            midiState.setEventTimeSeconds(messageEventTime, posOfBlock.second);
        }
        if (message.isNoteOff()) 
        {
            // DBG("Note off: " + message.getDescription());
            midiState.addNoteEventAtTime(messageEventTime, noteNumber, false);
            midiState.setEventTimeSeconds(messageEventTime, posOfBlock.second);
        }
        //juce::String raw = String::toHexString(message.getRawData(), message.getRawDataSize());
        //juce::String raw = String::toHexString(message.getSysExData(), message.getSysExDataSize());
        //DBG("Raw: " + raw);
    }

}

/**
 * @brief Retrieve the current playhead position in time
 * This is from code provided in response to my question about how to find this:
 * https://forum.juce.com/t/processblock-sampleposition-gettimestamp-interpretation/56172/3?u=tetrachord
 *
 * This also updates the most recently seen time values in the reference track for use by the display
 * 
 * @return pair<int64, double> event time and time in seconds
 */
pair<int64, double> MidiChordsAudioProcessor::currentPlayheadPosition()
{
    int64 currentTime = 0;
    double timeInSeconds = 0.0;
    if (auto *playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
        {
            if (auto samplePos = position->getTimeInSamples())
            {
                currentTime = *samplePos;
                midiState.setLastEventTime(currentTime);
            }
            if (auto seconds = position->getTimeInSeconds())
            {
                timeInSeconds = *seconds;
                midiState.setLastEventTimeInSeconds(*seconds);
            }
        }
    }

    return {currentTime, timeInSeconds};
}

//==============================================================================
bool MidiChordsAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MidiChordsAudioProcessor::createEditor()
{
    return new MidiChordsAudioProcessorEditor (*this, midiState);
}

//==============================================================================
void MidiChordsAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    ValueTree &vt = this->midiState.getState();
    std::unique_ptr<juce::XmlElement> xml(vt.createXml());
    copyXmlToBinary(*xml, destData);
}

void MidiChordsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        auto restored = juce::ValueTree::fromXml(*xmlState);
        this->midiState.replaceState(restored);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MidiChordsAudioProcessor();
}
