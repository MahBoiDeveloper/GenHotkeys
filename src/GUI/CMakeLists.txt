# General settings
    # Definitions from ../CMakeLists.txt
    get_property(PROJECT_DESTINATION_RESOURCES_FOLDER GLOBAL PROPERTY PROJECT_DESTINATION_SOURCES_FOLDER)
    get_property(PROJECT_RESOURCE_FOLDER              GLOBAL PROPERTY PROJECT_RESOURCE_FOLDER)
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
    target_link_libraries(${TARGET_NAME} PRIVATE GenHotkeysInfo)

    # Define a list of all GUI sources
    file(
        GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]pp
    )

    # Assign source files to the GUI target
    target_sources(${TARGET_NAME} PRIVATE ${SOURCES})

    # Assign qt sources file to the GUI target
    target_sources(${TARGET_NAME} PRIVATE ${PROJECT_RESOURCE_FOLDER}/QtSources/CompilationSources.qrc)

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
        set(TS_FILE ${PROJECT_RESOURCE_FOLDER}/Translations/${LANGUAGE}.ts)

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
