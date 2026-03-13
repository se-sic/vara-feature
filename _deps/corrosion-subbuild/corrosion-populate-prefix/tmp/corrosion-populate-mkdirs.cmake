# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/michael/Documents/vara-feature/_deps/corrosion-src"
  "/home/michael/Documents/vara-feature/_deps/corrosion-build"
  "/home/michael/Documents/vara-feature/_deps/corrosion-subbuild/corrosion-populate-prefix"
  "/home/michael/Documents/vara-feature/_deps/corrosion-subbuild/corrosion-populate-prefix/tmp"
  "/home/michael/Documents/vara-feature/_deps/corrosion-subbuild/corrosion-populate-prefix/src/corrosion-populate-stamp"
  "/home/michael/Documents/vara-feature/_deps/corrosion-subbuild/corrosion-populate-prefix/src"
  "/home/michael/Documents/vara-feature/_deps/corrosion-subbuild/corrosion-populate-prefix/src/corrosion-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/michael/Documents/vara-feature/_deps/corrosion-subbuild/corrosion-populate-prefix/src/corrosion-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/michael/Documents/vara-feature/_deps/corrosion-subbuild/corrosion-populate-prefix/src/corrosion-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
