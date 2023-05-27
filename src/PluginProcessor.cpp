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
    return 0;
}

void MidiChordsAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MidiChordsAudioProcessor::getProgramName (int index)
{
    return {};
}

void MidiChordsAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MidiChordsAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
      
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    DBG("prepareToPlay called");
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
    int64 posOfBlock = currentPlayheadPosition();

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        
        int noteNumber = message.getNoteNumber();
        lastNote = message.getMidiNoteName(noteNumber, true, false, 4);
        auto globalPosOfEvent = (int64)message.getTimeStamp() + posOfBlock;
        if (message.isNoteOn())
        {
            currentNotes.insert(lastNote);
            lastEventTime = metadata.samplePosition;
            lastEventTimestamp = globalPosOfEvent; // message.getTimeStamp();
            DBG("Note on: " + message.getDescription() + " at time " + std::to_string(lastEventTime));
            referenceTrack.addNoteEventAtTime(globalPosOfEvent, noteNumber, true);
        }
        else if (message.isNoteOff()) 
        {
            // mlwtbd TODO: is the isMidiStop event included in this? This would let me know if this event is a "true"
            // note off event versus one generated by stopping playback
            std::unordered_set<juce::String>::iterator pos = currentNotes.find(lastNote);
            if (pos != currentNotes.end()) {
                currentNotes.erase(pos);
            }
            DBG("Note off: " + message.getDescription());
            referenceTrack.addNoteEventAtTime(globalPosOfEvent, noteNumber, false);
        }
        else if (message.isMidiClock())
        {
            DBG("Midi clock event");
        }
        else if (message.isMidiStart())
        {
            DBG("Midi start event");
        }
        else if (message.isMidiStop())
        {
            DBG("Midi stop event");
        }
        else
        {
            double ts = message.getTimeStamp();
            message.getDescription();
            //DBG("note not on or off " + std::to_string(ts));
            //DBG("Other type: " + message.getDescription());

        }
        //juce::String raw = String::toHexString(message.getRawData(), message.getRawDataSize());
        juce::String raw = String::toHexString(message.getSysExData(), message.getSysExDataSize());
        DBG("Raw: " + raw);
    }
    // midiMessages.clear(0, 1000);

}

/**
 * @brief Retrieve the current playhead position in time
 * This is from code provided in response to my question about how to find this:
 * https://forum.juce.com/t/processblock-sampleposition-gettimestamp-interpretation/56172/3?u=tetrachord
 *
 * @return int64
 */
int64 MidiChordsAudioProcessor::currentPlayheadPosition()
{
    int64 currentTime = 0;
    if (auto *playHead = getPlayHead())
        if (auto position = playHead->getPosition())
            if (auto samplePos = position->getTimeInSamples())
                currentTime = *samplePos;

    return currentTime;
}

//==============================================================================
bool MidiChordsAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MidiChordsAudioProcessor::createEditor()
{
    return new MidiChordsAudioProcessorEditor (*this);
}

//==============================================================================
void MidiChordsAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MidiChordsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MidiChordsAudioProcessor();
}