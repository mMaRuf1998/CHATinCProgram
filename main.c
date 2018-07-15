#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>
#include <fcntl.h>
#include <errno.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

/* <DESC>
 * Performs an FTP upload and renames the file just after a successful
 * transfer.
 * </DESC>
 */
        /*


                    UPDATE THESE ADDRESSES

                    */
#define LOCAL_FILE      "C:\\Users\\Home\\Desktop\\New folder\\Chat1.txt"   //File Location On Your Computer
#define UPLOAD_FILE_AS  "Chat3.txt"
#define REMOTE_URL      "ftp://HOME:030201@192.168.106.1/Maruf/"  UPLOAD_FILE_AS  //URL of the server to upload
#define RENAME_FILE_TO  "Chat3.txt"         //FILE Name TO be saved on the server . You can keep the name same as your pc file

/* NOTE: if you want this example to work on Windows with libcurl as a
   DLL, you MUST also provide a read callback with CURLOPT_READFUNCTION.
   Failing to do so will give you a crash since a DLL may not use the
   variable's memory when passed in to it from an app like this. */
   struct FtpFile {
  const char *filename;
  FILE *stream;
};
void text_printf();
size_t write_data(ptr,size,nmemb,stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
  struct FtpFile *out = (struct FtpFile *)stream;
  if(out && !out->stream) {
    /* open file for writing */
    out->stream = fopen(out->filename, "wb");
    if(!out->stream)
      return -1; /* failure, can't open file to write */
  }
  return fwrite(buffer, size, nmemb, out->stream);
}
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  curl_off_t nread;
  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  size_t retcode = fread(ptr, size, nmemb, stream);

  nread = (curl_off_t)retcode;

  //fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
    //      " bytes from file\n", nread);
  return retcode;
}


//*****************************DOWNLOAD*************************
int FTP_Download(void)
{
  CURL *curl;
    FILE *fp,*file;
    CURLcode res;
    char *url = "ftp://HOME:030201@192.168.106.1/Maruf/Chat3.txt";   //Server URL Update Needed
    //curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    char outfilename[FILENAME_MAX] = "C:\\Users\\Home\\Desktop\\New folder\\Chat1.txt";  // FIle directory in your pc
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(outfilename,"wb");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        //printf("%s",curl_easy_perform(curl));

        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
    }
    return 0;
}

//******************************UPLOAD*****************************

int upload(void)
{
  CURL *curl,*curl_m;
  CURLcode res;
  FILE *hd_src;
  struct stat file_info;
  curl_off_t fsize;

  struct curl_slist *headerlist = NULL;
  static const char buf_1 [] = "RNFR " UPLOAD_FILE_AS;
  static const char buf_2 [] = "RNTO " RENAME_FILE_TO;

  /* get the file size of the local file */
  if(stat(LOCAL_FILE, &file_info)) {
    printf("Couldn't open '%s': %s\n", LOCAL_FILE, strerror(errno));
    return 1;
  }
  remove("Chat3.txt");
  fsize = (curl_off_t)file_info.st_size;

  //printf("Local file size: %" CURL_FORMAT_CURL_OFF_T " bytes.\n", fsize);

  /* get a FILE * of the same file */
  hd_src = fopen(LOCAL_FILE, "rb");

  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {

    /* build a list of commands to pass to libcurl */
    headerlist = curl_slist_append(headerlist, buf_1);
    headerlist = curl_slist_append(headerlist, buf_2);

    /* we want to use our own read function */
//curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

    /* enable uploading */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    /* specify target */
    curl_easy_setopt(curl, CURLOPT_URL, REMOTE_URL);

    /* pass in that last of FTP commands to run after the transfer */
    curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);

    /* now specify which file to upload */
    curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

    /* Set the size of the file to upload (optional).  If you give a *_LARGE
       option you MUST make sure that the type of the passed-in argument is a
       curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
       make sure that to pass in a type 'long' argument. */
       size_t write_data1(void *buffer, size_t size, size_t nmemb, void *userp)
{
   return size * nmemb;
}
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)fsize);
    //curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, write_data1);

    /* Now run off and do what you've been told! */
    //FILE *f = fopen("target.txt", "wb");
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
    curl_easy_setopt(curl, CURLOPT_NOBODY, NULL);
    //curl_easy_setopt(curl, CURLOPT_WRITEDATA, &f);

    res = curl_easy_perform(curl);



    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* clean up the FTP commands list */
    curl_slist_free_all(headerlist);

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  fclose(hd_src); /* close the local file */

  curl_global_cleanup();
  return 0;
}

int main()
    {
        char user[100];
        printf("Please Enter Your UI Name :");
        gets(user);
        fflush(stdin);
        system("cls");
        FILE *to_up,*to_down;
       // to_down=fopen("C:\\Users\\Home\\Desktop\\New folder\\Chat1.txt","w");
        //fclose(to_down);

        char input[10000],msg[10000];

        FTP_Download();
        FILE *MyFile,*print_new_data;
        int c,t=0;
        /*
        Reload:
        MyFile=fopen("C:\\Users\\Home\\Desktop\\New folder\\Chat1.txt","r");
        c = fgetc(MyFile);
        while (c!=EOF)
        {
            printf("%c",c);
            c = fgetc(MyFile);
        }
        fclose(MyFile);
        */
        text_printf();
        Continue:
        to_up=fopen("C:\\Users\\Home\\Desktop\\New folder\\Chat1.txt","a+");  //Your Directory Address
        printf("%s : ",user);
        gets(input);
        fflush(stdin);
        fprintf(to_up,"%s : %s\n",user,input);
        fclose(to_up);
        upload();
        int a;
        loop:
        a=getch();
        if(a==13)
        {
            system("cls");
            text_printf();
            goto loop;

        }
        else if (a==27)
        {
            to_down= fopen("C:\\Users\\Home\\Desktop\\New folder\\Chat1.txt","w+"); //Your Directory
            upload();
            printf("Chat Clearing");
            int i;
            for(i=0;i<5;i++)
            {
                printf(".");
                Sleep(1000);
            }
            fclose(to_down);
            system("cls");
            FTP_Download();
            text_printf();
            goto Continue;
        }
        else
            goto Continue;
    }

    void text_printf()
        {
            FILE *MyFile=fopen("C:\\Users\\Home\\Desktop\\New folder\\Chat1.txt","r"); //Your Directory
        int c = fgetc(MyFile);
        while (c!=EOF)
        {
            printf("%c",c);
            c = fgetc(MyFile);
        }
        fclose(MyFile);
        }
