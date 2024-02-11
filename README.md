# File Change Tracker "Daemon"

This program tracks changes made to a specified file and sends notifications when certain events occur. It runs as a "daemon" on Linux systems.

## Features

- Monitors file access, deletion, modification, and write/close events.
- Sends desktop notifications using `notify-send` when significant file events are detected.
- Runs as a background, allowing continuous monitoring of file changes.

## Installation

1. Clone the repository or download the source code.

```
git clone https://github.com/kingbian/track-it.git
```

2. Compile the program

```
gcc -o track_it trackIt.c
```

3. move the executable to your systems PATH to make it accessible system wide

```
sudo cp track_it /usr/local/bin
```

## Usage

#### To start the program as a background process

```
track_it /path/to/file
```

#### To end the program

```
kill track_it
```

## Dependencies

- `libnotify` used for desktop notifications

## Acknowledgments

- This project was inspired by:

```
https://github.com/sinasun/count-it
```
