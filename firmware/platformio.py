from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

env.Append(CCFLAGS=["-std=gnu++11"])			# options to both C and C++ compilers
env.Append(CXXFLAGS=["-Wno-literal-suffix"])	# options to only C++ compiler
