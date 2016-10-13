# $^	The names of all the prerequisites, with spaces between them.
# $@	The file name of the target of the rule.
# $?	The names of all the prerequisites that are newer than the target
# $(INCLUDE) contains MSVC's default include paths, separated by semicolons

# [Compiler options]
# /nologo: suppress copyright message
# /Gm-: Disable minimal rebuild
# /MDd: Runtime library: Multi-threaded Debug DLL
# /GR-: disable RTTI
# /EHs-c-: disable exceptions
# /fp:fast: "fast" floating-point model; results are less predictable
# /fp:except-: disable floating-point exceptions when generating code
# /Oi: enable intrinsic functions
# /Ob1: only __inline function expansion
# /WX: treat warnings as errors
# /W4: warning level 4
# /wd4100:
# /wd4505:
# /Bt: display timings (undocumented)

ASSETS := ..\assets
BUILD := build
EXTERNAL := ..\external
INC := ..\cpp

IMGUI := $(EXTERNAL)\imgui-1.49
LUA := $(EXTERNAL)\LuaJIT-2.0.4\src
PROTOBUF := $(EXTERNAL)\protobuf-2.6.1\src
SFML := $(EXTERNAL)\SFML-2.4.0

mjBasicCompile := /nologo /Gm- /MDd /GR- /EHs-c- /fp:fast /fp:except- /Oi
mjDefinitions := -Dmj_CL -DMJ_DEBUG -DUNICODE -D_UNICODE -D_SCL_SECURE_NO_WARNINGS -D_HAS_EXCEPTIONS=0 -DNOMINMAX -DVC_EXTRALEAN -DSTRICT -DWIN32_LEAN_AND_MEAN -DNOGDI -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS
mjInclude := $(addprefix /I, . $(INC) $(IMGUI) $(PROTOBUF) $(LUA) $(SFML)\include)
mjCompile := $(mjBasicCompile) $(mjDefinitions) $(mjInclude) /Ob1 /WX /W4 /wd4100 /wd4505 /Z7

# [Linker options]
# /time: display timings (undocumented)

mjLink := /incremental:no
mjLink += winmm.lib ws2_32.lib ole32.lib user32.lib
mjLink += /LIBPATH:$(BUILD)/lib imgui.lib lua51.lib
# TODO: Debug vs Release
mjLink += /LIBPATH:$(SFML)\lib flac.lib freetype.lib jpeg.lib ogg.lib openal32.lib sfml-audio-d.lib sfml-graphics-d.lib sfml-main-d.lib sfml-network-d.lib sfml-system-d.lib sfml-window-d.lib vorbis.lib vorbisenc.lib vorbisfile.lib

all: replay.dll replay.exe assets

assets:
	xcopy $(IMGUI)\extra_fonts\\*.ttf $(ASSETS) /Y /Q

replay.dll: $(INC)/*.cpp $(INC)/*.h
	if not exist $(BUILD)\bin mkdir $(BUILD)\bin
	cl $(mjCompile) /Fo$(BUILD)/bin/ /LD /Fe$@ $(INC)/replay_ub.cpp /link $(mjLink) /PDB:$(BUILD)/bin/starlight_$(MJ_RANDOM).pdb

replay.exe: *.cpp $(INC)/*.h
	if not exist $(BUILD)\bin mkdir $(BUILD)\bin
	cl $(mjCompile) /Fo$(BUILD)/bin/ /Fe$@ replay_win32_ub.cpp /link $(mjLink)

clean:
	del $(ASSETS)\*.ttf
	del *.h
	del *.log
#	del imgui.ini
	del log.txt
	if exist $(BUILD) rmdir /s /q $(BUILD)
