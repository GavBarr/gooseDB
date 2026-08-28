#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define PAGE_SIZE 4096

typedef struct {
    char     magic[8];      // "GOOSEDB" + null terminator
    uint32_t page_size;
    uint32_t root_page;
    uint32_t version;
} DatabaseHeader;

int main(void) {
    const char* filename = "db_file";

    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // Full page-sized buffer, zero-filled — this is what actually gets
    // written to disk, so page 0 is guaranteed to be exactly PAGE_SIZE bytes.
    uint8_t page_buf[PAGE_SIZE] = {0};

    // Overlay the header struct on the front of the buffer and fill it in.
    DatabaseHeader* header = (DatabaseHeader*)page_buf;
    memset(header->magic, 0, sizeof(header->magic));
    memcpy(header->magic, "GOOSEDB", 7);   // 7 chars + 1 null byte from the memset above
    header->page_size = PAGE_SIZE;
    header->root_page = 1;
    header->version   = 1;

    ssize_t written = pwrite(fd, page_buf, PAGE_SIZE, 0);
    if (written != PAGE_SIZE) {
        perror("pwrite");
        close(fd);
        return 1;
    }

    printf("Wrote header page to %s:\n", filename);
    printf("  magic      = %s\n", header->magic);
    printf("  page_size  = %u\n", header->page_size);
    printf("  root_page  = %u\n", header->root_page);
    printf("  version    = %u\n", header->version);

    close(fd);

    // --- Read it back, from a fresh open, to prove round-tripping works ---
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open (read-back)");
        return 1;
    }

    uint8_t read_buf[PAGE_SIZE] = {0};
    ssize_t r = pread(fd, read_buf, PAGE_SIZE, 0);
    if (r != PAGE_SIZE) {
        perror("pread");
        close(fd);
        return 1;
    }

    DatabaseHeader* read_header = (DatabaseHeader*)read_buf;
    printf("\nRead back from disk:\n");
    printf("  magic      = %s\n", read_header->magic);
    printf("  page_size  = %u\n", read_header->page_size);
    printf("  root_page  = %u\n", read_header->root_page);
    printf("  version    = %u\n", read_header->version);

    close(fd);
    return 0;
}
