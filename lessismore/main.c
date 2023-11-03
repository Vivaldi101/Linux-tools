#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define implies(p, q) (!(p)) || (q)
#define pre(p) if (!(p)) assert(0)
#define inv(p) if (!(p)) assert(0)
#define post(p) if (!(p)) assert(0)
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define haltIfNull(p) if (!(p)) abort()
#define haltIfEOF(p) if ((p) == EOF) abort()
#define haltIf(p, i) if ((p) == (i)) abort()

#define halt abort()

#define MAX_PAGE_COUNT    26
#define MAX_LINE_COUNT    512

static size_t
see_more(int fp)
{
    pre(fp != 0);

    char c;
    haltIf(read(fp, &c, 1), -1);

    if (c == '\n')
        return MAX_PAGE_COUNT;
    else if (c == 'f')
    {
        // Remove the enter from stdin.
        haltIf(read(fp, &c, 1), -1);
        return 1;
    }

    return 0;
}

static void
do_more(FILE* fp, size_t line_count)
{
    char line[MAX_LINE_COUNT];
    size_t lines_on_screen = 0;
    size_t new_lines_to_see = 0;
    size_t lines_seen = 0;

    const int tty_fp = open("/dev/tty", O_RDONLY);
    haltIfNull(tty_fp);

    while(fgets(line, MAX_LINE_COUNT, fp))
    {
        ++lines_seen;
        inv(lines_on_screen <= MAX_PAGE_COUNT);

        haltIfEOF(fputs(line, stdout));

        if (lines_on_screen < MAX_PAGE_COUNT)
            lines_on_screen++;
        else if (lines_on_screen == MAX_PAGE_COUNT)
        {
            //line_percentage += (size_t)((((float)lines_seen / (float)line_count)*100) + 0.5f);
            printf("More?(%zu\%% done.)", (size_t)((float)lines_seen / (float)line_count*100));
            //printf("More? Lines seen so far: (%zu)", lines_seen);
            fflush(stdout);

            new_lines_to_see = see_more(tty_fp);

            if (new_lines_to_see == 0)
                break;

            if (new_lines_to_see == 1)
                lines_on_screen = MAX_PAGE_COUNT;
            else if (new_lines_to_see == MAX_PAGE_COUNT)
                lines_on_screen = 0;

            post(lines_on_screen <= MAX_PAGE_COUNT);
        }

        inv(lines_on_screen <= MAX_PAGE_COUNT);
    }

    close(tty_fp);

    inv(lines_on_screen <= MAX_PAGE_COUNT);
}

int 
main(int argc, char* argv[])
{
    FILE* fp;

    if (argc == 1) 
    {
        printf("Listing the contents of standard input.\n");
        do_more(stdin, 0);
    }
    else
        while(--argc > 0)
            if((fp = fopen(*++argv, "r")) != NULL)
            {
                char line[MAX_LINE_COUNT];
                size_t line_count = 0;
                while(fgets(line, MAX_LINE_COUNT, fp)) ++line_count;
                fseek(fp, 0, SEEK_SET);
                printf("Listing the contents of file: %s (%zu lines).\n", *(argv), line_count);

                do_more(fp, line_count);
                fclose(fp);
            }
            else 
                fprintf(stderr, "Cannot open file: %s\n", *(argv));

    return 0;
}
