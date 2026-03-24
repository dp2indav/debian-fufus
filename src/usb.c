// Complete libusb implementation for USB device handling
#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

// Function to detect and list USB devices
void list_usb_devices() {
    libusb_context *ctx = NULL;
    libusb_device **devs;
    ssize_t cnt;

    // Initialize libusb
    if (libusb_init(&ctx) < 0) {
        fprintf(stderr, "Failed to initialize libusb\n");
        return;
    }

    // Get the list of USB devices
    cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        fprintf(stderr, "Failed to get device list\n");
        return;
    }

    printf("Found %ld USB devices:\n", cnt);
    for (ssize_t i = 0; i < cnt; i++) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(devs[i], &desc) < 0) {
            fprintf(stderr, "Failed to get device descriptor\n");
            continue;
        }
        printf("Device %ld: Vendor ID: 0x%04x, Product ID: 0x%04x\n", i, desc.idVendor, desc.idProduct);
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);
}

// Function to open a USB device
libusb_device_handle* open_usb_device(uint16_t vendor_id, uint16_t product_id) {
    libusb_device_handle *handle = libusb_open_device_with_vid_pid(NULL, vendor_id, product_id);
    if (!handle) {
        fprintf(stderr, "Failed to open device (Vendor ID: 0x%04x, Product ID: 0x%04x)\n", vendor_id, product_id);
    }
    return handle;
}

// Function to close a USB device
void close_usb_device(libusb_device_handle *handle) {
    if (handle) {
        libusb_close(handle);
        printf("USB device closed\n");
    } else {
        fprintf(stderr, "No device to close\n");
    }
}

// Main function
int main() {
    list_usb_devices();
    // Example of opening and closing a device (Vendor ID and Product ID should match a device in the list)
    libusb_device_handle *handle = open_usb_device(0x1234, 0x5678);
    close_usb_device(handle);
    return 0;
}