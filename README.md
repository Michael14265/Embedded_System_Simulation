# Embedded Tank Control System Simulation
This codebase contains a simulation of an embedded control system for an underground tank network. The project is inspired by the final example in An Embedded Software Primer by David E. Simon, which presents a similar system as a comprehensive case study in embedded software design.

The book places strong emphasis on the role of Real-Time Operating Systems (RTOS) in embedded applications, covering critical concepts such as task structuring, mutexes, interrupt handling, and overall system architecture.

The original implementation was designed for a DOS-based environment using the µC/OS RTOS. However, running the original setup would have required a DOS virtual machine. Instead, I chose to port the project to a Windows environment using FreeRTOS. This decision not only made the system more accessible but also served as excellent practice in adapting and modernizing embedded software.

Working on this project has been instrumental in deepening my understanding of RTOS fundamentals and embedded system design. It highlighted the interrupt-driven nature of many embedded systems and taught me how to design efficient ISRs (Interrupt Service Routines) that remain lightweight. I also gained hands-on experience using queues to delegate compute-intensive work to background tasks, a common and essential pattern in real-time system design.