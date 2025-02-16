# mon-on

mon-on is a small utility for X11 that monitors for a specific keypress and, when detected, reconfigures all connected monitors using the Xrandr API.

## What It Does

- **Keypress Monitoring:**  
  - By default, mon-on listens for the key combination Control+m.
  - If the environment variable `CKEYCODE` is set (to a numeric keycode), mon-on uses that key instead and does not require the Control modifier.
  
- **Monitor Reconfiguration:**  
  - When the designated key is pressed, mon-on iterates over all outputs obtained from Xrandr.
  - For each connected output with available modes, it selects the first available mode and applies it using the current CRTC settings.
  
- **Running Modes:**  
  - **Foreground Mode:**  
    Run with the `--no-daemon` option. In this mode, log messages are printed to both syslog and the console.
  - **Daemon Mode:**  
    Run without `--no-daemon` to have the process fork into the background and log only via syslog.

## Requirements

- X11 development libraries (Xlib, Xrandr)

## Build Instructions

1. **Compile the Program:**

   Run the provided Makefile from the project root:
   ```bash
   make
    ```
2. **Optional Installation via make:**

    mon-on can be optionally installed as a system service using the provided Makefile targets.

    The service file must be edited before running this commnad. Please see the service file for more info.
    ```bash
    sudo make install
    ```