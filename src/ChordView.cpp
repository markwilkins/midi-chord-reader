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

using namespace std;
using namespace juce;

ChordView::ChordView(MidiStore &ms) : chordClipper(ms)
{
    setFramesPerSecond(60);
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
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    int x = static_cast<int>(getWidth() * chordClipper.getCurrentNotePosition() / chordClipper.getViewWidthInSeconds());
    g.setColour(juce::Colours::red);
    g.drawVerticalLine(x, 0, getHeight());

    g.setColour(getLookAndFeel().findColour(juce::Slider::thumbColourId));
    // map<float, string> chords = this->getChordsToDisplay();
    map<float, string> chords = chordClipper.getChordsToDisplay();
    this->drawChords(chords, g);
}


/**
 * @brief Draw the given set of chords onto the graphics area
 * 
 * @param chords   map of chords by time relative to the window
 * @param g 
 */
void ChordView::drawChords(map<float, string> chords, juce::Graphics &g)
{
    map<float, string>::iterator it;
    auto area = getLocalBounds();
    juce::Rectangle<float> textBox;
    textBox = area.toFloat();
    g.setFont(25.0);
    float ratio = textBox.getWidth() / chordClipper.getViewWidthInSeconds();

    for (it = chords.begin(); it != chords.end(); ++it)
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