# Changelog

## 1.0.0 - 26/04/2022
- Complete rewrite of the DLL in C++ for more stability and performance
- Support for Arduino "clones" in USB
- Possibility of establishing a Wi-Fi connection (Websockets) for the command station based on Wemos
- Saving connection settings
- Stabilization of the connection at DLL startup (retry redundancy, very useful for slow Arduino)
- Improved handling of incoming and outgoing messages

### Features :
- Emergency stops
- Speed and Functions of locomotives
- Activation of accessories
- Support for S88 detection protocol
- Shutdown and start-up of the command station
- Reading of the status of the command station

### Bugs found :
- Loss of operation of accessories after an emergency stop (extremely random and equally rare therefore difficult to grasp and corrected..)
--------------------

### Features not supported:
- Reading/Writing CVs
- Display of errors
- Display of statistics
- Buffer display

### Features in progress :
- Reading/Writing CVs
- Handle speed of a loco (in case of parallel control by another controller, which can be the case with a Wifi command station)
- Handle functions of a loco (in the event of parallel control by another controller, which may be the case with a Wifi command station)