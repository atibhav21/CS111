#!/usr/local/cs/bin/python3

# NAME: Atibhav Mittal
# EMAIL: atibhav.mittal6@gmail.com
# ID: 804598987

import csv
import sys
import math

freeBlocksList = {}
freeInodesList = {}
allocatedBlocksList = {}
allocatedInodesList = {}
actualReferences = {}
dirents = {}
invalidParentChildDirents = {}

total_blocks = 0
total_inodes = 0
blocksize = 0
inode_size = 0
blocks_per_group = 0
inodes_per_group = 0
first_inode = 0

def printUsage():
	print("usage: ./lab3b filename.csv", file=sys.stderr)

def parseSuperBlock(row):
	global total_blocks, total_inodes, blocksize, inode_size, blocks_per_group, inodes_per_group, first_inode

	total_blocks = int(row[1])
	total_inodes = int(row[2])
	blocksize = int(row[3])
	inode_size = int(row[4])
	blocks_per_group = int(row[5])
	inodes_per_group = int(row[6])
	first_inode = int(row[7])

def parseInode(row):
	if int(row[1]) in allocatedInodesList.keys():
		print('DUPLICATE INODE CREATED ERROR!!!')
		sys.exit(1)
	else:
		allocatedInodesList[int(row[1])] = int(row[6])
	

	for i in range(12, 24):
		if not int(row[i]) == 0:
			if int(row[i]) in allocatedBlocksList.keys():
				allocatedBlocksList[int(row[i])].append(('BLOCK', row[1], '0'))
			else:
				allocatedBlocksList[int(row[i])] = [('BLOCK', row[1], '0')]

	if not (int(row[24]) == 0):
		if int(row[24]) in allocatedBlocksList.keys():
			allocatedBlocksList[int(row[24])].append(('INDIRECT BLOCK', row[1], '12'))
		else:
			allocatedBlocksList[int(row[24])] = [('INDIRECT BLOCK', row[1], '12')]

	if not (int(row[25]) == 0):
		if int(row[25]) in allocatedBlocksList.keys():
			allocatedBlocksList[int(row[25])].append(('DOUBLE INDIRECT BLOCK', row[1], '268'))
		else:
			allocatedBlocksList[int(row[25])] = [('DOUBLE INDIRECT BLOCK', row[1], '268')]

	if not (int(row[26]) == 0):
		if int(row[26]) in allocatedBlocksList.keys():
			allocatedBlocksList[int(row[26])].append(('TRIPLE INDIRECT BLOCK', row[1], '65804'))
		else:
			allocatedBlocksList[int(row[26])] = [('TRIPLE INDIRECT BLOCK', row[1], '65804')]

def parseDirent(row):


	if(int(row[3]) in actualReferences.keys()):
		actualReferences[int(row[3])] += 1
	else:
		actualReferences[int(row[3])] = 1
	
	dirents[int(row[3])] = (row[1], row[6])

	if(row[6] == '\'.\''):
		# Reference to itself
		if(row[3] != row[1]):
			if(int(row[3]) in invalidParentChildDirents.keys()):
				invalidParentChildDirents[int(row[3])].append((row[1], row[6], row[3], row[1]))
			else:
				invalidParentChildDirents[int(row[3])] = [(row[1], row[6], row[3], row[1])]


	# TODO: Figure out how to get incorrect Parent Entries
	if(row[6] == '\'..\''):
		
		if(row[3] != row[1]):
			if(int(row[3])) in invalidParentChildDirents.keys():
				invalidParentChildDirents[int(row[3])].append((row[1], row[6], row[3], row[1]))
			else:
				invalidParentChildDirents[int(row[3])] = [(row[1], row[6], row[3], row[1])]


def returnTupleOfIndirect(row):
	if(row[2] == '1'):
		return ('INDIRECT BLOCK', row[1], row[3])
	elif(row[2] == '2'):
		return ('DOUBLE INDIRECT BLOCK', row[1], row[3])
	elif(row[2] == '3'):
		return ('TRIPLE INDIRECT BLOCK', row[1], row[3])

def parseIndirect(row):
	if int(row[-1]) in allocatedBlocksList.keys():
		allocatedBlocksList[int(row[-1])].append(returnTupleOfIndirect(row))
	else:
		allocatedBlocksList[int(row[-1])] = [returnTupleOfIndirect(row)]


def main():
	if(len(sys.argv) != 2):
		printUsage()
		sys.exit(1)
	try:
		with open(sys.argv[1], "r") as csvfile:
			reader = csv.reader(csvfile) # read the csv file
			# Extract all the information required from each entry in the CSV file
			for row in reader:
				if(row[0] == 'SUPERBLOCK'):
					parseSuperBlock(row)
				elif(row[0] == 'GROUP'):
					continue # TODO: Check if need to do anything for GROUP
				elif(row[0] == 'BFREE'):
					if int(row[1]) in freeBlocksList.keys():
						freeBlocksList[int(row[1])] += 1
					else:
						freeBlocksList[int(row[1])] = 1
				elif(row[0] == 'IFREE'):
					if int(row[1]) in freeInodesList.keys():
						freeInodesList[int(row[1])] += 1
					else:
						freeInodesList[int(row[1])] = 1
				elif(row[0] == 'INODE'):
					parseInode(row)
				elif(row[0] == 'INDIRECT'):
					parseIndirect(row)
				elif(row[0] == 'DIRENT'):
					parseDirent(row)
				'''else:
					print("Bad csv file")
					sys.exit(1)'''
			reserved_block_list = [0] # Block number 0 is always reserved for boot table and stuff

			curr_block_num = 1
			if(blocksize <= 1024):
				# Smaller block size so superblock is in block 1
				reserved_block_list.append(1)
				curr_block_num += 1
			# build up the reserved block list
			inode_table_blocks = int(math.ceil((total_inodes * inode_size) / blocksize))
			for _ in range(3 + inode_table_blocks):
				reserved_block_list.append(curr_block_num) # 1 block for block group descriptor table, 1 for block_bitmap, 1 for inode_bitmap, and inode_table_blocks for the inode_table
				curr_block_num += 1

			# Go through the allocated blocks and find invalid and reserved blocks, and duplicate blocks
			for block_num, references in allocatedBlocksList.items():
				for r in references:
					if(block_num >= total_blocks):
						print("INVALID %s %d IN INODE %s AT OFFSET %s" % (r[0], block_num, r[1], r[2]))
					if block_num in reserved_block_list:
						print("RESERVED %s %d IN INODE %s AT OFFSET %s" % (r[0], block_num, r[1], r[2]))

				if(len(references) > 1):
					for r in references:
						print("DUPLICATE %s %d IN INODE %s AT OFFSET %s" % (r[0], block_num, r[1], r[2]))

			# Go through the freeBlocksList and AllocatedBlocksList and find unreferenced and allocated blocks on the free list
			for i in range(total_blocks):
				if i in freeBlocksList.keys() or i in allocatedBlocksList.keys() or i in reserved_block_list:
					pass
				else:
					print('UNREFERENCED BLOCK %d' % i)

				if i in freeBlocksList.keys() and i in allocatedBlocksList.keys():
					print('ALLOCATED BLOCK %d ON FREELIST' %i)

			for i in range(first_inode, total_inodes + 1):
				if i in allocatedInodesList.keys() or i in freeInodesList.keys():
					pass
				else:
					print('UNALLOCATED INODE %d NOT ON FREELIST' %i)

			for key in allocatedInodesList.keys():
				if(key in freeInodesList.keys()):
					print('ALLOCATED INODE %d ON FREELIST' % key)

			for key in allocatedInodesList.keys():
				if not allocatedInodesList[key] == 0:
					if not key in actualReferences.keys():
						print('INODE %d HAS 0 LINKS BUT LINKCOUNT IS %d' % (key, allocatedInodesList[key]))
					elif not allocatedInodesList[key] == actualReferences[key]:
						print('INODE %d HAS %d LINKS BUT LINKCOUNT IS %d' % (key, actualReferences[key], allocatedInodesList[key]))


			for key in dirents.keys():
				value = dirents[key]
				if key > total_inodes or key < 1:
					print("DIRECTORY INODE %s NAME %s INVALID INODE %d" %(value[0],value[1],key))
					continue

				if not key in allocatedInodesList.keys():
					print("DIRECTORY INODE %s NAME %s UNALLOCATED INODE %d" %(value[0],value[1],key))

			for key in invalidParentChildDirents.keys():
				for value in  invalidParentChildDirents[key]:
					print('DIRECTORY INODE %s NAME %s LINK TO INODE %s SHOULD BE %s' % (value[0], value[1], value[2], value[3]))

	except IOError as e:
		print("Couldn't open file: %s" %e, file=sys.stderr)
		sys.exit(1)



if __name__ == '__main__':
	main()