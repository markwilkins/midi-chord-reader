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

ChordView::ChordView(MidiStore &ms) : chordClipper(ms), midiState(ms)
{
    getLookAndFeel().setDefaultLookAndFeel(&lookAndFeel);
    getLookAndFeel().setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::white);
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

    g.setColour(juce::Colours::black);
    // Need to figure out how to make this thing draw its own border
    g.drawRect(getLocalBounds(), 1);
    MeasurePositionType bars = chordClipper.getMeasuresToDisplay();
    this->drawMeasures(bars, g);

    // Draw the "now" marker
    int x = static_cast<int>(getWidth() * chordClipper.getCurrentNotePosition() / chordClipper.getViewWidthInSeconds());
    g.setColour(juce::Colours::red);
    // double thickness ... draw it twice. Maybe there is a better way, but this works
    g.drawVerticalLine(x, 0, getHeight());
    g.drawVerticalLine(x + 1, 0, getHeight());

    g.setColour(juce::Colours::black);
    ChordVectorType chords = chordClipper.getChordsToDisplay();
    this->drawChords(chords, g);
}


/**
 * @brief Draw the vertical bars to represent the measures
 * 
 * @param bars 
 * @param g 
 */
void ChordView::drawMeasures(MeasurePositionType bars, juce::Graphics &g)
{
    auto area = getLocalBounds();
    juce::Rectangle<float> textBox;
    textBox = area.toFloat();
    // Compute width of hash marks between measurers to mark the beats
    optional<int> bpMeasure = midiState.getBPMeasure();
    float beatWidth = 1.0;
    if (bars.size() > 1) 
        beatWidth = ((bars.begin() + 1)->second - bars.begin()->second) / *bpMeasure;
    

    textBox.setTop(5);
    float ratio = static_cast<float>(getWidth()) / chordClipper.getViewWidthInSeconds();
    beatWidth *= ratio;
    g.setFont(15.0);
    for (MeasurePositionType::iterator it = bars.begin(); it != bars.end(); ++it)
    {
        float xPos = it->second * ratio;
        g.drawVerticalLine(static_cast<int>(xPos), 0, getHeight());
        // hash marks for beats in the measure
        for (int i = 1; i < *bpMeasure; i++)
            g.drawVerticalLine(static_cast<int>(xPos + i * beatWidth), 0, 20);

        // draw in measure number
        textBox.setLeft(xPos + 5);
        g.drawText(to_string(it->first), textBox, juce::Justification::topLeft);
    }

    // Hack - draw beat hashes for the first bar where the measure bar is not visible
    if (bars.begin() != bars.end()) 
    {
        float xPos = bars.begin()->second * ratio;
        for (int i = 1; i < *bpMeasure; i++)
            g.drawVerticalLine(static_cast<int>(xPos - i * beatWidth), 0, 20);
    }


}

/**
 * @brief Draw the given set of chords onto the graphics area
 * TODO: This really needs a unit test. And that (kind of) requires that I get mocks working
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
    int spacer = 2;
    float ratio = textBox.getWidth() / chordClipper.getViewWidthInSeconds();
    ChordName cn;

    int fontSize = static_cast<int>(midiState.getChordNameSize());
    g.setFont(fontSize);
    auto ofont = g.getCurrentFont();
    auto lfont = Font("Bravura Text", fontSize, Font::plain);
    String fontName = lfont.getTypefaceName();
    bool hasSymbols = true;
    if (fontName != "Bravura Text")
        hasSymbols = false;

    for (auto it = chords.begin(); it != chords.end(); ++it)
    {
        float leftPos = it->first * ratio;
        textBox.setLeft(leftPos);
        if (!hasSymbols || !nameHasSymbols(it->second))
        {
            // no sharp/flat so we can just draw it with the default font
            g.drawText(it->second, textBox, juce::Justification::centredLeft);
        }
        else
        {
            // There is at least one symbol (e.g., a flat) to draw in a separate font. This seems like
            // a horrible kludge. There has to be a cleaner way to do this, but it is escaping me at the moment.
            // So loop through and draw the symbols in the bravura font and the others in the default font
            string sPart = "";
            for (auto c = it->second.begin(); c != it->second.end(); ++c)
            {
                optional<string> symbol = cn.getUnicodeSymbol(*c);
                if (symbol != std::nullopt)
                {
                    if (sPart != "") {
                        // Draw what we have collected so far
                        g.drawText(sPart, textBox, juce::Justification::centredLeft);
                        textBox.setX(textBox.getX() + ofont.getStringWidthFloat(sPart) + spacer);
                        sPart = "";
                    }
                    // switch to the symbol font, draw the symbol and switch back
                    g.setFont(lfont);
                    g.drawText(*symbol, textBox, juce::Justification::centredLeft);
                    textBox.setX(textBox.getX() + lfont.getStringWidthFloat(*symbol) + spacer);
                    g.setFont(ofont);
                }
                else
                {
                    sPart += {*c};
                }
            }
            // Draw remainder if there is any
            if (sPart != "") {
                g.drawText(sPart, textBox, juce::Justification::centredLeft);
            }
        }
    }
}


/**
 * @brief Determine if a given chord has symbols that need to be mapped to a different
 * font for display purposes
 * 
 * @param string chord 
 * @return bool  true if it has one or more to be mapped, false if not
 */
bool ChordView::nameHasSymbols(string chord)
{
    if (chord.find('b') != string::npos || chord.find('#') != string::npos)
        return true;
    else
        return false;

}



void ChordView::resized()
{
}