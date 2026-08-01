*** Settings ***
Documentation    Automated Acceptance Test Suite for Zephyr Biomedical Firmware in Renode
Suite Setup      Setup
Suite Teardown   Teardown
Test Setup       Reset Emulation
Resource         ${RENODEKeywords}

*** Variables ***
${BIN}           @${CURDIR}/../build/zephyr/zephyr.elf
${URI}           @platforms/cpus/nrf52840.repl

*** Test Cases ***
Should Boot Firmware And Report Normal Status
    [Documentation]    Verifies that the firmware initializes all RTOS threads and streams NORMAL telemetry status over UART.
    Create Machine
    Create Terminal Tester    sysbus.uart0
    Start Emulation
    
    Wait For Line On Uart     Starting mocked sensor sampling with Input Simulator support...    timeout=10
    Wait For Line On Uart     [FSM State Change] -> NORMAL                 timeout=10
    Wait For Line On Uart     [TELEMETRY] {"status":"NORMAL"               timeout=10

Should Detect Tachycardia Anomaly Via Input Simulator
    [Documentation]    Injects a 160 BPM heart rate over UART1 and verifies the FSM transitions to ALARM.
    Create Machine
    Create Terminal Tester    sysbus.uart0    defaultTester
    
    Start Emulation
    Wait For Line On Uart     Input Simulator active on uart1              timeout=10
    
    # Inject 160 BPM into the Input Simulator (UART1)
    Execute Command           sysbus.uart1 WriteChar 66  # 'B'
    Execute Command           sysbus.uart1 WriteChar 80  # 'P'
    Execute Command           sysbus.uart1 WriteChar 77  # 'M'
    Execute Command           sysbus.uart1 WriteChar 58  # ':'
    Execute Command           sysbus.uart1 WriteChar 49  # '1'
    Execute Command           sysbus.uart1 WriteChar 54  # '6'
    Execute Command           sysbus.uart1 WriteChar 48  # '0'
    Execute Command           sysbus.uart1 WriteChar 10  # '\n'

    # Wait for the RTOS to detect it and output the ALARM JSON
    Wait For Line On Uart     [TELEMETRY] {"status":"ALARM: TACHYCARDIA DETECTED"    timeout=10

*** Keywords ***
Create Machine
    Execute Command           mach create "nrf52840"
    Execute Command           machine LoadPlatformDescription ${URI}
    Execute Command           sysbus LoadELF ${BIN}
