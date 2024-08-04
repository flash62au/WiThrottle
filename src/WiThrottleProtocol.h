/* -*- c++ -*-
 *
 * WiThrottleProtocol
 *
 * This package implements a WiThrottle protocol connection,
 * allow a device to communicate with a JMRI server or other
 * WiThrottleProtocol device (like the Digitrax LNWI).
 *
 * Copyright © 2018-2019 Blue Knobby Systems Inc.
 *
 * This work is licensed under the Creative Commons Attribution-ShareAlike
 * 4.0 International License. To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-sa/4.0/ or send a letter to
 * Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
 *
 * Attribution — You must give appropriate credit, provide a link to the
 * license, and indicate if changes were made. You may do so in any
 * reasonable manner, but not in any way that suggests the licensor
 * endorses you or your use.
 *
 * ShareAlike — If you remix, transform, or build upon the material, you
 * must distribute your contributions under the same license as the
 * original.
 *
 * All other rights reserved.
 *
 */

/*
Version information:

1.1.21   - Add support for setting the Speed Step mode 28/128/14 etc.  setSpeedSteps(), getSpeedSteps()
1.1.20   - Corrected the EStop for 'all' throttles
1.1.18/19- Added support for logLevel. Assigned levels to every log message
         - changed all the log messages so that it is clearer which came from this library
1.1.17   - fix version numbers. no code change
1.1.15/16   - fix a bug where the speeds were not being updated when changed on another device
1.1.14   - addition of 'sendCommand(String cmd);
1.1.13   - Bug fix for ESTOP
1.1.12   - Add support for broadcast messages and alerts
1.1.11   - Change to the fix for the _wifiTrax WFD-30, so that leading CR+LF is always sent
         - Removal of setSpeedCommandShouldBeSenttwice(bool twice)
1.1.10   - discarded
1.1.9    - discarded         
1.1.8    - Addition of setSpeedCommandShouldBeSenttwice(bool twice)
1.1.7    - Addition of minimum time separation/delay between commands sent
1.1.6    - Change to support 32 functions
1.1.5    - Adjust the aggressive heartbeat
1.1.4    - Use more aggressive heartbeat commands
1.1.3    - Add add ability to specify which loco in a consist is sent a function co…
1.1.2    - Bug fix
1.1.1    - Improve consist support
1.1.0    - Muiti-throttle support added

*/

#ifndef WITHROTTLE_H
#define WITHROTTLE_H

#include "Arduino.h"
#include <vector>  //https://github.com/arduino-libraries/Arduino_AVRSTL

// Protocol special characters
// see: https://www.jmri.org/help/en/package/jmri/jmrit/withrottle/Protocol.shtml#StringParsing
#define PROPERTY_SEPARATOR 	"<;>"
#define ENTRY_SEPARATOR 	"]\\["
#define SEGMENT_SEPARATOR 	"}|{"
#define NEWLINE 			'\n'
#define CR 					'\r'
#define DEFAULT_MULTITHROTTLE 'T'
#define ALL_LOCOS_ON_THROTTLE "*"

#define MAX_WIT_THROTTLES 6
#define MAX_FUNCTIONS 32

/// @brief Loco/Throttle Direction options
enum Direction {
    Reverse = 0,
    Forward = 1
};

/// @brief Track Power options
enum TrackPower {
    PowerOff = 0,
    PowerOn = 1,
    PowerUnknown = 2
};

/// @brief Turnout/Point state options
enum TurnoutState {
    TurnoutClosed = 2,
    TurnoutThrown = 4,
    TurnoutUnknown = 1,
    TurnoutInconsistent = 8
};

/// @brief Turnout/Point action options
enum TurnoutAction {
    TurnoutClose = 0,
    TurnoutThrow = 1,
    TurnoutToggle = 2
};

/// @brief Route states
enum RouteState {
    RouteActive = 2,
    RouteInactive = 4,
    RouteInconsistent = 8
};

///
/// ----
///

/// @brief TBA
class NullStream : public Stream {
  
  public:
    /// @brief TBA
	NullStream() {}

    /// @brief TBA
	int available() { return 0; }

    /// @brief TBA
	void flush() {}

    /// @brief TBA
	int peek() { return -1; }

    /// @brief TBA
	int read() { return -1; }

    /// @brief TBA
    /// @param c TBA
	size_t write(uint8_t c) { return 1; }

    /// @brief TBA
    /// @param buffer TBA
    /// @param size TBA
	size_t write(const uint8_t *buffer, size_t size) { return size; }
};

///
/// ----
///

/// @brief Class for the Delegate methods
class WiThrottleProtocolDelegate
{
  public:
    /// @brief Delegate method to receive the WiThrottle version
    /// @param version Version Number
    virtual void receivedVersion(String version) {}

    /// @brief Delegate method to receive the Server Type
    /// @param type Server Type
    virtual void receivedServerType(String type) {}

    /// @brief Delegate method to receive the Server Decription
    /// @param description Server Description
    virtual void receivedServerDescription(String description) {}

    /// @brief Delegate method to receive a message from the Withrottle Server
    /// @param message Message Content
    virtual void receivedMessage(String message) {}

    /// @brief Delegate method to receive a broadcast Alert from the Withrottle Server
    /// @param alert Broadcast Alert content
    virtual void receivedAlert(String alert) {}

    /// @brief Delegate method to receive the total number of Roster Entries from the Withrottle Server
    /// @param rosterSize total number of Roster Entries from the Withrottle Server
    virtual void receivedRosterEntries(int rosterSize) {}

    /// @brief Delegate method to receive the a single Roster Entry from the Withrottle Server
    /// @param index sequence number
    /// @param name Roster entry name
    /// @param address DCC Address
    /// @param length S|L Short or Long address
    virtual void receivedRosterEntry(int index, String name, int address, char length) {}

    /// @brief Delegate method to receive the total number of Turnouts/Points Entries from the Withrottle Server
    /// @param turnoutListSize total number of Turnout/Point Entries in the Withrottle Server
    virtual void receivedTurnoutEntries(int turnoutListSize) {}

    /// @brief Delegate method to receive the a single Turnout/Point Entry from the Withrottle Server
    /// @param index sequence number
    /// @param sysName Turnout/Point system name
    /// @param userName Turnout/Point entry name
    /// @param state current state of the Turnout/Point
    virtual void receivedTurnoutEntry(int index, String sysName, String userName, int state) {}

    /// @brief Delegate method to receive the total number of Routes Entries from the Withrottle Server
    /// @param routeListSize total number of Route Entries in the Withrottle Server
    virtual void receivedRouteEntries(int routeListSize) {}

    /// @brief Delegate method to receive a single Route Entry from the Withrottle Server
    /// @param index sequence number
    /// @param sysName Route system name
    /// @param userName Route entry name
    /// @param state current state of the Route
    virtual void receivedRouteEntry(int index, String sysName, String userName, int state) {}

    /// @brief Delegate method to receive
    /// @param time TBA
    virtual void fastTimeChanged(uint32_t time) { }

    /// @brief Delegate method to receive
    /// @param rate Rate of the fast time cloce
    virtual void fastTimeRateChanged(double rate) { }

    /// @brief Delegate method to receive the the Server heartbeat configurtio  from the Withrottle Server
    /// @param seconds Number of seconds betwean heartbeats
    virtual void heartbeatConfig(int seconds) { }

    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param func TBA
    /// @param state TBA
    virtual void receivedFunctionState(uint8_t func, bool state) { }
    
    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param functions TBA
    virtual void receivedRosterFunctionList(String functions[MAX_FUNCTIONS]) { }

    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param func TBA
    /// @param state TBA
    virtual void receivedFunctionStateMultiThrottle(char multiThrottle, uint8_t func, bool state) { }
    
    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param functions TBA
    virtual void receivedRosterFunctionListMultiThrottle(char multiThrottle, String functions[MAX_FUNCTIONS]) { }

    /// @brief Delegate method to receive the speed for the default (first) throttle from the Withrottle Server
    /// @param speed TBA
    virtual void receivedSpeed(int speed) { }             // Vnnn
    
    /// @brief Delegate method to receive the direction for the default (first) throttle from the Withrottle Server
    /// @param dir TBA
    virtual void receivedDirection(Direction dir) { }     // R{0,1}
    
    /// @brief Delegate method to receive the number of speed steps for the default (first) throttle from the Withrottle Server
    /// @param steps 1=128step, 2=28step, 4=27step or 8=14step
    virtual void receivedSpeedSteps(int steps) { }        // snn

    /// @brief Delegate method to receive the speed for a specific throttle from the Withrottle Server
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param speed TBA
    virtual void receivedSpeedMultiThrottle(char multiThrottle, int speed) { }             // Vnnn
    
    /// @brief Delegate method to receive the direction for a specific throttle from the Withrottle Server
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param dir TBA
    virtual void receivedDirectionMultiThrottle(char multiThrottle, Direction dir) { }     // R{0,1}

    /// @brief Delegate method to receive the speed steps for a specific throttle from the Withrottle Server
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param steps 1=128step, 2=28step, 4=27step or 8=14step
    virtual void receivedSpeedStepsMultiThrottle(char multiThrottle, int steps) { }        // snn

    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param port TBA
    virtual void receivedWebPort(int port) { }            // PWnnnnn

    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param state TBA
    virtual void receivedTrackPower(TrackPower state) { } // PPAn

    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param address DCC Address
    /// @param entry TBA
    virtual void addressAdded(String address, String entry) { }  // MT+addr<;>roster entry

    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param address DCC Address
    /// @param command TBA
    virtual void addressRemoved(String address, String command) { } // MT-addr<;>[dr]

    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param address DCC Address
    /// @param entry TBA
    virtual void addressStealNeeded(String address, String entry) { } // MTSaddr<;>addr

    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param address DCC Address
    /// @param entry TBA
    virtual void addressAddedMultiThrottle(char multiThrottle, String address, String entry) { }  // M0+addr<;>roster entry

    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param address DCC Address
    /// @param command TBA
    virtual void addressRemovedMultiThrottle(char multiThrottle, String address, String command) { } // M0-addr<;>[dr]

    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param address DCC Address
    /// @param entry TBA
    virtual void addressStealNeededMultiThrottle(char multiThrottle, String address, String entry) { } // MTSaddr<;>addr

    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param systemName System name of the Turnout/Point
    /// @param state TBA
    virtual void receivedTurnoutAction(String systemName, TurnoutState state) { } //  PTAturnoutstatesystemname

    /// @brief Delegate method to receive  from the Withrottle Server
    /// @param systemName System name of the Route
    /// @param state TBA
    virtual void receivedRouteAction(String systemName, RouteState state) { } //  PTAturnoutstatesystemname
};

///
/// WiThrottleProtocol
/// ==================
///

/// @brief TBA
class WiThrottleProtocol
{

    ///
    /// Public
    /// ------
    ///

  public:
    
    /// @brief TBA
    /// @param server TBA
	WiThrottleProtocol(bool server = false);

    /// @brief TBA
    /// @param delegate TBA
	void setDelegate(WiThrottleProtocolDelegate *delegate);

    // /// @brief TBA
    // /// @param delegate TBA
    // /// @param delayBetweenCommandsSent TBA
	// void setDelegate(WiThrottleProtocolDelegate *delegate, int delayBetweenCommandsSent);

    /// @brief TBA
    /// @param console TBA
    void setLogStream(Stream *console);

    /// @brief TBA
    /// @param level 0 = off 1 = basic 2 = high
    void setLogLevel(int level);

    /// @brief TBA
    /// @param needed TBA
    void setCommandsNeedLeadingCrLf(bool needed);

    /// @brief TBA
    /// @param stream TBA
	void connect(Stream *stream);

    /// @brief TBA
    /// @param stream TBA
    /// @param delayBetweenCommandsSent TBA
    void connect(Stream *stream, int delayBetweenCommandsSent);

    /// @brief TBA
    void disconnect();

    /// @brief TBA
    /// @param deviceName TBA
    void setDeviceName(String deviceName);

    /// @brief TBA
    /// @param deviceId TBA
    void setDeviceID(String deviceId);

    /// @brief TBA
    bool check();

    /// @brief Send command to the Command Station
    /// @param cmd Command to send
    void sendCommand(String cmd);
    
    //int fastTimeHours();
    //int fastTimeMinutes();

    /// @brief Get the current Fast Time value
	double getCurrentFastTime();

    /// @brief Get the current Fast Time rate
    float getFastTimeRate();

    /// @brief TBA
    bool clockChanged;

    /// @brief TBA
    String currentDeviceName;

    /// @brief TBA
    /// @param needed TBA
    void requireHeartbeat(bool needed=true);

    /// @brief Recrdes if the server heatbeat value has changed
    bool heartbeatChanged;

    /// @brief TBA
    /// @param address TBA
    bool addLocomotive(String address);  // address is [S|L]nnnn (where n is 0-10000)

    /// @brief TBA
    /// @param address TBA
    bool stealLocomotive(String address);   // address is [S|L]nnnn (where n is 0-10000)

    /// @brief TBA
    /// @param address TBA
    bool releaseLocomotive(String address = "*");

    /// @brief TBA
    String getLeadLocomotive();

    /// @brief TBA
    /// @param position TBA
    String getLocomotiveAtPosition(int position);

    /// @brief TBA
    int getNumberOfLocomotives();

    // multiThrottle support
    /// @brief Add a specfied loco to a specified throttle. Will be added to the end of the consist of one or more locos are currently assigned to that Throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param address Address of the loco to add.
    bool addLocomotive(char multiThrottle, String address);  // address is [S|L]nnnn (where n is 0-10000)

    /// @brief Steal a specified loco. Only relevant to DigiTrax system
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param address Address of the Loco to steal

    bool stealLocomotive(char multiThrottle, String address);   // address is [S|L]nnnn (where n is 0-10000)
    /// @brief Release one or all locos from a specied throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param address Address of the loco to release. Default is to release all locos from the Throttle

    bool releaseLocomotive(char multiThrottle, String address = "*");
    /// @brief Get the address of the loco in the lead positon, currently assigned to a specified Throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.

    String getLeadLocomotive(char multiThrottle);
    /// @brief Get the address of the loco at a specified positon, currently assigned to a specified Throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param position Postion of the loco to retrieve

    String getLocomotiveAtPosition(char multiThrottle, int position);
    /// @brief Get the number of locos currently assigned to a specified Throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    int getNumberOfLocomotives(char multiThrottle);

    /// @brief Set a Function on the default (first) Throttle
    /// @param funcnum Function Number
    /// @param pressed TBA
    void setFunction(int funcnum, bool pressed);

    // multiThrottle support
    /// @brief Set a Function on the a specified Throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param funcnum Function Number
    /// @param pressed TBA
    void setFunction(char multiThrottle, int funcnum, bool pressed);

    /// @brief Set a Function on a specified Loco only, on a specified Throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param address DCC Address of the loco to set
    /// @param funcnum Function Number
    /// @param pressed TBA
    void setFunction(char multiThrottle, String address, int funcnum, bool pressed);

    /// @brief Set the speed of the default (first) Throttle
    /// @param speed Speed 0-126
    bool setSpeed(int speed);

    /// @brief Get the speed of the default (first) Throttle
    int getSpeed();

    /// @brief Set the direction of the default (first) Throttle
    /// @param direction Direction. One of - Reverse = 0, Forward = 1
    bool setDirection(Direction direction);

    /// @brief Get the direction of the default (first) Throttle
    Direction getDirection();

    /// @brief Get the speed step of the default (first) Throttle
    int getSpeedSteps();
    
    // multiThrottle support
    /// @brief Get the speed step of a specified Throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    int getSpeedSteps(char multiThrottle);

    /// @brief Set the speed step of the default (first) Throttle
    /// @param steps 1=128step, 2=28step, 4=27step or 8=14step
    bool setSpeedSteps(int steps);
    
    // multiThrottle support
    /// @brief Set the speed step of a specified Throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param steps 1=128step, 2=28step, 4=27step or 8=14step
    bool setSpeedSteps(char multiThrottle, int steps);

    // multiThrottle support
    /// @brief Set the speed of a specified Throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param speed Speed 0-126
    bool setSpeed(char multiThrottle, int speed);

    /// @brief Set the speed of a specified Throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param speed Speed 0-126
    /// @param forceSend Option to force the command to be sent, even if the protocol thinks it is at that speed
    bool setSpeed(char multiThrottle, int speed, bool forceSend);

    /// @brief Get the speed of a specified Throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    int getSpeed(char multiThrottle);

    /// @brief Set the direction of a specified Throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param direction Direction. One of - Reverse = 0, Forward = 1
    bool setDirection(char multiThrottle, Direction direction);

    /// @brief Set the direction of a specified Throttle, with the option to force the send
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param direction Direction. One of - Reverse = 0, Forward = 1
    /// @param forceSend Option to force the command to be sent, even if the protocol thinks it is in that Direction
    bool setDirection(char multiThrottle, Direction direction, bool forceSend);

    /// @brief Set the direction of a specific locomotive on a specified Throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param address DCC Address of the loco to set
    /// @param direction Direction. One of - Reverse = 0, Forward = 1
    bool setDirection(char multiThrottle, String address, Direction direction);

    /// @brief et the direction of a specific locomotive on a specified Throttle, with the option to force the send
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param address DCC Address of the loco to set
    /// @param direction Direction. One of - Reverse = 0, Forward = 1
    /// @param ForceSend Option to force the command to be sent, even if the protocol thinks it is in that Direction
    bool setDirection(char multiThrottle, String address, Direction direction, bool ForceSend);

    /// @brief Get the direction of a specific throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    Direction getDirection(char multiThrottle);

    /// @brief Get the direction of a specific locomotives on a specific throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param address DCC Address of the loco to get
    Direction getDirection(char multiThrottle, String address);

    /// @brief Emergency Stop all locomotives on a specific throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    void emergencyStop(char multiThrottle);

    /// @brief Emergency Stop a specific locomotives on a specific throttle
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param address DCC Address of the loco to stop
    void emergencyStop(char multiThrottle, String address);
	
    /// @brief Set the state of a Track Power
    /// @param state State required. One of - PowerOff = 0, PowerOn = 1
	void setTrackPower(TrackPower state);

    /// @brief Emergency Stop all locomotives on the default (first) throttle
    void emergencyStop();

    /// @brief Set the state of a Turnout/Point 
    /// @param address Identifier of the Turnout/Point
    /// @param action Action to perform on the Turnout/Point. one of - TurnoutClose = 0, TurnoutThrow = 1, TurnoutToggle = 2
    bool setTurnout(String address, TurnoutAction action);   // address is turnout system name e.g. LT92

    /// @brief Set (activate) a Route
    /// @param address Identifier of the Route to activate
    bool setRoute(String address);   // address is turnout system name e.g. IO:AUTO:0008

    /// @brief Used to record the locos in a consist (on each Throttle)
    std::vector<String> locomotives[6];

    /// @brief Used to record the direction the locos in a consist (on each Throttle) are facing
    std::vector<Direction> locomotivesFacing[6];

    /// @brief Get the Throttle index number from a char Throttle Id. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param multiThrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    int getMultiThrottleIndex(char multiThrottle);
    
    /// @brief Get the last time that the server sent a resonse to the client 
    long getLastServerResponseTime();  
    
    ///
    /// Private
    /// -------
    ///


  private:
  
    bool server;
    
	Stream *stream;
    int logLevel = 1;
    Stream *console;
	NullStream nullStream;
    String outboundBuffer;
    double outboundCmdsTimeLastSent;
    int outboundCmdsMininumDelay;
    bool commandsNeedLeadingCrLf = false;
	
	WiThrottleProtocolDelegate *delegate = NULL;

    ///
    /// Inbound Command processing
    /// --------------------------
    ///

    /// @brief Process an incoming command from the Command Station
    /// @param c Command to process
    /// @param len length of the command
    bool processCommand(char *c, int len);

    /// @brief Process an incoming command from the Command Station - specific to locomotives
    /// @param multithrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param c Command to process
    /// @param len length of the command
    bool processLocomotiveAction(char multiThrottle, char *c, int len);

    /// @brief Process an incoming fast time command from the Command Station
    /// @param c Command/information to process
    /// @param len length of the command/information
    bool processFastTime(char *c, int len);

    /// @brief Process an incoming Heartbeat command from the Command Station
    /// @param c Information to process
    /// @param len length of the command/information
    bool processHeartbeat(char *c, int len);

    /// @brief Process an incoming Roster command from the Command Station
    /// @param multithrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param c Command to process
    /// @param len length of the command
    bool processRosterFunctionList(char multiThrottle, char *c, int len);

    /// @brief Process an incoming Protocol information from the Command Station
    /// @param c Protocol information to process
    /// @param len length of the command/information
    void processProtocolVersion(char *c, int len);

    /// @brief Process an incoming Server Type information from the Command Station
    /// @param c Information to process
    /// @param len length of the information
    void processServerType(char *c, int len);

    /// @brief Process an incoming Server Description information from the Command Station
    /// @param c Information to process
    /// @param len length of the information
    void processServerDescription(char *c, int len);	

    /// @brief Process an incoming Broadcast Message from the Command Station
    /// @param c Message to process
    /// @param len length of the message
    void processMessage(char *c, int len);	

    /// @brief Process an incoming Broadcast Alert from the Command Station
    /// @param c Alert message to process
    /// @param len length of the Alert message
    void processAlert(char *c, int len);	

    /// @brief Process an incoming Web Port information from the Command Station.  The port that serves HTTP. This can be used to retrieve loco images.
    /// @param c Message to process
    /// @param len length of the message
    void processWebPort(char *c, int len);

    /// @brief Process an incoming Roster List from the Command Station. This is the complete list of all the locos in the Roster.
    /// @param c Roster list to process
    /// @param len length of the list
	void processRosterList(char *c, int len);

    /// @brief Process an incoming Turnout/Points List from the Command Station. This is the complete list of all the locos in the Roster.
    /// @param c Turnout/Points list to process
    /// @param len length of the list
    void processTurnoutList(char *c, int len);

    /// @brief Process an incoming Routes List from the Command Station. This is the complete list of all the locos in the Roster.
    /// @param c Routes list to process
    /// @param len length of the list
    void processRouteList(char *c, int len);

    /// @brief Process an incoming Track Power information from the Command Station
    /// @param c Information to process
    /// @param len length of the information
    void processTrackPower(char *c, int len);

    /// @brief TBA
    /// @param multithrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param functionData TBA
    void processFunctionState(char multiThrottle, const String& functionData);

    /// @brief TBA
    /// @param multithrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param s TBA
    void processRosterFunctionListEntries(char multiThrottle, const String& s);

    /// @brief TBA
    /// @param multithrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param speedStepData TBA
    void processSpeedSteps(char multiThrottle, const String& speedStepData);

    /// @brief Process an incoming Direction command from the Command Station for a specific multiThrottle
    /// @param multithrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param directionStr TBA
    void processDirection(char multiThrottle, const String& directionStr);

    /// @brief Process an incoming Speed command from the Command Station for a specific multiThrottle
    /// @param speedData TBA
    void processSpeed(char multiThrottle, const String& speedData);

    /// @brief Process an incoming command from the Command Station to add or remove on or more locomotives from a specified multiThrottle
    /// @param multithrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param c Command to process
    /// @param len length of the command
    void processAddRemove(char multiThrottle, char *c, int len);

    /// @brief Process an incoming command from the Command Station to to advice that a steal command is required to aquire a locomotive. Specific the DigiTrack Command Stations only.
    /// @param multithrottle Which Throttle. Supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
    /// @param c Command to process
    /// @param len length of the command
    void processStealNeeded(char multiThrottle, char *c, int len);

    /// @brief TBA
    /// @param c Command to process
    /// @param len length of the command
    void processTurnoutAction(char *c, int len);

    /// @brief TBA
    /// @param c Command to process
    /// @param len length of the command
    void processRouteAction(char *c, int len);

    // /// @brief TBA
    // /// @param unknownCommand TBA
    // void processUnknownCommand(const String& unknownCommand);

    ///
    /// Outbound Commands
    /// ^^^^^^^^^^^^^^^^^
    ///

    /// @brief TBA
    bool checkFastTime();

    /// @brief TBA    
    bool checkHeartbeat();

    /// @brief TBA
    /// @param cmd TBA
    void sendDelayedCommand(String cmd);

    /// @brief TBA
    /// @param s TBA
    void setCurrentFastTime(const String& s);

    char inputbuffer[32767];
    ssize_t nextChar;  // where the next character to be read goes in the buffer

    //Chrono heartbeatTimer;
	unsigned long heartbeatTimer;
    int heartbeatPeriod;
	unsigned long timeLastLocoAcquired;

    //Chrono fastTimeTimer;
	unsigned long fastTimeTimer;
    double currentFastTime;
    float currentFastTimeRate;

    void resetChangeFlags();

    void init();

    bool locomotiveSelected[MAX_WIT_THROTTLES] = {false, false, false, false, false, false};

    String currentAddress[MAX_WIT_THROTTLES];

    int currentSpeed[MAX_WIT_THROTTLES];
    int speedSteps[MAX_WIT_THROTTLES];  // 1=128, 2=28, 4=27, 8=14, 16=28Mot
    Direction currentDirection[MAX_WIT_THROTTLES];

    String mostRecentTurnout;
    TurnoutState mostRecentTurnoutState;

    long lastServerResponseTime;
};

#endif // WITHROTTLE_H
