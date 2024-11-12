add_executable(calc "main.cc")
add_dependencies(calc mklisp)
target_link_libraries(calc mklisp)

set_property(TARGET calc PROPERTY CXX_STANDARD 17)
