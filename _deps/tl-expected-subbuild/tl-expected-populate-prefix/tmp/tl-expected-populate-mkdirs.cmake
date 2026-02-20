# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/michael/Documents/vara-feature/_deps/tl-expected-src"
  "/home/michael/Documents/vara-feature/_deps/tl-expected-build"
  "/home/michael/Documents/vara-feature/_deps/tl-expected-subbuild/tl-expected-populate-prefix"
  "/home/michael/Documents/vara-feature/_deps/tl-expected-subbuild/tl-expected-populate-prefix/tmp"
  "/home/michael/Documents/vara-feature/_deps/tl-expected-subbuild/tl-expected-populate-prefix/src/tl-expected-populate-stamp"
  "/home/michael/Documents/vara-feature/_deps/tl-expected-subbuild/tl-expected-populate-prefix/src"
  "/home/michael/Documents/vara-feature/_deps/tl-expected-subbuild/tl-expected-populate-prefix/src/tl-expected-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/michael/Documents/vara-feature/_deps/tl-expected-subbuild/tl-expected-populate-prefix/src/tl-expected-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/michael/Documents/vara-feature/_deps/tl-expected-subbuild/tl-expected-populate-prefix/src/tl-expected-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
