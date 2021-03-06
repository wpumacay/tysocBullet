message( "Configuring tysocBullet::tests" )

include_directories( "${CMAKE_SOURCE_DIR}/ext/bullet3/src" )
include_directories( "${CMAKE_SOURCE_DIR}/ext/tiny_math/include" )
include_directories( "${CMAKE_SOURCE_DIR}/ext/tiny_renderer/include" )
include_directories( "${CMAKE_SOURCE_DIR}/ext/imgui" )
include_directories( "${CMAKE_SOURCE_DIR}/ext/imgui/examples" )

add_subdirectory( base )
include_directories( base )

# single builder function

function( FcnBuildTest pSourcesList pExecutableName )

    add_executable( ${pExecutableName}
                    ${pSourcesList} )

    # Link only against tysocCore, as the specific backend is linked dynamically
    target_link_libraries( ${pExecutableName}
                           test_interface )

endfunction()

file( GLOB TestFiles *.cpp )

foreach( testFile ${TestFiles} )
    # remove extension from sample file
    string( REPLACE ".cpp" "" executableLongName ${testFile} )
    # remove other path components from file
    get_filename_component( executableName ${executableLongName} NAME )

    FcnBuildTest( ${testFile} ${executableName} )

endforeach( testFile )