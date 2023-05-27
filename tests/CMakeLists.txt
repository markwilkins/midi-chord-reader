cmake_minimum_required(VERSION 3.15)
project(tests) 

find_package(Catch2 3 REQUIRED)
    

#FetchContent_MakeAvailable(JUCE)

#include_directories (../Source
#${CMAKE_SOURCE_DIR}
#)

#note to self: For next test, just add it to this list
add_executable(${PROJECT_NAME} 
    midiStoreTest.cpp
    chordNameTest.cpp
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        MidiChords  
        juce::juce_audio_utils
        Catch2::Catch2WithMain
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
        )

add_compile_options(-fstandalone-debug)


# juce_generate_juce_header(tests)
include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
