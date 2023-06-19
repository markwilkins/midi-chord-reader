# MIDI Chord Display (midi-chord-reader)
DAW plugin for displaying the chords for a MIDI track during playback

Built on JUCE: https://juce.com/

This plugin is for MIDI tracks and is for displaying chord names; it does not perform any transformation on the notes. It detects the chord names based on the notes being played in the track and displays them in the scrolling window. 

The reason I created this is because my ear for music is not very good and I create "non-standard" chord sequences when writing songs and have difficulty adding new tracks on the fly by ear. I wrote this plugin to allow me to view the chords and to be able to anticipate what is coming next.

The plugin "company" name is `tetrachord` and the plugin name is `MidiChords`. An example of what it looks like:

![Example view](/images/image1.png)

# Building
The build process uses cmake and integrates nicely with VSCode. Note that the first cmake call will download JUCE, so it might take a bit. If you already have JUCE downloaded somewhere, update the CMakeLists.txt file to point to it to save time and space.

I only have the build working on a Mac. The dependencies (besides the Apple dev tools):
- cmake (e.g., `brew install cmake`)
- catch2 (`brew install catch2`)

The following is one way of doing the build. If you have cmake knowledge, then you probably have your own favorite commands. This is just what I have been using. Navigate to the root folder of the project, then:
## debug build
    > mkdir build
    > cd build
    > cmake ..
    > cmake --build .
## release build
    > mkdir release
    > cd release (mkdir if needed)
    > cmake -DCMAKE_BUILD_TYPE=Release ..
    > cmake --build .

## create Xcode project file
This will create a folder name xcode that contains the project file. Sometimes debugging works better in Xcode than in VSCode. If you find that to be true, then this should get you bootstrapped into an Xcode project:

    > cd build
    > cmake ..  -B xcode -G Xcode
## running unit tests
    > cd build
    > ctest -j50 --output-on-failure
# Usage Notes
## The algorithm and thoughts behind it
The goal of the plugin is to display chords (e.g, Am/C) in a scrolling view window of measures during the playback of a track. The basic idea is to update the current chord name each time it changes (at note on/off events). 

The chord naming logic (in the source file ChordName.cpp) works generally as follows:
- At a given moment in time, examine all notes that are currently on
- Normalize those into a single octave and remove duplicates
- Track the original lowest note to use it as a sort of "true bass note". For example, if a C note was the lowest note played and the other notes composed an A minor chord (e.g., notes C-A-C-E), then the resulting set of notes ends up as A-C-E to form the A minor. The display logic uses the lower C to name the chord `Am/C`
- When there is only a single MIDI note ON at a time, that note is named as the chord. 
- When two notes are on, then the lower note is the chord name and the interval is used as a modifier (e.g., C2, Cm, C4, C6, C7)
- For three or more notes, the logic I used is:
    - Normalize them into the single octave and remove duplicates
    - Remove 2nd, 4th, and 6th notes (measured from the lowest note). Note that I am not currently using that information. I could enhance the chord naming (e.g., add9), but that doesn't help me for my use case.
    - Use the remaining lowest 3 notes to determine chord quality (major, minor, diminished, etc.)


## Capturing the chords
I was not able to figure out how to read the entire track at once; the only way to capture all the data is to simply play the track and capture it live. The general anticipated flow is this:

- Enable "Record Notes" (checkbox on right side of plugin)
- Play the track and note information will be captured by the plugin. 
- Uncheck the "Record Notes" option for future usage (until changes are made). This is not absolutely necessary, but the playback from time to time does not present the exact same timing to the plugin. So it may capture more note on/off events than necessary and be less efficient.

**Faster capture** If you enable Record Notes, and then freeze the track, it might capture the data more quickly. Then unfreeze the track and disable the Record Notes option. The reason I say "might" is because freezing a complex track with an expensive synth engine and many affects sometimes seems slower than just playing the track.

**Edits & Recapture** If you make edits to the MIDI track, it might be simplest just to recapture the notes. Click the Clear Notes! button and follow the steps above. Alternatively, you can enable Record Notes and play the track over the section of the edits and it will capture that section. 

## Playback
The plugin editor (which displays the chords) is resizable. It currently has the following controls:

- **Playhead** This adjusts the relative position (percentage) of the "now" position of the playhead (the currently playing note/chord). It is represented by the vertical red line in the display. In order to have the largest view of upcoming notes, place it to the far left.

- **View seconds** This slider controls the size of the view window in number of seconds. Increasing this logical width allows you to see more measures and more chords but may result in them all being squished together if it is a busy score. Moving it to the left to shorten the logical time frame provides for more "space" between chords but also means the chords scroll by more quickly.

- **Minimimum chord** This controls the granularity of the chord detection. The basic algorithm potentially detects a new chord at every single note on/off event. If you play a 1/16th note on top of a 4 beat chord, it probably doesn't make sense to rename the chord for that 1/16th note. This control lets you choose the required minimum length for a chord.

- **Font size** This controls the font size of the displayed chords. 

## Random bits of information
- If you are not currently playing the track, then the DAW will not necessarily send any events to the plugin. I have found, however, that if the track that the plugin is on has the focus, then the current position information always seems to trigger events. This can be handy if you are moving the track back and forth with hot keys and want the chord view to stay in sync.
- Flat/Sharp symbols are not available in all fonts. The only one that I was able to make work on my Mac was Bravura. So if you have the **Bravura Text** font installed, the plugin will use that to display flats and sharps. Otherwise they will be displayed as b and #
- Speaking of sharps... In this current version of the plugin, I do not detect the key signature, so every chord is simply displayed as a standalone chord without reference to a specific key signature. The chords are named according to the typical "wheel of fifths". This works pretty well for most things, but it is potentially slightly weird for some chords. For example, if you are playing a song in E major, and then play the minor iii chord (G# minor), it will be displayed as Ab minor. 
- The current detction logic does not work well for disjointed chords (e.g., non-legato arpeggios). If there is no overlap between the notes, then each individual note will be detected as a new chord. I have ideas for handling this in a future version. If this is the only type of track you have, then this plugin won't be very useful for you in this current state. One "solution" is to add a "chord track" where you just play the chord sequence (e.g., a series of triads). This is one of my standard crutches for writing songs. It gives me audio and visual clues when playing new tracks.
- The plugin (at least within Logic Pro X) shows up under the MIDI Effect slot menu under the item "Audio Units".