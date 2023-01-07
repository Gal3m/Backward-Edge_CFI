For using this Pintool, Please follow the instruction:

1. set the pin framework path to the PIN_ROOT variable in the makefile
2. cd Backward-Edge_CFI_AttackDetection
3. make all
4. cd ./tests
5. make all
6. cd ..
5. use this command to execute my Pintools for each of the exploits file that start with driver.
    command consist of address to pin framework + -follow-execv to instrument childs -t address of a pintool + -- address of a program to instrument.

   /home/ali/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI_AttackDetection.so -- ./tests/driver_auth_db
   No Attack.
   /home/ali/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI_AttackDetection.so -- ./tests/driver_return2_helper
   ****Attack Detected, Return Address doesn't Match!****
   /home/ali/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI_AttackDetection.so -- ./tests/driver_return2_helper2
   ****Attack Detected, Return Address doesn't Match!****
   /home/ali/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI_AttackDetection.so -- ./tests/driver_return2_injectedcode
   ****Attack Detected, Return Address doesn't Match!****

6. Instrumenting driver_auth_db does not raise an alert because its a data only attack. In this attack return address does not overwrite. But, for other attacks my Pintool raise an alert, since these attacks overwrite the return address to point somewhere else.

7. I wrote a sample longjmp program that violate call/ret pair. it is in the test directory. you can run it to see the result of detection Pintool
  /home/ali/Desktop/a4/pin-3.20-98437-gf02b61307-gcc-linux/pin -follow_execv -t ./obj-intel64/Backward-Edge_CFI_AttackDetection.so -- ./tests/longjmp
  ****Attack Detected, Return Address doesn't Match!****
	