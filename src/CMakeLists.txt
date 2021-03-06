CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.hpp"
)

INCLUDE_DIRECTORIES(
    "."
    "${CMAKE_CURRENT_BINARY_DIR}"
    ../include
)

SET(Boost_NO_BOOST_CMAKE ON)

#
# because of automatic linking to corresponding Boost libraries on
# Visual Studio we must handle it differently
#
IF(MSVC)
    IF("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        SET(_LIB_BEG "lib64")
    ELSE("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
        SET(_LIB_BEG "lib")
    ENDIF()

    IF(MSVC14)
        SET(_boost_LIB_PATH "${_LIB_BEG}-msvc-14.0")
    ELSEIF(MSVC12)
        SET(_boost_LIB_PATH "${_LIB_BEG}-msvc-12.0")
    ELSEIF(MSVC11)
        SET(_boost_LIB_PATH "${_LIB_BEG}-msvc-11.0")
    ELSEIF(MSVC10)
        SET(_boost_LIB_PATH "${_LIB_BEG}-msvc-10.0")
    ELSEIF(MSVC90)
        SET(_boost_LIB_PATH "${_LIB_BEG}-msvc-9.0")
    ELSEIF(MSVC80)
        SET(_boost_LIB_PATH "${_LIB_BEG}-msvc-8.0")
    ELSE()
        SET(_boost_LIB_PATH ${_LIB_BEG})
    ENDIF()

    SET(BOOST_ROOT C:/local/boost_1_59_0)
    SET(BOOST_LIBRARYDIR "${BOOST_ROOT}/${_boost_LIB_PATH}")
ENDIF()

FIND_PACKAGE(Boost 1.55.0 COMPONENTS program_options filesystem)
IF(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    IF(MSVC)
        LINK_DIRECTORIES("${Boost_INCLUDE_DIR}/${_boost_LIB_PATH}")
        ADD_EXECUTABLE(hashkat hashkat.cpp)
    ELSE()
        ADD_EXECUTABLE(hashkat hashkat.cpp)
        TARGET_LINK_LIBRARIES(hashkat ${Boost_LIBRARIES})
    ENDIF()
    INSTALL(TARGETS hashkat RUNTIME DESTINATION bin COMPONENT hashkat)
    IF(CONCURRENT)
        IF(MSVC)
            LINK_DIRECTORIES("${Boost_INCLUDE_DIR}/${_boost_LIB_PATH}")
            ADD_EXECUTABLE(hashkat_mt hashkat_mt.cpp)
            IF(TBB_FOUND)
                TARGET_LINK_LIBRARIES(hashkat_mt ${TBB_LIBRARIES})
            ELSE()
                TARGET_LINK_LIBRARIES(hashkat_mt tbb)
            ENDIF()
        ELSE()
            ADD_EXECUTABLE(hashkat_mt hashkat_mt.cpp)
            IF(TBB_FOUND)
                TARGET_LINK_LIBRARIES(hashkat_mt ${TBB_LIBRARIES} ${Boost_LIBRARIES})
            ELSE()
                TARGET_LINK_LIBRARIES(hashkat_mt tbb ${Boost_LIBRARIES})
            ENDIF()
        ENDIF()
        INSTALL(TARGETS hashkat_mt RUNTIME DESTINATION bin COMPONENT hashkat_mt)
    ENDIF()
    FILE(COPY INFILE.xml DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    INSTALL(FILES INFILE.xml DESTINATION share/hashkat COMPONENT hashkat)
ENDIF()
