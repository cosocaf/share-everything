cmake_minimum_required(VERSION 3.21)

add_executable(
  share-everything
  pch.cpp
  tchar_util.cpp
  main.cpp
  logger.cpp
  app.cpp
  notify_icon.cpp
  fetch.cpp
)
target_sources(
  share-everything PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/.manifest
  ${CMAKE_CURRENT_SOURCE_DIR}/resource/menu.rc
)

target_link_libraries(share-everything Ws2_32)
