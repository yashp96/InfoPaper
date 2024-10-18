# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "E:/Espressif/frameworks/esp-idf-v5.1.2/components/bootloader/subproject"
  "F:/Git/Experimental/InfoPaper/build/bootloader"
  "F:/Git/Experimental/InfoPaper/build/bootloader-prefix"
  "F:/Git/Experimental/InfoPaper/build/bootloader-prefix/tmp"
  "F:/Git/Experimental/InfoPaper/build/bootloader-prefix/src/bootloader-stamp"
  "F:/Git/Experimental/InfoPaper/build/bootloader-prefix/src"
  "F:/Git/Experimental/InfoPaper/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "F:/Git/Experimental/InfoPaper/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "F:/Git/Experimental/InfoPaper/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
