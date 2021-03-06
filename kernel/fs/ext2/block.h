#ifndef _EXT2_BLOCK_H_
#define _EXT2_BLOCK_H_

#include <fs/ext2/inode.h>

ext2_bgd_t ext2_read_bgd(partition_t *part, uint32_t index);

void ext2_write_bgd(partition_t *part, ext2_bgd_t bgd, uint32_t index);

uint32_t ext2_alloc_block(partition_t *part);

void ext2_free_block(partition_t *part, uint32_t block);

#endif
