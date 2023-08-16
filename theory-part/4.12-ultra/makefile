all: gps loop

loop: loopback.c
	$(CROSS_COMPILE)gcc -o loopback loopback.c set_opt.c

gps: readGPS.c
	$(CROSS_COMPILE)gcc -o readGPS readGPS.c set_opt.c
