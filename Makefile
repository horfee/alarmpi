#CC           = $(CROSS_COMPILE)g++
DESTDIR      = $(prefix)

RPIFLAG=
ifdef RPI
RPIFLAG = -DRPI
endif

override CFLAGS += -std=c++0x -O3 -Wall $(RPIFLAG) -pthread -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"

ifneq ($(USE_WIRINGPI),)
LIBS := -lssl -levent -lpthread -lcrypto -lsqlite3 -ldl -lwiringPi
else
ifdef RPI
LIBS := -lssl -lpigpiod_if2 -lpigpio -levent -lpthread -lcrypto -lsqlite3 -ldl
else
LIBS := -lssl -levent -lpthread -lcrypto -lsqlite3 -ldl
endif
endif

#PREFIX  := 
BINDIR = $(prefix)/bin
HEADERS += $(prefix)/include
LIBDIR += $(DESTDIR)/lib
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
StringUtils.cpp \
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
StringUtils.o \
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
StringUtils.d \
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
	$(CROSS_COMPILE)$(CXX) -c -I"$(HEADERSPATH)" $(CFLAGS) $^ -o $@



# Tool invocations
alarmPI: $(OBJS)
	@echo 'Building target: $@'
	$(CROSS_COMPILE)$(CXX) -o "alarmPI" $(OBJS) $(USER_OBJS) $(LIBS) $(CC_LIBS)
	@echo 'Finished building target: $@'

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(C_DEPS)$(CPP_DEPS) alarmPI
	-@echo ' '


install:	$(ALL)
	install -m 0755 alarmPI           $(DESTDIR)$(BINDIR)

.PHONY: all clean dependents
.SECONDARY: