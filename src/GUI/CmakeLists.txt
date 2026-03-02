# General settings
    # Definitions from ../CMakeLists.txt
    get_property(PROJECT_DESTINATION_RESOURCES_FOLDER GLOBAL PROPERTY PROJECT_DESTINATION_SOURCES_FOLDER)
    get_property(PROJECT_LIBS_FOLDER                  GLOBAL PROPERTY PROJECT_LIBS_FOLDER)

    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    # Declare required qt libs
    set(QT_LIBS Core Widgets)

    # Find qt packages
    find_package(QT NAMES Qt5 Qt6 REQUIRED)

    # Find qt libs
    foreach(LIB ${QT_LIBS})
        find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${LIB})
    endforeach()

# GUI target configuration
    # Declare GUI target name
    set(TARGET_NAME GUI)

    # Define GUI target
    add_library(${TARGET_NAME} OBJECT)

    # Define a list of all GUI sources
    file(
        GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]pp
    )

    # Assign source files to the GUI target
    target_sources(${TARGET_NAME} PRIVATE ${SOURCES})

    # Assign qt sources file to the GUI target
    target_sources(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/QtSources/CompilationSources.qrc)

    # link qt libs
    foreach(LIB ${QT_LIBS})
        target_link_libraries(${TARGET_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::${LIB})
    endforeach()

# Translation files configuration
    # Declare available translation languages
    set(LANGUAGES ru)

    # Find translation programs
    find_program(LUPDATE_EXECUTABLE  lupdate)
    find_program(LRELEASE_EXECUTABLE lrelease)

    # Create translation binaries folder
    set(TRANSLATION_DESTINATION_FOLDER ${PROJECT_DESTINATION_RESOURCES_FOLDER}/Translations)
    file(MAKE_DIRECTORY ${TRANSLATION_DESTINATION_FOLDER})

    # Generate translation files
    foreach(LANGUAGE ${LANGUAGES})

        # Define source translation file
        set(TS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/translations/${LANGUAGE}.ts)

        # Define all source translation files
        set(TRANSLATION_SOURCES ${TRANSLATION_SOURCES} ${TS_FILE})

        # Define binary translation file
        set(QM_FILE ${TRANSLATION_DESTINATION_FOLDER}/${LANGUAGE}.qm)
        
        # Define command to create translation files
        add_custom_command(
            # PRE_BUILD
            OUTPUT ${QM_FILE}
            # Generate translation source file
            COMMAND ${LUPDATE_EXECUTABLE} ${CMAKE_SOURCE_DIR} -recursive -ts ${TS_FILE} # -no-obsolete
            # Generate translation binary file
            COMMAND ${LRELEASE_EXECUTABLE} ${TS_FILE} -qm ${QM_FILE}
            MAIN_DEPENDENCY ${SOURCES} ${TS_FILE}
            DEPENDS ${SOURCES} ${TS_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    endforeach()

    # Assign translation source files to the GUI target
    target_sources(${TARGET_NAME} PRIVATE ${TRANSLATION_SOURCES})

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
    add_dependencies(${TARGET_NAME} DiscordRPCTarget)

    # Make the libwebp target output file visible to the GUI target
    target_link_directories(${TARGET_NAME} PUBLIC ${PROJECT_LIBS_FOLDER})

    # Link the libwebp target to the GUI target
    target_link_libraries(${TARGET_NAME} PRIVATE ${DRPC_TARGET_NAME})

# LibWEBP configuration
    set(LIB_WEBP_TARGET_NAME webpdecoder)                         # Define lib folder path
    set(LIB_WEBP_TARGET_FILE "lib${LIB_WEBP_TARGET_NAME}.a")      # Define lib file
    set(LIB_WEBP_SOURCE_DIR  ${PROJECT_LIBS_FOLDER}/libwebp)      # Define lib folder path
    set(LIB_WEBP_BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/libwebp) # Define lib build folder

    # Create a lib build folder
    file(MAKE_DIRECTORY ${LIB_WEBP_BINARY_DIR})
    
    # Compile WEBP library
    add_custom_command(
        OUTPUT "${PROJECT_LIBS_FOLDER}/${LIB_WEBP_TARGET_FILE}"
        # Cmake configure
        COMMAND ${CMAKE_COMMAND} --no-warn-unused-cli
                                 -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE
                                 -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                 -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                                 -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                                 -G ${CMAKE_GENERATOR}
                                 ${LIB_WEBP_SOURCE_DIR}
        # Cmake build
        COMMAND ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE} --target ${LIB_WEBP_TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_WEBP_BINARY_DIR}/${LIB_WEBP_TARGET_FILE}" "${PROJECT_LIBS_FOLDER}"
        WORKING_DIRECTORY ${LIB_WEBP_BINARY_DIR}
    )

    # Define the target that is launched by the libwebp build command and activates the rebuild of the main target 
    add_custom_target(LibWebpTarget DEPENDS "${PROJECT_LIBS_FOLDER}/${LIB_WEBP_TARGET_FILE}")

    # Add a libwebp target as a dependency to GUI target
    add_dependencies(${TARGET_NAME} LibWebpTarget)

    # Make the libwebp target output file visible to the GUI target
    target_link_directories(${TARGET_NAME} PUBLIC ${PROJECT_LIBS_FOLDER})

    # Link the libwebp target to the GUI target
    target_link_libraries(${TARGET_NAME} PRIVATE ${LIB_WEBP_TARGET_NAME})

# Copying folders with configs, but without update
    # Copy icons
    file(
        COPY "${CMAKE_CURRENT_SOURCE_DIR}/Icons"
        DESTINATION ${PROJECT_DESTINATION_RESOURCES_FOLDER}
    )
    
    # Copy theme files
    file(
        COPY "${CMAKE_CURRENT_SOURCE_DIR}/Theme"
        DESTINATION ${PROJECT_DESTINATION_RESOURCES_FOLDER}
    )

# Copy Styles.css with updates
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Theme/Styles.css" "${PROJECT_DESTINATION_RESOURCES_FOLDER}/Theme/Styles.css")
