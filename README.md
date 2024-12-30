# Pandora

Persistence in data streams refers to items that recur across multiple non-overlapping time windows. Tackling persistence-related tasks, such as identifying highly persistent items and estimating persistence of all items, is vital for applications ranging from recommendation systems to anomaly detection in high-velocity data environments. These tasks present significant challenges due to the need for rapid processing and constraints on memory resources. Current methods often fall short in accuracy, particularly when faced with highly skewed data distributions and limited fastest memory budgets like L1 cache, where hash collisions become prevalent. To address these issues, we present Pandora, an innovative approximate data structure. Our approach leverages the insight that items absent for extended durations are likely non-persistent, thus increasing their eviction probability to more effectively accommodate potentially persistent items. We empirically validate this insight and incorporate it into our update strategy, enhancing protection for persistent items. Through this design, Pandora offers a more efficient and accurate solution for persistence-based tasks in challenging data stream environments.


## Pandora: Compilation and Execution Guide
Pandora is implemented in C++. This guide outlines the process to compile and run the examples on Ubuntu using g++ and make.

### System Requirements
Ensure your system meets the following prerequisites:

- g++ and make (tested with g++ version 9.4.0 on Ubuntu 20.04)

- libpcap library (typically available through package managers like apt-get in Ubuntu)

### Dataset Preparation

- Download the required pcap file.

- Update the iptraces.txt file with the appropriate file path.

### Compilation Process
- To compile the examples, navigate to the respective folder and execute:

```
    $ make main_hitter
```
  

### Execution
- Run the compiled program using:

```
$ ./main_hitter
```

- The program will output statistics on detection accuracy and update speed.
