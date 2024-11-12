find_package(re2c REQUIRED)

file(GLOB SRC *.h *.hh *.c *.cc)

add_library(mklisp)
set_target_properties(mklisp PROPERTIES CXX_STANDARD 17)

add_re2c_target(mklispLexer lexer.in.cpp ${CMAKE_CURRENT_BINARY_DIR}/lexer.in.cc)

target_sources(mklisp PRIVATE ${SRC} ${CMAKE_CURRENT_BINARY_DIR}/lexer.in.cc)
add_dependencies(mklisp mklispLexer)
