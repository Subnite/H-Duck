add_subdirectory(${JUCE_SOURCE_DIR} JUCE)
message("****Included JUCE at " ${JUCE_SOURCE_DIR})

# To configure these things look at the options in: https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md
juce_add_plugin(${PLUGIN_PROJECT_NAME}
    COMPANY_NAME "Subnite Plugins"                          # Specify the name of the plugin's author
    IS_SYNTH FALSE                       # Is this a synth or an effect?
    NEEDS_MIDI_INPUT TRUE               # Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT FALSE              # Does the plugin need midi output?
    IS_MIDI_EFFECT FALSE                 # Is this plugin a MIDI effect?
    PLUGIN_MANUFACTURER_CODE SBNT               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Htdk                            # A unique four-character plugin id with exactly one upper-case character
    FORMATS VST3                  # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME ${PLUGIN_VST3_NAME}        # The name of the final executable, which can differ from the target name
    DESCRIPTION "An easy ducking tool for all the weebs out there"
    VST3_CATEGORIES "Fx"
    COPY_PLUGIN_AFTER_BUILD FALSE       # doing this with custom script so don't enable
    VST3_COPY_DIR ${POST_BUILD_PLUGIN_COPY_DIR}
)

message("****Added juce plugin")
# add your source files here
target_sources(${PLUGIN_PROJECT_NAME}
    PRIVATE
        GUI/Curve.cpp
        GUI/CustomSliders.cpp
        PluginEditor.cpp
        PluginProcessor.cpp
)
message("****Added target sources")

target_compile_definitions(${PLUGIN_PROJECT_NAME}
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
)

message("****Added target compile definitions")

target_link_libraries(${PLUGIN_PROJECT_NAME}
    PRIVATE
        # AudioPluginData           # If we'd created a binary data target, we'd link to it here
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
message("****Linked target libraries")

message("****Modules includes at: ${JUCE_SOURCE_DIR}/modules")

target_include_directories(${PLUGIN_PROJECT_NAME}
    PUBLIC
        "${JUCE_SOURCE_DIR}/modules"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/GUI"
        "${CMAKE_CURRENT_SOURCE_DIR}/DSP"
        "${CMAKE_CURRENT_SOURCE_DIR}/Common"
)

# optional for less warnings with CLANG
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
message("****No sign conversion etc")
target_compile_options(${PLUGIN_PROJECT_NAME}
    PRIVATE
    -w
)
endif()

juce_generate_juce_header(${PLUGIN_PROJECT_NAME})

# add command that copies the output to another directory
get_target_property(VST3_PATH ${PROJECT_NAME}_VST3 JUCE_PLUGIN_ARTEFACT_FILE)

if (${COPY_VST3_AFTER_BUILD})
    foreach(item ${LIST_OF_POST_BUILD_VST3_COPY_PATHS})
        message("****Added Copy Command To: ${item}")
        add_custom_command(
            TARGET ${PROJECT_NAME}_VST3 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${VST3_PATH}/Contents/x86_64-win ${item}/${PLUGIN_VST3_NAME}
        )
    endforeach()

endif()

if (${COPY_JUCEHEADER_AFTER_BUILD})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_PROJECT_NAME}_artefacts/JuceLibraryCode/JuceHeader.h ${JUCEHEADER_COPY_DIR}
        COMMENT "****Copying JuceHeader.h to: ${JUCEHEADER_COPY_DIR}"
    )
endif()
# Try to run something post build
if (${LAUNCH_EXE_AFTER_BUILD})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${POST_BUILD_EXECUTABLE_PATH}"
        COMMENT "****Running custom post build executable: ${POST_BUILD_EXECUTABLE_PATH}"
    )
endif()
