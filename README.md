# 🦾Anima - Modular Sauce Dispensing Machine  

🎛️ Overview: Anima is a modular, industry-level sauce dispensing machine controlled using NEMA-17 stepper motors.   
It features synchronization logic across five machine modules and a human-machine interface (HMI) to allow operators to select different sauces and quantities with precision.

⚙️ Features:  
Modular Design: Code for controlling five distinct machine modules, each responsible for different aspects of the dispensing process.    
NEMA-17 Stepper Motors: Firmware designed to precisely control the movement of stepper motors for the accurate dispensing of sauce.    
Real-Time Synchronization: Ensures high-performance and real-time coordination across modules for efficient sauce dispensing.    
Human-Machine Interface (HMI): LCD 16x2 and joystick navigation allow operators to select sauce types, set quantities, and confirm operations with ease.    

🛠 Tools & Technologies:    
Languages: C++, Arduino Libraries   
Hardware: Arduino ecosystem (LCD 16x2, joystick, NEMA-17 stepper motors, Arduino microcontrollers)     

📂 Project Structure:  
SLAVE1.c++: Controls modules 1 & 2, i.e., the container dispenser and the pump.  
SLAVE2.c++: Controls modules 3 & 4, i.e., lids and stamp.  
Screen.c++: Human-machine interface with joystick navigation for sauce selection and confirmation.  

📷Screenshot:

