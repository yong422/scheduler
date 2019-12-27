#/usr/bin/env python
#-*- coding:utf-8 -*-

"""
conanfile.py
프로젝트에서 사용하는 libraries install, build
conan
"""
from conans import ConanFile, CMake

class MylibConan(ConanFile):
    setting = "os", "compiler", "build_type", "arch"
    requires = (("cctz/2.2@bincrafters/stable"),\
                ("gtest/1.8.1@bincrafters/stable"),\
                ("google-benchmark/1.4.1@mpusz/stable"))
    generators = "cmake"
    option = {"cctz:fPIC" : True}

    def imports(self):
         self.copy("license*", dst="licenses", folder=True, ignore_case=True)
