# Build configuration for all projects...
.PHONY: debug
debug: $(eval Target := debug) \
	all_projects

.PHONY: release
release: $(eval Target := release) \
	all_projects

# Builds all the projects...
.PHONY: all_projects
all_projects: \
	IbDemoCan_gcc \
	IbDemoCanNewSync_gcc \
	IbDemoConfigBuilder_gcc \
	IbDemoEthernet_gcc \
	IbDemoExecutionController_gcc \
	IbDemoExecutionControllerProxy_gcc \
	IbDemoFlexray_gcc \
	IbDemoGenericMessage_gcc \
	IbDemoIO_gcc \
	IbDemoLin_gcc \
	IbDemoPassiveSystemMonitor_gcc \
	IbDemoSystemController_gcc \
	IbDemoTickTickDone_gcc

# Builds project 'IbDemoCan_gcc'...
.PHONY: IbDemoCan_gcc
IbDemoCan_gcc:
	make --directory="Can/" --file=IbDemoCan_gcc.makefile $(Target)

# Builds project 'IbDemoCanNewSync_gcc'...
.PHONY: IbDemoCanNewSync_gcc
IbDemoCanNewSync_gcc:
	make --directory="Can/" --file=IbDemoCanNewSync_gcc.makefile $(Target)

# Builds project 'IbDemoConfigBuilder_gcc'...
.PHONY: IbDemoConfigBuilder_gcc
IbDemoConfigBuilder_gcc:
	make --directory="ConfigBuilder/" --file=IbDemoConfigBuilder_gcc.makefile $(Target)

# Builds project 'IbDemoEthernet_gcc'...
.PHONY: IbDemoEthernet_gcc
IbDemoEthernet_gcc:
	make --directory="Ethernet/" --file=IbDemoEthernet_gcc.makefile $(Target)

# Builds project 'IbDemoExecutionController_gcc'...
.PHONY: IbDemoExecutionController_gcc
IbDemoExecutionController_gcc:
	make --directory="ExecutionController/" --file=IbDemoExecutionController_gcc.makefile $(Target)

# Builds project 'IbDemoExecutionControllerProxy_gcc'...
.PHONY: IbDemoExecutionController_gcc
IbDemoExecutionControllerProxy_gcc:
	make --directory="ExecutionControllerProxy/" --file=IbDemoExecutionControllerProxy_gcc.makefile $(Target)

# Builds project 'IbDemoFlexray_gcc'...
.PHONY: IbDemoFlexray_gcc
IbDemoFlexray_gcc:
	make --directory="FlexRay/" --file=IbDemoFlexray_gcc.makefile $(Target)

# Builds project 'IbDemoGenericMessage_gcc'...
.PHONY: IbDemoGenericMessage_gcc
IbDemoGenericMessage_gcc:
	make --directory="GenericMessage/" --file=IbDemoGenericMessage_gcc.makefile $(Target)

# Builds project 'IbDemoIO_gcc'...
.PHONY: IbDemoIO_gcc
IbDemoIO_gcc:
	make --directory="IO/" --file=IbDemoIO_gcc.makefile $(Target)

# Builds project 'IbDemoLin_gcc'...
.PHONY: IbDemoLin_gcc
IbDemoLin_gcc:
	make --directory="Lin/" --file=IbDemoLin_gcc.makefile $(Target)

# Builds project 'IbDemoPassiveSystemMonitor_gcc'...
.PHONY: IbDemoPassiveSystemMonitor_gcc
IbDemoPassiveSystemMonitor_gcc:
	make --directory="SystemMonitor/" --file=IbDemoPassiveSystemMonitor_gcc.makefile $(Target)

# Builds project 'IbDemoSystemController_gcc'...
.PHONY: IbDemoSystemController_gcc
IbDemoSystemController_gcc:
	make --directory="SystemController/" --file=IbDemoSystemController_gcc.makefile $(Target)

# Builds project 'IbDemoTickTickDone_gcc'...
.PHONY: IbDemoTickTickDone_gcc
IbDemoTickTickDone_gcc:
	make --directory="TickTickDone/" --file=IbDemoTickTickDone_gcc.makefile $(Target)

# Cleans all projects...
.PHONY: clean
clean:
	make --directory="Can/" --file=IbDemoCan_gcc.makefile clean
	make --directory="Can/" --file=IbDemoCanNewSync_gcc.makefile clean
	make --directory="ConfigBuilder/" --file=IbDemoConfigBuilder_gcc.makefile clean
	make --directory="Ethernet/" --file=IbDemoEthernet_gcc.makefile clean
	make --directory="ExecutionController/" --file=IbDemoExecutionController_gcc.makefile clean
	make --directory="ExecutionControllerProxy/" --file=IbDemoExecutionControllerProxy_gcc.makefile clean
	make --directory="FlexRay/" --file=IbDemoFlexray_gcc.makefile clean
	make --directory="GenericMessage/" --file=IbDemoGenericMessage_gcc.makefile clean
	make --directory="IO/" --file=IbDemoIO_gcc.makefile clean
	make --directory="Lin/" --file=IbDemoLin_gcc.makefile clean
	make --directory="SystemMonitor/" --file=IbDemoPassiveSystemMonitor_gcc.makefile clean
	make --directory="SystemController/" --file=IbDemoSystemController_gcc.makefile clean
	make --directory="TickTickDone/" --file=IbDemoTickTickDone_gcc.makefile clean
