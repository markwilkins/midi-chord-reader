
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

    // cgab
    notes = {12, 19, 23, 26};
    name = cn.nameChord(notes);
    REQUIRE(name == "G/C");
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
}