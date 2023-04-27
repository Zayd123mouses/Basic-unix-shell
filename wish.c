#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define EXIT "exit"
#define CD "cd"
#define PATH "path"

// linked list to keep track of the path strings
typedef struct path_list {
   char *path;
   struct path_list *next;
} path_list;

path_list *head = NULL;

// void *remove_trailing_space(  char *, size_t );
void insert( char**, size_t );
void insertatbegin();
void  freeNodes();
void parse_command (char** ,  size_t );
void Error_printer();
void main_parser( char **, size_t  );
void execute_normal( char **, size_t  );
void execute_redirection( char **, size_t  );
char *generate_path(char *);
void execute_command(char**, size_t, char* );
size_t built_in_commands (char ** commands, size_t command_index );

const char error_message[30] = "An error has occurred\n";

int is_empty(const char *s) {
  while (*s != '\0') {
    if (!isspace((unsigned char)*s)){
      return 0;
    }
    s++;
  }
  return 1;
}

int main(int argc, char **argv) {

insertatbegin(); // inial path is /bin

void (* printError)() = Error_printer; //  function pointer to print errors

  char *str = NULL; //string to save the input from the user
  size_t s = 0;  // for getlone function
   
   // exit when provoking the programm with more than one file
   if ( argc > 2 ) {
      printError();
      exit(1);
   } 

   FILE *file; 
   if (argc > 1 ){   // batch mode
      file = fopen(argv[1],"r" ); 
      if(file == NULL){
         printError();
         exit(1);
      }
      
   }else{  // interactive mode
      file = stdin;
   }

   while ( getline( &str, &s, file )  > -1 )  //reead the input from the file
   {
     char *temp = calloc(strlen(str) + 1, sizeof(char)); // hold the the string to be muted
     strcpy( temp, str );
     char *ptr;
     size_t command_index = 0;     
     char *commands[256];

    while( (ptr = strsep( &str, "\n,&" )) != NULL) { 
      if( *ptr != 0 )  {
         commands[command_index] = malloc((strlen(ptr) + 1)*sizeof(ptr));
          strcpy( commands[command_index], ptr );
         command_index++;
      }
    }

    if(command_index > 0 ){
    commands[command_index] = NULL;
    }

   main_parser(commands, command_index);
  }
 free(str);
 return 0;
}

void main_parser( char **commands, size_t command_index ) {
char *ptr;
char *command;
char *myargv[100];
 
   for ( size_t i = 0; i < command_index; i++ ) {
       
       command = malloc((strlen(commands[i])+1) *sizeof(char));
       
      strcpy(command, commands[i]);
       
      size_t command_tracker = 0;
      while((ptr = strsep(&command, " ")) != NULL ) {
         if(*ptr != 0 && strcmp(ptr, " ") != 0){
      
         myargv[command_tracker] = calloc(strlen(ptr), sizeof(ptr));
         strcpy(myargv[command_tracker],ptr);
         command_tracker++;
         }
      }
      
      if((int)built_in_commands(myargv,command_tracker) > -1) {
         return;
      }
      
      int rc = fork();
      if( rc == 0) {
      parse_command(myargv, command_tracker);
      exit(0);
      }else{
      }
   }
 size_t i = 0;
         while( i < command_index && commands[i] != NULL) {
            free(commands[i]);
            commands[i] = NULL;
            i++;
         }
}

size_t built_in_commands (char ** commands, size_t command_index ) {
    if(command_index == 0){
      return 0;
    }
 if ( strcmp( EXIT, commands[0] ) == 0 ) {    // quit the programm
 
      if (command_index > 1){
           Error_printer();
           return 0;
      }else{
         exit(0);
      }
      
   } else if ( strcmp(CD,  commands[0] ) == 0 ) { // always take one  argument 
      if( command_index > 2 || command_index == 1) {   // zero or > 1 is an error/
       Error_printer();
       return 0;
      }else{
      size_t err = chdir(commands[1]);
      if ( err != 0 ){
         Error_printer();
         return 0;
      }
      }
      return 1;

   } else if ( strcmp(PATH,  commands[0] ) == 0 ) { // takes 0 or more argument   $Path /bin /usr/bin
      freeNodes();   //overwrite the nodes
      insert(commands, command_index);    // the path overwrite the old one, I should have read the specifics before implement it :)
      return 1;
   }
   return -1;  //not a built in command
}

void parse_command (char **commands,  size_t len ) {
   
   size_t number_redirect = 0; //check the entire command if there is more than one > symbol
   size_t index_of_symbol = 0;
   for (size_t i = 0; i < len; i++ ){
      if((strchr(commands[i], '>')) != NULL ){
         
         char* s = commands[i];
         
         for ( size_t j = 0 ; s[j]; j++) {
            if(s[j] == '>') {
               number_redirect++;
            }
         }        
         //handle multible redirection
         if(number_redirect > 1) {
            Error_printer();
            exit(0);
         }
         // point to > then if more than 1 string print error 
         if ( number_redirect > 0 ) {
              index_of_symbol = i;
         }

      }
   }
   //handle more than one number of files
if ( number_redirect > 0 ){
  if( len - index_of_symbol >= 3 ) {
   Error_printer();
   exit(0);
  }else if ( len - index_of_symbol == 2 ){
    char *s = commands[ index_of_symbol ];
    if(s[strlen(s) - 1] != '>'){
      // printf("first error %s\n",s);
      Error_printer();
      exit(0);
    }
    for( size_t j = 1; s[j]; j++ ) {
      if( s[j]=='>' ){
         if( j != strlen(s) - 1 ){
            // printf("second error %ld,%ld\n",j,strlen(s));
            Error_printer();
            exit(0);
         }
      }
    }
  }else if( len - index_of_symbol == 1 ) {
   char *s = commands[ index_of_symbol ];
   if(s[strlen(s) - 1] == '>'){
      Error_printer();
      exit(0);
    }
  }

  // handle no command
 char *b = commands[ index_of_symbol ];
   if(b[0] == '>' && index_of_symbol == 0) {
      
      Error_printer();
      exit(0);
   }  
  }
  
  if ( number_redirect > 0 ){
   execute_redirection(commands, len);
  }else{
       
      execute_normal(commands, len);
  }
}

void execute_redirection( char **commands, size_t len ) {
  char *first_str = commands[0];
  char *ptr = NULL;
   char *path = NULL;
   char str[20] = {'\0'};
   char temp[2] =  {'\0'};
  if( (ptr = strchr( first_str, '>')) == NULL ) {
    path = generate_path(first_str);
  }else {
   for ( size_t i = 0; first_str[i]; i++ ) {
      if( first_str[i] == '>') {
         path = generate_path(str);
         break;
      }
      temp[0] = first_str[i];
      temp[1] = 0;
      strcat( str, temp);
   }
  }
  execute_command( commands, len, path );
}

void execute_command(char** commands, size_t len, char *path) {
   char *ptr;
   char str[100] = {0};
   char *myargv[250];
   size_t j = 0;
   while ( j < len )
   {
      strcat(strcat(str, commands[j]), " ");
       j++;
   }
   char *argv = calloc(strlen(str) + 1, sizeof(char));
   strcpy(argv,str);
   
   size_t i = 0;
   while ( (ptr = strsep(&argv," ,>") ) != NULL) {
      if(*ptr != 0){   
         myargv[i] = calloc(strlen(ptr) + 1, sizeof(char));
         strcpy(myargv[i],ptr);
         i++;
      }
   }

   char *filename = calloc(strlen(myargv[i - 1] + 1 ), sizeof(char));
   strcpy(filename, myargv[i-1]);
  
   myargv[i - 1] = NULL;
   // redirect the output
   freopen( filename, "w", stdout );
   //execute the command
   execv( path, myargv);
    perror("error");
   printf("dame it we fucked\n"); //should not print
  }

void execute_normal( char **commands, size_t len ) {
  commands[len] = NULL;
  char *path = generate_path(commands[0]);
  if( path == NULL ) {
   Error_printer();
   exit(0);
  }
  execv( path, commands);
  perror("error in normal\n");
}

// assume the command is parsed
char *generate_path(char *command) {
 path_list *temp = head; 
 char *path = calloc( ( strlen(command) + 1 ) , sizeof( char ) );
 while (temp != NULL)
 { 
    strcpy(path, temp->path);        
    strcat(strcat(path,"/"), command);
    if ( access( path, X_OK ) == 0 ){
          return path;
       }
       temp = temp->next;
}
 return NULL;
}

//////////////////////////////////////
void insert( char **argv, size_t command_number ) {
   for ( size_t i = 1; i < command_number; i++ ){
          path_list *lk = malloc(sizeof(path_list));
          lk->path = argv[i];
         if(head == NULL){  //insert at the beginng
            lk->next = head;
            head =lk;
             }else{ // insert at the end
            path_list *linkList = head;

            while( linkList->next != NULL ) {
                linkList = linkList->next;
            }

            lk->next = NULL;
            linkList->next = lk;
             }
   }
}

void insertatbegin(  ) { 
   path_list *lk = malloc(sizeof(path_list));
    lk->path = "/bin";
    lk->next = head;
    head =lk;
}

void freeNodes() {
   path_list *temp;
   // printf("maybe here\n");
   while ( head != NULL ) {
     temp = head;
     head  = head->next;
     free(temp);
   }
}

void Error_printer (){
   write(STDERR_FILENO, error_message, strlen(error_message));
}

