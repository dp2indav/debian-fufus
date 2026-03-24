// USB device detection and handling using sysfs and /dev/sdX
// This is the proper approach used by Rufus, dd, and Balena Etcher

#ifndef USB_H
#define USB_H

#include <stdint.h>

// USB device structure
typedef struct {
    char device_path[256];      // /dev/sdb, /dev/sdc, etc.
    char model[256];            // Device model name
    char serial[256];           // Serial number
    uint64_t capacity;          // Size in bytes
    int is_removable;           // Is removable media?
    int is_mounted;             // Currently mounted?
} usb_device_t;

// Function declarations
int detect_usb_devices(usb_device_t **devices, int *count);
void list_usb_devices(void);
int is_device_mounted(const char *device_path);
int unmount_device(const char *device_path);
int write_iso_to_device(const char *iso_path, const char *device_path, void (*progress_callback)(int));
int verify_iso(const char *iso_path);
void free_usb_devices(usb_device_t *devices);

#endif // USB_H