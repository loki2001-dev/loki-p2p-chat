CMAKE_MINIMUM_REQUIRED(VERSION 3.28)
PROJECT(p2p_chat LANGUAGES CXX)

# OPTION (Standard)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# OPTION (3rdparty)
SET(BUILD_SHARED_LIBS ON)

# ADD 3rdparty
ADD_SUBDIRECTORY(3rdparty/juice)
ADD_SUBDIRECTORY(3rdparty/nlohmann)
ADD_SUBDIRECTORY(3rdparty/spdlog)

ADD_EXECUTABLE(p2p_chat
        src/main.cpp
        src/PeerConnection.cpp
)

# INCLUDE directories (OPTIONAL)
TARGET_INCLUDE_DIRECTORIES(p2p_chat PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# LINK
TARGET_LINK_LIBRARIES(p2p_chat
        juice
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)