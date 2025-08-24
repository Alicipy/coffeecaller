# CoffeeCaller

CoffeeCaller is a Zephyr RTOS module that provides the source code for the
[TiaC CoffeeCaller](https://github.com/tiacsys/ecad-coffeecaller) board.

## Project Purpose

This repository contains the source code for the 'CoffeeCaller' application,
a simple self-designed development board that coordinates coffee breaks
between colleagues that sit in different office rooms in a building.

In each room, there is one prepared Coffee Caller board, which communicates
to others via a Mesh technology (e.g., OpenThread). When one wants to drink
coffee, them press the button, which triggers a buzzer and LEDs on each
CoffeeCaller it can reach. Then, others can communicate that they would
also drink coffee by pressing the button on their CoffeeCaller too,
which triggers a short acknowledgement beep.

After some time, the Coffeecaller signals if someone wants to go with you
for coffee or not by doing beeping noices and flashing LEDs.

## Project state

This project is still in development, and multiple features are still missing.

## Features

- Build with Zephyr RTOS
- Everything builds and is executable for [TiaC CoffeeCaller](https://github.com/tiacsys/ecad-coffeecaller) and ``native_sim`` (for development purposes).
- Test setups for multiple test levels

## Supported boards

We tested the code on the following boards:

- [TiaC CoffeCaller](https://github.com/tiacsys/ecad-coffeecaller)
- ``native_sim``

Other boards may work but are not actively tested.

### Toolchain setup

To compile the applications, you need the Zephyr SDK installed. For that,
follow the instructions on the [Zephyr SDK installation page](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html)

## Getting Started

This is a super short intro to what needs to be done to setup the workspace.
More details at [Zephyrs 'Getting started' guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).

Create a new workspace and change the working directory:

```shell
mkdir cc_ws
cd cc_ws
```

Create + activate a Python virtual environment:

```shell
python3 -m venv .venv
. .venv/bin/activate
```

Now install the Zephyr RTOS meta-tool `west`.

```shell
pip install west
```

Setup the workspace and clone the repository with all modules and Zephyr:

```shell
west init -m https://github.com/CoffeeCaller/CoffeeCaller
west update
```

Now, install all Python requirements for Zephyr so all features work:

```shell
pip install -r zephyr/scripts/requirements.txt
```

Afterward, you can work with the repository and workspace.

## Usage

These list the most common use cases during development.
To cover all use cases, please consult the
[`west` documentation](https://docs.zephyrproject.org/latest/develop/west/index.html).

Execute all commands in the workspace `cc_ws`.

### Building an application for a specific board

```shell
west build -b <board_name> <path_to_application>
west build -b tiac_coffeecaller ./coffeecaller/applications/coffeecaller
```

### Flashing to the board

The `tiac_coffeecaller` board has an uf2 bootloader. So connect the board and after mounting, do:

```
west flash -r uf2
```

### Executing tests

Executing (a subset of) tests for all supported boards:

```shell
west twister -T <path_to_test_folder>
# example to execute all integration tests in `tests` folder
west twister --integration -T ./coffeecaller/tests
# example to execute all tests for `native_sim`
west twister -p native_sim -T ./coffeecaller
```
