

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

    vector<int> normalized = normalizeNotes(notes);
    
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


string ChordName::twoNoteChordName(vector<int> notes)
{
    int interval = notes[1] - notes[0];
    string chord;
    string modifier = "";

    // Not sure all the intervals even make sense (at least they don't to me in a musical sense)
    switch (interval) 
    {
        case 1: 
            break;
        case 2:
            modifier = "2";
            break;
        case 3:
            modifier = "min";
            break;
        case 4:
            // example is C-E. Just consider it a C
            break;
        case 5:
            modifier = "4";
            break;
        case 6:
            // not even sure what this would be ... C + F# ... not really a chord in my book
            break;
        case 7:
            // Could add a "no 3" to this ... for my own use I don't care about that
            break;
        case 8:
            break;
        case 9:
            modifier = "6";
            break;
        case 10:
        case 11:
            modifier = "7";
            break;
    }

    chord = midiNoteToName(notes[0]) + modifier;
    return chord;
}

string ChordName::multiNoteChordName(vector<int> notes)
{
    string chord;
    string modifier = "";

    // if the interval between the bottom (bass-most) note and the next one is 5 or more semitones, then
    // bump the bottom note up an octave. I think it is more likely to fit into a "normal" chord in that
    // position. Simple example is CFA: This is the 2nd inversion of an F. The interval from C to F is 5. 
    // If we bump it to the higher C, then it is a simple Major F chord.
    bool inversion = false;
    int origBass = notes[0];
    if (notes[1] - notes[0] >= 5)
    {
        inversion = true;
        notes[0] += 12;
        rotate(notes.begin(), notes.begin() + 1, notes.end());
    }
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
 * @brief Put the notes into a "normalized" state to simplify chord quality identification.
 * - Move the notes into a single octave (the semitone interval between the min and max will be <= 11)
 * - Duplicates are removed (e.g., if there are C2 and C3, the returned vector will only have one C in it)
 * - Preserve the relative position of the bottom-most note (if a C is the lowest note, it will retain that status)
 * 
 * @param notes 
 * @return vector<int>   The notes are returned in sorted MIDI order
 */
vector<int> ChordName::normalizeNotes(vector<int> notes) 
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


void ChordName::rotateNotes(vector<int> &notes) 
{
    vector<int>::iterator it;
    int maxGap = 0;
    int gapPos = 0;

    for (int i = 1; i < notes.size(); i++)
    {
        if (notes[i] - notes[i - 1] > maxGap)
        {
            maxGap = notes[i] - notes[i - 1];
            gapPos = i;
        }
    }
    if (maxGap >= 5)
    {
        // We found a gap of at least 5 semitones. Add 12 to the notes prior to that
        // gap and rotate them to the end of the vector.
        transform(notes.begin(), notes.begin() + gapPos - 1, notes.begin(), [&](auto &value)
                  { return value + 12; });
        rotate(notes.begin(), notes.begin() + gapPos, notes.end());
    }
}


string ChordName::midiNoteToName(int note)
{
    note %= 12;
    auto it = midiNoteLookup.find(note);
    if (it != midiNoteLookup.end())
    {
        pair<int, string> nl = *it;
        return nl.second;
    }
    return "unkonwn note";
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
