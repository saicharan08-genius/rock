#include <fs/ext2/ext2.h>
#include <fs/vfs.h>
#include <output.h>
#include <bitmap.h>

static partition_t *partitions;
static uint64_t partition_cnt = 0;

typedef struct {
    uint8_t drive_status;
    uint8_t starting_chs[3];
    uint8_t partition_type;
    uint8_t ending_chs[3];
    uint32_t starting_lba;
    uint32_t sector_cnt;
} __attribute__((packed)) mbr_partition_t;

typedef struct {
    uint64_t identifier;
    uint32_t version;
    uint32_t hdr_size;
    uint32_t checksum;
    uint32_t reserved0;
    uint64_t hdr_lba;
    uint64_t alt_hdr_lba;
    uint64_t first_block;
    uint64_t last_block;
    uint64_t guid[2];
    uint64_t guid_lba;
    uint32_t partition_ent_cnt;
    uint32_t partition_ent_size;
    uint32_t crc32_partition_array;
} __attribute__((packed)) gpt_partition_table_hdr;

static uint8_t partition_check_fs(partition_t *part) {
    part->ext2_fs = fs_check_ext2(*part);
    if(part->ext2_fs != NULL) {
        part->read = ext2_read;
        part->write = ext2_write;
        part->touch = ext2_touch;
        part->mkdir = ext2_mkdir;
        return EXT2; 
    }
    return UNKNOWN;
}

static void add_partition(partition_t *new_partition) {
    if((partition_cnt + 1) % 4 == 0) {
        partitions = krealloc(partitions, sizeof(partition_t) * (partition_cnt + 4));
    }

    partitions[partition_cnt++] = *new_partition;
}

static partition_t *find_mount_point(char *path) {
    for(uint64_t i = 0; i < partition_cnt; i++) {
        if(strncmp(partitions[i].mount_point, path, strlen(partitions[i].mount_point)) == 0)
            return &partitions[i];
    }
    return NULL;
}

static void fstab_mount(char *line) {
    char *save = line;
    char *drive_uid = strtok_r(save, " ", &save);
    char *partition_index = strtok_r(save, " ", &save);
    char *mount_point = strtok_r(save, " ", &save);

    for(uint64_t i = 0; i < partition_cnt; i++) {
        if(partitions[i].partition_index == atoi(partition_index)) { // TODO take into consdieration the device uid
            partitions[i].mount_point = mount_point;
            return;
        }
    }
}

int fs_read(char *path, uint64_t start, uint64_t cnt, void *buf) {
    partition_t *part = find_mount_point(path);
    if(part == NULL) {
        kprintf("[KDEBUG]", "Invalid mount point");
        return -1; 
    }
    return part->read(part, path, start, cnt, buf);
}

int fs_write(char *path, uint64_t start, uint64_t cnt, void *buf) {
    partition_t *part = find_mount_point(path);
    if(part == NULL) {
        kprintf("[KDEBUG]", "Invalid mount point");
        return -1; 
    }
    return part->write(part, path, start, cnt, buf);
}

int fs_mkdir(char *path, uint16_t permissions) {
    partition_t *part = find_mount_point(path);
    if(part == NULL) {
        kprintf("[KDEBUG]", "Invalid mount point");
        return -1; 
    }
    return part->mkdir(part, "/", path, permissions);
}

int fs_touch(char *path, uint16_t permissions) {
    partition_t *part = find_mount_point(path);
    if(part == NULL) {
        kprintf("[KDEBUG]", "Invalid mount point");
        return -1; 
    }

    char *buffer = kmalloc(strlen(path));
    strcpy(buffer, path);
    
    int index = find_last_char(buffer, '/');
    if(index != 0) {
        buffer[index] = '\0';
        return part->touch(part, buffer, buffer + index + 1, permissions);
    }

    return part->touch(part, "/", path, permissions);
}

void partition_mount_all() {
    char *fstab = kmalloc(0x1000);

    for(uint64_t i = 0; i < partition_cnt; i++) { 
        if(partitions[i].device_type == PRIMARY_DEVICE) {
            partitions[i].mount_point = "/";
            partitions[i].read(&partitions[i], "/fstab", 0, 0x1000, fstab);
            char *line = strtok(fstab, "\n");
            while(line != NULL) {
                fstab_mount(line); 
                line = strtok(NULL, "\n");
            }
            break; 
        }
    }

    kfree(fstab);
}

static void scan_partitions(device_t *device) {
    uint16_t mbr_signature;
    device->read(device->device_index, 510, 2, &mbr_signature);

    if(mbr_signature == 0xaa55) { // mbr partitioned drive detected
        mbr_partition_t mbr_partitions[4];
        device->read(device->device_index, 0x1be, sizeof(mbr_partitions), &mbr_partitions);

        uint8_t uid[10];
        device->read(device->device_index, 0x1b4, 10, &uid);
        memcpy8((uint8_t*)device->uid, uid, 10);

        for(uint8_t i = 0; i < 4; i++) {
            if(mbr_partitions[i].partition_type == 0) // empty partition entry
                continue;

            partition_t partition = {   .device = device,
                                        .device_offset = mbr_partitions[i].starting_lba * 0x200,
                                        .sector_size = 512,
                                        .fs_type = UNKNOWN,
                                        .mount_point = (mbr_partitions[i].partition_type & (1 << 7 )) ? "/" : NULL,
                                        .device_type = (mbr_partitions[i].partition_type & (1 << 7)) ? PRIMARY_DEVICE : SECONDARY_DEVICE,
                                        .partition_index = i
                                    };

            partition.fs_type = partition_check_fs(&partition);

            add_partition(&partition);
        }
        return; 
    }

    gpt_partition_table_hdr gpt_hdr;
    device->read(device->device_index, 512, sizeof(gpt_partition_table_hdr), &gpt_hdr);

    if(gpt_hdr.identifier == 0x4546492050415254) { // "EFI PART"
        // TODO parse gpt partition tables
        return;
    }

    kprintf("[KDEBUG]", "Device detected with no partition table");
}

void partition_read(partition_t *partition, uint64_t start, uint64_t cnt, void *ret) {
    partition->device->read(partition->device->device_index, start + partition->device_offset, cnt, ret);
}

void partition_write(partition_t *partition, uint64_t start, uint64_t cnt, void *ret) {
    partition->device->write(partition->device->device_index, start + partition->device_offset, cnt, ret);
}

void add_device(device_t *new_device) {
    device_t *device = kmalloc(sizeof(device_t));
    *device = *new_device; 
    scan_partitions(device);
}

void vfs_init() {
    partitions = kmalloc(sizeof(partition_t) * 4);
}
