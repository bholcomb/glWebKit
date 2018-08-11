solution ("WebkitTest")
  configurations { "Debug", "Release" }
  location("..")
  platforms { 'x64' }
  symbols "On"
  startproject ("glWebkit")

  filter "platforms:x64"
    architecture "x86_64"

  filter {"platforms:x64", "kind:ConsoleApp or WindowedApp or SharedLib"}
    targetdir ('../bin64')

  configuration { "Debug" }
    defines { "DEBUG", "TRACE"}
    optimize "Off"
 
  configuration { "Release" }
    optimize "Speed"
-------------------------------------------------------------------------------
-- Projects

project ("glWebkit")
  kind "SharedLib"
  location("glWebKit")
  language "C++"
  debugdir("../bin64")
  targetdir("../bin64")
  links {
    'OpenGL32.lib',
    'ws2_32.lib',
    'glew32.lib',
    'bcrypt.lib'
  }
  
  defines {
	GL_WEBKIT_EXPORTS
  }
  
  includedirs {
    '../include',
    '../libsrc/glWebKit'
  }
  
  libdirs {
   '../lib64'
  }
  
  files {
	"../include/glWebKit/**h",
    "../libsrc/glWebKit/**.h",
    "../libsrc/glWebKit/**.cxx",
    "../libsrc/glWebKit/**.cpp"
  }
  
  vpaths {
     ["Public"] = {"../include/glWebKit/**.h"},
	 ["Headers"] = {"../libsrc/glWebKit/**.h"},
	 ["Sources"] = {"../libsrc/glWebKit/**.cxx", "../libsrc/glWebKit/**.cpp"},
   }
   
project ("glWebkitExample")
  kind "ConsoleApp"
  location("glWebKitExample")
  language "C++"
  debugdir("../bin64")
  targetdir("../bin64")
  links {
    'SDL2.lib',
    'OpenGL32.lib',
    'glew32.lib',
	'glWebkit'
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
    "../libsrc/example/**.h",
    "../libsrc/example/**.cxx",
    "../libsrc/example/**.cpp"
  }
  
  vpaths {
     ["Headers"] = {"../libsrc/example/**.h"},
	 ["Sources"] = {"../libsrc/example/**.cxx", "../libsrc/example/**.cpp"},
   }

