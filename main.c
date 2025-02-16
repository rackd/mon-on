#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>

static void daemonize(void) {
    pid_t pid;

    pid = fork();
    if (pid < 0) {
        perror("fork 1st");
        exit(1);
    }

    if (pid > 0) {
        exit(1);
    }

    if (setsid() < 0) {
        perror("setsid");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork 2nd");
        exit(1);
    }
    if (pid > 0) {
        exit(1);
    }

    umask(0);
    if (chdir("/") < 0) {
        perror("chdir");
        exit(1);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void turn_on_all_monitors(Display *dpy) {
    Window root = DefaultRootWindow(dpy);
    XRRScreenResources *res = XRRGetScreenResourcesCurrent(dpy, root);
    if (!res) {
        syslog(LOG_ERR, "Failed to get screen resources.");
        return;
    }

    for (int i = 0; i < res->noutput; i++) {
        RROutput output = res->outputs[i];
        XRROutputInfo *output_info = XRRGetOutputInfo(dpy, res, output);
        if (!output_info) {
            continue;
        }
    
        if (output_info->connection == RR_Connected && output_info->nmode > 0) {
            RRMode mode = output_info->modes[0]; // first available mode

            if (output_info->crtc) {
                XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(dpy, res, output_info->crtc);
        
                if (crtc_info) {
                    int status = XRRSetCrtcConfig(dpy, res, output_info->crtc, CurrentTime, crtc_info->x, crtc_info->y, mode, crtc_info->rotation, &output, 1);
                    if (status == Success) {
                        syslog(LOG_INFO, "Turned on output: %s", output_info->name);
                    } else {
                        syslog(LOG_ERR, "Failed to set CRTC config for %s", output_info->name);
                    }
                    XRRFreeCrtcInfo(crtc_info);
                }
            }
        }
        XRRFreeOutputInfo(output_info);
    }

    XRRFreeScreenResources(res);
    XFlush(dpy);
}

int main(int argc, char **argv) {
    int no_daemon = 0;
    char *displayName = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-daemon") == 0) {
            no_daemon = 1;
        } else {
            displayName = argv[i];
        }
    }

    if (!displayName) {
        displayName = getenv("DISPLAY");
    }

    if (!displayName) {
        fprintf(stderr, "No DISPLAY specified. Exiting.\n");
        return 1;
    }

    if (!no_daemon) {
        daemonize();
        openlog("mon-on", LOG_PID | LOG_CONS, LOG_DAEMON);
    } else {
        openlog("mon-on", LOG_PID | LOG_CONS | LOG_PERROR, LOG_DAEMON);
        printf("Running in foreground mode (--no-daemon) using DISPLAY=%s\n", displayName);
    }
    syslog(LOG_INFO, "Starting mon-on using DISPLAY=%s", displayName);

    Display *dpy = XOpenDisplay(displayName);
    if (!dpy) {
        syslog(LOG_ERR, "Unable to open display %s", displayName);
        exit(EXIT_FAILURE);
    }

    Window root = DefaultRootWindow(dpy);

    int custom_key = 0;
    KeyCode keycode;
    char *ckey = getenv("CKEYCODE");
    if (ckey) {
        custom_key = 1;
        keycode = (KeyCode)atoi(ckey);
        syslog(LOG_INFO, "Using custom keycode from CKEYCODE: %d", keycode);
    } else {
        keycode = XKeysymToKeycode(dpy, XK_m);
        if (keycode == 0) {
            syslog(LOG_ERR, "Failed to get keycode for 'm'");
            return 1;
        }
        syslog(LOG_INFO, "Using default keycode for Control+m: %d", keycode);
    }

    unsigned int modifiers = custom_key ? AnyModifier : ControlMask;
    XGrabKey(dpy, keycode, modifiers, root, True, GrabModeAsync, GrabModeAsync);
    XSelectInput(dpy, root, KeyPressMask);
    syslog(LOG_INFO, "Listening for key press to turn on monitors");

    while (1) {
        XEvent ev;
        XNextEvent(dpy, &ev);
        if (ev.type == KeyPress) {
            XKeyEvent keyEvent = ev.xkey;
            if (keyEvent.keycode == keycode && (custom_key ? 1 : (keyEvent.state & ControlMask))) {
                syslog(LOG_INFO, custom_key ? "Custom key pressed. Turning on all monitors." : "Control+m pressed. Turning on all monitors.");
                turn_on_all_monitors(dpy);
            }
        }
    }

    closelog();
    XCloseDisplay(dpy);
    return 0;
}
