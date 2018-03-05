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
#include <time.h>

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

char filetype(uint32_t filemode)
{
	uint8_t msb_second_byte = (filemode >> 12) & 0xF;
	if(msb_second_byte == 0xA)
	{
		return 's';
	}
	else if(msb_second_byte == 0x8)
	{
		return 'f';
	}
	else if(msb_second_byte == 0x4)
	{
		return 'd';
	}
	return '?';

}

int indirectBlock(int fd, uint32_t inode_num, uint32_t block_num, int level)
{
	void* block = malloc(blocksize);
	if(pread(fd, block, blocksize, blocksize * block_num) < 0)
	{
		free(block);
		return 1;
	}

	uint16_t entry_num = 0;
	uint32_t* iterator_pointer;
	uint32_t i;
	for(i = 0; i < blocksize / 4; i += 1)
	{
		// go over each of these indexed entries and if entry is non-zero, print to stdout
		iterator_pointer = block + i * 4;
		if((*iterator_pointer) != 0)
		{
			//printf("Block Num Referenced: %d\n", blocksize/4);
			printf("Non Zero something found for i=%d\n", i);
		}
	}

	free(block);
	return 0;
}

int printDirectory(int fd, uint32_t parent_inode_num, uint32_t block_num)
{
	void* block = malloc(blocksize);
	if(pread(fd, block, blocksize, blocksize * block_num) < 0)
	{
		free(block);
		return 1;
	}
	void* iterator = block;
	
	if( *((uint32_t*) iterator) == 0)
	{
		return 0; // no data in this block
	}
	uint32_t byte_offset = 0; // TODO: Check if this should be moved outside the block
	while(iterator - block < blocksize - 1) // get number of bytes difference
	{
		struct ext2_dir_entry* dir = (struct ext2_dir_entry*) iterator;
		// read the directory entries
		if(dir->inode != 0)
		{
			printf("DIRENT,%d,%d,%d,%d,%d,%s\n", parent_inode_num, byte_offset, dir->inode, dir->rec_len,dir->name_len, 
								(char*) dir->name); 
		}
		iterator += dir->rec_len;
		byte_offset += dir->rec_len;
	}
}

// only processes the first 12 blocks (which are the direct and not the indirect blocks)
int directory(int fd, uint32_t parent_inode_num, struct ext2_inode* inode_block, int level, uint32_t block_num) // last two parameters are for the indirect calls
{
	
	
	uint8_t i = 0;
	
	// TODO: Check if need to process the indirect blocks i = 13,14,15?
	if(level == 0)
	{
		for(i = 0; i < 15; i += 1)
		{	
			if(i < 12)
			{
				// print info about this block
				printDirectory(fd, parent_inode_num, inode_block->i_block[i]);
			}
			else
			{
				// print info about the block it points to
				if( i == 12)
				{
					// 1 level of indirect inode
					directory(fd, parent_inode_num, inode_block, 1, inode_block->i_block[i]);
				}
				else if(i == 13)
				{
					// 2 levels of indirect inodes
					directory(fd, parent_inode_num, inode_block, 2, inode_block->i_block[i]);
				}
				else if( i == 14)
				{
					directory(fd, parent_inode_num, inode_block, 3, inode_block->i_block[i]);
				}
			}
		}
	}
	else
	{
		// indirect blocks
		void* indirect_block = malloc(blocksize);
		void* block = malloc(blocksize);
		if(pread(fd, indirect_block, blocksize, blocksize * block_num) < 0)
		{
			free(block);
			free(indirect_block);
			return 1;
		}
		uint32_t i;
		for(i = 0; i < blocksize / 4; i += 1)
		{
			uint32_t* block_addr = indirect_block + (4 * i);
			if( *block_addr != 0)
			{
				if(pread(fd, block, blocksize, *block_addr * blocksize) < 0)
				{
					free(indirect_block);
					free(block);
					return 1;
				}
				if(level == 1)
				{
					free(indirect_block);
					free(block);
					return printDirectory(fd, parent_inode_num, *block_addr);
				}
				else
				{
					free(indirect_block);
					free(block);
					return directory(fd, parent_inode_num, inode_block, level-1, *block_addr);
				}
			}
		}

		// TODO: Free pointers
		
	}
	

	
	return 0;
}

int printInodeSummary(int fd, uint32_t first_block_inode)
{
	// read data using the struct inode, parse each of those inodes and print it to stdout
	// TODO: figure out the inode_number somehow
	uint32_t i;
	struct ext2_inode* inode_block = malloc(sizeof(struct ext2_inode));
	for(i = 0; i < inodepergroup; i+= 1)
	{
		if(pread(fd, (void*)inode_block, sizeof(struct ext2_inode), first_block_inode * blocksize + i * sizeof(struct ext2_inode) ) < 0)
		{
			free(inode_block);
			return 1;
		}
		if(inode_block->i_mode != 0 && inode_block->i_links_count != 0)
		{

			// print the INODE summary
			char buffer[80];
			char c = filetype(inode_block->i_mode); // get the file types
			uint16_t low_order_bits = inode_block->i_mode & 0xFFF; // get lower order 16 bits

			// TODO: Check if need to remove 0 for low_order_bits
			printf("INODE,%d,%c,%o,%d,%d,%d,", i+1, c, low_order_bits, inode_block->i_uid, inode_block->i_gid, 
					inode_block->i_links_count);
			// print the dates stuff

			// Updated spec use creation time
			time_t change_time = inode_block->i_ctime;
			strftime(buffer, 80, "%D %r", gmtime(&change_time));
			printf("%s,", buffer);

			// print the last modified time
			time_t modified_time = inode_block->i_mtime;
			strftime(buffer, 80, "%D %r", gmtime(&modified_time));
			printf("%s,", buffer);

			time_t access_time = inode_block->i_atime;
			strftime(buffer, 80, "%D %r", gmtime(&access_time));
			printf("%s,", buffer);

			// print file size
			printf("%d,", inode_block->i_size);

			// print number of blocks
			printf("%d", inode_block->i_blocks);

			// TODO: Print out the last 15 fields of the summary
			uint8_t j = 0;
			for(j = 0; j < 15; j+= 1)
			{
				printf(",%d", inode_block->i_block[j]);
			}

			// Done printing this Inode
			printf("\n");

			// print out DIRENT
			if(c == 'd')
			{
				if(directory(fd, i+1, inode_block, 0, 0) != 0)
				{
					free(inode_block);
					return 1;
				}
			}
		}

	}

	free(inode_block);
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
		if(printInodeSummary(fd, first_block_inode) != 0)
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