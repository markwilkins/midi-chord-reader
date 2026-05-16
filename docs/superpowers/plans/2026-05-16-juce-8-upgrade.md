# JUCE 7.0.5 → 8.0.12 Upgrade Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Upgrade the MidiChords plugin from JUCE 7.0.5 to JUCE 8.0.12 with a clean build, passing tests, and equivalent rendered output.

**Architecture:** JUCE is pulled in via CMake `FetchContent`. The upgrade is two-part: (1) lift `GIT_TAG` and `cmake_minimum_required` in `CMakeLists.txt`, and (2) migrate deprecated/removed JUCE APIs the codebase uses — primarily `Font` construction and `Font::getStringWidthFloat`, plus a stale `AudioPlayHead::CurrentPositionInfo` line. Tests are Catch2-based and exercise non-UI logic (`MidiStore`, `ChordName`, `ChordClipper`); UI changes are validated by a Standalone smoke test.

**Tech Stack:** JUCE 8.0.12, CMake ≥3.22, C++17, Catch2 v3, macOS plugin formats (AU, Standalone, VST3).

---

## Pre-flight: known JUCE 8 breaking changes that affect this codebase

Confirmed by inspection of `src/`:

1. **CMake minimum** — JUCE 7.0.7+ requires CMake ≥3.22. Project currently sets 3.15 at `CMakeLists.txt:1` and `tests/CMakeLists.txt:1`.
2. **`Font` constructors deprecated** (JUCE 8.0.0):
   - `src/AboutBox.cpp:22` — `juce::Font(16.0f, juce::Font::bold)`
   - `src/ChordView.cpp:81` — `g.setFont(15.0)` (implicit `Font(float)`)
   - `src/ChordView.cpp:124` — `g.setFont(fontSize)` (implicit `Font(float)`)
   - `src/ChordView.cpp:127` — `Font("Bravura Text", fontSize, Font::plain)`
   - Replacement: build a `Font` via `juce::FontOptions{}.withName(...).withHeight(...).withStyle(...)`.
3. **`Font::getStringWidthFloat` deprecated** (JUCE 8.0.2):
   - `src/ChordView.cpp:152`, `src/ChordView.cpp:158`
   - Replacement: `juce::GlyphArrangement::getStringWidth(font, text)`.
4. **Stale `AudioPlayHead::CurrentPositionInfo`** — `src/PluginProcessor.cpp:148` declares an unused `info` of a deprecated type. Remove.
5. **Default font metrics changed** to "portable" — text positions may shift visually. Acceptable, but verify in the Standalone smoke test (Task 8). If layout is materially worse, restore legacy metrics by subclassing `LookAndFeel_V3` and overriding `getDefaultMetricsKind()`; see Task 8 for the contingency.

Items expected to still work without change: `juce::Graphics`, `juce::Colours`, `juce::Slider`, `juce::Label`, `juce::ToggleButton`, `juce::GroupComponent`, `juce::DialogWindow::showDialog`, `juce::AudioProcessor` overrides, `getPlayHead()->getPosition()`, Catch2 tests.

---

## File map

- **Modify** `CMakeLists.txt` — bump CMake min and JUCE tag (Task 1).
- **Modify** `tests/CMakeLists.txt` — bump CMake min (Task 1).
- **Modify** `src/AboutBox.cpp` — Font construction (Task 3).
- **Modify** `src/ChordView.cpp` — Font construction + string width (Task 4).
- **Modify** `src/PluginProcessor.cpp` — drop stale `CurrentPositionInfo` line (Task 5).
- **Clean & rebuild** `build/` — wipe stale FetchContent (Task 2, Task 6).
- **No changes expected** to `src/MidiStore.{cpp,h}`, `src/ChordName.{cpp,h}`, `src/ChordClipper.{cpp,h}`, `src/PluginEditor.{cpp,h}`, `src/OptionsComponent.{cpp,h}`, `tests/*.cpp`. Reconfirm during Task 6.

---

### Task 1: Bump CMake minimum and JUCE tag

**Files:**
- Modify: `CMakeLists.txt:1`
- Modify: `CMakeLists.txt:21`
- Modify: `tests/CMakeLists.txt:1`

- [ ] **Step 1: Update root `CMakeLists.txt`**

Change line 1 from:

```cmake
cmake_minimum_required(VERSION 3.15)
```

to:

```cmake
cmake_minimum_required(VERSION 3.22)
```

And change line 21 from:

```cmake
        GIT_TAG 7.0.5
```

to:

```cmake
        GIT_TAG 8.0.12
```

- [ ] **Step 2: Update `tests/CMakeLists.txt`**

Change line 1 from:

```cmake
cmake_minimum_required(VERSION 3.15)
```

to:

```cmake
cmake_minimum_required(VERSION 3.22)
```

- [ ] **Step 3: Commit**

```bash
git add CMakeLists.txt tests/CMakeLists.txt
git commit -m "build: bump JUCE to 8.0.12 and CMake min to 3.22"
```

---

### Task 2: Wipe stale build cache and reconfigure

**Files:**
- Delete: `build/` (regenerated)

This forces FetchContent to pull JUCE 8.0.12. Without it, CMake reuses the cached 7.0.5 checkout.

- [ ] **Step 1: Remove the build directory**

```bash
rm -rf build
```

- [ ] **Step 2: Reconfigure**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

Expected: configure succeeds, FetchContent downloads JUCE 8.0.12 into `build/_deps/juce-src/`. If configure fails on `juce_set_vst3_sdk_path`, proceed to Task 2a; otherwise skip it.

- [ ] **Step 3 (verification): Confirm JUCE 8 was fetched**

```bash
grep -E "^#define JUCE_(MAJOR|MINOR|BUILDNUMBER)_VERSION" build/_deps/juce-src/modules/juce_core/juce_core.h | head -3
```

Expected:

```
#define JUCE_MAJOR_VERSION      8
#define JUCE_MINOR_VERSION      0
#define JUCE_BUILDNUMBER        12
```

No commit yet — this task only mutates the build dir, which is git-ignored.

---

### Task 2a (contingency): Remove `juce_set_vst3_sdk_path` if it errors

Only run this task if Task 2 Step 2 reported an error referencing `juce_set_vst3_sdk_path` or a missing VST3 SDK path. JUCE 8 bundles the VST3 SDK at a different location; the explicit path setter is no longer necessary in most configurations.

**Files:**
- Modify: `CMakeLists.txt:28`

- [ ] **Step 1: Delete the explicit VST3 SDK path line**

Remove this line:

```cmake
juce_set_vst3_sdk_path("build/_deps/juce-src/modules/juce_audio_processors/format_types/VST3_SDK")
```

(Leave the `# juce_set_aax_sdk_path(...)` comment line untouched.)

- [ ] **Step 2: Reconfigure**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

Expected: configure succeeds.

- [ ] **Step 3: Commit**

```bash
git add CMakeLists.txt
git commit -m "build: drop explicit juce_set_vst3_sdk_path (bundled by JUCE 8)"
```

---

### Task 3: Migrate `AboutBox` Font construction

**Files:**
- Modify: `src/AboutBox.cpp:22`

- [ ] **Step 1: Replace the deprecated `Font` constructor**

Change line 22 from:

```cpp
    title.setFont(juce::Font(16.0f, juce::Font::bold));
```

to:

```cpp
    title.setFont(juce::Font(juce::FontOptions{}.withHeight(16.0f).withStyle("Bold")));
```

Rationale: JUCE 8 deprecates `Font(float, int)`. `FontOptions` is the supported builder. "Bold" matches JUCE's named style.

- [ ] **Step 2: Build the plugin target only**

```bash
cmake --build build --target MidiChords -- -j
```

Expected: target compiles without `AboutBox.cpp` deprecation warnings about `Font(float, int)`. Other deprecation warnings (e.g. from ChordView) may remain — they are addressed in Task 4.

- [ ] **Step 3: Commit**

```bash
git add src/AboutBox.cpp
git commit -m "fix: migrate AboutBox title font to FontOptions for JUCE 8"
```

---

### Task 4: Migrate `ChordView` Font construction and string width

**Files:**
- Modify: `src/ChordView.cpp:81`
- Modify: `src/ChordView.cpp:124`
- Modify: `src/ChordView.cpp:127`
- Modify: `src/ChordView.cpp:152`
- Modify: `src/ChordView.cpp:158`

`Graphics::setFont(float)` and `Font(name, size, style)` are both deprecated; `Font::getStringWidthFloat` was deprecated in JUCE 8.0.2 — replace with `juce::GlyphArrangement::getStringWidth(font, text)`.

- [ ] **Step 1: Replace `g.setFont(15.0)` in `drawMeasures`**

Change line 81 from:

```cpp
    g.setFont(15.0);
```

to:

```cpp
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(15.0f)));
```

- [ ] **Step 2: Replace `g.setFont(fontSize)` and the two `Font` constructions in `drawChords`**

Change lines 124–127 from:

```cpp
    g.setFont(fontSize);
    Font defaultFont = g.getCurrentFont();
    defaultFont.setExtraKerningFactor(static_cast<float>(-0.05));
    Font symbolFont = Font("Bravura Text", fontSize, Font::plain);
```

to:

```cpp
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(fontSize)));
    Font defaultFont = g.getCurrentFont();
    defaultFont.setExtraKerningFactor(static_cast<float>(-0.05));
    Font symbolFont(juce::FontOptions{}.withName("Bravura Text").withHeight(fontSize).withStyle("Regular"));
```

Notes:
- `Font::plain` maps to `"Regular"` in the new FontOptions string-style API.
- `getCurrentFont()` returns the new portable-metrics `Font`; `setExtraKerningFactor` is unchanged.

- [ ] **Step 3: Replace the two `getStringWidthFloat` calls**

Change line 152 from:

```cpp
                        textBox.setX(textBox.getX() + defaultFont.getStringWidthFloat(sPart) + spacer);
```

to:

```cpp
                        textBox.setX(textBox.getX() + juce::GlyphArrangement::getStringWidth(defaultFont, sPart) + spacer);
```

Change line 158 from:

```cpp
                    textBox.setX(textBox.getX() + symbolFont.getStringWidthFloat(*symbol) + spacer);
```

to:

```cpp
                    textBox.setX(textBox.getX() + juce::GlyphArrangement::getStringWidth(symbolFont, *symbol) + spacer);
```

- [ ] **Step 4: Build the plugin target**

```bash
cmake --build build --target MidiChords -- -j
```

Expected: compiles. JUCE 8 deprecation warnings for `ChordView.cpp` should be gone. If `GlyphArrangement` is reported as undefined, add `#include <juce_graphics/juce_graphics.h>` near the top of `ChordView.cpp` — but it is normally pulled in via JuceHeader and should not be needed.

- [ ] **Step 5: Commit**

```bash
git add src/ChordView.cpp
git commit -m "fix: migrate ChordView fonts and string-width to JUCE 8 APIs"
```

---

### Task 5: Drop stale `AudioPlayHead::CurrentPositionInfo`

**Files:**
- Modify: `src/PluginProcessor.cpp:148`

The variable `info` is never read; the code already uses the newer `getPosition()` API on the following line. The `CurrentPositionInfo` type is fully deprecated in JUCE 8.

- [ ] **Step 1: Delete the dead declaration**

Remove line 148:

```cpp
    AudioPlayHead::CurrentPositionInfo info;
```

(The blank line that follows can stay or go — either is fine.)

- [ ] **Step 2: Build the plugin target**

```bash
cmake --build build --target MidiChords -- -j
```

Expected: compiles with no references to `CurrentPositionInfo`.

- [ ] **Step 3: Commit**

```bash
git add src/PluginProcessor.cpp
git commit -m "cleanup: remove unused AudioPlayHead::CurrentPositionInfo declaration"
```

---

### Task 6: Full build green

**Files:** none (verification only).

- [ ] **Step 1: Full build of all targets**

```bash
cmake --build build -- -j
```

Expected: every target builds — `MidiChords`, `MidiChords_AU`, `MidiChords_Standalone`, `MidiChords_VST3`, and `tests`. No errors.

- [ ] **Step 2: Triage remaining warnings**

```bash
cmake --build build -- -j 2>&1 | grep -E "warning|deprecated" | grep -E "/src/" | sort -u
```

Expected: zero deprecation warnings originating from files in `src/`. JUCE-internal warnings are not the project's problem and can stay.

If any new project-level warnings appear, fix them in a small follow-up commit before moving on.

---

### Task 7: Run tests

**Files:** none (verification only).

- [ ] **Step 1: Execute the Catch2 suite**

```bash
ctest --test-dir build -j 8 --output-on-failure
```

Expected: all tests pass. The test suite covers `MidiStore`, `ChordName`, and `ChordClipper` — none of which use the font APIs touched above, so they should pass without code changes. If a test fails, treat it as a real regression and stop the plan.

---

### Task 8: Manual UI smoke test (Standalone)

**Files:** none (verification only). The user must perform this step; the agent cannot drive a GUI.

- [ ] **Step 1: Locate and launch the Standalone build**

```bash
find build -name "MidiChords" -path "*Standalone*" -perm -u+x
```

Pick the executable inside the `Standalone.app` bundle (e.g. `build/MidiChords_artefacts/Debug/Standalone/MidiChords.app/Contents/MacOS/MidiChords`) and open it:

```bash
open build/MidiChords_artefacts/Debug/Standalone/MidiChords.app
```

- [ ] **Step 2: Visually verify**

Check that:

1. The About box renders the title in bold at a reasonable size (the FontOptions migration in Task 3 was equivalent to the old `Font(16.0f, Font::bold)`).
2. The chord view renders chord names at the configured font size; sharps/flats (`♯`/`♭`) appear as glyphs from the Bravura Text font if it is installed — otherwise fall back to plain "#"/"b" (existing behavior).
3. Measure numbers in `ChordView::drawMeasures` are positioned reasonably (this is the most likely place to see drift from the "portable" metrics default).
4. Chord-name spacing inside `drawChords` looks correct — the `GlyphArrangement::getStringWidth` replacement is the only width math in that loop.

- [ ] **Step 3 (contingency): if text drifts noticeably**

If any of the above renders meaningfully worse than on JUCE 7, restore legacy metrics by adding a `getDefaultMetricsKind()` override to the existing LookAndFeel subclass:

File: `src/PluginEditor.h:32` declares `juce::LookAndFeel_V3 lookAndFeel;`. Replace that with a small subclass.

Patch sketch — add inside the `MidiChordsAudioProcessorEditor` class, above the `lookAndFeel` member:

```cpp
struct LegacyMetricsLookAndFeel : public juce::LookAndFeel_V3
{
    juce::TypefaceMetricsKind getDefaultMetricsKind() const override
    {
        return juce::TypefaceMetricsKind::legacy;
    }
};
```

…and change the `lookAndFeel` member to `LegacyMetricsLookAndFeel lookAndFeel;`.

Rebuild (Task 6 Step 1) and recheck. Commit only if the override is actually needed:

```bash
git add src/PluginEditor.h
git commit -m "fix: restore legacy font metrics for editor LookAndFeel"
```

If Step 2 looked good without this, skip this step entirely — do not preemptively pin legacy metrics.

---

### Task 9: Update README mention of JUCE version (if present)

**Files:**
- Modify: `README.md` (only if it references a JUCE version).

- [ ] **Step 1: Check for a JUCE version mention**

```bash
grep -n -iE "juce.{0,15}[0-9]+\.[0-9]+" README.md
```

If no matches: skip the rest of this task.

If matches: update the version string to "8.0.12" (or "JUCE 8") in place, preserving surrounding text.

- [ ] **Step 2: Commit (only if README was edited)**

```bash
git add README.md
git commit -m "docs: note JUCE 8 upgrade in README"
```

---

## Self-review

- **Spec coverage:** Every JUCE 7→8 breaking change identified in pre-flight has a task: CMake bump (Task 1), Font constructors (Tasks 3, 4), `getStringWidthFloat` (Task 4), stale `CurrentPositionInfo` (Task 5), metrics-kind contingency (Task 8 Step 3). VST3 SDK path has a contingency (Task 2a).
- **No placeholders:** Each step has the literal old/new code, the exact command to run, and the expected outcome.
- **Type consistency:** `juce::FontOptions{}` builder is used identically across Tasks 3 and 4. `juce::GlyphArrangement::getStringWidth(font, text)` signature is identical at both call sites.
- **Commit cadence:** One commit per logical change (Tasks 1, 3, 4, 5, optionally 2a, optionally 8 contingency, optionally 9). Tasks 2, 6, 7, 8 are verification-only and produce no commits.
