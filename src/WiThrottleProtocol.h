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
	NullStream() {}
	int available() { return 0; }
	void flush() {}
	int peek() { return -1; }
	int read() { return -1; }
	size_t write(uint8_t c) { return 1; }
	size_t write(const uint8_t *buffer, size_t size) { return size; }
};

class WiThrottleProtocolDelegate
{
  public:
  
    virtual void receivedVersion(String version) {}
    virtual void receivedServerType(String type) {}
    virtual void receivedServerDescription(String description) {}
    virtual void receivedRosterEntries(int rosterSize) {}
    virtual void receivedRosterEntry(int index, String name, int address, char length) {}

    virtual void receivedTurnoutEntries(int turnoutListSize) {}    
    virtual void receivedTurnoutEntry(int index, String sysName, String userName, int state) {}

    virtual void receivedRouteEntries(int routeListSize) {}
    virtual void receivedRouteEntry(int index, String sysName, String userName, int state) {}

    virtual void fastTimeChanged(uint32_t time) { }
    virtual void fastTimeRateChanged(double rate) { }

    virtual void heartbeatConfig(int seconds) { }

    virtual void receivedFunctionState(uint8_t func, bool state) { }
    virtual void receivedRosterFunctionList(String functions[MAX_FUNCTIONS]) { }

    virtual void receivedFunctionStateMultiThrottle(char multiThrottle, uint8_t func, bool state) { }
    virtual void receivedRosterFunctionListMultiThrottle(char multiThrottle, String functions[MAX_FUNCTIONS]) { }

    virtual void receivedSpeed(int speed) { }             // Vnnn
    virtual void receivedDirection(Direction dir) { }     // R{0,1}
    virtual void receivedSpeedSteps(int steps) { }        // snn

    virtual void receivedSpeedMultiThrottle(char multiThrottle, int speed) { }             // Vnnn
    virtual void receivedDirectionMultiThrottle(char multiThrottle, Direction dir) { }     // R{0,1}
    virtual void receivedSpeedStepsMultiThrottle(char multiThrottle, int steps) { }        // snn

    virtual void receivedWebPort(int port) { }            // PWnnnnn
    virtual void receivedTrackPower(TrackPower state) { } // PPAn

    virtual void addressAdded(String address, String entry) { }  // MT+addr<;>roster entry
    virtual void addressRemoved(String address, String command) { } // MT-addr<;>[dr]
    virtual void addressStealNeeded(String address, String entry) { } // MTSaddr<;>addr

    virtual void addressAddedMultiThrottle(char multiThrottle, String address, String entry) { }  // M0+addr<;>roster entry
    virtual void addressRemovedMultiThrottle(char multiThrottle, String address, String command) { } // M0-addr<;>[dr]
    virtual void addressStealNeededMultiThrottle(char multiThrottle, String address, String entry) { } // MTSaddr<;>addr

    virtual void receivedTurnoutAction(String systemName, TurnoutState state) { } //  PTAturnoutstatesystemname
    virtual void receivedRouteAction(String systemName, RouteState state) { } //  PTAturnoutstatesystemname
};


class WiThrottleProtocol
{
  public:
    
	WiThrottleProtocol(bool server = false);

	void setDelegate(WiThrottleProtocolDelegate *delegate);
	void setDelegate(WiThrottleProtocolDelegate *delegate, int delayBetweenCommandsSent);
    void setLogStream(Stream *console);

    void setSpeedCommandsNeedToBeSentTwice(bool twice);

	void connect(Stream *stream);
    void connect(Stream *stream, int delayBetweenCommandsSent);
    void disconnect();

    void setDeviceName(String deviceName);
    void setDeviceID(String deviceId);

    bool check();

    //int fastTimeHours();
    //int fastTimeMinutes();
	double getCurrentFastTime();
    float getFastTimeRate();
    bool clockChanged;

    String currentDeviceName;
    void requireHeartbeat(bool needed=true);
    bool heartbeatChanged;

    bool addLocomotive(String address);  // address is [S|L]nnnn (where n is 0-10000)
    bool stealLocomotive(String address);   // address is [S|L]nnnn (where n is 0-10000)
    bool releaseLocomotive(String address = "*");
    String getLeadLocomotive();
    String getLocomotiveAtPosition(int position);
    int getNumberOfLocomotives();

    // multiThrottle support
    bool addLocomotive(char multiThrottle, String address);  // address is [S|L]nnnn (where n is 0-10000)
    bool stealLocomotive(char multiThrottle, String address);   // address is [S|L]nnnn (where n is 0-10000)
    bool releaseLocomotive(char multiThrottle, String address = "*");
    String getLeadLocomotive(char multiThrottle);
    String getLocomotiveAtPosition(char multiThrottle, int position);
    int getNumberOfLocomotives(char multiThrottle);

    void setFunction(int funcnum, bool pressed);
    // multiThrottle support
    void setFunction(char multiThrottle, int funcnum, bool pressed);
    void setFunction(char multiThrottle, String address, int funcnum, bool pressed);

    bool setSpeed(int speed);
    int getSpeed();
    bool setDirection(Direction direction);
    Direction getDirection();

    // multiThrottle support
    bool setSpeed(char multiThrottle, int speed);
    bool setSpeed(char multiThrottle, int speed, bool forceSend);
    int getSpeed(char multiThrottle);
    bool setDirection(char multiThrottle, Direction direction);
    bool setDirection(char multiThrottle, Direction direction, bool forceSend);
    bool setDirection(char multiThrottle, String address, Direction direction);
    bool setDirection(char multiThrottle, String address, Direction direction, bool ForceSend);
    Direction getDirection(char multiThrottle);
    Direction getDirection(char multiThrottle, String address);
    void emergencyStop(char multiThrottle);
    void emergencyStop(char multiThrottle, String address);
	
	void setTrackPower(TrackPower state);

    void emergencyStop();

    bool setTurnout(String address, TurnoutAction action);   // address is turnout system name e.g. LT92
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
    bool speedCommandsNeedToBeSentTwice = false;
	
	WiThrottleProtocolDelegate *delegate = NULL;

    bool processCommand(char *c, int len);
    bool processLocomotiveAction(char multiThrottle, char *c, int len);
    bool processFastTime(char *c, int len);
    bool processHeartbeat(char *c, int len);
    bool processRosterFunctionList(char multiThrottle, char *c, int len);
    void processProtocolVersion(char *c, int len);
    void processServerType(char *c, int len);
    void processServerDescription(char *c, int len);	
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

    void sendCommand(String cmd);
    void sendCommand(String cmd, bool twice);
    void sendDelayedCommand(String cmd);
    void sendDelayedCommand(String cmd, bool twice);

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
