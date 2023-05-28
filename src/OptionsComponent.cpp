
#include "OptionsComponent.h"

using namespace juce;

OptionsComponent::OptionsComponent(MidiStore &ms) : midiState(ms)
{
    propsPanel.setColour(juce::GroupComponent::ColourIds::outlineColourId, juce::Colours::darkgrey);
    propsPanel.addChildComponent(&resetChords);
    addAndMakeVisible(propsPanel);
    resetChords.setButtonText("Reset!");
    propsPanel.addAndMakeVisible(&resetChords);
    resetChords.onClick = [this] { resetClick(); };
}

/**
 * @brief Remove the midi events on the midi state object
 */
void OptionsComponent::resetClick()
{
    DBG("Reset click happened");
    midiState.clear();
}

void OptionsComponent::paint(juce::Graphics &g) // override
{
    g.fillAll(juce::Colours::lightgrey);
}

void OptionsComponent::resized() // override
{
    auto area = getLocalBounds();
    int  buttonHeight = 40;

    propsPanel.setBounds(area);
    resetChords.setBounds(20, area.getHeight() / 2 - buttonHeight / 2, 80, buttonHeight);

}