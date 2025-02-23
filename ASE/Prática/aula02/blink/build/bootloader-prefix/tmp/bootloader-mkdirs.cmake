# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/joao/esp/esp-idf/components/bootloader/subproject"
  "/home/joao/Desktop/mect_1ano/ASE/Prática/aula02/blink/build/bootloader"
  "/home/joao/Desktop/mect_1ano/ASE/Prática/aula02/blink/build/bootloader-prefix"
  "/home/joao/Desktop/mect_1ano/ASE/Prática/aula02/blink/build/bootloader-prefix/tmp"
  "/home/joao/Desktop/mect_1ano/ASE/Prática/aula02/blink/build/bootloader-prefix/src/bootloader-stamp"
  "/home/joao/Desktop/mect_1ano/ASE/Prática/aula02/blink/build/bootloader-prefix/src"
  "/home/joao/Desktop/mect_1ano/ASE/Prática/aula02/blink/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/joao/Desktop/mect_1ano/ASE/Prática/aula02/blink/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/joao/Desktop/mect_1ano/ASE/Prática/aula02/blink/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
