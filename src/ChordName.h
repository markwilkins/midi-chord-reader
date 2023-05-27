
#pragma once
#include <vector>
#include <string>
#include <map>

using namespace std;


struct chordInfo
{
    bool isSharp;
    bool isMinor;
};

/**
 * @brief Name a chord given individual notes in the chord
 * 
 */
class ChordName 
{
public:
    //==============================================================================
    ChordName();
    ~ChordName();

    string nameChord(vector<int> notes);
    string midiNoteToName(int note);

    // These "maybe" should be private methods. But 
    bool normalizeNotes(vector<int> &notes);
    vector<int> extract246(vector<int> &notes);
    vector<int> reduceNotes(vector<int> notes);

private:
    string twoNoteChordName(vector<int> notes);
    string multiNoteChordName(vector<int> notes);
    bool isSharpKey(string key);

    // one could have semantic arguments about these, but the F# vs Gb one is probably the main one
    // that seems like it could be argued either way. Going with the flat option for now (gives 6 for each)
    inline static const vector<string> sharpKeys = { "C", "G", "D", "A", "E", "B"};
    inline static const vector<string> flatKeys  = { "Gb", "Db", "Ab", "Eb", "Bb", "F"};
    inline static const vector<string> minorSharpKeys = {"a", "e", "b", "f#", "c#", "g#"};
    inline static const vector<string> minorFlatKeys = {"eb", "bb", "f", "c", "g", "d"};
    inline static const map<int, chordInfo> chordMap = {
        {0, {true, false}},
        {1, {true, false}}
    };

    // Define modifiers for "two note" chords.
    inline static const map<int, string> twoNoteIntervals =
    {
        {1, ""},
        {2, "2"},
        {3, "min"},
        {4, ""},
        {5, "4"},
        {6, ""},   // not really sure what this would be. C+F# for example. Dunno
        {7, ""},   // This is a major 5th. Could maybe use "no 3" as the modifier.
        {8, ""},   // again not sure what this should be. C+G#
        {9, "6"},
        {10, "7"},
        {11, "7"},

    };

    // Lookup table for chord quality based on intervals between three or four notes
    // The zero value indicates there are only 3 notes total
    inline static const map<tuple<int, int, int>, string> chordQuality = 
    {
        {{4, 3, 0}, ""},     // Could include "maj" here ... but I don't want to see "maj" on every one of these
        {{3, 4, 0}, "min"},
        {{3, 3, 0}, "dim"},
        {{4, 4, 0}, "aug"},
        {{4, 3, 3}, "7"},   // dominant 7th
        {{4, 3, 4}, "M7"},   // major 7th
        {{3, 4, 3}, "m7"},   // minor 7th
        {{3, 3, 4}, "m7b5"},   // half-dimished 7th
        {{3, 3, 3}, "m7"},   // dimished 7th (for my own use, treating it same as minor 7th)
        {{4, 4, 3}, "M7+"},   // Augmented 7
        {{3, 4, 4}, "m-7"},   // minor-major 7
        
    };

    // TODO: still need to determine if F# or Gb
    inline static const map<int, string> midiNoteLookup = {
        {0, "C"},
        {1, "Db"},
        {2, "D"},
        {3, "Eb"},
        {4, "E"},
        {5, "F"},
        {6, "Gb"},
        {7, "G"},
        {8, "Ab"},
        {9, "A"},
        {10, "Bb"},
        {11, "B"},
    };
    inline static const map<string, chordInfo> chordLookup = {
        {"C", {true, false} },
        {"G", {true, false} },
        {"D", {true, false} },
        {"A", {true, false} },
        {"E", {true, false} },
        {"B", {true, false} },

        {"Gb", {false, false} },
        {"Db", {false, false} },
        {"Ab", {false, false} },
        {"Eb", {false, false} },
        {"Bb", {false, false} },
        {"F", {false, false} },

        {"a", {true, true} },
        {"e", {true, true} },
        {"b", {true, true} },
        {"f#", {true, true} },
        {"c#", {true, true} },
        {"g#", {true, true} },

        {"eb", {false, true} },
        {"bb", {false, true} },
        {"f", {false, true} },
        {"c", {false, true} },
        {"g", {false, true} },
        {"d", {false, true} }
    };

};
