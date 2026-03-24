// USB device detection and ISO writing using /dev/sdX
// This approach is used by professional tools like Rufus, dd, and Balena Etcher

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <linux/fs.h>
#include "../include/usb.h"

#define SYSFS_BLOCK_PATH "/sys/block"
#define SECTOR_SIZE 512
#define BUFFER_SIZE (4 * 1024 * 1024)  // 4MB buffer for faster writes

/**
 * Get device capacity from sysfs
 */
static uint64_t get_device_capacity(const char *device_name) {
    char path[512];
    FILE *fp;
    uint64_t size = 0;

    snprintf(path, sizeof(path), "%s/%s/size", SYSFS_BLOCK_PATH, device_name);
    fp = fopen(path, "r");
    if (fp) {
        fscanf(fp, "%llu", &size);
        fclose(fp);
        size *= SECTOR_SIZE;  // Convert sectors to bytes
    }
    return size;
}

/**
 * Check if device is removable
 */
static int is_removable(const char *device_name) {
    char path[512];
    FILE *fp;
    int removable = 0;

    snprintf(path, sizeof(path), "%s/%s/removable", SYSFS_BLOCK_PATH, device_name);
    fp = fopen(path, "r");
    if (fp) {
        fscanf(fp, "%d", &removable);
        fclose(fp);
    }
    return removable;
}

/**
 * Check if device is mounted by checking /etc/mtab
 */
int is_device_mounted(const char *device_path) {
    FILE *fp;
    char line[512];
    int mounted = 0;

    fp = fopen("/etc/mtab", "r");
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, device_path)) {
                mounted = 1;
                break;
            }
        }
        fclose(fp);
    }
    return mounted;
}

/**
 * Detect USB devices by scanning /sys/block
 */
int detect_usb_devices(usb_device_t **devices, int *count) {
    DIR *dir;
    struct dirent *entry;
    usb_device_t *dev_list = NULL;
    int dev_count = 0;
    int capacity = 10;  // Initial capacity

    dev_list = (usb_device_t *)malloc(sizeof(usb_device_t) * capacity);
    if (!dev_list) {
        perror("malloc");
        return -1;
    }

    dir = opendir(SYSFS_BLOCK_PATH);
    if (!dir) {
        perror("opendir");
        free(dev_list);
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Skip . and .. and partitions (sd*, hd*)
        if (entry->d_name[0] == '.' || 
            entry->d_name[1] == '.' ||
            entry->d_name[0] == 'l' ||  // loop devices
            (entry->d_name[0] != 's' && entry->d_name[0] != 'h')) {
            continue;
        }

        // Skip if not removable (to filter USB drives)
        if (!is_removable(entry->d_name)) {
            continue;
        }

        // Expand capacity if needed
        if (dev_count >= capacity) {
            capacity *= 2;
            usb_device_t *temp = (usb_device_t *)realloc(dev_list, sizeof(usb_device_t) * capacity);
            if (!temp) {
                perror("realloc");
                free(dev_list);
                closedir(dir);
                return -1;
            }
            dev_list = temp;
        }

        // Populate device info
        snprintf(dev_list[dev_count].device_path, sizeof(dev_list[dev_count].device_path), "/dev/%s", entry->d_name);
        snprintf(dev_list[dev_count].model, sizeof(dev_list[dev_count].model), "%s", entry->d_name);
        dev_list[dev_count].capacity = get_device_capacity(entry->d_name);
        dev_list[dev_count].is_removable = 1;
        dev_list[dev_count].is_mounted = is_device_mounted(dev_list[dev_count].device_path);
        strcpy(dev_list[dev_count].serial, "N/A");

        dev_count++;
    }

    closedir(dir);
    *devices = dev_list;
    *count = dev_count;

    return 0;
}

/**
 * List all detected USB devices
 */
void list_usb_devices(void) {
    usb_device_t *devices;
    int count;
    int i;

    if (detect_usb_devices(&devices, &count) < 0) {
        fprintf(stderr, "Error detecting USB devices\n");
        return;
    }

    printf("\n=== Detected USB Devices ===\n");
    if (count == 0) {
        printf("No USB devices found.\n");
        free(devices);
        return;
    }

    for (i = 0; i < count; i++) {
        printf("\nDevice %d: %s\n", i + 1, devices[i].device_path);
        printf("  Model: %s\n", devices[i].model);
        printf("  Capacity: %.2f GB\n", (double)devices[i].capacity / (1024 * 1024 * 1024));
        printf("  Removable: %s\n", devices[i].is_removable ? "Yes" : "No");
        printf("  Mounted: %s\n", devices[i].is_mounted ? "Yes (UNMOUNT FIRST)" : "No");
    }
    printf("\n");

    free(devices);
}

/**
 * Unmount device using system command
 */
int unmount_device(const char *device_path) {
    char command[512];
    int result;

    if (!is_device_mounted(device_path)) {
        printf("%s is not mounted.\n", device_path);
        return 0;
    }

    printf("Unmounting %s...\n", device_path);
    snprintf(command, sizeof(command), "sudo umount %s* 2>/dev/null", device_path);
    result = system(command);

    if (result != 0) {
        fprintf(stderr, "Failed to unmount %s\n", device_path);
        return -1;
    }

    printf("%s unmounted successfully.\n", device_path);
    return 0;
}

/**
 * Verify ISO file exists and is readable
 */
int verify_iso(const char *iso_path) {
    struct stat st;

    if (stat(iso_path, &st) < 0) {
        perror("stat");
        fprintf(stderr, "ISO file not found: %s\n", iso_path);
        return -1;
    }

    if (st.st_size == 0) {
        fprintf(stderr, "ISO file is empty: %s\n", iso_path);
        return -1;
    }

    printf("ISO file: %s (%.2f MB)\n", iso_path, (double)st.st_size / (1024 * 1024));
    return 0;
}

/**
 * Write ISO to USB device with progress callback
 */
int write_iso_to_device(const char *iso_path, const char *device_path, void (*progress_callback)(int)) {
    int iso_fd, dev_fd;
    unsigned char *buffer;
    ssize_t bytes_read, bytes_written;
    struct stat st;
    uint64_t total_written = 0;
    int progress;

    // Verify ISO
    if (verify_iso(iso_path) < 0) {
        return -1;
    }

    // Get ISO file size
    stat(iso_path, &st);
    uint64_t iso_size = st.st_size;

    // Open ISO for reading
    iso_fd = open(iso_path, O_RDONLY);
    if (iso_fd < 0) {
        perror("open (ISO)");
        return -1;
    }

    // Open device for writing (requires root)
    dev_fd = open(device_path, O_WRONLY | O_SYNC);
    if (dev_fd < 0) {
        perror("open (device)");
        fprintf(stderr, "Error: You need root privileges to write to %s\n", device_path);
        close(iso_fd);
        return -1;
    }

    // Allocate buffer
    buffer = (unsigned char *)malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("malloc");
        close(iso_fd);
        close(dev_fd);
        return -1;
    }

    printf("\nWriting %s to %s...\n", iso_path, device_path);
    printf("This will erase all data on %s. Continue? [y/N] ", device_path);
    fflush(stdout);

    if (getchar() != 'y') {
        printf("Write cancelled.\n");
        free(buffer);
        close(iso_fd);
        close(dev_fd);
        return -1;
    }

    // Write ISO to device
    while ((bytes_read = read(iso_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(dev_fd, buffer, bytes_read);
        if (bytes_written < 0) {
            perror("write");
            fprintf(stderr, "Error writing to device\n");
            free(buffer);
            close(iso_fd);
            close(dev_fd);
            return -1;
        }

        total_written += bytes_written;
        progress = (int)((total_written * 100) / iso_size);

        if (progress_callback) {
            progress_callback(progress);
        } else {
            printf("\rProgress: %d%% (%llu / %llu bytes)", progress, total_written, iso_size);
            fflush(stdout);
        }
    }

    printf("\n\nWrite completed successfully!\n");
    printf("Total written: %.2f MB\n", (double)total_written / (1024 * 1024));

    // Sync to ensure all data is written
    sync();

    free(buffer);
    close(iso_fd);
    close(dev_fd);

    return 0;
}

/**
 * Free allocated device list
 */
void free_usb_devices(usb_device_t *devices) {
    if (devices) {
        free(devices);
    }
}