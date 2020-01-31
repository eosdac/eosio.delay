add_contract(eosio.delay eosio.delay ${CMAKE_CURRENT_SOURCE_DIR}/src/eosio.delay.cpp)

target_include_directories(eosio.delay
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(eosio.delay
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/eosio.delay.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/eosio.delay.contracts.md @ONLY )

target_compile_options( eosio.msig PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
