if(WIN32)
        add_definitions(-DUNICODE -D_UNICODE)
endif()

add_library(pr-7z STATIC
	7zAlloc.c
	7zBuf.c
	7zBuf2.c
	7zCrc.c
	7zCrcOpt.c
	7zDec.c
	7zIn.c
	CpuArch.c
	LzmaDec.c
	Lzma2Dec.c
	Bra.c
	Bra86.c
	Bcj2.c
	Ppmd7.c
	Ppmd7Dec.c
	7zFile.c
	7zStream.c
)

