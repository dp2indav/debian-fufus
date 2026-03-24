// Debian Fufus - Bootable USB Creator for Debian
// Main entry point with CLI interface

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/usb.h"

void print_usage(const char *program_name) {
    printf("\n=== Debian Fufus - Bootable USB Creator ===\n\n");
    printf("Usage: %s [COMMAND] [OPTIONS]\n\n", program_name);
    printf("Commands:\n");
    printf("  list                          List all USB devices\n");
    printf("  write <ISO> <DEVICE>          Write ISO to USB device\n");
    printf("  help                          Show this help message\n\n");
    printf("Examples:\n");
    printf("  sudo %s list\n", program_name);
    printf("  sudo %s write debian-11.iso /dev/sdb\n\n", program_name);
    printf("WARNING: Writing to a device will erase ALL data on it!\n\n");
}

void progress_callback(int percentage) {
    printf("\rProgress: %d%% ", percentage);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "list") == 0) {
        printf("Scanning for USB devices...\n");
        list_usb_devices();
        return 0;
    }
    else if (strcmp(command, "write") == 0) {
        if (argc < 4) {
            printf("Error: write requires ISO file and device path\n");
            printf("Usage: %s write <ISO> <DEVICE>\n", argv[0]);
            return 1;
        }

        const char *iso_path = argv[2];
        const char *device_path = argv[3];

        printf("ISO File: %s\n", iso_path);
        printf("Target Device: %s\n\n", device_path);

        // Check if running as root
        if (getuid() != 0) {
            fprintf(stderr, "Error: write operation requires root privileges\n");
            fprintf(stderr, "Please run: sudo %s write %s %s\n", argv[0], iso_path, device_path);
            return 1;
        }

        // Unmount device if mounted
        unmount_device(device_path);

        // Write ISO to device
        if (write_iso_to_device(iso_path, device_path, progress_callback) < 0) {
            fprintf(stderr, "\nError: Failed to write ISO to device\n");
            return 1;
        }

        printf("\nBoot medium created successfully!\n");
        printf("You can now use %s to boot into Debian.\n", device_path);
        return 0;
    }
    else if (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    else {
        printf("Error: Unknown command '%s'\n", command);
        print_usage(argv[0]);
        return 1;
    }
}