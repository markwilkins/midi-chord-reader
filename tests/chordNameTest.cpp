
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

TEST_CASE("test single note chord", "chordident")
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

TEST_CASE("test two note chords", "chordident")
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

}