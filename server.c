/*CSci4061 F2018 Project 3
*section:  1
* date: 12.12.18
* name: Ali Adam, Abdirahman Abdirahman, Mohamed Said, Hassan Ali
* id: 5330610, 5330419, 5115657, 4570634 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include "util.h"
#include <stdbool.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>

#define MAX_THREADS 100
#define MAX_queue_len 100
#define MAX_CE 100
#define INVALID -1
#define BUFF_SIZE 1024


int num_dispatch, num_workers, qlen, cache_entries;
int numE=0; // number of elements in the queue
int cache_counter;
FILE* to_file;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CL=PTHREAD_MUTEX_INITIALIZER; // cache lock for adding into cache
pthread_cond_t put = PTHREAD_COND_INITIALIZER;
pthread_cond_t get = PTHREAD_COND_INITIALIZER;
//pthread_cond_t CC = PTHREAD_COND_INITIALIZER; // wating for L to add into cache
pthread_t w[MAX_THREADS];
int con=0;
int prod=0;
int sig=1;

int * cache_pol;


/*
  THE CODE STRUCTURE GIVEN BELOW IS JUST A SUGESSTION. FEEL FREE TO MODIFY AS NEEDED
*/

// structs:
typedef struct request_queue {
   int fd;
   void *request;
} request_t;

typedef struct cache_entry {
    int len;
    char *request;
    char *content;
} cache_entry_t;

cache_entry_t * arrCash;
request_t rq[MAX_queue_len];

/*
void catchint (int signo) {
  printf("i received signal");
  sig=0;
}
*/

void show_cache()
{
  printf("[");
  int i;

  for(int i=0; i<cache_entries; i++)
  {
    printf("%d,", cache_pol[i]);
  }
  printf("]\n");
}

int min_cache_hits()
{
  int min_index, min,i;

  min=cache_pol[0];
  min_index=0;

  for (i=1; i<cache_entries; i++)
  {
    if (cache_pol[i] < min)
        {
           min = cache_pol[i];
           min_index=i;
        }

  }
  return min_index;

}


void logger(int threadInt, int reqNum, int fd, char* request_string, int bytes, long int time, int cache_status){

 char buffer[BUFF_SIZE];
 char *s1="HIT";
 char *s2="MISS";
 char *s;
 if(cache_status)
 {
  s=s1;
 }
 else
 {
  s=s2;
 }

 int j = snprintf(buffer,BUFF_SIZE,"[%d][%d][%d][%s][%d][%ld us][%s]\n",threadInt, reqNum, fd, request_string, bytes, time,s);
 printf("[%d][%d][%d][%s][%d][%ld us][%s]\n",threadInt, reqNum, fd, request_string, bytes, time,s);
 fputs(buffer, to_file);
 fflush(to_file);
}


/* ************************ Dynamic Pool Code ***********************************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
void * dynamic_pool_size_update(void *arg) {
  while(1) {
    // Run at regular intervals
    // Increase / decrease dynamically based on your policy

  }
}
/**********************************************************************************/

/* ************************************ Cache Code ********************************/

// Function to check whether the given request is present in cache
int getCacheIndex(char *request){
   int i;
  for(int i=0; i<cache_entries; i++)
  {
    if(strcmp(arrCash[i].request,request)==0)
      return i;
  }

  return -1;
}
    /// return the index if the request is present in the cache or -1 if not


// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *Request , int memory_size){
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memeory when adding or replacing cache entries
 int s_size;
 static int i=0;
 int index;



 //printf("ADD IN CASH I IS %d\n",i);


 s_size=strlen(Request);

 if (i > cache_entries -1)
 {
    //show_cache();
    index=min_cache_hits();
    //printf("The INDEX is %d\n",index);
    cache_counter=index;
 }


 if(strcmp(arrCash[cache_counter].request,"")!=0)
 {



      //printf("I WILL FREE THE CURRENT INDEX %d\n", (cache_counter));
      free(arrCash[cache_counter].request);
      free(arrCash[cache_counter].content);
      cache_pol[cache_counter]=0;

 }

 arrCash[cache_counter].len = memory_size;

 //printf(" this is in memory_size :%d\n", memory_size);
 //printf(" this is in len  :%d\n", arrCash[cache_counter].len);


 arrCash[cache_counter].request =malloc(s_size + 1);
 arrCash[cache_counter].content = malloc(memory_size+ 1);



 memcpy(arrCash[cache_counter].request,Request,s_size);
 memcpy(arrCash[cache_counter].content,mybuf,memory_size);
 arrCash[cache_counter].request[s_size]='\0';
 arrCash[cache_counter].content[memory_size]='\0';

 i=i+1;

  if (i<cache_entries)
  {
    cache_counter +=1;
  }

}

// clear the memory allocated to the cache
void deleteCache(){
  // De-allocate/free the cache memory
  free(arrCash);
}

// Function to initialize the cache
void initCache(){
  // Allocating memory and initializing the cache array
  arrCash =(cache_entry_t *)malloc(cache_entries*sizeof(cache_entry_t));
  cache_pol=(int *)malloc(cache_entries*sizeof(int));
  cache_counter = 0;

  int i;

  for(i=0; i<cache_entries; i++)
  {
    cache_pol[i]=0;
    arrCash[i].len=0;
    arrCash[i].request="";
    arrCash[cache_counter].content ="";

  }

}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
char * readFromDisk(char *path , int * nb) {
  // Open and read the contents of file given the request
  char *string;
  struct stat sb;
  char cwd[BUFF_SIZE];


  FILE *f = fopen((path+1), "r");
  //printf("THE PATH IS %s \n", (path+1));


  if (f==NULL) //open is unsuccesful
  {
      printf("file didn't open %d\n", errno);
      return NULL;
  }

  if (stat((path+1), &sb) == -1) {
               perror("stat");
               return NULL;
           }
     *nb=sb.st_size; //change number of bytes by refference
     string = malloc(sb.st_size + 1);

     if (fread(string, sb.st_size, 1, f)==0)
     {
       printf("fread failed\n");
     }



     string[sb.st_size] = 0;

     //printf("%s \n", string);

     fclose(f);
     return string;
}

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char * mybuf) {
  // Should return the content type based on the file type in the request
  // (See Section 5 in Project description for more details)
  char *types[] = {"txt","htm","html","jpg","gif"};
  char *content[] = {"text/html","image/jpeg","image/gif" , "text/plain"};

  if (strstr(mybuf,types[0])!=NULL)
  {
    return content[3];
  }
  else if (strstr(mybuf,types[1])!=NULL)
  {
    return content[0];
  }
  else if (strstr(mybuf,types[2])!=NULL)
  {
    return content[0];
  }
  else if (strstr(mybuf,types[3])!=NULL)
  {
    return content[1];
  }
   else if (strstr(mybuf,types[4])!=NULL)
  {
    return content[2];
  }
  else
  {
    return content[3];
  }
}



// This function returns the current time in milliseconds
int getCurrentTimeInMicro() {
  struct timeval curr_time;
  gettimeofday(&curr_time, NULL);
  return curr_time.tv_usec;
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
void * dispatch(void *arg) {
  request_t r;
  pthread_t self;
  int fd;

  char filename[BUFF_SIZE];
  int id = *(int *)arg;

   if (pthread_detach(pthread_self()) == 0)
   {
       //printf("this is dispatcher and i detached\n");
   }
   else
   {
      printf("this is dispatcher i had an error detaching\n");
      return NULL;
   }

  while (1) {


    fd=accept_connection();
    // Get request from the client

    pthread_mutex_lock(&lock);
    // Accept client connection



    if(fd > -1)
    {
      if (get_request(fd,filename) != 0)
      {
        printf("faulty client request");
      }
      else
      {

        // Add the request into the queue
        while ((prod+1)%qlen == con) //numE==qlen
        {
            pthread_cond_wait(&put, &lock);
        }


        r.fd=fd;
        r.request=filename;
        rq[prod]=r;




        prod=prod+1;
        prod=prod%qlen;
        //numE=numE+1;
        pthread_cond_signal(&get);

      }
    }

    pthread_mutex_unlock(&lock);

   }
   return NULL;
}

/**********************************************************************************/
int rCount=0;
int nCount = 0;
// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {
  int start, stop, in_cache, numbytes, c_i; //start is start time and stop is stop time. in_chache is used to check if what we are looking for is in chache
  long int time;
  request_t look;           // this would be the item we get from the queue
  char find[BUFF_SIZE]; // the file name we are looking for
  char *content_type, *buf; // contentype is the type of the file and buf is what it contains


  int reqNum=0; // amount of work
  int id = *(int *)arg;
  int found;

   if (pthread_detach(pthread_self()) == 0)
   {
       //printf("this is worker and i detached\n");
   }
   else
   {
      printf("this is worker i had an error detaching\n");
      return NULL;
   }

   while (1) {

    pthread_mutex_lock(&lock);

    // Start recording time

    // Get the request from the queue a
    while (prod == con) //numE == 0
    {
      pthread_cond_wait(&get, &lock);
    }


    look=rq[con];
    con=con+1;
    con=con%qlen;
    start=getCurrentTimeInMicro();

    nCount++;
    //printf("Req Cnt:%d\n", nCount);


    //numE=numE-1;


    //pthread_cond_signal(&put);
    //pthread_mutex_unlock(&lock);

    content_type=getContentType(look.request);
    // Get the data from the disk or the cache
    if((c_i=getCacheIndex((char *)look.request)) !=-1) //(c_i=getCacheIndex((char *)look.request)) !=-1
    {
      cache_pol[c_i]=cache_pol[c_i]+1;
      found=1;
      //printf("found this in CASH");
      //printf(" this is in memory_size before cache :%d\n", arrCash[c_i].len);
      if(return_result(look.fd,content_type,arrCash[c_i].content,arrCash[c_i].len)!=0)
       {
          printf("return_result failed");
       }
      else
      {
       rCount++;
       //printf("return Cnt:%d\n", rCount);
      }
      stop=getCurrentTimeInMicro();
      time=stop-start;
      logger(id, reqNum, look.fd, (char*)look.request, arrCash[c_i].len, time, found);
      pthread_cond_signal(&put);
      pthread_mutex_unlock(&lock);
    }
    else
    {
        pthread_cond_signal(&put);
        pthread_mutex_unlock(&lock);
        found=0;

        // look in a disk
        //printf("looking in Disk");
        if ((buf=readFromDisk(look.request , &numbytes))==NULL) // if i cant read from the disk.
        {   buf="NOT FOUND IN DISK";
            //printf("should not be here\n");
            if (return_error(look.fd,buf) != 0)
            {
                printf("return_error failed");
            }
        }
        else
        {

           if(return_result(look.fd,content_type,buf,numbytes)!=0)
           {
              printf("return_result failed");
           }
           else
           {
            rCount++;
            //printf("return Cnt:%d\n", rCount);
            stop=getCurrentTimeInMicro();
            time=stop-start;
            logger(id, reqNum, look.fd, (char*)look.request, numbytes, time, found);
            pthread_mutex_lock(&CL);
            addIntoCache(buf,look.request ,numbytes);
            pthread_mutex_unlock(&CL);

           }




        }


        free(buf);
     }
    // Stop recording the time
    //stop=getCurrentTimeInMicro();
    // Log the request into the file and terminal
    //time=stop-start;

    //logger(id,reqNum,look.fd,look.request,numbytes,time,found);
    //logger(id, reqNum, look.fd, (char*)look.request, numbytes, time, found);


    reqNum++;

    // return the result

  }
  return NULL;
}








/**********************************************************************************/

int main(int argc, char **argv) {

  //Initialization
    pthread_t d[MAX_THREADS]; //if we want to do the extra credit we have i think we have to make this global.
    int wid[MAX_THREADS], did[MAX_THREADS];   // arguments of the threads i dont even think they matter
    int port, dynamic_flag;                   // port is the port this process is using, dynamic_flag determines whether the worker pool is static or dynamic
    char path[BUFF_SIZE];                     //this is the path (folder) that we input.
    pthread_t nin_culus;

    //signal(SIGINT,catchint);

 // Error check on number of arguments

  if(argc != 8){
    printf("usage: %s port path num_dispatcher num_workers dynamic_flag queue_length cache_size\n", argv[0]);
    return -1;
  }

  int i;
  for(i=0; i<8; i++)
  {
    //printf("%s\n", argv[i] );
  }

  // Get the input args
     port=atoi(argv[1]);
     num_dispatch=atoi(argv[3]);
     num_workers=atoi(argv[4]);
     dynamic_flag=atoi(argv[5]);
     qlen=atoi(argv[6]);
     cache_entries=atoi(argv[7]);
     strcpy(path,argv[2]);
     printf("Starting server on port %d: %d disp, %d work\n",port,num_dispatch,num_workers);

     //printf("%d %d %d %d %d %d \n", port ,num_dispatch, num_workers, dynamic_flag, qlen, cache_entries);


  // Perform error checks on the input arguments  ?? what should be the error check for port and path ??

     if(num_workers>MAX_THREADS)
     {
        printf("the amound of worker you want is %d but num_workers can take a maximum number of %d \n", num_workers, MAX_THREADS);
        return -1;
     }
     if(num_dispatch>MAX_THREADS)
     {
        printf("num_dispatch can take a maximum number of %d \n", MAX_THREADS);
        return -1;
     }
     if((dynamic_flag != 0) & (dynamic_flag != 1))
     {
        //printf("%s\n",dynamic_flag);
        printf("Dynamic has to be either 0 or 1\n");
        return -1;
     }
     if(qlen>MAX_queue_len)
     {
        printf("qlen has to be lesser or equal to %d", MAX_queue_len);
        return -1;
     }
     if(cache_entries>MAX_CE)
     {
        printf("cache_entries has to be lesser or equal to %d", MAX_CE);
        return -1;
     }




  // Change the current working directory to server root directory
     if (chdir(path) == -1)
     {
       printf("path doesn't exist\n");
        return -1;
     }


    if((to_file = fopen("webserver_log","w")) == NULL)
    {
      printf("Log File didn't open!\n");
    }

  // Start the server and initialize cache
     init(port); // now you can communicate with the server throught the port.
     initCache(); // I malloc MAX_CE of

  // Create dispatcher and worker threads
   for(int i=0; i<num_dispatch; i++)
   {
      did[i] = i;
      if (pthread_create(&d[i],NULL , dispatch, (void *) &(did[i])) != 0)
      {
        perror("couldn't create dispatch thread");
      }
   }

   for(int i=0; i<num_workers; i++)
   {
      wid[i] = i;
      if (pthread_create(&w[i],NULL , worker, (void *) &(wid[i])) != 0)
      {
        perror("couldn't create worker thread");
      }
   }

   if (pthread_create(&nin_culus,NULL , dynamic_pool_size_update, (void *) &(wid[i])) != 0)
   {
        perror("couldn't create worker thread");
   }



   while(1);

   /*
   printf("will clean up");
   pthread_mutex_destroy(&lock);               // Clean up and exit
   pthread_cond_destroy(&put);
   pthread_cond_destroy(&get);
   deleteCache();
   */

  // Clean up
  return 0;
}
