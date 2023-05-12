SET PATH=D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin;C:\Program Files (x86)\VMware\VMware Workstation\bin\;C:\Program Files\Python311\Scripts\;C:\Program Files\Python311\;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;C:\Program Files\dotnet\;C:\Program Files\TortoiseSVN\bin;D:\Program Files\Microchip\xc8\v2.36\bin;C:\mingw-w64\x86_64-8.1.0-release-win32-seh-rt_v6-rev0\mingw64\bin;D:\Program Files\Git\cmd;C:\Users\80023166\AppData\Local\Microsoft\WindowsApps;;D:\Program Files\VS Code\Microsoft VS Code\bin
SET CPU_TYPE=MKE17Z256VLH7
SET CPU_VENDOR=NXP
SET UV2_TARGET=ftm_simple_pwm debug
SET CPU_CLOCK=0x00B71B00
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\board.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\clock_config.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\pin_mux.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\fsl_component_generic_list.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\fsl_adapter_lpuart.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\system_mke17z7.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\fsl_clock.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\fsl_common.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\fsl_common_arm.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\fsl_ftm.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\fsl_gpio.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\fsl_lpuart.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\fsl_smc.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\ftm_simple_pwm.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\startup_mke17z7._ac"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\fsl_assert.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\fsl_debug_console.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"debug\fsl_str.__i"
"D:\Program Files (x86)\keil_v5\ARM\ARMCLANG\Bin\ArmLink" --Via "debug\ftm_simple_pwm.lnp"
