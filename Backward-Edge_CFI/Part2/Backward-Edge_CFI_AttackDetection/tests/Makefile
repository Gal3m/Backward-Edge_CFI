export GRP_ID=501
CC=gcc
CFLAGS=-g -Wall -z execstack -DGRP=$(GRP_ID) -DLEN1=1024 -DLEN2=128 -DLEN3=512 -DRANDOM=0

all: vuln vuln.dis driver_auth_db driver_return2_helper2 driver_return2_helper driver_return2_injectedcode longjmp helloworld vulncp

vuln: vuln.o
	$(CC) $(CFLAGS) -o vuln vuln.o

vuln.o: padding.h vuln.c
	$(CC) $(CFLAGS) -c vuln.c

vuln.dis: vuln
	rm -f vuln.dis
	objdump -S vuln > vuln.dis

driver_auth_db: driver_auth_db.c
	$(CC) -O $(CFLAGS) -o driver_auth_db driver_auth_db.c
	
driver_return2_helper2: driver_return2_helper2.c
	$(CC) -O $(CFLAGS) -o driver_return2_helper2 driver_return2_helper2.c

driver_return2_helper: driver_return2_helper.c
	$(CC) -O $(CFLAGS) -o driver_return2_helper driver_return2_helper.c

driver_return2_injectedcode: driver_return2_injectedcode.c
	$(CC) -O $(CFLAGS) -o driver_return2_injectedcode driver_return2_injectedcode.c
	
padding.h:
	./mkpad $(GRP_ID)

longjmp: longjmp.c
	$(CC) -o longjmp longjmp.c
	
helloworld: helloworld.c
	$(CC) -o helloworld helloworld.c
vulncp:
	cp vuln ../vuln

clean:
	rm -f *.o *.dis padding.h driver_auth_db vuln driver_return2_helper2 driver_return2_helper driver_return2_injectedcode longjmp helloworld
