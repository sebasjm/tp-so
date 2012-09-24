#include <string.h>
#include <strings.h>
#include <regex.h>

#include <stdio.h>

regmatch_t matchs[200];

int main(int argc, char *argv[]) {
        regex_t regex;
        int reti;
/* Compile regular expression */

/* Execute regular expression */
    reti = regcomp(&regex, "^GET http://([a-z0-9\\.]*)/([a-z/]*)(.do(\\?{0,1}))(.*)( )HTTP.*$", REG_ICASE|REG_EXTENDED);
    reti = regexec(&regex, argv[1], 200, matchs, 0);
        if( !reti ){
            int i = 0;
                puts("Match");
                int len = strlen(argv[1]);
                for (; matchs[i].rm_eo != -1; i++ ) {
                    printf("    found: %d start %d end %d \n", i, matchs[i].rm_so,matchs[i].rm_eo );
                    int cut = matchs[i+1].rm_eo != -1 ? matchs[i+1].rm_so : len;
                    char save = argv[1][cut];
                    argv[1][cut] = '\0';
                    printf("%s \n", &argv[1][matchs[i].rm_so]);
                    argv[1][cut] = save;
                }
        }
        else if( reti == REG_NOMATCH ){
                puts("No match");
        }

/* Free compiled regular expression if you want to use the regex_t again */
	regfree(&regex);

        return 0;
}

