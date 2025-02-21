# Real-Time Thread Programming on Raspberry Pi with PREEMPT_RT

This project delves into the world of real-time programming on the Raspberry Pi platform, leveraging the power of the PREEMPT_RT Linux kernel patch and POSIX thread APIs. The primary objective is to gain a deep understanding of how to create and manage real-time and non-real-time threads, while observing the impact of various scheduling policies, priorities, and CPU affinities on thread performance.

## Background

In the realm of computing, real-time systems stand out as a unique breed where the correctness of the system hinges not only on the logical accuracy of computations but also on the timeliness of the results. The Linux kernel, in its default configuration, does not possess real-time capabilities. However, the PREEMPT_RT patch serves as a game-changer, transforming Linux into a real-time system by making the kernel fully preemptible and minimizing the maximum latency of kernel operations.

POSIX threads, commonly known as pthreads, are a set of C language programming types and procedure calls, implemented through the `pthread.h` header and a thread library. They offer a robust and portable way to create and manage threads, adhering to a standard approach.

## Project Structure

The project encompasses several key components, each playing a crucial role in achieving real-time thread programming on the Raspberry Pi:

1. **Patching the Linux Kernel**: The journey begins by patching the Linux kernel with the PREEMPT_RT patch, unlocking real-time capabilities. This process involves downloading the kernel source, applying the patch, configuring the kernel for full preemption, building the kernel, and finally installing it on the Raspberry Pi.

2. **Thread Management Framework**: A C++ framework is meticulously crafted to facilitate the creation and management of real-time and non-real-time threads. This framework includes:
   - `ThreadRT` and `ThreadNRT` classes, serving as the foundation for creating real-time and non-real-time threads, respectively.
   - `AppTypeX` and `AppTypeY` classes, representing specific types of application workloads that run on the threads.
   - Functions for setting thread attributes, locking memory, and controlling CPU affinity.

3. **Experiment Cases**: The `main()` function serves as the orchestrator, defining several experiment cases, each with a unique configuration of threads, priorities, scheduling policies, and CPU affinities. These cases provide a controlled environment to observe and analyze how these factors influence thread performance.

## Usage

To embark on this real-time thread programming journey, follow these steps:

1. Ensure that you have a Raspberry Pi equipped with a 64-bit operating system.
2. Download and apply the PREEMPT_RT patch to your Linux kernel, then proceed to build and install the patched kernel.
3. Clone this repository to your Raspberry Pi.
4. Build the project using the provided Makefile by executing `make`.
5. Run the program with an experiment ID as a command-line argument: `sudo ./program <exp_id>`.
   - `exp_id` can be 0 (default case), 1, 2, 3, 4, or 5, each corresponding to a specific experiment case.
6. Observe the program's output to gain insights into how the threads behave under the given configuration.

## Results

The program's output provides a detailed view of the threads' execution timeline, including the precise moments when each thread starts and ends, the CPU on which each thread runs, and the execution time of each thread's workload. By comparing the results from different experiment cases, we can develop a deep understanding of how thread priority, scheduling policy, and CPU affinity impact real-time performance.

## Contributing

We welcome contributions from the community to enhance the project and expand its educational value. If you encounter any bugs or have ideas for improvements, please feel free to submit issues or pull requests. This project serves as an educational resource, and contributions that improve clarity or extend its capabilities are greatly appreciated.

## License

This project is open-source and released under the [MIT License](LICENSE), promoting collaboration, learning, and innovation in the field of real-time thread programming.
