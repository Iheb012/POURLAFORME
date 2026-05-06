## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = project3

project3_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	common.c common.h \
	membre.c membre.h \
	entraineur.c entraineur.h \
	cours.c cours.h \
	equipement.c equipement.h \
	centre.c centre.h

project3_LDADD = @PACKAGE_LIBS@ $(INTLLIBS) -lcurl
