For using this Pintool, Please follow the instruction:

1. set the pin framework path to the PIN_ROOT variable in the makefile
2. cd Backward-Edge_CFI
3. make all
4. use this command to execute my Pintools for each of the exploits file that start with driver.
    command consist of address to pin framework + -follow-execv to instrument childs -t address of a pintool + -- address of a program to instrument.

   /home/sekar/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI.so -- ./[Target program]
5. The summary result of mismatches is printed on the screen and the detailed one will saved on the result.txt file.
6. I implemented one stack for each thread of target program and matched call/ret pair based on their thread id of these instructions. In this I do not get any false positive that directly come from process scheduling of multi-thread an application.
7. Pin can not instrument firefox completely. It detached from firefox in the middle of loading if we use -follow-execv argument which is necessaary for instrumenting a fork/child base application.
8. I used my Pintools against these test-cases(result also included):
9. cd Backward-Edge_CFI/tests
10. make all
11. cd ..

	/home/sekar/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI.so -- ./tests/helloworld
	No Mismatch.
	/home/sekar/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI.so -- ./tests/longjmp
	**************************************************************************************************************
	Address: 0x7fef3f836eb0 |Hit Count:    1 |Function Name: __libc_longjmp
	*************************************************************************--Report Generated From Hash Table--*

	/home/sekar/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI.so -- /bin/ls
	No Mismatch.
	/home/sekar/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI.so -- /bin/nano
	No Mismatch.
	/home/sekar/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI.so -- /bin/cat
	No Mismatch.
	/home/sekar/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI.so -- /bin/pwd
	No Mismatch.
	/home/sekar/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI.so -- /bin/sh -c /bin/ls
	**************************************************************************************************************
	Address: 0x55f71743d6d0 |Hit Count:    1 |Function Name: .plt.sec
	*************************************************************************--Report Generated From Hash Table--*
	
	/home/sekar/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -t ./obj-intel64/Backward-Edge_CFI.so -- gedit
	**************************************************************************************************************
	Address: 0x7f97069daeb0 |Hit Count:   40 |Function Name: __libc_longjmp
	*************************************************************************--Report Generated From Hash Table--*
	
	/home/sekar/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI.so -- gedit
	**************************************************************************************************************
	Address: 0x7f51c6ddceb0 |Hit Count:   40 |Function Name: __libc_longjmp
	*************************************************************************--Report Generated From Hash Table--*
	
	/home/sekar/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI.so -- gedit ./[textfile]
	**************************************************************************************************************
	Address: 0x7f228bdd4eb0 |Hit Count:  592 |Function Name: __libc_longjmp
	*************************************************************************--Report Generated From Hash Table--*
	
	/home/sekar/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI.so -- firefox www.google.com
	//firefox detached in the early stage of firefox loading, but result by then is as follow.
	**************************************************************************************************************
	Address: 0x7fd958680eb0 |Hit Count:  114 |Function Name: __libc_longjmp
	*************************************************************************--Report Generated From Hash Table--*

	/home/sekar/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -t ./obj-intel64/Backward-Edge_CFI.so -- firefox
	No Mismatch.
			
12. My analysis:
The are some mismatches between call and ret instructions especially in some complex applications because: 
•	Programmers uses some branches, exceptions, goto statements, longjumps, signals in their codes
•	Compilers has changed the flow of the program: It uses a ret instruction to generate a jump, it means using push <add> instruction and then ret instruction.Compilers uses this way in the situation the number of available register are rare. 

Almost all mismatches that my Pintool have recorded are form longjmp function from Libc library. The basic idea behind using setjmp and longjmp is implementing a exception handling scheme to save CPU state whenever you  encounter a try keyword and then do a longjmp whenever you throw an exception. If there are few try blocks in the program, like small programs, the frequency of the mismatches is not high. However, often in complex application like gedit there are alot of try blocks.As a result, the the frequency of the mismatches is high.


	

	
	
	

   
   

