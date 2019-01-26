
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

#define MAX_BUF 500
char teststr[] = "The quick brown fox jumped over the lazy dog.";
char buf[MAX_BUF];
int
main(int argc, char * argv[])
{
  int fd, r, i, j , k;
//  char * p;
  (void) argc;
  (void) argv;

  printf("\n**********\n* File Tester\n");
  
  snprintf(buf, MAX_BUF, "**********\n* write() works for stdout\n");
  write(1, buf, strlen(buf));
  snprintf(buf, MAX_BUF, "**********\n* write() works for stderr\n");
  write(2, buf, strlen(buf));

  printf("**********\n* opening new file \"test.file\"\n");
  fd = open("test.file", O_RDWR | O_CREAT );
  printf("* open() got fd %d\n", fd);
  if (fd < 0) {
    printf("ERROR opening file: %s\n", strerror(errno));
    exit(1);
  }

  printf("* writing test string\n");  
  r = write(fd, teststr, strlen(teststr));
  printf("* wrote %d bytes\n", r);
  if (r < 0) {
    printf("ERROR writing file: %s\n", strerror(errno));
    exit(1);
  }

  printf("* writing test string again\n");  
  r = write(fd, teststr, strlen(teststr));
  printf("* wrote %d bytes\n", r);
  if (r < 0) {
    printf("ERROR writing file: %s\n", strerror(errno));
    exit(1);
  }
  printf("* closing file\n");  
  close(fd);
  
  printf("**********\n* opening old file \"test.file\"\n");
  fd = open("test.file", O_RDONLY);
  printf("* open() got fd %d\n", fd);
  if (fd < 0) {
    printf("ERROR opening file: %s\n", strerror(errno));
    exit(1);
  }

  printf("* reading entire file into buffer \n");
  i = 0;
  do  {  
  	printf("* attemping read of %d bytes\n", MAX_BUF -i);
  	r = read(fd, &buf[i], MAX_BUF - i);
	printf("* read %d bytes\n", r);
	i += r;
  } while (i < MAX_BUF && r > 0);
  
  printf("* reading complete\n");
  if (r < 0) {
    printf("ERROR reading file: %s\n", strerror(errno));
    exit(1);
  }
  k = j = 0;
  r = strlen(teststr);
  do {
  	if (buf[k] != teststr[j]) {
		printf("ERROR  file contents mismatch\n");
    		exit(1);
	}
	k++;
	j = k % r;
  } while (k < i);
	printf("* file content okay\n");

	printf("**********\n* testing lseek\n");
	r = lseek(fd, 5, SEEK_SET);
	if (r < 0) {
    		printf("ERROR lseek: %s\n", strerror(errno));
    	exit(1);
  	}
	
  printf("* reading 10 bytes of file into buffer \n");
  i = 0;
  do  {  
  	printf("* attemping read of %d bytes\n", 10 - i );
  	r = read(fd, &buf[i], 10 - i);
	printf("* read %d bytes\n", r);
	i += r;
  } while (i < 10 && r > 0);
  printf("* reading complete\n");
  if (r < 0) {
    printf("ERROR reading file: %s\n", strerror(errno));
    exit(1);
  }
 
  k = 0;
  j = 5;
  r = strlen(teststr);
  do {
  	if (buf[k] != teststr[j]) {
		printf("ERROR  file contents mismatch\n");
    		exit(1);
	}
	k++;
	j = (k + 5)% r;
  } while (k < 5);

  printf("* file lseek  okay\n");
  printf("* closing file\n");  
  close(fd);
 
  /* Forking */
  printf("**********\n* testing fork\n");

  switch(fork()) {
  case 1:
      printf("* Forked, in parent\n");
      break;
      
  case 0:
      printf("* Forked, in child\n");
      break;

  default:
      printf("* fork FAILED.\n");
  }
 
  return 0;
}


