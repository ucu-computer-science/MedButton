# MedButton 2.0 Project (ModusToolbox 2.4 environment)

This repository contains all parts of firmware for the MedButton 2.0 project. The main ones are LoRa, GPS, and GPRS tasks, configuration files, and an implementation of a queue for storing messages.

Warning: Module can't be debugged for first launch after connection. Problem is that host trying to aquire device before powering it up. So you need just restart debug after failure.

## What is needed to start
- Open modus_shell on your device and go to the project's folder.
- In project's folder, go to `\firmware\MedButton` folder.
- Run `make getlibs` command to ensure all libraries the project uses are up to date.
- Run `make vscode` command to build the project for VS Code.
- Run `code .` command to open the whole project in VS Code.
- In file `.vscode\launch.json', replace `"Launch PSoC6 CM4 (KitProg3_MiniProg4)"` field with the following one:
```
{
    "name": "Launch PSoC6 CM4 (KitProg3_MiniProg4)",
    "type": "cortex-debug",
    "request": "launch",
    "cwd": "${workspaceRoot}",
    "executable": "./build/CY8CKIT-062-BLE/Debug/MedButton_First.elf",
    "servertype": "openocd",
    "searchDir": [
        "${workspaceRoot}",
        "${config:modustoolbox.toolsPath}/openocd/scripts/"
    ],
    "openOCDPreConfigLaunchCommands": [
        "set PROGRAMMER kitprog3",
        "set ENABLE_ACQUIRE 0",
        "set ENABLE_CM0 0"
    ],
    "configFiles": [
        "openocd.tcl"
    ],
    "overrideLaunchCommands": [
        "set mem inaccessible-by-default off",
        "-enable-pretty-printing",
        "set remotetimeout 15",
        // Comment this next line out if you don't want to reload program
        "monitor program {./build/CY8CKIT-062-BLE/Debug/MedButton_First.hex}",
        "monitor reset run",
        "monitor sleep 200",
        "monitor psoc6 reset_halt sysresetreq"
    ],
    "numberOfProcessors": 1,
    "targetProcessor": 1,  // Set to 0 for the CM0+, set to 1 for the CM4
    "postStartSessionCommands": [
        // Needed if runToMain is false
        // Following two commands are needed to get gdb and openocd and HW all in sync.
        // Or, execution context (PC, stack, registers, etc.) look like they are from before reset.
        // The stepi, is a pretend instruction that does not actually do a stepi, but MUST be done
        // Its a documented workaround in openocd. Do a 'monitor help' to see more info
        //
        // An alternative command to use is "continue" instead of the following two
        "monitor gdb_sync",
        "stepi"
    ],
    "overrideRestartCommands": [
        "monitor reset init",
        "monitor reset run",
        "monitor sleep 200",
        "monitor psoc6 reset_halt sysresetreq"
    ],
    "postRestartSessionCommands": [
        "monitor gdb_sync",
        "stepi"
    ],
    // svdFile is optional, it can be very large.
    "svdFile": "",
    "runToMain": true,          // if true, program will halt at main. Not used for a restart
    "preLaunchTask": "Build: Build [Debug]",    // Set this to run a task from tasks.json
                                                // before starting a debug session
    "showDevDebugOutput": false // When set to true, displays output of GDB.
                                // This is helpful when something is not working right
},
```
- Change the `.\openocd.tcl` file contents to the following ones:
```
source [find interface/kitprog3.cfg]
source [find target/psoc6.cfg]
kitprog3 power_config on 3300
${TARGET}.cm4 configure -rtos auto -rtos-wipe-on-reset-halt 1
psoc6 sflash_restrictions 1
```

- Run `main.c`.
