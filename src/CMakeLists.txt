# General settings
    # Definitions from ../CMakeLists.txt
    get_property(PROJECT_DESTINATION_FOLDER     GLOBAL PROPERTY PROJECT_DESTINATION_FOLDER)     # Exe folder
    get_property(PROJECT_DESTINATION_BIN_FOLDER GLOBAL PROPERTY PROJECT_DESTINATION_BIN_FOLDER) # Bin folder
    get_property(PROJECT_LIBS_FOLDER            GLOBAL PROPERTY PROJECT_LIBS_FOLDER)            # Lib folder
    get_property(PROJECT_RESOURCE_FOLDER        GLOBAL PROPERTY PROJECT_RESOURCE_FOLDER)        # Resource folder

    # Additional paths
    set(STEAMAPIDLL ${PROJECT_LIBS_FOLDER}/SteamworksSDK/redistributable_bin/win64/steam_api64.dll)

    # Extracting commit hash
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    else()
        set(GIT_COMMIT_HASH "unknown")
    endif()

    # Add to the argument list variable
    add_definitions(-DGIT_COMMIT_HASH="${GIT_COMMIT_HASH}")

    # Set debug compilation flags
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fexceptions")

    # Set release compilation flags
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

    # Look at cmake.org about how to link .dlls           - https://cmake.org/pipermail/cmake/2018-August/068011.html
    # And look at gnu.org about how to work ld.exe linker - https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_node/ld_3.html#:~:text=Output%20Sections).-,%2D%2Dwhole%2Darchive,-For%20each%20archive
    # --no-whole-archive used because --whole-archive gives an error at linking time with .rc files
    # 3 days cmake and this fucking config pissing me off (c) mah_boi
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static -lwinpthread")

# Main target configuration
    # Editor target name
    set(MAIN_TARGET_NAME "HotkeyEditor")
    set(ACTUAL_EXE_NAME  "LoadEditor")
    
    # Define editor executable
    add_executable(${MAIN_TARGET_NAME})
    target_link_libraries(${MAIN_TARGET_NAME} PRIVATE GenHotkeysInfo)

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

# LZMA SDK configuration
    set(LZMA_TARGET_NAME lzma-sdk)                                     # Define lib target name
    set(LZMA_TARGET_FILE "lib${LZMA_TARGET_NAME}.a")                   # Define lib file
    set(LZMA_SOURCE_DIR  "${PROJECT_LIBS_FOLDER}/lzma-sdk")            # Define lib source folder
    set(LZMA_BINARY_DIR  "${CMAKE_CURRENT_BINARY_DIR}/lzma-sdk")       # Define lib build folder
    set(LZMA_LIBRARY     "${PROJECT_LIBS_FOLDER}/${LZMA_TARGET_FILE}")

    # Reuse the saved archive after deleting build, including with Ninja.
    if(NOT EXISTS "${LZMA_LIBRARY}")
        # The reduced 7z reader/writer from Bundles/Format7zR/makefile.
        set(LZMA_SOURCES)
        function(lzma_sdk_sources directory)
            foreach(name IN LISTS ARGN)
                list(APPEND LZMA_SOURCES "${LZMA_SOURCE_DIR}/${directory}/${name}")
            endforeach()
            set(LZMA_SOURCES "${LZMA_SOURCES}" PARENT_SCOPE)
        endfunction()

        lzma_sdk_sources(CPP/Common
            CRC.cpp           IntToString.cpp LzFindPrepare.cpp MyString.cpp
            StringConvert.cpp StringToInt.cpp MyVector.cpp      Wildcard.cpp)
        
        lzma_sdk_sources(CPP/Windows
            FileDir.cpp         FileFind.cpp FileIO.cpp    FileName.cpp
            Synchronization.cpp System.cpp   TimeUtils.cpp PropVariant.cpp)
        
        lzma_sdk_sources(CPP/7zip/Common
            CreateCoder.cpp CWrappers.cpp      InBuffer.cpp   InOutTempBuffer.cpp
            FilterCoder.cpp LimitedStreams.cpp MethodId.cpp   MethodProps.cpp
            OutBuffer.cpp   ProgressUtils.cpp  PropId.cpp     StreamBinder.cpp 
            StreamUtils.cpp UniqBlocks.cpp     VirtThread.cpp StreamObjects.cpp)
        
        lzma_sdk_sources(CPP/7zip/Archive/Common
            CoderMixer2.cpp      HandlerOut.cpp      InStreamWithCRC.cpp 
            OutStreamWithCRC.cpp ParseProperties.cpp ItemNameUtils.cpp)
        
        lzma_sdk_sources(CPP/7zip/Archive/7z
            7zCompressionMode.cpp 7zDecode.cpp  7zEncode.cpp     7zExtract.cpp
            7zFolderInStream.cpp  7zHandler.cpp 7zHandlerOut.cpp 7zHeader.cpp
            7zIn.cpp              7zOut.cpp     7zProperties.cpp 7zSpecStream.cpp
            7zUpdate.cpp)
        
        lzma_sdk_sources(CPP/7zip/Compress
            Bcj2Coder.cpp     Bcj2Register.cpp   BcjCoder.cpp     BcjRegister.cpp
            BranchMisc.cpp    BranchRegister.cpp ByteSwap.cpp     CopyCoder.cpp
            CopyRegister.cpp  DeltaFilter.cpp    Lzma2Decoder.cpp Lzma2Encoder.cpp
            Lzma2Register.cpp LzmaDecoder.cpp    LzmaEncoder.cpp  LzmaRegister.cpp)
        
        lzma_sdk_sources(C
            7zStream.c   Alloc.c    Bcj2.c    Bcj2Enc.c  Bra.c       Bra86.c
            CpuArch.c    Delta.c    LzFind.c  LzFindMt.c LzFindOpt.c Lzma2Dec.c
            Lzma2DecMt.c Lzma2Enc.c LzmaDec.c LzmaEnc.c  MtCoder.c   MtDec.c
            SwapBytes.c  Threads.c  7zCrc.c   7zCrcOpt.c BraIA64.c)

        # Compile the SDK.
        add_library(${LZMA_TARGET_NAME} STATIC EXCLUDE_FROM_ALL ${LZMA_SOURCES})
        target_include_directories(${LZMA_TARGET_NAME} PRIVATE "${LZMA_SOURCE_DIR}")
        target_compile_definitions(${LZMA_TARGET_NAME} PRIVATE Z7_NO_CRYPTO UNICODE _UNICODE)
        set_target_properties(${LZMA_TARGET_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${LZMA_BINARY_DIR}")

        # Store the static library outside build for subsequent builds.
        add_custom_command(
            OUTPUT "${LZMA_LIBRARY}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${LZMA_TARGET_NAME}>" "${LZMA_LIBRARY}"
            DEPENDS ${LZMA_TARGET_NAME}
            VERBATIM
        )
    endif()

    # Allow the SDK to be built separately from the editor.
    add_custom_target(LzmaSDKTarget DEPENDS "${LZMA_LIBRARY}")

    # Expose the saved archive, headers and required definitions to the editor.
    add_library(LzmaSdk STATIC IMPORTED GLOBAL)
    set_target_properties(LzmaSdk PROPERTIES
        IMPORTED_LOCATION "${LZMA_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${LZMA_SOURCE_DIR}"
        INTERFACE_COMPILE_DEFINITIONS Z7_NO_CRYPTO
        INTERFACE_LINK_LIBRARIES "oleaut32;uuid"
    )
    add_dependencies(LzmaSdk LzmaSDKTarget)
    add_dependencies(${MAIN_TARGET_NAME} LzmaSDKTarget)

    # Keep codec registration objects when linking only this static library.
    target_link_libraries(${MAIN_TARGET_NAME} PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,LzmaSdk>")

# Discord RPC configuration
    set(DRPC_TARGET_NAME discord-rpc)                             # Define lib folder path
    set(DRPC_TARGET_FILE "lib${DRPC_TARGET_NAME}.a")              # Define lib file
    set(DRPC_SOURCE_DIR  ${PROJECT_LIBS_FOLDER}/discord-rpc)      # Define lib folder path
    set(DRPC_BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/discord-rpc) # Define lib build folder

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

    # Add a DiscordRPC target as a dependency to GUI exe target
    add_dependencies(${MAIN_TARGET_NAME} DiscordRPCTarget)

    # Make the DiscordRPC target output file visible to the GUI exe target
    target_link_directories(${MAIN_TARGET_NAME} PUBLIC ${PROJECT_LIBS_FOLDER})

    # Link the DiscordRPC target to the GUI exe target
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
    target_link_libraries(${STARTER_TARGET_NAME} PRIVATE GenHotkeysInfo)

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

# Copying Resource folder with subfolders, configs and files
    # https://discourse.cmake.org/t/copying-files-with-destination-folder-depending-on-build-config/10686
    add_custom_target(SyncResourceDir
	    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different 
            "${PROJECT_RESOURCE_FOLDER}"
            "${PROJECT_DESTINATION_RESOURCES_FOLDER}"
        COMMENT "Synchronize resources directories"
    )

    add_dependencies(${MAIN_TARGET_NAME} SyncResourceDir)

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
