# Complete libusb header definitions for USB device detection and management

#ifndef USB_H
#define USB_H

#include <libusb-1.0/libusb.h>

// Function declarations for USB management

void list_usb_devices();
libusb_device_handle* open_usb_device(uint16_t vendor_id, uint16_t product_id);
void close_usb_device(libusb_device_handle* handle);

// Function to initialize libusb
int init_libusb();

// Function to handle USB events
void handle_usb_event();

#endif // USB_H