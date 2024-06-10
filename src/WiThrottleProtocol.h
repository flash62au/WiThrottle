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

#define MAX_FUNCTIONS 32

typedef enum Direction {
    Reverse = 0,
    Forward = 1
} Direction;

typedef enum TrackPower {
    PowerOff = 0,
    PowerOn = 1,
    PowerUnknown = 2
} TrackPower;

typedef enum TurnoutState {
    TurnoutClosed = 2,
    TurnoutThrown = 4,
    TurnoutUnknown = 1,
    TurnoutInconsistent = 8
} TurnoutState;

typedef enum TurnoutAction {
    TurnoutClose = 0,
    TurnoutThrow = 1,
    TurnoutToggle = 2
} TurnoutAction;

typedef enum RouteState {
    RouteActive = 2,
    RouteInactive = 4,
    RouteInconsistent = 8
} RouteState;

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

    /// @brief Delegate method to receive the a single Route Entry from the Withrottle Server
    /// @param index sequence number
    /// @param sysName Route system name
    /// @param userName Route entry name
    /// @param state current state of the Route
    virtual void receivedRouteEntry(int index, String sysName, String userName, int state) {}

    /// @brief TBA
    /// @param time TBA
    virtual void fastTimeChanged(uint32_t time) { }
    /// @brief TBA
    /// @param rate TBA
    virtual void fastTimeRateChanged(double rate) { }

    /// @brief TBA
    /// @param seconds TBA
    virtual void heartbeatConfig(int seconds) { }

    /// @brief TBA
    /// @param func TBA
    /// @param state TBA
    virtual void receivedFunctionState(uint8_t func, bool state) { }
    /// @brief TBA
    /// @param functions TBA
    virtual void receivedRosterFunctionList(String functions[MAX_FUNCTIONS]) { }

    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param func TBA
    /// @param state TBA
    virtual void receivedFunctionStateMultiThrottle(char multiThrottle, uint8_t func, bool state) { }
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param functions TBA
    virtual void receivedRosterFunctionListMultiThrottle(char multiThrottle, String functions[MAX_FUNCTIONS]) { }

    /// @brief TBA
    /// @param speed TBA
    virtual void receivedSpeed(int speed) { }             // Vnnn
    /// @brief TBA
    /// @param dir TBA
    virtual void receivedDirection(Direction dir) { }     // R{0,1}
    /// @brief TBA
    /// @param steps TBA
    virtual void receivedSpeedSteps(int steps) { }        // snn

    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param speed TBA
    virtual void receivedSpeedMultiThrottle(char multiThrottle, int speed) { }             // Vnnn
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param dir TBA
    virtual void receivedDirectionMultiThrottle(char multiThrottle, Direction dir) { }     // R{0,1}
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param steps TBA
    virtual void receivedSpeedStepsMultiThrottle(char multiThrottle, int steps) { }        // snn

    /// @brief TBA
    /// @param port TBA
    virtual void receivedWebPort(int port) { }            // PWnnnnn
    /// @brief TBA
    /// @param state TBA
    virtual void receivedTrackPower(TrackPower state) { } // PPAn

    /// @brief TBA
    /// @param address TBA
    /// @param entry TBA
    virtual void addressAdded(String address, String entry) { }  // MT+addr<;>roster entry
    /// @brief TBA
    /// @param address TBA
    /// @param command TBA
    virtual void addressRemoved(String address, String command) { } // MT-addr<;>[dr]
    /// @brief TBA
    /// @param address TBA
    /// @param entry TBA
    virtual void addressStealNeeded(String address, String entry) { } // MTSaddr<;>addr

    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param address TBA
    /// @param entry TBA
    virtual void addressAddedMultiThrottle(char multiThrottle, String address, String entry) { }  // M0+addr<;>roster entry
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param address TBA
    /// @param command TBA
    virtual void addressRemovedMultiThrottle(char multiThrottle, String address, String command) { } // M0-addr<;>[dr]
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param address TBA
    /// @param entry TBA
    virtual void addressStealNeededMultiThrottle(char multiThrottle, String address, String entry) { } // MTSaddr<;>addr

    /// @brief TBA
    /// @param systemName TBA
    /// @param state TBA
    virtual void receivedTurnoutAction(String systemName, TurnoutState state) { } //  PTAturnoutstatesystemname
    /// @brief TBA
    /// @param systemName TBA
    /// @param state TBA
    virtual void receivedRouteAction(String systemName, RouteState state) { } //  PTAturnoutstatesystemname
};


class WiThrottleProtocol
{
  public:
    
    /// @brief TBA
    /// @param server TBA
	WiThrottleProtocol(bool server = false);

    /// @brief TBA
    /// @param delegate TBA
	void setDelegate(WiThrottleProtocolDelegate *delegate);
    /// @brief TBA
    /// @param delegate TBA
    /// @param delayBetweenCommandsSent TBA
	void setDelegate(WiThrottleProtocolDelegate *delegate, int delayBetweenCommandsSent);
    /// @brief TBA
    /// @param console TBA
    void setLogStream(Stream *console);

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

    /// @brief TBA
    /// @param cmd TBA
    void sendCommand(String cmd);
    
    //int fastTimeHours();
    //int fastTimeMinutes();
    /// @brief TBA
	double getCurrentFastTime();
    /// @brief TBA
    float getFastTimeRate();
    bool clockChanged;

    String currentDeviceName;
    /// @brief TBA
    /// @param needed TBA
    void requireHeartbeat(bool needed=true);
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
    int getNumberOfLocomotives();

    // multiThrottle support
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param address TBA
    bool addLocomotive(char multiThrottle, String address);  // address is [S|L]nnnn (where n is 0-10000)
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param address TBA
    bool stealLocomotive(char multiThrottle, String address);   // address is [S|L]nnnn (where n is 0-10000)
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param address TBA
    bool releaseLocomotive(char multiThrottle, String address = "*");
    /// @brief TBA
    /// @param multiThrottle TBA
    String getLeadLocomotive(char multiThrottle);
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param position TBA
    String getLocomotiveAtPosition(char multiThrottle, int position);
    /// @brief TBA
    /// @param multiThrottle TBA
    int getNumberOfLocomotives(char multiThrottle);

    /// @brief TBA
    /// @param funcnum TBA
    /// @param pressed TBA
    void setFunction(int funcnum, bool pressed);
    // multiThrottle support
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param funcnum TBA
    /// @param pressed TBA
    void setFunction(char multiThrottle, int funcnum, bool pressed);
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param address TBA
    /// @param funcnum TBA
    /// @param pressed TBA
    void setFunction(char multiThrottle, String address, int funcnum, bool pressed);

    /// @brief TBA
    /// @param speed TBA
    bool setSpeed(int speed);
    /// @brief TBA
    int getSpeed();
    /// @brief TBA
    /// @param direction TBA
    bool setDirection(Direction direction);
    /// @brief TBA
    Direction getDirection();

    // multiThrottle support
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param speed TBA
    bool setSpeed(char multiThrottle, int speed);
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param forceSend TBA
    bool setSpeed(char multiThrottle, int speed, bool forceSend);
    /// @brief TBA
    /// @param multiThrottle TBA
    int getSpeed(char multiThrottle);
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param direction TBA
    bool setDirection(char multiThrottle, Direction direction);
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param direction TBA
    /// @param ForceSend TBA
    bool setDirection(char multiThrottle, Direction direction, bool forceSend);
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param address TBA
    /// @param direction TBA
    bool setDirection(char multiThrottle, String address, Direction direction);
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param address TBA
    /// @param direction TBA
    /// @param ForceSend TBA
    bool setDirection(char multiThrottle, String address, Direction direction, bool ForceSend);
    /// @brief TBA
    /// @param multiThrottle TBA
    Direction getDirection(char multiThrottle);
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param address TBA
    Direction getDirection(char multiThrottle, String address);
    /// @brief TBA
    /// @param multiThrottle TBA
    void emergencyStop(char multiThrottle);
    /// @brief TBA
    /// @param multiThrottle TBA
    /// @param address TBA
    void emergencyStop(char multiThrottle, String address);
	
    /// @brief TBA
    /// @param state TBA
	void setTrackPower(TrackPower state);

    /// @brief TBA
    void emergencyStop();

    /// @brief TBA
    /// @param address TBA
    /// @param action TBA
    bool setTurnout(String address, TurnoutAction action);   // address is turnout system name e.g. LT92
    /// @brief TBA
    /// @param address TBA
    bool setRoute(String address);   // address is turnout system name e.g. IO:AUTO:0008

    std::vector<String> locomotives[6];
    std::vector<Direction> locomotivesFacing[6];

    int getMultiThrottleIndex(char multiThrottle);
    
    long getLastServerResponseTime();  // seconds since Arduino start
    

  private:
  
    bool server;
    
	Stream *stream;
    Stream *console;
	NullStream nullStream;
    String outboundBuffer;
    double outboundCmdsTimeLastSent;
    int outboundCmdsMininumDelay;
    bool commandsNeedLeadingCrLf = false;
	
	WiThrottleProtocolDelegate *delegate = NULL;

    bool processCommand(char *c, int len);
    bool processLocomotiveAction(char multiThrottle, char *c, int len);
    bool processFastTime(char *c, int len);
    bool processHeartbeat(char *c, int len);
    bool processRosterFunctionList(char multiThrottle, char *c, int len);
    void processProtocolVersion(char *c, int len);
    void processServerType(char *c, int len);
    void processServerDescription(char *c, int len);	
    void processMessage(char *c, int len);	
    void processAlert(char *c, int len);	
    void processWebPort(char *c, int len);
	void processRosterList(char *c, int len);
    void processTurnoutList(char *c, int len);
    void processRouteList(char *c, int len);
    void processTrackPower(char *c, int len);
    void processFunctionState(char multiThrottle, const String& functionData);
    void processRosterFunctionListEntries(char multiThrottle, const String& s);
    void processSpeedSteps(char multiThrottle, const String& speedStepData);
    void processDirection(char multiThrottle, const String& speedStepData);
    void processSpeed(char multiThrottle, const String& speedData);
    void processAddRemove(char multiThrottle, char *c, int len);
    void processStealNeeded(char multiThrottle, char *c, int len);
    void processTurnoutAction(char *c, int len);
    void processRouteAction(char *c, int len);
    void processUnknownCommand(const String& unknownCommand);

    bool checkFastTime();
    bool checkHeartbeat();

    void sendDelayedCommand(String cmd);

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

    bool locomotiveSelected[6] = {false, false, false, false, false, false};

    String currentAddress[6];

    int currentSpeed[6];
    int speedSteps[6];  // 1=128, 2=28, 4=27, 8=14, 16=28Mot
    Direction currentDirection[6];

    String mostRecentTurnout;
    TurnoutState mostRecentTurnoutState;

    long lastServerResponseTime;
};

#endif // WITHROTTLE_H
