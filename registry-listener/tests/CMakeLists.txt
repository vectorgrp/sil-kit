find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_executable(registry-listener-test-participant Participant.cpp ../src/RegistryEventListener.cpp)
target_compile_features(registry-listener-test-participant PRIVATE cxx_std_17)
target_link_libraries(registry-listener-test-participant PRIVATE I_SilKit S_SilKitImpl)
target_compile_definitions(registry-listener-test-participant PRIVATE EXPORT_SilKitAPI)

add_test(NAME registry-listener-smoke
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/smoke.py"
        "$<TARGET_FILE:registry-listener>"
        "$<TARGET_FILE:registry-listener-test-participant>"
)
set_tests_properties(registry-listener-smoke PROPERTIES TIMEOUT 90)
