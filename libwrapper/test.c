#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
	FILE *fp;
	char file[] = "foobar";
	struct stat st;

	if ((fp = fopen(file, "a")) == NULL) {
		perror("fopen");
		return EXIT_FAILURE;
	}
	fprintf(fp, "Hello World\n");
	fflush(fp);
	fclose(fp);

	printf("Waiting 2 seconds...\n");
	sleep(2);

	if (unlink(file) < 0) {
		perror("unlink");
	}

	if (stat(file, &st) < 0) {
		printf("File no longer exists.\n");
		return EXIT_FAILURE;
	} else {
		printf("File is still here!\n");
	}

	return EXIT_SUCCESS;
}
