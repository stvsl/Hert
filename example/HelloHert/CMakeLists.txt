add_executable(HelloHert
    main.cpp
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

target_link_libraries(HelloHert
    PRIVATE
        Hert::Hert
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

