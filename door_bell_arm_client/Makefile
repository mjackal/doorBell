cc = arm-none-linux-gnueabi-gcc

arm_cln : arm_cln.o
	$(cc) -o arm_cln arm_cln.o 

arm_cln.o : arm_cln.c
	$(cc) -c arm_cln.c
clean:
	rm -rf *.o
