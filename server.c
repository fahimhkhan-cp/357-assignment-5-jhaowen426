#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#define PORT 2828

void handle_request(int nfd)
{
   FILE *network = fdopen(nfd, "r");

   if (network == NULL)
   {
      perror("fdopen");
      close(nfd);
      return;
   }

   // variable declarations
   char *input_line = NULL;
   size_t size;
   ssize_t num;
   char *request = NULL;
   char *filename = NULL;
   char *line_dup = NULL;
   char *line_dup_start;
   FILE *fd;
   char *response_type = NULL;
   char reply[8192];
   struct stat stat_buffer;
   char file_contents[8192];
   size_t read_bytes;
   char temp[10];

   // get input from httpd
   while ((num = getline(&input_line, &size, network)) >= 0)
   {
      // separate input line into request and filename
      line_dup = strdup(input_line);
      line_dup_start = line_dup;
      request = strsep(&line_dup, " ");

      if ((strcmp(request, "GET") == 0) || (strcmp(request, "HEAD") == 0)) {
         filename = strsep(&line_dup, " ");
         if (filename[0] == '/') {
            filename++;
         }

         // determine if file is in cgi-like directory
         strncat(temp, filename, 9);
         temp[9] = '\0';
         if ((fd = fopen(filename, "r")) == NULL) {
            response_type = "404 Not Found";
         }
         else if (strcmp(temp, "cgi-like/") != 0) {
            response_type = "NO PERMISSION";
         }
      }

      // handle GET request
      if (strcmp(request, "GET") == 0) {
         // open filename for reading
         fd = fopen(filename, "r");

         // determine if file is valid / found
         if (fd == NULL) {
            response_type = "404 Not Found";
         }
         else {
            response_type = "200 OK";
         }

         // read contents from file
         if (fd != NULL) {
            read_bytes = fread(file_contents, 1, sizeof(file_contents), fd);
            file_contents[read_bytes] = '\0';
            if (stat(filename, &stat_buffer) == -1) {
               response_type = "500 Internal Error";
            }
         }

         // construct header + file contents
         if (strcmp(response_type, "200 OK") == 0) {
            sprintf(reply, "HTTP/1.0 %s\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s", response_type, stat_buffer.st_size, file_contents);
         }
         else {
            sprintf(reply, "HTTP/1.0 %s\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n", response_type);
         }
         
         // write reply to httpd
         printf("%s\n", reply);
         if (fd != NULL) {
            fclose(fd);
         }
         write(nfd, reply, strlen(reply));
      }
      // handle HEAD request
      else if (strcmp(request, "HEAD") == 0) {
         // open filename for reading
         fd = fopen(filename, "r");

         // determine if file is valid / found
         if (fd == NULL) {
            response_type = "404 Not Found";
         }
         else {
            response_type = "200 OK";
            stat(filename, &stat_buffer);
         }

         // construct header
         if (strcmp(response_type, "200 OK") == 0) {
            sprintf(reply, "HTTP/1.0 %s\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", response_type, stat_buffer.st_size);
         }
         else {
            sprintf(reply, "HTTP/1.0 %s\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n", response_type);
         }
         
         // write reply to httpd
         printf("%s\n", reply);
         if (fd != NULL) {
            fclose(fd);
         }
         write(nfd, reply, strlen(reply));
      }
      // permission denied
      else if (strcmp(request, "NO PERMISSION") == 0) {
         response_type = "403 Permission Denied";
         sprintf(reply, "HTTP/1.0 %s\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n", response_type);

         // write reply to httpd
         printf("%s\n", reply);
         write(nfd, reply, strlen(reply));
      }
      // invalid request
      else {
         response_type = "400 Bad Request";
         sprintf(reply, "HTTP/1.0 %s\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n", response_type);

         // write reply to httpd
         printf("%s\n", reply);
         write(nfd, reply, strlen(reply));
      }

      free(line_dup_start);
   }

   fclose(network);
}

void run_service(int fd)
{
   while (1)
   {
      int nfd = accept_connection(fd);
      if (nfd != -1)
      {
         printf("Connection established\n");
         handle_request(nfd);
         printf("Connection closed\n");
      }
   }
}

int main(void)
{
   int fd = create_service(PORT);

   if (fd == -1)
   {
      perror(0);
      exit(1);
   }

   printf("listening on port: %d\n", PORT);
   run_service(fd);
   close(fd);

   return 0;
}
