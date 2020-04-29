CXX = g++
RM = rm -f
AR=ar
CFLAGS = -c -O2 -Wall -D_7ZIP_ST
MYCOMMON_LIB=/home/cyj/MyCommonClass
OUT_LIB_NAME=$(MYCOMMON_LIB)/lib/libmylzma.a

OBJS = \
  LzFind.o \
  LzmaDec.o \
  LzmaEnc.o

HEADER_FILES= LzmaDec.h LzmaEnc.h Types.h

all: $(OBJS)
	mkdir -p $(MYCOMMON_LIB)/include/mylzma
	cp $(HEADER_FILES) $(MYCOMMON_LIB)/include/mylzma
	ls -l $(OUT_LIB_NAME)

.c.o:
	@echo [ Compiling ] "$<" ...
	@$(CXX) -c $(CFLAGS) -o $@ $<
	@$(AR) -r $(OUT_LIB_NAME) $@

clean:
	-$(RM)  $(OBJS) $(OUT_LIB_NAME)
