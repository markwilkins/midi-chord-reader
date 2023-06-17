/**
 * @file MidiStore.h
 * @author Mark Wilkins
 * @brief Part of MidiChords project (plugin to display chord names from a MIDI track on playback)
 * @version 0.1
 * 
 * @copyright Copyright (c) 2023
 * 
 */



#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
using namespace juce;
using namespace std;

/**
 * This is the "data store" for the midi notes from a track. It effectively represents the state
 * of the plugin: Current set of note events (on/off) at specific times, whether state change is
 * allowed, etc.
 * Primary data store is constructed as a juce::ValueTree that contains one level of child ValueTree objects
 * Each child ValueTree contains the midi note events for a given time. 
 * - The identifier for each child tree is a string of the form "notesat:<int64>" where
 *   the int value is the event time
 * - Each child tree has a property with the identifier "eventTime" where the value is the event time
 * - The child trees are kept in sorted order by event time
 * - The midi note numbers are stored as properties (in no particular order) in the child tree 
 *   with the identifier being the midi note number (string version of the int value) and the 
 *   value as a bool (true/false) representing if it is a note on or off event
 */
class MidiStore 
{
public:
    // Current known version of the midi state info that is potentially saved/restored. The idea
    // is that if I need to make a breaking change to the store, then I can bump this up and an
    // older version of the plugin won't load the newer saved state. Also, it provides a mechanism
    // to know if the saved (restored) state is recognized at all.
    static const int currentVersion = 1;

    // Some of the property names for the value tree state. I've been out of C++ dev for a while.
    // I imagine there is a best (at least better) practice for this. 
    // These should maybe be created as juce::Identifier values, but the bulk of the props in the
    // usage are going to be dynamically created, so I'm not sure I see the value in that.
    inline static const char* midiChordsVersionProp = "midiChordsVersion";
    inline static const char* eventTimeInSecondsProp = "eventTimeInSeconds";
    inline static const char* eventTimeProp = "eventTime";
    inline static const char* quantizationValueProp = "quantizationValue";
    inline static const char* allowRecordingProp = "allowRecording";
    // property with the position of the playhead in % (within the view window)
    inline static const char* playHeadPositionProp = "playheadPosition";
    inline static const char* viewWidthProp = "viewWidthProp";
    inline static const char* shortChordThresholdProp = "shortChordThresholdProp";
    inline static const char* chordNameSizeProp = "chordNameSizeProp";



    MidiStore();
    ~MidiStore();
    bool hasData();
    void setName(juce::String name);
    String getName();

    // These are the methods that update state in the chord view
    // -------------------------
    void addNoteEventAtTime(int64 time, int note, bool isOn);
    void setEventTimeSeconds(int64 time, double seconds);
    bool replaceState(ValueTree &newState);
    // -------------------------

    ValueTree& getState() {return this->chordState;}


    void updateStaticView();
    void updateStaticViewIfOutOfDate();
    vector<int> getNoteOnEventsAtTime(int64 time);
    vector<int> getAllNotesOnAtTime(int64 startTime, int64 endTime);
    double getEventTimeInSeconds(int64 time);
    vector<int64> getEventTimes();
    vector<pair<float, string>> getChordsInWindow(pair<float, float> viewWindow);
    int getViewWindowChordCount() {return viewWindowChordCount;}
    void clear();
    void allowStateChange(bool allow);
    void setPlayHeadPosition(float percentage);
    float getPlayHeadPosition();

    void setTimeWidth(float width);
    float getTimeWidth();
    void setShortChordThreshold(float threshold);
    float getShortChordThreshold();
    void setChordNameSize(float fontSize);
    float getChordNameSize();
    void setBPMinute(double bpm);
    optional<double> getBPMinute();
    void setBPMeasure(int bpmeasure);
    optional<int> getBPMeasure();

    bool getRecordingState() { return allowDataRecording; }

    // setters/getters for most recently seen event time
    void setLastEventTime(int64 time) {lastEventTime = time;}
    void setLastEventTimeInSeconds(double time) {lastEventTimeInSeconds = time;}
    int64 getLastEventTime() {return lastEventTime;}
    double getLastEventTimeInSeconds() {return lastEventTimeInSeconds;}

    void setIsPlaying(bool playing) {isPlaying = playing;}
    bool getIsPlaying() {return isPlaying;}

    int getQuantizationValue();
    void setQuantizationValue(int q);

    // For testing ... Ideally I would mock the time function for this; this is a cheat but still a good test
    void setLastViewUpdateTime(int64 time) {this->lastViewUpdateTime = time;}

private:
    // Critical section for concurrent access. The editor will be reading it. processor updates it
    CriticalSection storeLock;
    // This is a lock on the static view that is used for the common usages of viewing chords
    CriticalSection viewLock;
    // the bulk of the data is stored in this value tree. It, effectively, represents the entire
    // state of the plugin (mostly the midi notes that have been played in the track)
    juce::ValueTree chordState;

    // Is the static view of the chords up to date?
    bool isViewUpToDate = false;
    int64 lastViewUpdateTime = 0;
    int viewWindowChordCount = 0;

    void refreshSettingsFromState();

    void setStateProp(const char *propName, juce::var value);
    float getStateFloatProp(const char *propName, float defaultValue, float min, float max);

    vector<pair<float, string>> staticView;
    // If this is true, then save state changes. Otherwise, don't
    // mlwtbd - I think I want this false by default for typical usage ... or maybe it just needs to be stored with the
    // settings ... as false, it causes test failures, though
    bool allowDataRecording = true;
    // flag indicating if we think playback is occuring
    bool isPlaying = false;

    // Beats per minute and measure. optional because the juce doc says it is optional from the host
    optional<double> bpMinute = std::nullopt;
    optional<int> bpMeasure = std::nullopt;

    // Maintain the most recent time we received an event. This is for keeping aware of where the 
    // current location is in the playback. If this doesn't provide enough detail, then I might
    // need to just keep a handle here to the pluginprocessor class so I can retrieve current
    // playback head position on the fly ... but that feels wrong so trying to avoid it for now.
    int64 lastEventTime = 0;
    // Make sure the read/write of this value is atomic. Not a big deal but it could result in some
    // glitchy scrolling if it wasn't.
    atomic<double> lastEventTimeInSeconds = 0.0;

    // simple optimization tool. The normal process of adding values to the midistore will
    // be "in order". For example, playing a track from the start (or any position) will mean
    // time is increasing. So a typical operation of adding an event an a previously unseen
    // time will be to add it to the end and that will be the right place. But playback
    // operations of modified (new) notes will likely add new events into the middle.
    // If we know the max stored, then this helps us decide if we need to insert in tne middle.
    int64 maxTimeStored = 0;

    vector <pair<float, string>> createStaticView();
    vector<pair<float, string>> getChordsInWindowRaw(pair<float, float> viewWindow);
    void removeShortChords(vector<pair<float, string>> &view);
    void updateCurrentlyOn(ValueTree &childEvents, set<int> &notes, int propIndex);
    Identifier noteIdentFromInt(int note);
    int64 quantizeEventTime(int64 time);

    Identifier notesAtIdent(int64 time);
    bool noteIdentToInt(String str, int *value);
    ValueTree ensureNoteEventAtTime(int64 time);

    int64 findMaxTime();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiStore)
};
