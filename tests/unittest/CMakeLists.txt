project(UnitTest)

set(SOURCES
    catch.cpp
    testUtils.cpp
    testCoFloCoWrapper.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME}
    PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS} ${WARNING_FLAGS}"
)
target_link_libraries(${PROJECT_NAME} gpscat-libs)
add_test(${PROJECT_NAME} ${PROJECT_BINARY_DIR}/${PROJECT_NAME})
