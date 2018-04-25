# Low Cost Syringe Pump
Repo for the ENPH 479 syringe pump project

![SystemOverview](https://raw.githubusercontent.com/akshivbansal/Low-Cost-Syringe-Pump/documentation/documentation/figures/System%20Overview.png)

## Introduction
This readme document serves as a guide to future development of the low cost syringe pump project. This document will outline the details of basic use and development. Further details on design approach and project motivation can be found in the engineering recommendation report for the project, which is located in this same git repository. 

## Getting Started with The Pump
The pump employs a Raspberry Pi Zero for controlling and powering all components. The first step for project development is to configure the Raspberry Pi for headless operation so that it can be controlled via Wifi. The pump’s electromechanical systems can then be setup, and the pump code can be ran on the raspberry Pi to operate the pump.

### Raspberry Pi Setup
The system uses a raspberry pi zero with pre-configured software loaded on the SD card so that the files needed to run the pump are available. 

To control the Raspberry Pi over wifi, the system must be setup to access the local wifi network by following step 3 of this tutorial document.

One can then connect to the Raspberry Pi using SSH either through the terminal on Linux, or by using a tool like PuTTY on Windows. Connect to the Pi at the adress pi@raspberrypi.local on port 22. The default password is raspberry

### Pump Setup
To connect all electrical connections in the pump system, follow this raspberry pi pinout chart, and the following diagrams which show the connections for the two pump circuit boards (the oscillator on the left and motor driver on the right).
![Driver Connections](https://raw.githubusercontent.com/akshivbansal/Low-Cost-Syringe-Pump/documentation/documentation/figures/Driver%20Connections.png)
![Oscillator Connections](https://raw.githubusercontent.com/akshivbansal/Low-Cost-Syringe-Pump/documentation/documentation/figures/Oscillator%20Connections.jpg)

The pneumatic connections between the pump, valve, and drive syringe must also be connected using a tee-junction. Then all components can be arranged within the 3d printed chassis and the two halves of the chassis can be secured together with elastic bands. 

### First Run
The syringe pump code repository has three main directories containing code which performs three different functions for developing and demonstrating the pump.

#### freqCount
The freqCount directory contains code which outputs the frequency counting values from the feedback system to the screen. This can be run to check the performance of the oscillator circuit, and to perform system calibration. 

To calibrate the system, make sure the SAMPLING_NUMBER value in freqCount.c and pidPump/setup.h are the same. Then run the freqCount code with both an empty and full syringe, noting the feedback values measured, and the volume difference between the two measurement points. Finally, input these values into pidPump/setup.h as the SYRINGE_MAX_READING (larger of two measurements), SYRINGE_MIN_READING, and SYRINGE_VOLUME_UL (this value in microliters).

#### pwmTest

The pwmTest code runs the pump at a set

#### pidPump

## Potential Future Projects 

### Feedback system design for a Low-Cost Syringe Pump for Global Medicine

#### Project Objectives, Background and Scope
The low cost syringe pump project is aimed at providing cheap accessible anesthesia in low resource surgical environments. The system uses capacitive contact feedback to improve accuracy and precision in lieu of mechanically precise components to bring down cost. An initial prototype, with a working capacitive contact/oscillator system exists but needs to be improved for both performance and reliability. The project will focus on designing the lowest cost oscillator, which can achieve the performance targets, improving the control loop so that the system can be used in a mission critical settings, and testing the reliability of both systems together. 

#### Design and Analysis
Design
Best possible oscillator used to amplify a variable capacitor scheme
Robust control loop which matches a flow rate based on oscillator input, desired pump rate, and current infusion status
Time permitting, sealing and packaging of electronic component inside of chassis for field deployment

Analysis
Reliability testing for both the control loop and the oscillator 
Edge case mitigation for errors/anomalies in clinical use

#### Expected Technical Background
Ideal candidates for this project will have an interest in circuit and control design. They will be comfortable using standard electrical prototyping equipment. They will be comfortable working in a Linux environment and writing code in C. 

### Smartphone Interface Development for a Low-Cost Syringe Pump for Global Medicine

#### Project Objectives, Background and Scope
The low cost syringe pump project is aimed at providing cheap accessible anesthesia in low resource surgical environments. The system uses capacitive contact feedback to improve accuracy and precision in lieu of mechanically precise components to bring down cost. To make the device as useful as possible we need it to be both powered by and operable using a smartphone. To achieve there needs to be an app which can allow an operate to set an infusion rate and volume and also receive feedback on the process. The smartphone will need to send this information via USB On the Go to a Raspberry Pi Zero, which will use it to actually drive the pump. Ideally, the app will allow phones to be swapped mid-procedure to allow for more lengthy infusions.

#### Design and Analysis
Design:
Setup a communication protocol using USB OTG, Android, and a Raspberry Pi Zero
Design an application which easily allows an operator to control and monitor an infusion process 
Create a system to allow the infusion to be carried out on multiple smartphone devices 

Analysis:
Assess reliability in case of various software/hardware failure on the smartphone side
Find edge cases present in the clinical environment and work to mitigate them
Reduce power consumption to make the circuitry last

#### Expected Technical Background
Ideal candidates for this project will understand development for Android devices, and be resourceful when working with communications devices.
