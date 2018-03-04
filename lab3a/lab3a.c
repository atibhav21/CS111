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

	return 0;
}