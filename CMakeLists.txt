# General info
set(JUCE_SOURCE_DIR "C:/Program Files/JUCE_v8.0.1") # set juce source dir
set(PLUGIN_PROJECT_NAME "HentaiDuck") # preferably no spaces
set(PLUGIN_VST3_NAME "HentaiDuck") # name of the actual vst3 file

# Post Build Params
set(COPY_VST3_AFTER_BUILD TRUE) # will copy to the paths in the list below if set to 'TRUE'
set(LIST_OF_POST_BUILD_VST3_COPY_PATHS 
    ${CMAKE_CURRENT_SOURCE_DIR} # the directory of this CMakeLists.txt file
    "C:/VST ALL/VST3 Custom"
    "C:/Program Files/Common Files/VST3"
)

set(LAUNCH_EXE_AFTER_BUILD FALSE) # launches the executable specified below if set to 'TRUE'
set(POST_BUILD_EXECUTABLE_PATH "C:/Program Files/JUCE/Builds/extras/AudioPluginHost/AudioPluginHost_artefacts/Debug/AudioPluginHost.exe")

set(COPY_JUCEHEADER_AFTER_BUILD FALSE) # copies the generated juce header into the selected directory
set (JUCEHEADER_COPY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source)

# ===========================================================================================
cmake_minimum_required(VERSION 3.15)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # will create compile_commands.json for include paths etc IF GENERATOR IS A MAKEFILE like MinGW Makefiles

project(${PLUGIN_PROJECT_NAME} VERSION 0.1.0)
add_subdirectory(Source) # creates the plugin, set other params in there
