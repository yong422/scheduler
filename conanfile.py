#/usr/bin/env python
#-*- coding:utf-8 -*-

"""
conanfile.py
libraries install, build
"""
from conans import ConanFile, CMake

class MylibConan(ConanFile):
    setting = "os", "compiler", "build_type", "arch"
    requires = (("gtest/1.8.1@bincrafters/stable"))
    generators = "cmake"

    def imports(self):
         self.copy("license*", dst="licenses", folder=True, ignore_case=True)
