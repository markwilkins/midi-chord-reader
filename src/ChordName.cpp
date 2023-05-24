

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
    vector<int> normalized = normalizeNotes(notes);
    
    switch (normalized.size())
    {
    case 1:
        // for a single note, assume the chord is the one note
        return midiNoteToName(normalized[0]);
        break;
    case 2:
        return twoNoteChordName(normalized);
        break;
    case 3:
        break;
    default:
        break;

    }

    return "";
}


string ChordName::twoNoteChordName(vector<int> notes)
{
    int interval = notes[1] - notes[0];
    string chord;
    string modifier = "";

    switch (interval) 
    {
        case 1:
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


vector<int> ChordName::normalizeNotes(vector<int> notes) 
{
    set<int> unique;
    vector<int> normalized;

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
    normalized.assign(unique.begin(), unique.end());
    return normalized;
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
