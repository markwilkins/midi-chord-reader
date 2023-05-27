

#include "ChordName.h"
#include <set>
#include <algorithm>
using namespace std;

/*
To create an algorithm that produces a chord name given up to 5 notes, follow these steps: 
1. Define the input: Accept up to 5 unique musical notes as input. For example, the input could be: C, E, G, B, D. 
2. Normalize the input: Convert each note to its pitch class integer representation. In this system, 
   C=0, C#=1, D=2, D#=3, E=4, F=5, F#=6, G=7, G#=8, A=9, A#=10, B=11. For example, the input C, E, G, B, D would be converted to 0, 4, 7, 11, 2. 
3. Sort the input: Sort the pitch class integers in ascending order. If the input is already sorted, you can skip this step. 
In our example, the sorted input would be 0, 2, 4, 7, 11. 
4. Calculate intervals: Calculate the intervals between each consecutive pitch class integer in the sorted input. 
   In our example, the intervals would be: 2, 2, 3, 4. 
5. Identify the chord quality: Based on the intervals, determine the chord quality (e.g., major, minor, diminished, augmented, etc.). 
   You can use the following common chord qualities and their corresponding interval patterns: 
     - Major: 4, 3 
     - Minor: 3, 4 
     - Diminished: 3, 3 
     - Augmented: 4, 4 
     - Dominant 7th: 4, 3, 3 
     - Major 7th: 4, 3, 4 
     - Minor 7th: 3, 4, 3 
     - Half-diminished 7th: 3, 3, 4 
     - Diminished 7th: 3, 3, 3 
     - Augmented 7th: 4, 4, 3 
     - Minor-major 7th: 3, 4, 4 
     - 9th, 11th, and 13th chords can be identified by extending the interval pattern. 
   In our example, the interval pattern 2, 2, 3, 4 corresponds to a 9th chord. 
6. Determine the root note: The first note in the sorted input is the root note. 
   Convert the root note from its integer representation back to its musical note. 
   In our example, the root note is 0, which corresponds to C. 
7. Construct the chord name: Combine the root note and the chord quality to form the chord name. 
   In our example, the chord name would be C9. The algorithm can be further refined to include more chord types and handle edge cases, 
   but this serves as a starting point for producing a chord name given up to 5 notes.
*/
ChordName::ChordName()
{
}

ChordName::~ChordName()
{
}

string ChordName::nameChord(vector<int> notes)
{
    if (notes.size() == 0)
        return "";

    vector<int> normalized = reduceNotes(notes);
    
    switch (normalized.size())
    {
    case 1:
        // for a single note, assume the chord is the one note
        return midiNoteToName(normalized[0]);
    case 2:
        return twoNoteChordName(normalized);
    default:
        return multiNoteChordName(normalized);
    }

    return "";
}


/**
 * @brief Create a chord name for a "two note chord".
 * @details This just uses the notes in the original order. In other words, it assumes that the lowest note
 * is the chord name. The interval between that note and the next one is treated as the modifier
 * 
 * @param vector<int> notes 
 * @return string 
 */
string ChordName::twoNoteChordName(vector<int> notes)
{
    int interval = notes[1] - notes[0];
    string chord;
    auto   intervalInfo = twoNoteIntervals.find(interval);
    string modifier = "";

    if (intervalInfo != twoNoteIntervals.end())
        modifier = intervalInfo->second;

    chord = midiNoteToName(notes[0]) + modifier;
    return chord;
}


/**
 * @brief Name chords that consist of 3 or more notes
 * 
 * @param vector<int> notes  The normalized vector of midi note values
 * @return string 
 */
string ChordName::multiNoteChordName(vector<int> notes)
{
    string chord;
    string modifier = "";
    bool inversion = false;
    vector<int> notes246;
    int origBass = notes[0];

    inversion = normalizeNotes(notes);
    notes246 = extract246(notes);

    int interval1 = notes[1] - notes[0];
    int interval2 = notes[2] - notes[1];
    int interval3 = 0;
    if (notes.size() > 3) 
        interval3 = notes[3] - notes[2];

    tuple<int, int, int> intervalPair = {interval1, interval2, interval3};
    auto quality = chordQuality.find(intervalPair);
    if (quality != chordQuality.end()) 
    {
        modifier = quality->second;
    }

    chord = midiNoteToName(notes[0]) + modifier;
    if (inversion) 
    {
        // Treat this as "Chord / Chord" notation. Example is notes CFA (2nd inversion of F maj). Notate it as F/C
        chord += "/" + midiNoteToName(origBass);
    }

    return chord;
}

/**
 * @brief Put the notes into a single octave to simplify chord quality identification.
 * - Move the notes into a single octave (the semitone interval between the min and max will be <= 11)
 * - Duplicates are removed (e.g., if there are C2 and C3, the returned vector will only have one C in it)
 * - Preserve the relative position of the bottom-most note (if a C is the lowest note, it will retain that status)
 * 
 * @param notes 
 * @return vector<int>   The notes are returned in sorted MIDI order
 */
vector<int> ChordName::reduceNotes(vector<int> notes) 
{
    set<int> unique;
    vector<int> normalized;

    if (notes.size() == 0) 
        return notes;

    // Use the bottom (most bass) note as a "reference" of sorts. Keep it as the bottom
    // and place the other above it.
    int bottomNote = *min_element(notes.begin(), notes.end());
    bottomNote %= 12;

    for (vector<int>::iterator it = notes.begin(); it != notes.end(); ++it) 
    {
        int note = *it % 12;
        if (note < bottomNote) 
            note += 12;
        
        unique.insert(note);
    }
    // copy them from the set (which will be sorted by its very nature) into the return vector
    normalized.assign(unique.begin(), unique.end());
    return normalized;
}


/**
 * @brief This attempts to put a chord into "normalized" form (no inversion)
 * @details
 * If the interval between the bottom (lowest) note and the next one is 5 or more semitones, then
 * bump the bottom note up an octave. I think it is more likely to fit into a "normal" chord in that
 * position. Simple example is CFA: This is the 2nd inversion of an F. The interval from C to F is 5. 
 * If we bump it to the higher C, then it is a simple Major F chord.
 * 
 * Note - I *think* that the term "normal" when applied to a chord has a specific meaning where it refers
 * to the set of notes being in their most compact form. This function does not entirely do that. For a
 * typical 3 note major or minor chord, it will result in that form. But for something like a major 7th
 * chord (e.g., CEGB), it does not do that. The true "normal" form of that would be EGBC. This method
 * only does the adjustment (rotation) if there is a semitone gap of 5 notes or more.
 * 
 * @param notes  This is modified in place. The lowest (bass) note is NOT preserved
 * @return  true if the notes are modified, false if no change
 * 
 */
bool ChordName::normalizeNotes(vector<int> &notes) 
{
    int maxGap = 0;
    vector<int>::difference_type gapPos = 0;

    for (vector<int>::size_type i = 1; i < notes.size(); i++)
    {
        if (notes[i] - notes[i - 1] > maxGap)
        {
            maxGap = notes[i] - notes[i - 1];
            gapPos = static_cast<vector<int>::difference_type>(i);
        }
    }
    if (maxGap >= 5)
    {
        // We found a gap of at least 5 semitones. Add 12 to the notes prior to that
        // gap and rotate them to the end of the vector.
        transform(notes.begin(), notes.begin() + gapPos, notes.begin(), [&](auto &value)
                  { return value + 12; });
        rotate(notes.begin(), notes.begin() + gapPos, notes.end());
        return true;
    }
    return false;
}


/**
 * @brief Remove 2nd, 4th, and 6th interval notes from the given vector assuming that the lowest/first
 * note in the list is the bass note. 
 * 
 * @param notes 
 * @return vector<int>  The removed notes
 */
vector<int> ChordName::extract246(vector<int> &notes) 
{
    vector<int>::iterator it;
    vector<int> removed;

    for (it = notes.begin(); it != notes.end();)
    {
        int interval = *it - notes[0];
        if (interval == 2 || interval == 5 || interval == 9)
        {
            removed.push_back(*it);
            it = notes.erase(it);
        }
        else
            ++it;
    }
    return removed;
}


/**
 * @brief Given a MIDI note number, return the note name
 * 
 * @param note 
 * @return string 
 */
string ChordName::midiNoteToName(int note)
{
    note %= 12;
    auto it = midiNoteLookup.find(note);
    if (it != midiNoteLookup.end())
    {
        pair<int, string> nl = *it;
        return nl.second;
    }
    return "unknown note";
}

/*
string ChordName::midiNoteToNameFirstCut(int note, string key)
{
    bool isSharpKey;
    bool isMinor;

    // Normalize to values 0 to 11 (0 is C, ... 11 is B)
    note %= 12;
    auto it = chordMap.find(note);
    if (it != chordMap.end())
    {
        pair<int, chordInfo> ci = *it;
        bool isSharp = ci.second.isSharp;

    }

    return "";

}
*/

bool ChordName::isSharpKey(string key)
{
    auto it = chordLookup.find(key);
    if (it != chordLookup.end())
    {
        pair<string, chordInfo> ci = *it;
        return ci.second.isSharp;
    }

    // Maybe should log something here
    return true;
}
