#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "ext2_fs.h"
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

extern int errno;

uint32_t numofblock = 0;
uint32_t blocksize = 0;
uint32_t blockpergroup = 0;
uint32_t numofgroup = 0; // TODO: Figure out where to get this
uint32_t inodepergroup = 0;
uint16_t inode_size = 0;

uint32_t inode_count = 0; // TODO: Check if this can be moved to superblock function
uint32_t first_ino = 0; // TODO: Check if this can be moved to superblock function

void printUsage()
{
	fprintf(stderr, "usage: ./lab3a filename.img\n");
}

void printErrorAndExit()
{
	fprintf(stderr, "An Error occured: %d\n", strerror(errno));
	exit(2);
}

void superBlock(int fd)
{
	struct ext2_super_block* superblock_pointer = malloc(sizeof(struct ext2_super_block));

	// read the ext2_super_block from fd 
	if(pread(fd, (void*) superblock_pointer, sizeof(struct ext2_super_block), 1024) < 0) // read the superblock from the fd
	{
		free(superblock_pointer);
		printErrorAndExit();
	}

	numofblock = superblock_pointer->s_blocks_count;
	blocksize = EXT2_MIN_BLOCK_SIZE << superblock_pointer->s_log_block_size;
	blockpergroup = superblock_pointer->s_blocks_per_group;
	inodepergroup = superblock_pointer->s_inodes_per_group;
	inode_size = superblock_pointer->s_inode_size;
	inode_count = superblock_pointer->s_inodes_count;
	first_ino = superblock_pointer->s_first_ino;

	// write the summary to STDOUT
	printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", numofblock, inode_count, blocksize, inode_size, blockpergroup, inodepergroup, first_ino);

	free(superblock_pointer);
}

int printFreeBlocks(int fd, uint32_t block_bitmap, uint32_t blocks_in_group, uint32_t group_num)
{
	void* m_block = malloc(blocksize);
	if(pread(fd, m_block, blocksize, block_bitmap * blocksize) < 0)
	{
		free(m_block);
		return 1;
	}
	uint32_t bit_num;
	for(bit_num = 0; bit_num < blocks_in_group; bit_num += 1)
	{
		uint8_t byte_in_bitmap = ((uint8_t *) m_block)[bit_num/8];
		if( ! (byte_in_bitmap & (1 << (bit_num % 8))))
		{
			printf("BFREE,%d\n", blockpergroup * group_num + bit_num + 1);
		}
	}

	free(m_block);
	return 0; // no errors occured
}

int printFreeInodes(int fd, uint32_t inode_bitmap, uint32_t group_num)
{
	void* m_block = malloc(blocksize);
	if(pread(fd, m_block, blocksize, blocksize * inode_bitmap) < 0)
	{
		free(m_block);
		return 1;
	}

	uint32_t bit_num;
	for(bit_num = 0; bit_num < inodepergroup; bit_num += 1)
	{
		uint8_t byte_in_bitmap = ((uint8_t *) m_block)[bit_num/8];
		if( ! (byte_in_bitmap & (1 << (bit_num % 8))))
		{
			printf("IFREE,%d\n", inodepergroup * group_num + bit_num + 1);
		}
	}

	free(m_block);
	return 0;
}

void groupSummary(int fd)
{
	//fprintf(stderr, "%d\n", sizeof(struct ext2_group_desc));
	struct ext2_group_desc* group_desc = malloc(sizeof(struct ext2_group_desc));
	uint32_t descriptor_blk_offset;

	numofgroup = numofblock / blockpergroup;
	if(blocksize == 1024)
	{
		descriptor_blk_offset = 2; // 3rd block on a 1 KiB block file system
	}
	else
	{
		descriptor_blk_offset = 1; // 2nd block for 2 KiB and larger block file systems
	}

	if(numofblock % blockpergroup != 0)
	{
		// additional group at the end with less than blockpergroup blocks
		numofgroup += 1;
	}

	uint32_t i;
	for(i = 0; i < numofgroup; i+= 1)
	{	
		uint32_t blocks_in_group = blockpergroup;

		// use pread with the group_desc structure to read the info about the group descriptor table
		if(pread(fd, (void*) group_desc, sizeof(struct ext2_group_desc), descriptor_blk_offset * blocksize + sizeof(struct ext2_group_desc) * i ) < 0)
		{
			free(group_desc);
			printErrorAndExit();
		}

		if(i == numofgroup - 1)
		{
			// last block so it might have lesser number of blocks
			blocks_in_group = numofblock - (blockpergroup * i);

		}
		// get information from the group_desc about this group
		uint16_t num_free_blocks = group_desc->bg_free_blocks_count;
		uint16_t num_free_inodes = group_desc->bg_free_inodes_count;
		uint32_t free_block_bitmap = group_desc->bg_block_bitmap; // TODO: Figure out why this works
		uint32_t free_inode_bitmap = group_desc->bg_inode_bitmap; // TODO: Figure out why this works
		uint32_t first_block_inode = group_desc->bg_inode_table;


		// write the group summary to stdout
		printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", i, blocks_in_group, inodepergroup, num_free_blocks, num_free_inodes, free_block_bitmap, free_inode_bitmap, first_block_inode);

		// do something with the free blocks and stuff
		if(printFreeBlocks(fd, free_block_bitmap, blocks_in_group, i) != 0)
		{
			free(group_desc);
			printErrorAndExit();
		}

		if(printFreeInodes(fd, free_inode_bitmap, i) != 0)
		{
			free(group_desc);
			printErrorAndExit();
		}
	}

	
	free(group_desc);
}

int main(int argc, char *argv[])
{
	/* code */
	if(argc != 2)
	{
		printUsage();
		return 1;
	}

	int img_fd = open(argv[1], O_RDONLY); // open img_fd file
	if(img_fd < 0)
	{
		fprintf(stderr, "Error\n");
	}

	superBlock(img_fd);
	groupSummary(img_fd);
	return 0;
}