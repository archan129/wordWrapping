#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<fcntl.h>
#include<unistd.h>
#include<limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

int err = 0;

char* wrapFile(int width, char* fileName){
	int fd = open(fileName, O_RDONLY);
	if(fd<0){ //check if file exists
		perror("r1"); 
		exit(1);
	}
	int check = 0; // to see and account for multiple new lines in a row
	int sz, wu = 0; // to see of there is an error in the code
	char* c = (char*) calloc(1, sizeof(char));
	char* word = (char*) calloc(1, sizeof(char));//creating word array
	int size = 1, newLine = 0;
	char* wholeText = (char*) calloc(1, sizeof(char));
	int count = 0;
	char temp;
	do { //loops until end of file
		sz = read(fd,c,1);
		if(sz == 0 && size == 1){
			free(c);
			free(word);
			return wholeText;
		}
		if(isspace(c[0]) == 0){ //adding char by char into word if it is not a space
			word[count] = c[0];
			count++;
			word = (char*) realloc(word, (count+1));
		} else {
			word[count] = '\0';
			wu+=strlen(word);
			int mem = count + 10;
			size += mem;
			count = 0;
			wholeText = (char*) realloc(wholeText, size);
			if(wu <= width){ // if word can fit on line... then print
				sprintf(wholeText + strlen(wholeText), "%s", word);
				newLine = 0;
			} else if (strlen(word) > width){ //if the word is bigger than the line, print on its own line and return exit failure at the end
				sprintf(wholeText + strlen(wholeText), "\n%s", word);
				newLine = 1;
				err = -1;
			} else { //if word cannot fit on this line, print on next line
				sprintf(wholeText + strlen(wholeText), "\n%s", word);
				wu = strlen(word);
				newLine = 0;
			}
			if(strlen(word) > 0 && wu < width){ //adding spaces between words
				sprintf(wholeText + strlen(wholeText), " ");
				wu++;
			}

			free(word);
			word = (char*) calloc(1, sizeof(char));
		}

		if(c[0] == '\n' && temp == '\n' && sz!=0){ //accounting for new lines and paragraph breaks
				if(check == 0){
					sprintf(wholeText + strlen(wholeText), "\n\n");
					check = 1;
				}
			
			wu = 0;
		}

		if(c[0] == '\n'){ //is a new line
			temp = c[0];
		}

		if(isspace(c[0]) == 0){ //is a non space/
			temp = c[0];
			check = 0;
		}

	} while(sz!=0);

	free(c);
	free(word);
	if(newLine == 0){
		sprintf(wholeText + strlen(wholeText), "\n");
	}
	close(fd);
	return wholeText;

}

int main(int argc, char* argv[]){ // for testing ./ww {width} {file/directory} {}
//***need to check if second arguement is a file or directory
	int width = atoi(argv[1]);
	char* tWord;
	char* name = argv[2];
	char* wholeText;
	struct stat s, f;
	if(stat(argv[2], &s) == 0){
   		if(S_ISDIR(s.st_mode)){ //it's a directory
   			DIR *d;
   			struct dirent *dir;
			d = opendir(argv[2]);
			while((dir = readdir(d)) != NULL){
				char* fod = dir->d_name; // dir->d_name is the name of the file in the directory.
				//printf("%s\n", fod); // prints name of file in directory
				if(!(fod[0] == 'w' && fod[1] == 'r' && fod[2] == 'a' && fod[3] == 'p')){
					if(stat(fod, &f) != 0){ //checks if file in the directory is a valid file.
						tWord = (char*) calloc(strlen(argv[2]) + strlen(fod)+1, sizeof(char));
						for(int i = 0; i< strlen(name); i++){
							tWord[i] = name[i];
						}
						tWord[strlen(name)] = '/';
						for(int i = 1; i <= strlen(fod); i++){
							tWord[strlen(name)+i] = fod[i-1];
						}
						//printf("%s\n", tWord);
						struct stat d;
						stat(tWord, &d);

						if(!S_ISDIR(d.st_mode)){
							//printf("%s\n", tWord);
							//tWord = testing/wrap.foo.txt
							//write(filename, "what u want to write", size of what you want to write);
							//write(dest, "hello", 5);
							wholeText = wrapFile(width, tWord);
							free(tWord);
							tWord = (char*) calloc(strlen(argv[2]) + strlen(fod)+6, sizeof(char));
							for(int i = 0; i< strlen(name); i++){
								tWord[i] = name[i];
	
							}
							char* pre = "/wrap.";
							for(int i = 0; i < 6; i++){
								tWord[strlen(name)+i] = pre[i];
							}
							for(int i = 1; i <= strlen(fod); i++){
								tWord[strlen(name)+i + 5] = fod[i-1];
							}

							//printf("%s\n\n", tWord);
							int fd = open(tWord, O_WRONLY | O_TRUNC | O_CREAT, S_IRGRP | S_IROTH | S_IRUSR | S_IWGRP | S_IWOTH | S_IWUSR);

							write(fd, wholeText, strlen(wholeText));
							free(wholeText);
							close(fd);

						}
						free(tWord);

					}
				}
			}
			closedir(d);

		} else if(S_ISREG(s.st_mode)){ //it's a file
    			wholeText = wrapFile(width, argv[2]);
			printf("%s", wholeText);
			free(wholeText);
		} else { //neither file or directory
			err = -1;
		}
	} else {
		err = -1;
	}


	if(err == 0){ 
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
} 
