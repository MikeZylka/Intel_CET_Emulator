# Intel_CET_Emulator

Intel's Control-flow Enforcement Technology (CET) requires hardware capabilities that legacy processors are not equiped with. This emulator will enforce Intel's CET with Intel's Pin Tools. The emulator allows developers to test and debug CET-protected code without the need for dedicated hardware as well as offer some security measures for users without the dedicated hardware.

## Usage
Using the emulator is simple. Just run the bash script `./cet_emulator` followed by the progam and its arguments.
```bash
    ./cet_emulator ls -la
```

That's it! There is no need to install anything as the emulator is already preconfigured to run on repository cloning.

## Features
The CET Emulator performs the two mechanism that are offered by Intel CET:
 - Indirect Branch Tracking (JOP and COP protection)
 - Shadow Stack (ROP protection)

There is plenty of documentation about this emulator in the pdf file 'Documentation.pdf'.

## Limitations
 - As of right now, the CET emulator cannot handle multi-threaded applications but an updated version with multi-threaded support is on its way!
 - It is not possible for this emulator to have the exact same security guarantees as its hardware-based counterpart.  This is because the Emulator is just another application running on the system, and it is possible for attackers to identify and exploit vulnerabilities in the Emulator to bypass its security measures. Thus, the Emulator is not a foolproof solution to protect against all forms of control-flow hijacking attacks.

If any issues arise please open a new issue in the repository 
