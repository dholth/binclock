#
# Build script for mesh networking firmware
#

# These variables must be set correctly

STM32PLUS_INSTALL_DIR = "~/opt/arm-none-eabi"
STM32PLUS_VERSION     = "040006"

# main build script

import os
import os.path
import platform
import subprocess

STM32PLUS_INSTALL_DIR = os.path.expanduser(STM32PLUS_INSTALL_DIR)

def usage():

    print """
Usage: scons mode=<MODE>

  <MODE>: debug/fast/small.
    debug = -O0
    fast  = -O3
    small = -Os

  Examples using -j to do a 4-job parallel build:
    scons mode=debug -j4
    scons mode=fast -j4
    scons mode=small -j4
"""

# mode argument must be supplied

mode=ARGUMENTS.get('mode')

if not (mode in ['debug', 'fast', 'small']):
    usage()
    Exit(1)

# set up build environment and pull in OS environment variables

env=Environment(ENV=os.environ)

# verify that stm32plus is installed in the defined location

stm32plus_lib=STM32PLUS_INSTALL_DIR+"/lib/stm32plus-"+STM32PLUS_VERSION+"/libstm32plus-"+mode+"-f051-8000000i.a"
if not os.path.isfile(stm32plus_lib):
    print stm32plus_lib+" does not exist."
    print "Please edit SConstruct and check the STM32PLUS_INSTALL_DIR and STM32PLUS_VERSION variables."
    Exit(1)

# replace the compiler values in the environment

env.Replace(CC="arm-none-eabi-gcc")
env.Replace(CXX="arm-none-eabi-g++")
env.Replace(AS="arm-none-eabi-as")

# create the C and C++ flags that are needed. We can't use the extra or pedantic errors on the ST library code.

env.Replace(CCFLAGS=["-Wall","-Werror","-ffunction-sections","-fdata-sections","-fno-exceptions","-mthumb","-gdwarf-2","-pipe","-mcpu=cortex-m0","-DSTM32PLUS_F0_51","-DHSI_VALUE=8000000"])
env.Replace(CXXFLAGS=["-Wextra","-pedantic-errors","-fno-rtti","-std=gnu++0x","-fno-threadsafe-statics"])
env.Append(ASFLAGS="-mcpu=cortex-m0")
env.Append(LINKFLAGS=["-Xlinker","--gc-sections","-mthumb","-g3","-gdwarf-2","-mcpu=cortex-m0"])
env.Append(LINKFLAGS=["-Tsystem/f051_48_8/Linker.ld","-Wl,-wrap,__aeabi_unwind_cpp_pr0","-Wl,-wrap,__aeabi_unwind_cpp_pr1","-Wl,-wrap,__aeabi_unwind_cpp_pr2"])

# set the include directories - not a simple task on cygwin

if "cygwin" in platform.system().lower():

  # g++ must see the windows style C:/foo/bar path, not the cygwin /usr/foo/bar style so we must translate the
  # paths here. also, scons will try to interpret ":" as a separator in cygwin which gives us the additional problem
  # of not being able to use the interpreted CPPPATH. We have to use CXX flags instead.

  proc=subprocess.Popen("cygpath --mixed "+STM32PLUS_INSTALL_DIR,stdout=subprocess.PIPE,shell=True)
  (cygbasepath,err)=proc.communicate()
  cygbasepath=cygbasepath.rstrip("\n");     # chomp the newline

  env.Append(CCFLAGS="-I"+cygbasepath+"/include/stm32plus-"+STM32PLUS_VERSION)
  env.Append(CXXFLAGS="-I"+cygbasepath+"/include/stm32plus-"+STM32PLUS_VERSION+"/stl")
  env.Append(LINKFLAGS="-L"+cygbasepath+"/lib/stm32plus-"+STM32PLUS_VERSION)

else:
  env.Append(CPPPATH=[
      STM32PLUS_INSTALL_DIR+"/include/stm32plus-"+STM32PLUS_VERSION,
      STM32PLUS_INSTALL_DIR+"/include/stm32plus-"+STM32PLUS_VERSION+"/stl"])

  env.Append(LIBPATH=STM32PLUS_INSTALL_DIR+"/lib/stm32plus-"+STM32PLUS_VERSION)

# set the common include directory

env.Append(CPPPATH="include")

# set the library path

env.Append(LIBS="stm32plus-"+mode+"-f051-8000000i.a")

# collect the source files

matches=["binclock.cpp", "microscript.c"]

# append the system startup files

matches.append("system/LibraryHacks.cpp")
matches.append("system/f051_48_8/Startup.asm")
matches.append("system/f051_48_8/System.c")

# here's where the linker script is located

linkerscript="system/f051_48_8/Linker.ld"

# add on the mode=specific optimisation definitions

if mode=="debug":
    env.Append(CCFLAGS=["-O0","-g3"])
elif mode=="fast":
    env.Append(CCFLAGS=["-O3"])
elif mode=="small":
    env.Append(CCFLAGS=["-Os"])

# trigger a build with the correct output name

elf=env.Program("binclock.elf",matches)
hex=env.Command("binclock.hex",elf,"arm-none-eabi-objcopy -O ihex binclock.elf binclock.hex")
bin=env.Command("binclock.bin",elf,"arm-none-eabi-objcopy -O binary binclock.elf binclock.bin")
lst=env.Command("binclock.lst",elf,"arm-none-eabi-objdump -h -S binclock.elf > binclock.lst")
size=env.Command("binclock.size",elf,"arm-none-eabi-size --format=berkeley binclock.elf | tee binclock.size")
