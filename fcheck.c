
// *
// ------------------------------------------------------------------------------------------------------------------------------

// CS 5348 Operating Systems (Project 4 - File System Checker)
// Instructor: Prof. Sridhar Alagar

// ------------------------------------------------------------------------------------------------------------------------------

// Team member contributions (though all of us worked together for all the modules, main contributions have been listed below): 

// 1) Yaswanth Adari
//    NET ID: YXA220006

// 2) Sai Pavan Goud Addla
//    NET ID: SXA210294

// ------------------------------------------------------------------------------------------------------------------------------

// How to execute the code? (csgrads1 server)

// 1. Compilation : gcc fcheck.c –o fcheck -Wall -Werror -O
// 2. Running the code : ./fcheck argv[1]

// ------------------------------------------------------------------------------------------------------------------------------

//Including Libraries 
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

//Including definitions
#include "types.h"
#include "fs.h"

//global variable
char* addr; 	              //starting address of the fs image

//struct definition
typedef struct fsimage_t
{			
	char* SuperBlockAddr; 	       //starting address of the super block	
	char* StartAddrInode; 	      //starting address of the inode block
	char* StartAddrBitmap; 	     //starting address of the bitmap block
	char* StartAddrDataBlock;	//starting address of the data block
	uint InodeBlocks; 		   //total number of the inode blocks 
	uint DataBitmapBlocks;	  //total number of the data bitmap blocks
	uint dataBlocks;	     //total number of the data blocks
} image_t;		
 
/************************ STRUCT DEFINITION ***************************/
struct superblock* sb;		

/************************ DEFINTION OF BLOCK SIZE *********************/
#define BLOCK_SIZE (BSIZE)	

/*************** FUNCTION TO CHECK THE BLOCK SIZE AND RT PER STRUCT **************/

int MaxSize1()
{
   if(BLOCK_SIZE > 0) //checking the block size greather than 0 if yes, it will divide it as per the struct size
   {
		int t = BLOCK_SIZE / (sizeof(struct dirent));
		return t;  
   }
   else
   {
		return 0;
   }

   //return 0;
}

/****************** checking the exact place of the blocks ********************/
//passing the image of the argument file
//returns the exact place of the blocks
//we are leaving the places, block 0 is usused and block 1 for superblock and other blocks are inode blocks and databitmap blocks
int Agg(image_t *image)
{
	if(BLOCK_SIZE > 0)
	{
		return 1 + 1 + image->InodeBlocks + image->DataBitmapBlocks;
	}
	else
	{
		return -1;
	}
}

/********************** PRINT FUNCTION is too print the error cases based on their ID. ***********************************/
void PrintFunction(int data) // here we are passing the integer based on the error case
{
	switch(data)
	{
		case 0: fprintf(stderr,"image not found.\n");    //if the argument passed is not found
				break;
		case 1: fprintf(stderr, "ERROR: bad inode.\n");  //if it has an bad inode, if the inode doesn't has the type
				break;
		case 2: fprintf(stderr, "ERROR: bad direct address in inode.\n");  //if the inode has the bad direct address
				break;
		case 3: fprintf(stderr, "ERROR: bad indirect address in inode.\n"); //if the inode has the bad indirect address
				break;
		case 4: fprintf(stderr, "ERROR: directory not properly formatted.\n"); //if the directory is not properly formatted
				break;
		case 5: fprintf(stderr, "ERROR: root directory does not exist.\n"); //if the root directory of the file, doesnt exist
				break;
		case 6: fprintf(stderr, "ERROR: address used by inode but marked free in bitmap.\n"); //Error incase if the bitmap is not updated
				break;
		case 7: fprintf(stderr, "ERROR: bitmap marks block in use but it is not in use.\n"); //Error if the bitmap is not updated
				break;
		case 8: fprintf(stderr, "ERROR: direct address used more than once.\n"); //checking the redudancy of the direct address block
				break;
		case 9:fprintf(stderr, "ERROR: indirect address used more than once.\n"); //checking the redudancy of the indirect address block
				break;
	    case 10:fprintf(stderr, "ERROR: indirect address used more than once.\n"); //checking the redudancy of the indirect address block
		        break;
		case 11:fprintf(stderr, "ERROR: inode marked use but not found in a directory.\n"); //error incase file system is not updated
				break;
		case 12:fprintf(stderr, "ERROR: inode referred to in directory but marked free.\n"); //error incase file system is not updated
				break;
		case 13:fprintf(stderr, "ERROR: directory appears more than once in file system.\n"); //checking the redudancy of the directory
				break;  
		case 14:fprintf(stderr, "ERROR: bad reference count for file.\n"); //checking if the reference count of the file is not valid
		break;       
		case 15:fprintf(stderr, "ERROR: directory appears more than once in file system.\n"); //checking the redudancy of the directory
		break;        
	}
}

/*************CHECKING THE RULE1 *****************************************/

void rule1_badinode_check(image_t *image)
{
	int temp;
	
	//declaring the inode struct using deferencing the start address of the inode
	struct dinode* inode = (struct dinode *) image->StartAddrInode;
	for(temp = 0; temp < image->InodeBlocks; temp++, inode++)
	{
		if(inode->type != 0 && inode->type != T_DIR)  //validating the type of the inode, if it falls under T_DIR, T_DEV, T_FILE, if not we are returnign an error
		{
			if(inode->type != T_DEV && inode->type != T_FILE)
			{
				PrintFunction(1);
				exit(1);
			}
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}
	}

}

/******************** RULE2 ************************/

void rule2_bad_directaddress_check(image_t *image) //first we are checking the direct blocks errors based on the block index
{
	int temp1; //temp1, temp2 declarations for iterating the loops
	int temp2;
	uint BlockIndex;

	//defining a struct dinode object to access the address of the inode
	struct dinode* inode = (struct dinode *) image->StartAddrInode;
    
	//intialsing we are iterating the whole inode blocks
	for(temp1 = 0; temp1 < image->InodeBlocks; temp1++, inode++)
	{
        //then we are checking the direct blocks 
		for(temp2 = 0; temp2 < NDIRECT; temp2++)
		{
			BlockIndex = inode->addrs[temp2];
 
			if(BlockIndex < 0)  //checking if the block index is less than zero, then we are printing the error
			{
				PrintFunction(2);
				exit(1);
			} 
			else if(BlockIndex > (sb->size))  //checking if the block index greater than teh superblock size,then we are returning an error
			{
				PrintFunction(2);
				exit(1);
			}
			else
			{
				continue;
			}
		}
	}
	return;
}
/******************** RULE2 ************************/
void rule2_bad_indirectaddress_check(image_t *image)
{
	int temp2; //temp1, temp2 declarations for iterating the loops
	int temp1;

	//defining a struct dinode object to access the address of the inode
	struct dinode* inode = (struct dinode *) image->StartAddrInode;

    //intialsing we are iterating the whole inode blocks
    for(temp1 = 0; temp1 < image->InodeBlocks; temp1++, inode++)
	{
		uint indirectBlockNo = inode->addrs[NDIRECT];

        if(indirectBlockNo > 0 || indirectBlockNo < sb->size) //checking if the indirect block falls under the testing conditions
        {
            uint* indirectEntry = (uint* ) (addr + indirectBlockNo * BLOCK_SIZE); //intialsing the indirect entry of the block

            for(temp2 = 0; temp2 < NINDIRECT; temp2++, indirectEntry++)   ////then we are checking the indirect blocks
            {
                if(*indirectEntry >= 0 && *indirectEntry < sb->size)
                {
					//if the indirect entry is less than the superblock size we are continue the loop
                    continue;
                } 
                else
                {
                    PrintFunction(3);
                    exit(1);
                }
            }
        }

		if(indirectBlockNo < 0 || indirectBlockNo >= sb->size)
		{
			PrintFunction(3);
			exit(1);
		}
        else
        {
            continue;
        }
	}
}

/************************* RULE-3 ******************************/

void rule3_bad_root(image_t *image) //passing the object of the image struct
{
	struct dinode* Inode1 = (struct dinode*)(image->StartAddrInode); //declaring the struct object with reference to teh starting address of the inode

	Inode1 =  Inode1 + 1;

	if(Inode1->type == T_DIR) //checking if the inode type is directory or not
    {

        int check1 = 0;
        int check2 = 0;
        int maxDir = MaxSize1(); //declaring the block size based on the directory
        int temp2;  //iterating the function through temp2
		char* a = "."; //for string comparsing of the path we are declaring two characters '.', '..'
		char* b = "..";

        for(temp2 = 0; temp2 < NDIRECT; temp2++) //iterating the direct blocks to find the block number and if rootdirectory is valid or not
        {

            uint InodeBlockNumber1 = Inode1->addrs[temp2];
            
			//if the inode block number is zero then we are breaking from the function
            if(InodeBlockNumber1 == 0)
			{
				break;
			}

			else
			{
				//else we are iterating through the function to check the conditions
				struct dirent* Inode2 = (struct dirent*) (addr + InodeBlockNumber1 * BLOCK_SIZE);
				int temp1;

				for(temp1 = 0; temp1 < maxDir; temp1++, Inode2++) //iterating through the loop based on the max directory size
				{
					if(strcmp(a, Inode2->name) == 0) //if the strcmps between . and inode name returns that we are checking the error
					{
						if(Inode2->inum == 1) 
						{
							check1 = 1;
						}
					}

					if(strcmp(b, Inode2->name) == 0) //if the strcmps between .. and inode name returns that we are checking the error
					{
						if(Inode2->inum == 1)
						{
							check2 = 1;
						}
					}

					if(check1 && check2)  //we are return the function incase of the test case
					{
						return;
					}
				}
			}
        }

        PrintFunction(5);
        exit(1);

	}
    else
    {
        PrintFunction(5);
		exit(1);
    }
}
/************************* RULE-3 ******************************/

void rule4_formatcheck(image_t *image) //accessing the image struct using image
{
	uint inodeNumber = 0;

	//declaring the inode using struct which is used to access the starting access of the inode to keep trace of the inodes/blocks
	struct dinode* inode = (struct dinode*)(image->StartAddrInode);

	inode = inode + 1;
	inodeNumber = inodeNumber + 1;

	int temp1;
	//int temp2;
	int temp3;
	int check1; 
	int check2;
	struct dirent* inode4; 
	int maxDir = MaxSize1();  //declaring the block size based on the directory

    //iterating the direct blocks to find the block number and if rootdirectory is valid or not
	for(temp1 = 0; temp1 < image->InodeBlocks; temp1++, inode++, inodeNumber++)
	{
		if(inode->type != 1)
		{
			continue;
		}
		else
		{
			check1 = 0; 
			check2 = 0;
			char* a = ".";
			char* b = "..";
			int temp2 = 0;
			while(temp2 < NDIRECT-1) //iterating throughout the NDIRECT blocks for checking the error
			{
				uint inodeBlockNumber = inode->addrs[temp2];
				if(inodeBlockNumber == 0) break;
				inode4 = (struct dirent*) (addr + inodeBlockNumber * BLOCK_SIZE);
 
				for(temp3 = 0; temp3 < maxDir; temp3++, inode4++) //iterating throughout the directory max and then checking the test cases
				{

					if(strcmp(a, inode4->name)== 0 && inode4->inum == inodeNumber) //comparing the strings based on inodes with the pointer ".", ".."
					{
						check1 = 1;
					}

					if(strcmp(b, inode4->name) == 0)
					{
						check2 = 1;      //we are return the function incase of the test case
					}

					if(check1 && check2) 
					{
						break;
					}
				}

				if(check1 && check2) break;
				temp2++;
			}

			if(!check1 || !check2)
			{
				PrintFunction(4);
				exit(1);
			}
		}
	}

	return;
}
int func(image_t *image, uint offset)  //this function returns the starting address of the bitmap based on teh assumption that we are considering the bitmap size as 8
{
	int temp5;
	temp5 = ((*(image->StartAddrBitmap + offset / 8) >> (offset % 8)) & 1);
	return temp5;                     //returns the temp5 to the bitmap function 
}


/************************* RULE- 5 ***************************/
void rule5_bitmapcheck(image_t *image)  //accessing the image struct using image
{
	uint inodeNumber = 0;

	//declaring the inode using struct which is used to access the starting access of the inode to keep trace of the inodes/blocks
	struct dinode* inode = (struct dinode*)(image->StartAddrInode);
	inode = inode + 1;
	inodeNumber = inodeNumber + 1;
	int temp1;
	int temp2;

    //iterating through the entire inode block to check if the bitmap is updating are not
	for(temp1 = 0; temp1 < image->InodeBlocks; temp1++, inode++, inodeNumber++)
	{
		if(inode->type != 0) //when inode type is not zero we are iterating the function
		{
			for(temp2 = 0; temp2 < NDIRECT+1; temp2++) //we are iterating through the entire DIRECT Blocks to check the error case
			{
				if(inode->addrs[temp2] == 0)
				{
					break;
				}
				else
				{
					uint offset = inode->addrs[temp2];
					if(!(func(image, offset))) //passing the arguments to teh function which returns offset based on the bitmap size
					{				
						PrintFunction(6);
						exit(1);
					}
				}

			}
 
			if(inode->addrs[NDIRECT] != 0) //if the inode address of the ndirect is not equal to zero
			{
				uint* indirectEntry = (uint* ) (addr + (inode->addrs[NDIRECT])*BLOCK_SIZE); //we are intalising an entry to iterate or to access throughout the indirect blocks

				for(temp2 = 0; temp2 < NINDIRECT; temp2++, indirectEntry++) //iterating throughout the NDIRECT Blocks to check if the entries are updated or not in the bitmap
				{
					uint offset = *indirectEntry;
					if(!(func(image, offset))) //based on the offset we are traversing the loop
					{				
						PrintFunction(6);
						exit(1);
					}
				}
			}
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}
		
	}

	return;
}

/************************* RULE- 6 ***************************/
void rule6_bitmapcheck2(image_t *image) //we are accessing the image_t struct using image block
{

    int BlockArry[sb->nblocks];  //intially we are intialsing the block array using superblock as reference
    int k = sb->nblocks*sizeof(int); //then dynamically allocating the memory based on the size of the arrayss in the super blocks
    memset(BlockArry, 0, k);  //setting the space occuped to 0 and updating the variables

    int temp;
    int firstDataBlock = Agg(image); //we are using AGG function to validate teh superblock and return the checkpoint based on  that

    //here we are accessing the dinode using inode block
	struct dinode* inode = (struct dinode*)(image->StartAddrInode);

	uint temp1;
	uint temp2;

	//iterating throughout the loop to find the blocks updated in teh superblocks usign n->inodes function
	for(temp1 = 0; temp1 < sb->ninodes; temp1++, inode++)
	{
		if(inode->type == 0)  //if inode type is zero, we are skipping the function
		{
			continue;
		}
		else
		{
			for(temp2 = 0; temp2 < NDIRECT; temp2++) //iterating throughout the function to check if the direct blocks accessing the bitmap
			{
				temp = inode->addrs[temp2];
				if(temp != 0) 
				{
					BlockArry[temp - firstDataBlock] = 1; //updating the blockarray to keep trace of the bitmap
				}
				else
				{
					continue;
				}
			}

			temp = inode->addrs[NDIRECT];
			if(temp == 0) continue;
			BlockArry[temp - firstDataBlock] = 1;
            

			//manually declaring an entry to access the indirect block based on teh block
			uint* indirectEntry = (uint* ) (addr + (temp)*BLOCK_SIZE);
			for(temp2 = 0; temp2 < NINDIRECT; temp2++, indirectEntry++)
			{
				temp = *indirectEntry;
				if(temp != 0)
				{ 
					BlockArry[temp - firstDataBlock] = 1;
				}
				else
				{
					continue;
				}
			}
		}
	}

	for(temp1 = 0; temp1 < sb->nblocks; temp1++)
	{
		temp2 = (uint)(temp1 + firstDataBlock);
		if(  ((*(image->StartAddrBitmap + temp2 / 8) >> (temp2 % 8))&1)   && BlockArry[temp1] == 0 )
		{
			PrintFunction(7);
			exit(1);
		}
	}

	return;
}

/************************* RULE- 7 ***************************/
void rule7_DirectAddrcheck(image_t *image)
{

	int directBlocks[sb->nblocks];       //declaring the direct block size as the superblock indication, or by referencing to the superblock
    memset(directBlocks, 0, sizeof(int)* sb->nblocks);
	
	//manually declaring the inode struct using dinode so that we can start declaring it based on its inode start address
    struct dinode* inode = (struct dinode*)(image->StartAddrInode);
    int temp;

	//the datablock before is validated based on teh number of blocks already occupied
	int firstDataBlock = Agg(image); //returns the value after the block 0 + superblock + no.of inodes + no.of bitmap blocks
	uint temp1;
	uint temp2;

	for(temp1 = 0; temp1 < sb->ninodes; temp1++, inode++)
	{
		if(inode->type != 0)
		{
			for(temp2 = 0; temp2 < NDIRECT; temp2++)
			{
				temp = inode->addrs[temp2];
				if(temp != 0) 
				{
					if(directBlocks[temp-firstDataBlock])
					{
						PrintFunction(8);
						exit(1);
					}
					
					directBlocks[temp-firstDataBlock] = 1;
				}
				else
				{
     				continue;
				}
			}


			temp = inode->addrs[NDIRECT];
			if(temp == 0) continue;

			if(directBlocks[temp-firstDataBlock])
			{
				PrintFunction(8);  //passing the error argument to the print function to print the error corresponding to that
				exit(1);
			}

			directBlocks[temp-firstDataBlock] = 1;
		}
		else
		{
			continue;
		}
	}

	return;
}

/************************* RULE- 7 ***************************/
void rule8_IndirectAddrCheck(image_t *image)  //referencing the struct image_t using image object
{
	int indirectBlocks[sb->nblocks];         //declaring the direct block size as the superblock indication, or by referencing to the superblock
    memset(indirectBlocks, 0, sizeof(int)* sb->nblocks);   //manually resetting the value of the indirectblocks using memset function

	//using struct we are using dinode to allocate to object that address the starting address of the inode
    struct dinode* inode = (struct dinode*)(image->StartAddrInode);
    int temp;
	int firstDataBlock = Agg(image);  //returns the value after the block 0 + superblock + no.of inodes + no.of bitmap blocks
	uint temp1;
	uint temp2;

	for(temp1 = 0; temp1 < sb->ninodes; temp1++, inode++)
	{
		if(inode->type != 0) //iterating the superblock inodes to check if we have an inode type for that function
		{
			temp = inode->addrs[NDIRECT];
			uint* indirectEntry = (uint* ) (addr + (temp)*BLOCK_SIZE);
			for(temp2 = 0; temp2 < NINDIRECT; temp2++, indirectEntry++)
			{
				temp = *indirectEntry;
				if(temp == 0) continue;

				if(indirectBlocks[temp-firstDataBlock])
				{
					PrintFunction(10);
					exit(1);
				}
	
				indirectBlocks[temp-firstDataBlock] = 1;
			}
		}
		else
		{
			continue;
		}


	}

	return;
	
}

//this function recursively calls the entire directories to check if we have any errors associated with it or not
void bridge(image_t *image, struct dinode* inode, int* InodeTracker)
{

	if(inode->type == T_DIR) //if the inode type is directories
	{
		int temp1;
		int temp2;
		int temp3;
		struct dirent* mk;
		char* a  =".";
		char* b  ="..";
		struct dinode* Inode5;

		for(temp1 = 0; temp1 < NDIRECT; temp1++)
		{
			if(inode->addrs[temp1] != 0) 
			{
				mk = (struct dirent*)(addr + (inode->addrs[temp1]) * BLOCK_SIZE);
				for(temp2 = 0; temp2 < DPB; temp2++, mk++)
				{
					if(mk->inum != 0 )
					{
						if(strcmp(a, mk->name)!=0)
						{
							if(strcmp(b, mk->name)!=0)
							{
								InodeTracker[mk->inum] = InodeTracker[mk->inum] + 1;
								Inode5 = ((struct dinode*)(image->StartAddrInode)) + mk->inum;
								bridge(image,Inode5, InodeTracker);
							}
						}
					}
				}
			}
			else
			{
               continue;
			}
		}

		if(inode->addrs[NDIRECT] != 0)
		{
			uint* indirectEntry = (uint* )(addr + (inode->addrs[NDIRECT]) * BLOCK_SIZE);
			for(temp1 = 0; temp1 < NINDIRECT; temp1++, indirectEntry++)
			{
				temp3 = *indirectEntry;
				if(temp3 != 0) 
				{
					mk = (struct dirent*)(addr + temp3 * BLOCK_SIZE);

					for(temp2 = 0; temp2 < DPB; temp2++, mk++)
					{
						if(mk->inum != 0)
						{
							if(strcmp(a, mk->name)!=0)
							{
								if(strcmp(b, mk->name)!=0)
								{
									InodeTracker[mk->inum] = InodeTracker[mk->inum] + 1;
									Inode5 = ((struct dinode*)(image->StartAddrInode)) + mk->inum;
									bridge(image, Inode5, InodeTracker);
								}
							}
						}
					}
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			return;
		}
	}
    else
	{
        return;
	}

	return;
}	

/************************* RULE- 9 ***************************/
void rule9_markings(image_t *image)
{

	int trackInodes[sb->ninodes];
    memset(trackInodes, 0, sizeof(int)* sb->ninodes);
    struct dinode* rootInode = (struct dinode*)image->StartAddrInode;
    rootInode = rootInode + 1;
    trackInodes[1] = 1;
    bridge(image, rootInode, trackInodes);
    struct dinode* inode = rootInode;
    int i;
    for(i = 1; i < sb->ninodes; i++, inode++)
	{

    	if(inode->type != 0)
		{
			if(trackInodes[i] == 0)
			{
				PrintFunction(11);
				exit(1);
			}
    	}
		
    	if(inode->type == T_DIR)
		{
			if( trackInodes[i] > 1)
			{
				PrintFunction(13);
				exit(1);
			}
    	}
    }

    return;
}

/************************* RULE- 10 ***************************/
void rule10_markings2(image_t *image)
{
	int trackInodes[sb->ninodes];
    memset(trackInodes, 0, sizeof(int)* sb->ninodes);
    struct dinode* rootInode = (struct dinode*)image->StartAddrInode;
    rootInode = rootInode + 1;
    trackInodes[1] = 1;
    bridge(image, rootInode, trackInodes);
    struct dinode* inode = rootInode;
    int temp;

	for(temp = 1; temp < sb->ninodes; temp++, inode++)
	{
		if(trackInodes[temp] > 0)
		{
			if(inode->type == 0)
			{
				PrintFunction(12);
				exit(1);
			}
    	}
	}

	return;

}

/************************* RULE- 11 ***************************/
void rule11_reference_counts1(image_t *image)
{
	int trackInodes[sb->ninodes];
	int temp1;
    memset(trackInodes, 0, sizeof(int)* sb->ninodes);
    struct dinode* rootInode = (struct dinode*)image->StartAddrInode;
    rootInode = rootInode + 1;
    trackInodes[1] = 1;
    bridge(image, rootInode, trackInodes);
    struct dinode* inode = rootInode;

	for(temp1 = 1; temp1 < sb->ninodes; temp1++, inode++)
	{
		if(inode->type == T_FILE)
		{
			if(trackInodes[temp1] != inode->nlink)
			{
				PrintFunction(14);
				exit(1);
			}
    	}
	}

	return;

}

/************************* RULE- 12 ***************************/
void rule12_directory_redudance(image_t *image)
{
	int trackInodes[sb->ninodes];
    memset(trackInodes, 0, sizeof(int)* sb->ninodes);
    struct dinode* rootInode = (struct dinode*)image->StartAddrInode;
    rootInode = rootInode + 1;
    trackInodes[1] = 1;
    bridge(image, rootInode, trackInodes);
    struct dinode* inode = rootInode;
    int i;

	for(i = 1; i < sb->ninodes; i++, inode++)
	{
    	if(inode->type == T_DIR)
		{
			if(trackInodes[i]>1)
			{
				PrintFunction(15);
				exit(1);
			}
    	}
	}

	return;

}
//passing the argv to this function to check errors and intialse the filesystem chcker
int file_system_check(char *file)
{
	int fd = open(file, O_RDONLY, 0); //opening the file descriptor
	struct stat fstat1;	
	image_t image;  //declaring an object to refer image struct

	if(fd < 0){
		perror(file);
		exit(1);
	}

	if(fstat(fd, &fstat1) < 0)
	{
		exit(1);
	}

	addr = mmap(NULL, fstat1.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap failed");
		exit(1);
	}
    
	//intialising the file system parameters
	sb = (struct superblock *) (addr + 1 * BLOCK_SIZE);
	image.InodeBlocks =  (sb->ninodes / (IPB)) + 1;
	image.DataBitmapBlocks = (sb->size / (BLOCK_SIZE * 8)) + 1;
	image.dataBlocks =  sb->nblocks;
	image.StartAddrInode = addr + (BLOCK_SIZE * 2);
	image.StartAddrBitmap = addr + (BLOCK_SIZE * (2 + image.InodeBlocks));
	image.StartAddrDataBlock  = addr + (BLOCK_SIZE * (2 + image.InodeBlocks + image.DataBitmapBlocks));
	assert(2 + image.InodeBlocks + image.DataBitmapBlocks + image.dataBlocks == sb->size);

    //testing test cases based on the rules
	rule1_badinode_check(&image);
	rule2_bad_directaddress_check(&image);
	rule2_bad_indirectaddress_check(&image);
	rule3_bad_root(&image);
	rule4_formatcheck(&image);
	rule5_bitmapcheck(&image);
	rule6_bitmapcheck2(&image);
	rule7_DirectAddrcheck(&image);
	rule8_IndirectAddrCheck(&image);
	rule9_markings(&image);
	rule10_markings2(&image);
	rule11_reference_counts1(&image);
	rule12_directory_redudance(&image);
	close(fd);

	return 0;
}
int main(int argc, char* argv[])
{
    //intially we are checking if more/less than 2 parameters in teh used defined arguments
	if(argc != 2 )
	{
		fprintf(stderr, "Usage: fcheck <file_system_image>\n");
		exit(1);
	} 

	return file_system_check(argv[1]);  //returning fd using;
}
