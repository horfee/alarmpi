#CC           = $(CROSS_COMPILE)g++
DESTDIR      = $(prefix)

LIBS := -lssl -levent -lpthread -lcrypto -lsqlite3 -ldl

ifdef RPI
RPIFLAG = -DRPI=1
LIBS := -lssl -lpigpiod_if2 -lpigpio -levent -lpthread -lcrypto -lsqlite3 -ldl
endif

ifdef WIRINGPI
WIRINGPIFLAG = -DWIRINGPI=1
LIBS := -lssl -levent -lpthread -lcrypto -lsqlite3 -ldl -lwiringPi
endif

override CFLAGS += -ggdb -std=c++0x -O0 -Wall $(RPIFLAG) $(WIRINGPIFLAG) -pthread -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"

#PREFIX  :=
DESTDIR=/Volumes/AlarmPI/workspace/alarmpi/lib
BINDIR = $(prefix)/bin
HEADERS += $(DESTDIR)/usr/include
LIBDIR += $(DESTDIR)/usr/lib
MANDIR = $(prefix)/man

RM := rm -rf

USEPIGPIO =

CPP_SRCS += \
AlarmPI.cpp \
AlarmSystem.cpp \
AlarmSystemDAOSQLite.cpp \
DAOFactory.cpp \
Mode.cpp \
NetworkModule.cpp \
Property.cpp \
RCReceiverTransmitter.cpp \
RCSwitch.cpp \
RF433Module.cpp \
SIM800Module.cpp \
Utils.cpp \
_433D.cpp \
servlets/AlarmPIActionsServlet.cpp \
servlets/AlarmPIAssociationsServlet.cpp \
servlets/AlarmPIDetectedDevicesServlet.cpp \
servlets/AlarmPIDevicesServlet.cpp \
servlets/AlarmPIModesServlet.cpp \
servlets/AlarmPIPhonesServlet.cpp \
servlets/AlarmPIPingServlet.cpp \
servlets/AlarmPIPropertiesServlet.cpp \
servlets/AlarmPIServlet.cpp \
servlets/AlarmPISimulateSignalReceivedServlet.cpp \
servlets/AlarmPISimulateMessageServlet.cpp \
servlets/AlarmPISystemServlet.cpp \
json/jsoncpp.cpp \
json/jsoncpp.h \
json/json-forwards.h \
httpserver/HTTPFileServlet.cpp \
httpserver/HTTPRequest.cpp \
httpserver/HTTPResponse.cpp \
httpserver/HTTPServer.cpp \
httpserver/HTTPServlet.cpp \
httpserver/InvalidConfigFileException.cpp \
devices/ActionnableDevice.cpp \
devices/BellDevice.cpp \
devices/Device.cpp \
devices/MagneticDevice.cpp \
devices/MotionDevice.cpp \
devices/RemoteDevice.cpp \
actions/Action.cpp \
actions/ActivateAction.cpp \
actions/ActivateModeAction.cpp \
actions/CallPhoneAction.cpp \
actions/DelayAction.cpp \
actions/RingBellAction.cpp \
actions/SendMessageAction.cpp 

OBJS += \
AlarmPI.o \
AlarmSystem.o \
AlarmSystemDAOSQLite.o \
DAOFactory.o \
Mode.o \
NetworkModule.o \
Property.o \
RCReceiverTransmitter.o \
RCSwitch.o \
RF433Module.o \
SIM800Module.o \
Utils.o \
_433D.o \
servlets/AlarmPIActionsServlet.o \
servlets/AlarmPIAssociationsServlet.o \
servlets/AlarmPIDetectedDevicesServlet.o \
servlets/AlarmPIDevicesServlet.o \
servlets/AlarmPIModesServlet.o \
servlets/AlarmPIPhonesServlet.o \
servlets/AlarmPIPingServlet.o \
servlets/AlarmPIPropertiesServlet.o \
servlets/AlarmPIServlet.o \
servlets/AlarmPISimulateSignalReceivedServlet.o \
servlets/AlarmPISimulateMessageServlet.o \
servlets/AlarmPISystemServlet.o \
httpserver/HTTPFileServlet.o \
httpserver/HTTPRequest.o \
httpserver/HTTPResponse.o \
httpserver/HTTPServer.o \
httpserver/HTTPServlet.o \
httpserver/InvalidConfigFileException.o \
devices/ActionnableDevice.o \
devices/BellDevice.o \
devices/Device.o \
devices/MagneticDevice.o \
devices/MotionDevice.o \
devices/RemoteDevice.o \
actions/Action.o \
actions/ActivateAction.o \
actions/ActivateModeAction.o \
actions/CallPhoneAction.o \
actions/DelayAction.o \
actions/RingBellAction.o \
actions/SendMessageAction.o \
json/jsoncpp.o 

CPP_DEPS += \
AlarmPI.d \
AlarmSystem.d \
AlarmSystemDAOSQLite.d \
DAOFactory.d \
Mode.d \
NetworkModule.d \
Property.d \
RCReceiverTransmitter.d \
RCSwitch.d \
RF433Module.d \
SIM800Module.d \
Utils.d \
_433D.d \
servlets/AlarmPIActionsServlet.d \
servlets/AlarmPIAssociationsServlet.d \
servlets/AlarmPIDetectedDevicesServlet.d \
servlets/AlarmPIDevicesServlet.d \
servlets/AlarmPIModesServlet.d \
servlets/AlarmPIPhonesServlet.d \
servlets/AlarmPIPingServlet.d \
servlets/AlarmPIPropertiesServlet.d \
servlets/AlarmPIServlet.d \
servlets/AlarmPISimulateSignalReceivedServlet.d \
servlets/AlarmPISimulateMessageServlet.d \
servlets/AlarmPISystemServlet.d \
json/jsoncpp.d \
httpserver/HTTPFileServlet.d \
httpserver/HTTPRequest.d \
httpserver/HTTPResponse.d \
httpserver/HTTPServer.d \
httpserver/HTTPServlet.d \
httpserver/InvalidConfigFileException.d \
devices/ActionnableDevice.d \
devices/BellDevice.d \
devices/Device.d \
devices/MagneticDevice.d \
devices/MotionDevice.d \
devices/RemoteDevice.d \
actions/Action.d \
actions/ActivateAction.d \
actions/ActivateModeAction.d \
actions/CallPhoneAction.d \
actions/DelayAction.d \
actions/RingBellAction.d \
actions/SendMessageAction.d 


CC_LIBS=
ifneq ($(LIBSPATH),)
	CC_LIBS=-L"$(LIBSPATH)"
endif

# All Target
all: alarmPI


%.o: %.cpp
	$(CROSS_COMPILE)$(CC) -c -I"$(HEADERS)" $(CFLAGS) $^ -o $@

	
# Tool invocations
alarmPI: $(OBJS)
	@echo 'Building target: $@'
	@echo 'Library to use :  $(LIBS)'
	$(CROSS_COMPILE)$(CC) -o "alarmPI" $(OBJS) $(USER_OBJS) $(LIBS) $(CC_LIBS) -L"$(LIBDIR)"
	@echo 'Finished building target: $@'

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(C_DEPS)$(CPP_DEPS) alarmPI
	-@echo ' '


install:	$(ALL)
	install -m 0755 alarmPI         	  $(DESTDIR)$(BINDIR)
	install -m 0755 connectToWifi.sh      $(DESTDIR)$(BINDIR)
	install -m 0755 createAP.sh           $(DESTDIR)$(BINDIR)

.PHONY: all clean dependents
.SECONDARY: