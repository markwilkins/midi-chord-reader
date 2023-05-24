
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

    string twoNoteChordName(vector<int> notes);

    vector<int> normalizeNotes(vector<int> notes);

    string midiNoteToName(int note);

    bool isSharpKey(string key);

private:
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
