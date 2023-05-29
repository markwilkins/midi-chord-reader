
#include <catch2/catch_test_macros.hpp>
#include "ChordName.h"
#include <algorithm>
#include <random>
using namespace std;

TEST_CASE("test note names", "chordident")
{
    ChordName cn;
    std::string name;

    // sort of a pointless-feeling chicken/egg tests. But it is good for regression testing
    name = cn.midiNoteToName(0);
    REQUIRE(name == "C");
    name = cn.midiNoteToName(60);
    REQUIRE(name == "C");
    name = cn.midiNoteToName(61);
    REQUIRE(name == "Db");
    name = cn.midiNoteToName(74);
    REQUIRE(name == "D");
    name = cn.midiNoteToName(3);
    REQUIRE(name == "Eb");
    name = cn.midiNoteToName(4);
    REQUIRE(name == "E");
    name = cn.midiNoteToName(5);
    REQUIRE(name == "F");
    name = cn.midiNoteToName(6);
    REQUIRE(name == "Gb");
    name = cn.midiNoteToName(7);
    REQUIRE(name == "G");
    name = cn.midiNoteToName(8);
    REQUIRE(name == "Ab");
    name = cn.midiNoteToName(9);
    REQUIRE(name == "A");
    name = cn.midiNoteToName(10);
    REQUIRE(name == "Bb");
    name = cn.midiNoteToName(11);
    REQUIRE(name == "B");

}

TEST_CASE("single note chord", "chordident")
{
    ChordName cn;
    string name;
    vector<int> notes;

    notes = {12};
    name = cn.nameChord(notes);
    REQUIRE(name == "C");
    notes = {24, 12};
    name = cn.nameChord(notes);
    REQUIRE(name == "C");

    notes = {8, 20};
    name = cn.nameChord(notes);
    REQUIRE(name == "Ab");
}


TEST_CASE("two note chords", "chordident")
{
    ChordName cn;
    string name;
    vector<int> notes;

    notes = {0, 3};
    name = cn.nameChord(notes);
    REQUIRE(name == "Cmin");
    notes = {3, 0, 12};
    name = cn.nameChord(notes);
    REQUIRE(name == "Cmin");
    notes = {3, 12};
    name = cn.nameChord(notes);
    REQUIRE(name == "Eb6");

    notes = {26, 12};
    name = cn.nameChord(notes);
    REQUIRE(name == "C2");

    notes = {17, 18};
    name = cn.nameChord(notes);
    REQUIRE(name == "F");
    notes = {17, 21};
    name = cn.nameChord(notes);
    REQUIRE(name == "F");
    notes = {17, 22};
    name = cn.nameChord(notes);
    REQUIRE(name == "F4");
    notes = {17, 23};
    name = cn.nameChord(notes);
    REQUIRE(name == "F");
    notes = {17, 24};
    name = cn.nameChord(notes);
    REQUIRE(name == "F");
    notes = {17, 25};
    name = cn.nameChord(notes);
    REQUIRE(name == "F");
    notes = {17, 26};
    name = cn.nameChord(notes);
    REQUIRE(name == "F6");
    notes = {17, 27};
    name = cn.nameChord(notes);
    REQUIRE(name == "F7");
    notes = {17, 28};
    name = cn.nameChord(notes);
    REQUIRE(name == "F7");

}


TEST_CASE("three note chords", "chordident")
{
    ChordName cn;
    string name;
    vector<int> notes;

    notes = {0, 4, 7};
    name = cn.nameChord(notes);
    REQUIRE(name == "C");
    notes = {0, 19, 28};
    name = cn.nameChord(notes);
    REQUIRE(name == "C");
    notes = {0, 3, 7};
    name = cn.nameChord(notes);
    REQUIRE(name == "Cmin");
    notes = {8, 11, 14};
    name = cn.nameChord(notes);
    REQUIRE(name == "Abdim");
    notes = {8, 12, 16};
    name = cn.nameChord(notes);
    REQUIRE(name == "Abaug");

    // this is 2nd inversion of an F major
    notes = {0, 5, 9};
    name = cn.nameChord(notes);
    REQUIRE(name == "F/C");

    // egc
    notes = {4, 7, 12};
    name = cn.nameChord(notes);
    REQUIRE(name == "C/E");

    // mlwtbd todo: Here are a couple of examples where I think it is necessary to specify the actual key that we are in
    // The current logic that deduces sharp/flat from the chord fails for these. Both of them return the flat chord
    // but they are ones that could be shard chord as the iii and V chords. I need to change the identifier method 
    // to allow (require?) the key to be passed in.
    // C#EG# . (the minor 3rd of A major)
    notes = {1, 4, 8};
    name = cn.nameChord(notes);
    //REQUIRE(name == "C#min");
    // F#A#C# (the V chord of B major)
    notes = {18, 22, 25};
    name = cn.nameChord(notes);
    //REQUIRE(name == "F#");

}

TEST_CASE("four note chords", "chordident")
{
    ChordName cn;
    string name;
    vector<int> notes;

    notes = {0, 4, 7, 10};
    name = cn.nameChord(notes);
    REQUIRE(name == "C7");
    notes = {10, 14, 17, 22};
    name = cn.nameChord(notes);
    REQUIRE(name == "Bb");
    notes = {7, 10, 14, 17};
    name = cn.nameChord(notes);
    REQUIRE(name == "Gm7");

    // CGBD
    notes = {12, 19, 23, 26};
    name = cn.nameChord(notes);
    REQUIRE(name == "G/C");

    // CGBbD
    notes = {12, 19, 22, 26};
    name = cn.nameChord(notes);
    REQUIRE(name == "Gmin/C");

    // mlwtbd - need to think about this one. Current code doesn't detect this ... but
    // it seems like maybe it doesn't have an obvious solution. In fact, I guess I'm not sure
    // what the actual chord is. It is a simple mess of notes ... is it really a Gm over F?
    // FGBbD
    // notes = {17, 19, 22, 26};
    // name = cn.nameChord(notes);
    // REQUIRE(name == "Gmin/F");

}

TEST_CASE("test gap rotation", "chordtransform")
{
    ChordName cn;
    vector<int> notes;
    vector<int> expected;
    bool rotated;

    // ceg - should be no change
    notes = {0, 4, 7};
    rotated = cn.normalizeNotes(notes);
    expected = {0, 4, 7};
    REQUIRE(notes == expected);
    REQUIRE(rotated == false);
    // egc
    notes = {4, 7, 12};
    rotated = cn.normalizeNotes(notes);
    // ceg
    expected = {12, 16, 19};
    REQUIRE(notes == expected);
    REQUIRE(rotated == true);

    // gce
    notes = {7, 12, 16};
    rotated = cn.normalizeNotes(notes);
    // ceg
    expected = {12, 16, 19};
    REQUIRE(notes == expected);
    REQUIRE(rotated == true);

    // cfb
    notes = {0, 5, 11};
    rotated = cn.normalizeNotes(notes);
    // bcf
    expected = {11, 12, 17};
    REQUIRE(notes == expected);
    REQUIRE(rotated == true);
}