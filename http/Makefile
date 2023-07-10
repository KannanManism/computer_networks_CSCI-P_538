CC	= gcc
CFLAGS	= -O2 -Wall -Werror -pedantic -std=gnu99
LIBS	= -lm -lpthread -no-pie

PROGRAM	= httpcmd

$(PROGRAM): main.c http.a
	$(CC) $(CFLAGS) $(LIBS) -o $@ main.c http.a

http.a:
	$(CC) -c $(CFLAGS) $(LIBS) -o $@ http.c


httpm1.a:
	$(CC) -c $(MAC_FLAGS) $(LIBS) -o $@ http.c

m1: main.c httpm1.a
	$(CC) -c $(MAC_FLAGS) $(LIBS) -o $(PROGRAM) $^


httpmacintel.a:
	$(CC) -c $(MAC_FLAGS) -mmacosx-version-min=10.15 -arch x86_64 $(LIBS) -o $@ http.c

macintel: main.c httpmacintel.a
	$(CC) -c $(MAC_FLAGS) -mmacosx-version-min=10.15 -arch x86_64  $(LIBS) -o $(PROGRAM) $^


.PHONY: macintel m1
