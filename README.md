# OS Homework Repository

This repository contains homework assignments for the [CS162 Operating Systems](https://cs162.org/) course from 2020 that has been presented in Sharif University Of Technology by [Dr.Kharrazi](https://sharif.edu/~kharrazi/)

## Homework Assignments

### Homework 0 (hw0)

Homework 0 is the initial assignment for the course, aimed at preparing the system for the project, which can be found [here](./HW0/homework0.pdf).


##### Words Folder
In the `words` folder, we create a linked list using dynamic allocations and read a file from the terminal or file descriptor with two modes:
1. Counting all the words.
2. Calculate the frequency of each word after inserting it into the sorted linked list.

- [Homework Details (PDF)](./HW0/words/list.pdf): Detailed instructions for the 'Words' part of homework 0.

### Homework 1 (hw1)

Homework 1 focuses on creating a shell that can run programs in the background or foreground and redirect read or write child processes to a file of your choice.

- [Homework Details](./hw1/hw1-shell.pdf): Detailed instructions for homework 1.

### Homework 2 (hw2)

Homework 2 involves building an HTTP server that processes GET requests and can also serve as a proxy server. It utilizes a thread pool to handle incoming requests efficiently. You can adjust the thread pool size during the booting stage.

- [Homework Details](./hw2/hw2-httpserver.pdf): Detailed instructions for homework 2.

### Homework 3 (hw3)

Homework 3 is centred around memory management. You'll implement the malloc, realloc and free functions using system calls like sbrk and handle fragmentation using the first-fit algorithm. Test cases for this assignment can be found [here](./hw3/mm_test.c).

- [Homework Details](./hw3/hw3-malloc.pdf): Detailed instructions for homework 3.

## Getting Started

To access and work on any of the homework assignments, follow these steps:

1. Clone this repository to your local machine:

   ```bash
   git clone https://github.com/hoseinaghaei/os_homework.git
