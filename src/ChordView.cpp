/**
 * @file ChordView.cpp
 * @author Mark Wilkins
 * @brief Part of MidiChords project (plugin to display chord names from a MIDI track on playback)
 * @version 0.1
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#include "ChordView.h"
#include "ChordName.h"
#include "ChordClipper.h"
#include "MidiChordsTypes.h"

using namespace std;
using namespace juce;

ChordView::ChordView(MidiStore &ms) : chordClipper(ms)
{
    setFramesPerSecond(30);
}


void ChordView::update()
{
    // This function is called at the frequency specified by the setFramesPerSecond() call
    // in the constructor. You can use it to update counters, animate values, etc.

    chordClipper.updateCurrentPosition(getMillisecondsSinceLastUpdate());
}

void ChordView::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    getLookAndFeel().setDefaultLookAndFeel(new juce::LookAndFeel_V3());
    getLookAndFeel().setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::white);
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    g.setColour(juce::Colours::black);
    // Need to figure out how to make this thing draw its own border
    g.drawRect(getLocalBounds(), 1);
    MeasurePositionType bars = chordClipper.getMeasuresToDisplay();
    this->drawMeasures(bars, g);

    // Draw the "now" marker
    int x = static_cast<int>(getWidth() * chordClipper.getCurrentNotePosition() / chordClipper.getViewWidthInSeconds());
    g.setColour(juce::Colours::red);
    g.drawVerticalLine(x, 0, getHeight());

    g.setColour(juce::Colours::black);
    ChordVectorType chords = chordClipper.getChordsToDisplay();
    this->drawChords(chords, g);
}


void ChordView::drawMeasures(MeasurePositionType bars, juce::Graphics &g)
{
    auto area = getLocalBounds();
    juce::Rectangle<float> textBox;
    textBox = area.toFloat();
    textBox.setTop(5);
    float ratio = static_cast<float>(getWidth()) / chordClipper.getViewWidthInSeconds();
    g.setFont(15.0);
    for (MeasurePositionType::iterator it = bars.begin(); it != bars.end(); ++it)
    {
        float xPos = it->second * ratio;
        g.drawVerticalLine(static_cast<int>(xPos), 0, getHeight());
        textBox.setLeft(xPos + 10);
        g.drawText(to_string(it->first), textBox, juce::Justification::topLeft);
    }

}

/**
 * @brief Draw the given set of chords onto the graphics area
 * 
 * @param chords   map of chords by time relative to the window
 * @param g 
 */
void ChordView::drawChords(vector<pair<float, string>> chords, juce::Graphics &g)
{
    // map<float, string>::iterator it;
    auto area = getLocalBounds();
    juce::Rectangle<float> textBox;
    textBox = area.toFloat();
    g.setFont(25.0);
    float ratio = textBox.getWidth() / chordClipper.getViewWidthInSeconds();

    for (auto it = chords.begin(); it != chords.end(); ++it)
    {
        float leftPos = it->first * ratio;
        textBox.setLeft(leftPos);
        g.drawText(it->second, textBox, juce::Justification::centredLeft);

    }

}




void ChordView::resized()
{
    // auto area = getLocalBounds();
    // setBounds(area);
}