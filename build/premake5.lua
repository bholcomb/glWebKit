solution ("WebkitTest")
  configurations { "Debug", "Release" }
  location("..")
  platforms { 'x64' }
  symbols "On"
  startproject ("glWebkit")

  filter "platforms:x64"
    architecture "x86_64"

  filter {"platforms:x64", "kind:ConsoleApp or WindowedApp or SharedLib"}
    targetdir ('Bin64')

-------------------------------------------------------------------------------
-- Projects

project ("glWebkit")
  kind "ConsoleApp"
  location("glWebKit")
  language "C++"
  debugdir("../bin64")
  targetdir("../bin64")
  links {
    'SDL2.lib',
    'OpenGL32.lib',
    'ws2_32.lib',
    'Shlwapi.lib',
    'glew32.lib'
  }
  
  defines {
  }
  
  includedirs {
    '../include',
    '../libsrc'
  }
  
  libdirs {
   '../lib64'
  }
  
  files {
    "../libsrc/**.h",
    "../libsrc/**.cxx",
    "../libsrc/**.cpp"
  }
  
  vpaths {
     ["Headers"] = {"../libsrc/**.h"},
	  ["Sources"] = {"../libsrc/**.cxx", "../libsrc/**.cpp"},
   }
