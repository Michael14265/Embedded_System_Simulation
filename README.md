# Embedded Tank Control System Simulation
This codebase contains a simulation of an embedded control system for an underground tank network. The project is inspired by the final example in An Embedded Software Primer by David E. Simon, which presents a similar system as a comprehensive case study in embedded software design.

The book places strong emphasis on the role of Real-Time Operating Systems (RTOS) in embedded applications, covering critical concepts such as task structuring, mutexes, interrupt handling, and overall system architecture.

The original implementation was designed for a DOS-based environment using the µC/OS RTOS. However, running the original setup would have required a DOS virtual machine. Instead, I chose to port the project to a Windows environment using FreeRTOS, knowing that this would give me the opportunity to learn FreeRTOS while forcing me to become intensely familiar with the codebase.

Working on this project has been instrumental in deepening my understanding of RTOS fundamentals and embedded system design. It highlighted the interrupt-driven nature of many embedded systems and taught me how to design efficient ISRs (Interrupt Service Routines) that remain lightweight. I also gained hands-on experience using queues to delegate compute-intensive work to background tasks, a common and essential pattern in real-time system design.

## 🔍 Main File
Among the three main source files in the repository, dbgmain.c is the primary file to focus on. It handles system initialization, task creation, and the core simulation logic. This file also simulates hardware events—such as button presses—that would normally be triggered by real-world inputs in a physical embedded system.

## 🧩 Supporting Files and Modular Design
The project is structured using a modular design, with supporting files like button.c encapsulating specific functional logic. This mirrors the architecture of real embedded systems, where responsibilities are divided between hardware-interfacing components and higher-level control logic:

* dbgmain.c simulates hardware-level behavior by generating software-triggered interrupts (e.g., simulating a button press).

* button.c contains the corresponding logic that would run in response to those interrupts.

This separation of concerns improves code maintainability and aligns with best practices in embedded software design—where ISRs handle only lightweight tasks and delegate heavier processing to background tasks or dedicated modules.
