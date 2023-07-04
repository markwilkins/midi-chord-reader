/**
 * @file AboutBox.cpp
 * @author Mark Wilkins
 * @brief Part of MidiChords project (plugin to display chord names from a MIDI track on playback)
 * @version 0.8.1
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "AboutBox.h"
#include "MidiChordsConfig.h"

using namespace juce;
using namespace std;

AboutBox::AboutBox()
{
    setSize(400, 200);
    addAndMakeVisible(title);
    title.setText("MidiChords", juce::dontSendNotification);
    title.setFont(juce::Font(16.0f, juce::Font::bold));
    title.setJustificationType(juce::Justification::centred);

    string s = "version: " + to_string(MidiChords_VERSION_MAJOR);
    ostringstream ossVersion;
    ossVersion << "Version " << MidiChords_VERSION_MAJOR << "." << MidiChords_VERSION_MINOR << "." << MidiChords_VERSION_PATCH;
    string versionStr = ossVersion.str();
    addAndMakeVisible(version);
    version.setText(versionStr, juce::dontSendNotification);
    version.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(myOwnName);

    myOwnName.setText(String(CharPointer_UTF8("Copyright \xc2\xa9 2023, Mark Wilkins")), juce::dontSendNotification);
    myOwnName.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(myGitLink);
    myGitLink.setButtonText("https://github.com/markwilkins/midi-chord-reader");
    myGitLink.setURL(URL("https://github.com/markwilkins/midi-chord-reader"));

    addAndMakeVisible(juceAttribution);
    juceAttribution.setText("Built with JUCE", juce::dontSendNotification);
    juceAttribution.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(juceLink);
    juceLink.setButtonText("https://juce.com");
    juceLink.setURL(URL("https://juce.com/"));
}

void AboutBox::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::lightgrey);
}

void AboutBox::resized() // override
{
    auto area = getLocalBounds();

    int yPos = 10;
    title.setBounds(0, yPos, area.getWidth(), 40);
    yPos += title.getHeight() + 5;
    version.setBounds(0, yPos, area.getWidth(), 20);
    yPos += version.getHeight() + 5;
    myOwnName.setBounds(0, yPos, area.getWidth(), 20);
    yPos += version.getHeight() + 2;
    myGitLink.setBounds(0, yPos, area.getWidth(), 20);
    yPos += version.getHeight() + 5;
    juceAttribution.setBounds(0, yPos, area.getWidth(), 20);
    yPos += version.getHeight() + 2;
    juceLink.setBounds(0, yPos, area.getWidth(), 20);
    yPos += version.getHeight() + 5;


}