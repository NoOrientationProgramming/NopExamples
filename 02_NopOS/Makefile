
dirOut = build-native

argsCpp = \
	-m32 \
	-std=gnu++11 \
	-fno-use-cxa-atexit \
	-fno-builtin \
	-fno-rtti \
	-fno-exceptions \
	-fno-leading-underscore \
	-nostdlib \
	-Wall -Wextra -Wpedantic -Werror -Wfatal-errors -Wreorder -Wswitch-enum \
	-Wuseless-cast -Wparentheses -Wshift-overflow -Wsign-compare \
	-Wzero-as-null-pointer-constant -Wcast-align -Wcast-qual -Wcatch-value \
	-Wchar-subscripts -Wswitch-default -Wctor-dtor-privacy -Wduplicated-branches \
	-Wduplicated-cond -Wempty-body -Wextra-semi -Wfloat-equal -Wformat \
	-Wformat-extra-args -Wimplicit-fallthrough -Wmissing-field-initializers \
	-Wnull-dereference -Wshadow \
	-DCONFIG_PROC_HAVE_LOG=0 \
	-DCONFIG_PROC_SHOW_ADDRESS_IN_ID=0 \
	-DCONFIG_PROC_INFO_BUFFER_SIZE=1024 \
	-DCONFIG_PROC_DISABLE_TREE_DEFAULT=1 \
	-DCONFIG_PROC_HAVE_DRIVERS=0
argsAs = \
	--32
argsLd = \
	-melf_i386

objs = \
	start2.o \
	boot2.o
objsFull = $(addprefix $(dirOut)/, $(objs))

all: $(dirOut)/nopos.iso

$(dirOut)/nopos.iso: $(dirOut) $(dirOut)/nopos.bin
	cd $(dirOut); ../tools/post.sh nopos.bin $(notdir $@)

$(dirOut)/nopos.bin: main/boot2.ld $(objsFull)
	ld $(argsLd) -T $< -o $@ $(objsFull)

$(dirOut)/%.o: main/%.cpp
	g++ $(argsCpp) -o $@ -c $<

$(dirOut)/%.o: main/%.s
	as $(argsAs) -o $@ $<

$(dirOut):
	mkdir $(dirOut)

