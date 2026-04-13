# General settings
    # Definitions from ../CMakeLists.txt
    get_property(PROJECT_DESTINATION_FOLDER     GLOBAL PROPERTY PROJECT_DESTINATION_FOLDER)     # Exe folder
    get_property(PROJECT_DESTINATION_BIN_FOLDER GLOBAL PROPERTY PROJECT_DESTINATION_BIN_FOLDER) # Bin folder
    get_property(PROJECT_LIBS_FOLDER            GLOBAL PROPERTY PROJECT_LIBS_FOLDER)            # Lib folder
    get_property(PROJECT_RESOURCE_FOLDER        GLOBAL PROPERTY PROJECT_RESOURCE_FOLDER)        # Resource folder

    # Additional paths
    set(STEAMAPIDLL ${PROJECT_LIBS_FOLDER}/SteamworksSDK/redistributable_bin/win64/steam_api64.dll)

    # Set debug compilation flags
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fexceptions")

    # Set release compilation flags
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

    # Look at cmake.org about how to link .dlls           - https://cmake.org/pipermail/cmake/2018-August/068011.html
    # And look at gnu.org about how to work ld.exe linker - https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_node/ld_3.html#:~:text=Output%20Sections).-,%2D%2Dwhole%2Darchive,-For%20each%20archive
    # --no-whole-archive used due to because --whole-archive gives error at linking time with .rc files
    # 3 days pissing off cmake and this fucking config (c) mah_boi
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static -lwinpthread")

# Main target configuration
    # Editor target name
    set(MAIN_TARGET_NAME "HotkeyEditor")
    set(ACTUAL_EXE_NAME  "LoadEditor")
    
    # Define editor executable
    add_executable(${MAIN_TARGET_NAME})

    # Define Steam API headers
    file(
        GLOB_RECURSE STEAMWORKS_SDK_HEADERS
        ${PROJECT_LIBS_FOLDER}/SteamworksSDK/public/steam/*.h
    )

    # Define a list of all sources 
    file(
        GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]pp
    )

    # Define a list of all GUI sources
    file(
        GLOB_RECURSE EXCLUDED_GRAPHIC_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/GUI/*.[ch]pp
    )

    # Remove all GUI sources from main sources list
    list(REMOVE_ITEM SOURCES ${EXCLUDED_GRAPHIC_SOURCES})
    
    # Remove launch target source file from main sources list
    list(REMOVE_ITEM SOURCES "MainStarter.cpp")

    # Assign source files to the main target
    target_sources(
        ${MAIN_TARGET_NAME}
        PRIVATE
        ${STEAMWORKS_SDK_HEADERS}
        ${SOURCES}
        "${PROJECT_RESOURCE_FOLDER}/QtSources/EditorDescription.rc"
    )

    # Set output directory
    set_target_properties(
        ${MAIN_TARGET_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_DESTINATION_BIN_FOLDER}
        OUTPUT_NAME ${ACTUAL_EXE_NAME}
    )
    
    # Add a Windows library dependency to the main target
    target_link_libraries(${MAIN_TARGET_NAME} PRIVATE rpcrt4)
    
    # Link Steamworks SDK win64 with main target's exe file and copy file
    target_link_libraries(${MAIN_TARGET_NAME} PRIVATE ${STEAMAPIDLL})
    configure_file(${STEAMAPIDLL} ${PROJECT_DESTINATION_BIN_FOLDER} COPYONLY)

    # Attach GUI subproject
    add_subdirectory(GUI)

    # Add graphic dependency to the main target
    target_link_libraries(${MAIN_TARGET_NAME} PRIVATE GUI)

# Discord RPC configuration
    set(DRPC_TARGET_NAME discord-rpc)                            # Define lib folder path
    set(DRPC_TARGET_FILE "lib${DRPC_TARGET_NAME}.a")             # Define lib file
    set(DRPC_SOURCE_DIR ${PROJECT_LIBS_FOLDER}/discord-rpc)      # Define lib folder path
    set(DRPC_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/discord-rpc) # Define lib build folder

    # Create a lib build folder
    file(MAKE_DIRECTORY ${DRPC_BINARY_DIR})

    # Compile Discord RPC library
    add_custom_command(
        OUTPUT "${PROJECT_LIBS_FOLDER}/${DRPC_TARGET_FILE}"
        # Cmake configure
        COMMAND ${CMAKE_COMMAND} --no-warn-unused-cli
                                 -DCMAKE_POLICY_VERSION_MINIMUM=3.10 # Solves error the minimum version doesn't fit
                                 -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE
                                 -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                 -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                                 -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                                 -G ${CMAKE_GENERATOR}
                                 ${DRPC_SOURCE_DIR}
        # Cmake build
        COMMAND ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE} --target ${DRPC_TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy "${DRPC_BINARY_DIR}/src/${DRPC_TARGET_FILE}" "${PROJECT_LIBS_FOLDER}"
        WORKING_DIRECTORY ${DRPC_BINARY_DIR}
    )

    # Define the target that is launched by the build command and activates the rebuild of the main target 
    add_custom_target(DiscordRPCTarget DEPENDS "${PROJECT_LIBS_FOLDER}/${DRPC_TARGET_FILE}")

    # Add a libwebp target as a dependency to GUI target
    add_dependencies(${MAIN_TARGET_NAME} DiscordRPCTarget)

    # Make the libwebp target output file visible to the GUI target
    target_link_directories(${MAIN_TARGET_NAME} PUBLIC ${PROJECT_LIBS_FOLDER})

    # Link the libwebp target to the GUI target
    target_link_libraries(${MAIN_TARGET_NAME} PRIVATE ${DRPC_TARGET_NAME})

# Starter target configuration
    # Starter target name
    set(PROXY_EXE_NAME      "GenHotkeys")
    set(STARTER_TARGET_NAME "HotkeyEditorStarter")

    # Define starter executable with source files
    add_executable(
        ${STARTER_TARGET_NAME}
        "MainStarter.cpp"
        "${PROJECT_RESOURCE_FOLDER}/QtSources/StarterDescription.rc"
    )

    # Define output directory and file name
    set_target_properties(
        ${STARTER_TARGET_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_DESTINATION_FOLDER}
        OUTPUT_NAME ${PROXY_EXE_NAME}
    )

    # Create a preprocessor variable with the path to the main program relative to the starter
    target_compile_definitions(${STARTER_TARGET_NAME} PRIVATE
        -DPROJECT_EXE_RELATIVE_PATH="$<PATH:RELATIVE_PATH,$<TARGET_FILE:${MAIN_TARGET_NAME}>,${PROJECT_DESTINATION_FOLDER}>"
    )

# Managing additional source files
    # Declare json files
    set(
        ADDITIONAL_RESOURCES
        "${PROJECT_RESOURCE_FOLDER}/TechTree.json"
        "${PROJECT_RESOURCE_FOLDER}/Settings.json"
    )

    # Copy json files
    foreach(CURRENT_FILE ${ADDITIONAL_RESOURCES})
        configure_file(
            ${CURRENT_FILE}
            ${PROJECT_DESTINATION_RESOURCES_FOLDER}
            COPYONLY
        )
    endforeach()

    # Create hotkey preset directory
    file(MAKE_DIRECTORY ${PROJECT_DESTINATION_RESOURCES_FOLDER}/Presets)

    # Find hotkey preset files
    file(
        GLOB_RECURSE DATA_SAMPLES_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Presets/*
    )
    
    # Copy hotkey preset files
    foreach(CURRENT_FILE ${DATA_SAMPLES_FILES})
        configure_file(
            ${CURRENT_FILE}
            ${PROJECT_DESTINATION_RESOURCES_FOLDER}/Presets
            COPYONLY
        )
    endforeach()

# Qt libs linking aka copying dependecies to the standalone release version of the project
    # Find the qt libs deploy program
    find_program(WINDEPLOYQT_PROGRAM windeployqt)

    # Find the environment setup script for the deployment
    find_program(QTENV_BAT qtenv2.bat)

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        # Deploy Qt and other dlls to the Editor/Resource folder
        add_custom_command(
            TARGET ${MAIN_TARGET_NAME}
            POST_BUILD
            # Configure environment
            COMMAND ${QTENV_BAT}
            # Deploy
            COMMAND ${WINDEPLOYQT_PROGRAM}
                --compiler-runtime
                --force
                --dir $<TARGET_PROPERTY:${MAIN_TARGET_NAME},RUNTIME_OUTPUT_DIRECTORY>
                $<TARGET_FILE:${MAIN_TARGET_NAME}>
        )
    endif()
