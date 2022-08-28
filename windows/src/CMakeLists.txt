cmake_minimum_required(VERSION 3.21)

add_executable(
  share-everything
  pch.cpp
  tchar_util.cpp
  result.cpp
  main.cpp
  logger.cpp
  storage.cpp
  app.cpp
  image_manager.cpp
  notify_icon.cpp
  api.cpp
)
target_sources(
  share-everything PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/.manifest
  ${CMAKE_CURRENT_SOURCE_DIR}/resource/menu.rc
)

add_dependencies(share-everything zlib)
target_link_libraries(share-everything Pathcch Crypt32 Gdiplus Wldap32 OpenSSL::SSL CURL::libcurl zlibstaticd)
