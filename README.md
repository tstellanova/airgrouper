# airgrouper

This application collects information advertised by compatible BLE beacons
and publishes periodically to the Particle Cloud.
This version was designed for and tested on the B404 M.2 BSOM device,
and is designed to be compatible with the 
[airshield](https://github.com/tstellanova/airshield) 
wearable Personal Protective Equipment (PPE) dust mask demo. 
The airgrouper hub collects air quality data advertised by
intermittently nearby airshields and forwards it to the Particle Cloud. 



### To Build & Flash with Particle Workbench (vscode)

This application may be built with Device OS version 2.1.0 (LTS) and above.

1. Clone this repository 
2. Init & Update Submodules `git submodule update --init --recursive`
3. Open Particle Workbench
4. Run the `Particle: Import Project` command, follow the prompts, to select this project's `project.properties` file and wait for the project to load
5. Run the `Particle: Configure Workspace for Device` command and select a compatible Device OS version and the `bsom` platform when prompted ([docs](https://docs.particle.io/tutorials/developer-tools/workbench/#cloud-build-and-flash))
6. Connect your M.2 BSOM eval kit to your computer with a usb cable
7. Compile & Flash using Workbench


### To Build & Flash with Particle CLI

This application may be built with Device OS version 2.1.0 (LTS) and above.

1. Clone this repository 
2. Init & Update Submodules `git submodule update --init --recursive`
3. Cloud build (for BRSOM board) with CLI :
`particle compile --target 2.1.0 bsom --saveTo airgrouper_bsom.bin`

4. Connect your M.2 BSOM eval kit to your computer with a usb cable
5. Use the CLI to flash the device using dfu:
`particle usb dfu && particle flash --usb airgrouper_bsom.bin`


