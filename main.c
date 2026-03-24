// Debian Fufus - Bootable USB Creator for Debian
// Main entry point with CLI and GUI support

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/usb.h"
#include "../include/gui.h"

void print_usage(const char *program_name) {
    printf("\n=== Debian Fufus - Bootable USB Creator ===\n\n");
    printf("Usage: %s [COMMAND] [OPTIONS]\n\n", program_name);
    printf("Commands:\n");
    printf("  (no command)                  Launch Graphical Interface\n");
    printf("  list                          List all USB devices\n");
    printf("  write <ISO> <DEVICE>          Write ISO to USB device\n");
    printf("  help                          Show this help message\n\n");
    printf("Examples:\n");
    printf("  sudo %s\n", program_name);
    printf("  sudo %s list\n", program_name);
    printf("  sudo %s write debian-11.iso /dev/sdb\n\n", program_name);
    printf("WARNING: Writing to a device will erase ALL data on it!\n\n");
}

void progress_callback(int percentage) {
    printf("\rProgress: %d%% ", percentage);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    // If no arguments are provided, launch the GUI
    if (argc < 2) {
        // Check for root privileges as GUI operations (writing/unmounting) require them
        if (getuid() != 0) {
            fprintf(stderr, "Error: Graphical interface requires root privileges to write disks.\n");
            fprintf(stderr, "Please run: sudo %s\n", argv[0]);
            return 1;
        }
        return gui_main(argc, argv);
    }

    const char *command = argv[1];
