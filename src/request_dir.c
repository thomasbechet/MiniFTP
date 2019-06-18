#include "request.h"

#include "tcp_connection.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define LINE_LENGTH 320
#define BLOCK_PER_LINE 40

int alphasort_case_insensitive(const struct dirent** a, const struct dirent** b) 
{
    return(strcasecmp((*(const struct dirent **)a)->d_name, (*(const struct dirent **)b)->d_name));
}

int find_digit(int num)
{
    if (num == 0) return 1;
    int n = 0;
    while(num) 
    {
        num /= 10;
        n++;
    }
    return n;
}

void print_permissions(char permissions[11], struct stat* statbuf)
{
    permissions[0] = S_ISDIR(statbuf->st_mode) ? 'd' : '-';
    permissions[1] = statbuf->st_mode & S_IRUSR ? 'r' : '-';
    permissions[2] = statbuf->st_mode & S_IWUSR ? 'w' : '-';
    permissions[3] = statbuf->st_mode & S_IXUSR ? 'x' : '-';
    permissions[4] = statbuf->st_mode & S_IRGRP ? 'r' : '-';
    permissions[5] = statbuf->st_mode & S_IWGRP ? 'w' : '-';
    permissions[6] = statbuf->st_mode & S_IXGRP ? 'x' : '-';
    permissions[7] = statbuf->st_mode & S_IROTH ? 'r' : '-';
    permissions[8] = statbuf->st_mode & S_IWOTH ? 'w' : '-';
    permissions[9] = statbuf->st_mode & S_IXOTH ? 'x' : '-';
    permissions[10] = '\0';
}

int send_line(char line[LINE_LENGTH], teakey_t key)
{
    teablock_t* ptr;
    int i;

    ptr = (teablock_t*)line;
    for(i = 0; i < BLOCK_PER_LINE; i++)
    {
        tea_encrypt(&ptr[i], key);
    }

    return tcp_send(line, sizeof(char) * LINE_LENGTH);
}
int recv_line(char line[LINE_LENGTH], teakey_t key)
{
    int i;
    size_t k;

    if(tcp_recv(line, sizeof(char) * LINE_LENGTH, &k) != -1 && k == sizeof(char) * LINE_LENGTH)
    {
        for(i = 0; i < BLOCK_PER_LINE; i++)
        {
            tea_decrypt(&(((teablock_t*)line)[i]), key);
        }
    }
    else
    {
        return -1;
    }
}

void client_dir(const char* dst, teakey_t key)
{
    struct request request;
    struct answer answer;
    char line[LINE_LENGTH];
    size_t k;
    int i;

    //////////////////////////

    //Request initialization
    memset(&request, 0, sizeof(struct request));
    request.kind = REQUEST_DIR;
    snprintf(request.path, MAX_PATH_LENGTH, "%s", dst);

    //Send request
    encrypt_request(&request, key);
    if(tcp_send(&request, sizeof(struct request)) != -1)
    {
        //Recv answer
        memset(&answer, 0, sizeof(struct answer));
        if(tcp_recv(&answer, sizeof(struct answer), &k) != -1 && k == sizeof(struct answer))
        {
            decrypt_answer(&answer, key);
            if(answer.ack == ANSWER_OK)
            {
                for(i = 0; i < answer.length; i++)
                {
                    if(recv_line(line, key) != -1)
                    {
                        printf("%s\n", line);
                    }
                    else
                    {
                        fprintf(stderr, "Failed to receive line %d.\n", i);
                    }
                }
            }
            else if(answer.ack == ANSWER_ERROR)
            {
                fprintf(stderr, "Failed to received answer.\n");
            }
        }
    }
    else
    {
        fprintf(stderr, "Failed to send request.\n");
    }
}
void server_dir(const struct request* request, teakey_t key)
{
    struct answer answer;
    struct dirent** dir;
    struct stat statbuf;
    struct passwd* pws;
    struct group* grp;
    struct tm tmfile, tmnow;
    time_t now;
    char full_path[256];
    char permissions[11];
    char time_str[64];
    char line[LINE_LENGTH];
    int n, i, max_digit, file_count;

    //////////////////////////

    now = time(NULL);

    n = scandir(request->path, &dir, NULL, alphasort_case_insensitive);
    file_count = 0;
    if(n > 0)
    {
        max_digit = 0;
        for(i = 0; i < n; i++)
        {
            if(dir[i]->d_name[0] == '.') continue; //ls -l

            file_count++;

            snprintf(full_path, 256, "%s/%s", request->path, dir[i]->d_name);
            stat(full_path, &statbuf);
            max_digit = find_digit(statbuf.st_size) > max_digit ? find_digit(statbuf.st_size) : max_digit;
        }

        //Send confirmation with length of line
        memset(&answer, 0, sizeof(struct answer));
        answer.ack = ANSWER_OK;
        answer.length = file_count;
        encrypt_answer(&answer, key);
        tcp_send(&answer, sizeof(struct answer));

        for(i = 0; i < n; i++)
        {
            if(dir[i]->d_name[0] == '.') continue; //ls -l

            snprintf(full_path, 256, "%s/%s", request->path, dir[i]->d_name);
            stat(full_path, &statbuf);

            print_permissions(permissions, &statbuf);
            pws = getpwuid(statbuf.st_uid);
            grp = getgrgid(statbuf.st_gid);

            localtime_r(&statbuf.st_mtime, &tmfile);
            localtime_r(&now, &tmnow);
            if(tmfile.tm_year == tmnow.tm_year)
                strftime(time_str, sizeof(time_str), "%b %e %H:%M", &tmfile);
            else
                strftime(time_str, sizeof(time_str), "%b %e  %Y", &tmfile);

            snprintf(line, LINE_LENGTH, "%s %2lu %s %s %*lu %s %s", 
                permissions,
                statbuf.st_nlink, 
                pws->pw_name, 
                grp->gr_name, 
                max_digit, statbuf.st_size,
                time_str,
                dir[i]->d_name);

            if(send_line(line, key) == -1)
            {
                fprintf(stderr, "Failed to send line %d.\n", i);
                break;
            }

            free(dir[i]);
        }
        free(dir);
    }
    else
    {
        fprintf(stderr, "Failed to scandir.\n");
        //Send error: failed to scan
    }
}