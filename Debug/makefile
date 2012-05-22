################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include proxy/subdir.mk
-include mmsrip/subdir.mk
-include minizip/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: httrack

# Tool invocations
httrack: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'invoking: GCC C Linker'
	gcc  -o"httrack" $(OBJS) $(USER_OBJS) $(LiBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) httrack
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
