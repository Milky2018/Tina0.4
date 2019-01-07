#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FILE "./image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."

#define SECTOR_SIZE 512			 
 
#define BOOTLOADER_SIG_OFFSET 0x1fe 
#define KERNEL_SECTOR_OFFSET 0x1e0 
 
void write_to_image(FILE **imagefile, FILE *readfile, Elf32_Ehdr *elf_hdr, Elf32_Phdr *prog_hdr);

Elf32_Phdr * read_exec_file(FILE **execfile, char *filename, Elf32_Ehdr **ehdr)
{
	uint8_t numread, num_phdr;
	Elf32_Phdr *phdr;	
	
	if(!(*execfile = fopen(filename, "r"))){
		fprintf(stderr, "Error reading %s: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}
	 
	numread = fread(*ehdr, 1, sizeof(Elf32_Ehdr), *execfile);
	assert(numread == sizeof(Elf32_Ehdr));
	num_phdr = (**ehdr).e_phnum;
	
	assert((**ehdr).e_phentsize == sizeof(Elf32_Phdr)); 
	fseek(*execfile, (**ehdr).e_phoff, SEEK_SET);
	phdr = calloc(sizeof(Elf32_Phdr), num_phdr);	
		 
	numread = fread(phdr, sizeof(Elf32_Phdr), num_phdr, *execfile);
	assert(numread == num_phdr);
	
	return phdr;
}


void write_bootblock(FILE **imagefile, FILE *bootfile, Elf32_Ehdr *boot_header, Elf32_Phdr *boot_phdr){
	
	fseek(*imagefile, 0, SEEK_SET);
	write_to_image(imagefile, bootfile, boot_header, boot_phdr);

	fseek(*imagefile, BOOTLOADER_SIG_OFFSET, SEEK_SET);
	fputc(0x55, *imagefile);
	fputc(0xAA, *imagefile);
}


void write_kernel(FILE **imagefile, FILE *kernelfile, Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr)
{
	fseek(*imagefile, SECTOR_SIZE, SEEK_SET);
	write_to_image(imagefile, kernelfile, kernel_header, kernel_phdr); 
}

void write_to_image(FILE **imagefile, FILE *readfile, Elf32_Ehdr *elf_hdr, Elf32_Phdr *prog_hdr) {
	uint32_t required_padding;	
	uint32_t totalread = 0;	
	uint32_t numremaining, num2read, numread;	
	int pidx;	
	char buffer[SECTOR_SIZE];	
	 
	for(pidx = 0; pidx < (*elf_hdr).e_phnum; pidx++) {
		numremaining = prog_hdr[pidx].p_filesz;
		required_padding = prog_hdr[pidx].p_memsz - numremaining;
		fseek(readfile, prog_hdr[pidx].p_offset, SEEK_SET);	
		while(numremaining > 0) {
			num2read = (numremaining < SECTOR_SIZE) ? numremaining : SECTOR_SIZE;
			numread = fread(buffer, 1, num2read, readfile);
			assert(numread == num2read);
			 
			numread = fwrite(buffer, 1, num2read, *imagefile);
			assert(numread == num2read);
			 
			totalread += numread;
			numremaining -= numread;
		}	 
		while(required_padding > 0) {
			fputc(0, *imagefile);
			required_padding--;
			totalread++;
		}
	}
	required_padding = (SECTOR_SIZE - totalread) % SECTOR_SIZE;
	while(required_padding > 0) {
		fputc(0, *imagefile);
		required_padding--;
	}
}

int count_kernel_sectors(Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr) 
{
	uint32_t kernel_size = 0;
	int pidx;
	for(pidx = 0; pidx < (*kernel_header).e_phnum; pidx++) {
		kernel_size += kernel_phdr[pidx].p_memsz;
		printf("Here is the kernel_size of i: %d of p[%d]\n", kernel_size, pidx);
	}
	return (int) kernel_size / SECTOR_SIZE + ((kernel_size % SECTOR_SIZE == 0)? 0 : 1);
}


void record_kernel_sectors(FILE **imagefile, Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr, int num_sec) 
{	
		fseek(*imagefile, KERNEL_SECTOR_OFFSET, SEEK_SET);
		fwrite(&num_sec, sizeof(int), 1, *imagefile);
}

void extended_opt(Elf32_Phdr *bph, int k_phnum, Elf32_Phdr *kph, int num_sec)
{	 
	char *boot_fname = "./bootblock";
	char *kernel_fname = "./kernel";
	int pidx;
	uint32_t sim_address;

	printf("image_size: %d sectors\n", num_sec);
	 
	pidx = 0;
	printf("0x%4x:\t%s\n", bph[pidx].p_vaddr, boot_fname);
	printf("\tsegment %d\n", pidx);
	printf("\t\t offset 0x%4x\t\tvaddr 0x%4x\n", bph[pidx].p_offset, bph[pidx].p_vaddr);
	printf("\t\t filesz 0x%4x\t\tmemsz 0x%4x\n", bph[pidx].p_filesz, bph[pidx].p_memsz);
	printf("\t\t writing 0x%4x bytes\n", bph[pidx].p_filesz);
	sim_address = ((bph[pidx].p_memsz - 1) / SECTOR_SIZE	+ 1) * SECTOR_SIZE;
	printf("\t\t padding up to 0x%4x\n", sim_address);
 
	printf("0x%4x:\t%s\n", kph[pidx].p_vaddr, kernel_fname);
	for(pidx = 0; pidx < k_phnum; pidx++) {
		printf("\tsegment %d\n", pidx);
		printf("\t\t offset 0x%4x\t\tvaddr 0x%4x\n", kph[pidx].p_offset, kph[pidx].p_vaddr);
		printf("\t\t filesz 0x%4x\t\tmemsz 0x%4x\n", kph[pidx].p_filesz, kph[pidx].p_memsz);
		printf("\t\t writing 0x%4x bytes\n", kph[pidx].p_filesz);
		sim_address += kph[pidx].p_memsz;
		if (pidx == k_phnum -1) sim_address = ((sim_address - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE;
		printf("\t\t padding up to 0x%4x\n", sim_address);
	}
	printf("os_size: %d sectors\n", num_sec-1);
}

int main(int argc, char **argv){
	FILE *kernelfile, *bootfile,*imagefile;	 
	Elf32_Ehdr *boot_header = malloc(sizeof(Elf32_Ehdr)); 
	Elf32_Ehdr *kernel_header = malloc(sizeof(Elf32_Ehdr)); 

	Elf32_Phdr *boot_phdr;	
	Elf32_Phdr *kernel_phdr;	
	int num_sectors;

	if (argc != 3 && argc !=4) {
	fprintf(stderr, "Error: call as '%s (--extended) bootblock-location kernel-location'\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if(!(imagefile = fopen(IMAGE_FILE, "w+"))) {
		fprintf(stderr, "Error creating %s: %s\n", IMAGE_FILE, strerror(errno));
		exit(EXIT_FAILURE);
	}

	boot_phdr = read_exec_file(&bootfile, argv[argc-2], &boot_header);

	write_bootblock(&imagefile, bootfile, boot_header, boot_phdr);
	kernel_phdr = read_exec_file(&kernelfile, argv[argc-1], &kernel_header);
	write_kernel(&imagefile, kernelfile, kernel_header, kernel_phdr);
	num_sectors = count_kernel_sectors(kernel_header, kernel_phdr);
	printf("1st kernel_phdr memsz: %x\n", kernel_phdr[0].p_memsz);
	record_kernel_sectors(&imagefile, kernel_header, kernel_phdr, num_sectors);
	
	if(!strncmp(argv[1], "--extended", 11)){
		extended_opt(boot_phdr, (int)kernel_header->e_phnum, kernel_phdr, num_sectors + 1);
	}

	free(boot_header);
	free(kernel_header);
	free(boot_phdr);
	free(kernel_phdr);

	fclose(kernelfile);
	fclose(bootfile);
	fclose(imagefile);
	
	return 0;
}	