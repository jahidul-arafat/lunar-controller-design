# Lunar Lander Controller

## Overview

This project implements a controller for a Lunar Lander simulation. The controller handles user input, communicates with a server modeling the lander, updates a dashboard, and logs data.

## Requirements

- GCC
- ncurses library
- POSIX Threads

## Setup

1. Ensure the necessary libraries are installed:
    ```bash
    sudo apt-get install libncurses5 libncurses5-dev libncursesw5 libncursesw5-dev
    ```

2. Download the Lunar Lander server and dashboard JAR files from the provided Blackboard repository and place them in the project directory.

## Compilation

To compile the project, run:
```bash
make all

