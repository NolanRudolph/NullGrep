/* 
 * Welcome to nolGrep!
 * 
 * File:   main.c
 * Author: nolan
 *
 * Created on August 28, 2018, 1:25 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <math.h>
#define MAXBUF 1000000
#define MAXLINE 100000
void inputConvert(char[]);
void countLines(void);
int argHandler(char *);
void help(void);
void delSpec(void);
void transUpLow(void);
void count(void);
void match(char *);
void totalMatches(void);
void formatGrep(char *);
void contextFormat(char *);

static int tL = 0;  // Total Lines
static int tM = 0;  // Total Matches
static int wC = 0;  // Total Words
static int cC = 0;  // Total Characters
static int bC = 0;  // Total Bits
char *buffer;  // Main Read Pointer
int *lines;  // MUST Initialize After count()
int *where;
int argMatch[8]; 
/*
 *argMatch used for matching argv[2]*
 Key: f, l, r, d, n, N, c, #
 Val: 0, 1, 2, 3, 4, 5, 6, 7
 
 f : File is passed as argv[3]
 l : User inquires word left of grep match
 r : User inquires word right of grep match
 d : Count Occurrences of each word(should work with l && r)
 n : Ignore Punctuation
 N : Ignore Capitalization
 c : Provide Quantifying Details
 # : Shift for -r and -l
*/

int main(int argc, char** argv) 
{
    
    // Buffer Is The Pointer To Read From, EVERYWHERE
    buffer = (char *)malloc(sizeof(char) * MAXBUF);
    
    // Testing Variables
    int i, j;
    
    /* Assessing Arguments */
    
    int isHelp, fill;
    
    isHelp = argHandler(argv[1]);  // Evaluate Arguments (!!)
        
    if (isHelp && argc == 2)  // Needed for --help
    {
        return 0;
    }
    if (!isHelp && argc == 2)
    {
        for (fill = 0; fill < sizeof(argMatch)/sizeof(int); ++fill)
            argMatch[fill] = 0;
    }
    
    /* End Assessing Arguments */

    
    /* Fill Buffer */

    if (argMatch[0]) // Is there's a file?
    {
        inputConvert(argv[2]);  // If so, Send to Global Pointer
    }
    else
        read(STDIN_FILENO, buffer, MAXBUF); // No File? Read Pipe / STDIN

    /* End Fill Buffer */
    
    
    /* Counting Total Number Of Lines */
    
    countLines();
    printf("Total Lines = %d\n", tL);
    
    /* End Counting Total Number Of Lines */
    
    
    /* Complying With User's Argument Specification */
    
    if (argMatch[4])   // Was -n Passed as Argument?
        delSpec();     // If so, Delete All Unwanted ASCII values
    
    if (argMatch[5])   // Was -N Passed As Argument?
        transUpLow();  // If so, Translate All Characters to Lowercase
    
    if (argMatch[6])   // Was -c Passed As Argument?
    {                  // If so, List All Quantifying Variables
        count();
        printf("\nLines: %d\n", tL);
        printf("Words: %d\n", wC);
        printf("Non-Control Code Characters: %d\n", cC);
        printf("Bits: %d -- Bytes: %d\n", bC, bC/8);
    }
    
    /* End Complying With User's Argument Specification */
    
    
    /* Matching and Regex Argument Handling */
    
    // Now We Can Initialize lines' Length Since tL is Figuratively Constant
    lines = (int *)malloc(sizeof(int) * tL);
    where = (int *)malloc(sizeof(int) * MAXBUF * 2);  // Be Weary: 0.004 GB space
    
    /* Different Argument Routes */
    if (argc == 2)         // User Only Gives Pattern
    {
        match(argv[1]);
        totalMatches(); 
        printf("Total Matches: %d\n", tM);
        formatGrep(argv[1]);
    }
    else if (argMatch[0])  // If There's a File
    {
        match(argv[3]);
        totalMatches();  
        printf("Total Matches: %d\n", tM);
        formatGrep(argv[3]);
    }
    else                   // Everything Else
    {
        match(argv[2]);
        totalMatches(); 
        printf("Total Matches: %d\n", tM);
        formatGrep(argv[2]);
    }  
    
    /* End Matching and Regex Argument Handling */
    
    free(buffer);
    free(lines);
    free(where);
    return 0;
}

void countLines(void)  // TESTED
{
    int i = 0;
    while (i < strlen(buffer))  // Stop Once i Reaches buffer's Length
        if (buffer[i++] == '\n' && tL++ != MAXLINE)  // Increments i and tL
            ;  
}

int argHandler(char* arg)  // TESTED
{
    /* Initialize all values of argMatch to 0 */
    if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0)
    {
        help();
        return 1;
    }
        
    int i, j, k;
    
    for (i = 0; i < sizeof(argMatch)/sizeof(int); ++i)
        argMatch[i] = 0;
    
    /* Increment values that match arg */
    // Key: f, l, r, d, n, N, c
    char temp[7] = "flrdnNc";
    

    int comp = 0;
    int totalComp = 0;
    int mult;
    const int zMult = 10;
    int powerTen = 0;
    for (j = 1; j < strlen(arg); ++j)
    {
        /* Warning: Viewer Discretion -- Awful Reverse Base 10 Interpreter Algorithm */
        /* This Checks for Number of Characters User Would Like to Shift*/
        
        if ((int)arg[j] > 47 && (int)arg[j] < 58)  // Between 0 And 9
        {
            ++powerTen;
            while ((int)arg[++j] > 47 && (int)arg[j] < 58)
            {
                ++powerTen;
            }
            j -= powerTen;
        }
        
        if (powerTen)
        {
            mult = (int)pow(10, powerTen - 1);
            int partTen = 1;
            
            for ( ; partTen <= powerTen; j++)
            {
                if ((int)arg[j] - '0' == 0)
                {
                    mult = (int)pow(10, powerTen - ++partTen);
                    continue;
                }
                else
                {
                    totalComp += ((int)arg[j] - '0') * mult;
                }

                mult = (int)pow(10, powerTen - ++partTen);
            }
                
        }
        argMatch[7] = totalComp;
        
        /* Awful Reverse Base 10 Interpreter Algorithm Finished*/
        
        for (k = 0; k < strlen(temp); ++k)
        {
            if (arg[j] == temp[k])
            {
                ++argMatch[k];  // Increment the corresponding position
                break;
            }
        }
    }
    return 0;
}

void help(void)  // TESTED
{
    /* Provides A Helpful Guide To User */
    // c, d, f, l, n, N, r
    printf("\n------------------------ WELCOME TO NOLGREP ------------------------\n\n");
    printf("Welcome to nolGrep! -- Your User-Friendly CLI Data Mining Command\n\n");
    printf("So you're having some troubles... Let me see if I can help!\n\n");
    printf("First and foremost, you MUST use this syntax:\n");
    printf("Usage: nolGrep [ARGUMENTS (optional)] [FILE (optional)] [PATTERN]\n\n");
    printf("The FILE aspect uses /tmp/example.txt formatting, ONLY IF -f argument.\n");
    printf("The PATTERN aspect is what exactly you're trying to find.\n\n");
    printf("If you're new to terminal, you may be asking what ARGUMENTS are.\n");
    printf("They're useful add-ons to tell your command what exactly to do.\n");
    printf("Here are all the potential arguments you may use:\n\n");
    
    printf("File input:\n");
    printf("  -f  \t Include a file as a second argument.\n\n");
    
    printf("Regexp add-ons:\n");
    printf("  -d  \t Count how many times a pattern appears.\n");
    printf("  -l  \t Grab the word -# characters left of PATTERN; Works with -d.\n");
    printf("  -r  \t Grab the word -# characters right of PATTERN; Works with -d.\n");
    printf("  -n  \t Ignore punctuation.\n");
    printf("  -N  \t Ignore capitalization sensitivity.\n");
    printf("  -#  \t The number used with -r or -l for context character push\n\n");
    
    printf("Miscellaneous:\n");
    printf("  -c  \t Print the Word, Character, and Byte Count of STDIN\n\n");

    printf("Examples of usage:\n");
    printf("  Direct File: $ nolGrep -f /tmp/example.txt findMe\n");
    printf("  Pipe Line:   $ cat /tmp/example.txt | nolGrep -drN findMe\n\n\n");
    
    printf("EXTRA NOTES:\n");
    printf("  1.) If context shifting, crossing a line break will result in the\n"
           "      original highlighting of PATTERN.\n\n");
    printf("  2.) If you context shift left/right to a non-alphabetical character,\n"
           "      the program will push you left/right respectively until you\n"
           "      come to an end line (at which see 1.) or you reach another \n"
           "      word. Therefore, please be precise in your character shifts.\n\n");
           
            
    
    printf("--------------- IF YOU'RE PLAYING THE $2 DEBUG GAME ---------------\n\n");
    printf("I award your bravery... Let me lay down some guidelines:\n\n");
    printf("1.) No Double Jeopardy\n\n");
    printf("    So you found a bug... Nice! However, if you found that \n"
           "    'runMe -f /tmp/test.txt *!example' bugs, I'm not giving you\n "
           "   credit if you send me 'runMe -f /tmp/test.txt *!exampleTwo\n "
           "   doesn't work either!'\n\n");
    printf("2.) Don't Purposefully Pass Parameters That Limit Your Search\n\n");
    printf("    If you wanted to find 'example.Pattern', passing the argument\n"
           "    -n would make it impossible. This is not a bug, just ignorance.\n\n");
    printf("3.) Don't Pass Max Buffer Space Allocation\n\n"
           "    When creating this software, I purposefully set a max line\n"
           "    buffer of 100000 and a max character buffer of 1000000. You\n"
           "    will receive a 'Segmentation Fault: Core Dumped' notification.\n"
           "    If you receive a 'Core Dump' error without surpassing my max\n"
           "    line and max character buffer, you sir found a bug.\n\n");
    printf("4.) Adhere to Syntactical Usage Defined on Line 4 of --help.\n\n");
    printf("    Make sure you're using the right syntax. You might find that\n"
           "    the command 'nolGrep -fcl3 examplePattern /tmp/example.txt'\n"
           "    does absolutely nothing. That's because the syntax should've\n"
           "    been: 'nolGrep -fcl3 /tmp/example.txt examplePattern'\n\n");
    printf("Well that concludes the guidelines. Anything else is fair game!\n"
           "When you find the bug, shoot me a venmo request @Nolan-Rudolph-1 \n"
           "for $2, with the command line prompted bug in the 'What's it for?'\n"
           "section. Happy debugging!!\n\n");
    printf("-------------------------------------------------------------------\n\n");
    return;
}

void inputConvert(char file[])  // TESTED
{
    FILE *fp;
    fp = fopen(file, "r");  // Open file passed in as argv[3]
    
    int i;
    char c;
    while((c = getc(fp)) != EOF)
    {
        buffer[i++] = c;  // Set pre-incremented buffer address to character
    }
    return;
}

void delSpec(void)  // TESTED
{
    /* Most Confusing Function Yet... 
     * All there is to know is that it uses a backtrack as
     * a relay point to compensate for skipped characters*/
    
    int i, c, temp, backTrack;
    
    backTrack = 0;
    for (i = 0; i < strlen(buffer) + backTrack; ++i)
    {
        if (((c = (int)buffer[i]) > 32 && c < 48) || (c > 90 && c < 97) ||
                (c > 57 && c < 65))  // Skip Unwanted ASCII Symbol Values
        {
            backTrack++;
            continue;
        }

        buffer[i - backTrack] = buffer[i];  // Organize for fluidity
    }
    return;
}

void transUpLow(void)  // TESTED
{
    int i, c;
    for (i = 0; i < strlen(buffer); ++i)
    {
        if((c = (int)buffer[i]) > 64 && c < 91)  // Is it Uppercase? (ASCII)
            buffer[i] = (char)((int)buffer[i] + 32);  // Then make it Lowercase
    }
    return;
}

void count(void)  // Must ALWAYS be called after argHandler() TESTED
{
    int i;  // Let's Not Increment Again if File
    char c;
    cC = strlen(buffer);
    
    for (i = 0; i < strlen(buffer); ++i)
    {
        if ((c = buffer[i]) == ' ' || c == '\n')  // Up wC if New Word
            ++wC;
        if (c == '\n')  // This is Why We Made Variable "File"
        {
            --cC;
        }
    }
    
    bC = strlen(buffer) * 8;
}

void match(char *match)
{
    /* Initialize all values of lines to 0 */
    int i;
    for (i = 0; i < tL; ++i)
        lines[i] = 0;
    
    /* Let's Start Searching For Matches */
    int j, m; 
    int k = 0;
    int matched = 0;
    int matchI = 0;
    char c;
    
    /* Grep Shift Variables */
    int temp, tempK, nC, tempR;
    
    for (j = 0; j < tL; ++j)  // Nest 0 : Lines of Files
    {
        matched = 0;
        
        for (m = 0; c = buffer[k]; ++k)  // Nest 1: Look Until "\n"
        {

            if (strlen(match) == m)  // Condition 1: Full Match is Met
            {
                /* Start Left Shift Grep */
                if (argMatch[1])  // If -l (Left Shift) Parameter
                {
                    
                    tempK = k - strlen(match);
                    temp = k - strlen(match) - argMatch[7];
                    
                    /* Finding Good Starting Spot */
                    for (tempR = 0 ; --tempK >= temp ; )
                    {
                        if ((nC = buffer[tempK]) == '\n')
                            goto normyGrepL;
                        
                        if ((nC = buffer[tempK]) > 96 && nC < 123 || 
                                nC > 64 && nC < 91)
                            tempR = tempK;

                    }
                    
                    if (tempR)  // If We Have A Good Place, Let's Evaluate Word
                    {
                        while((nC = buffer[--tempR]) > 96 && nC < 123 || 
                                nC > 64 && nC < 91)
                            ;
                        ++tempR;
                        where[matchI++] = tempR;  // Set to Beginning of Word
                        
                        while((nC = buffer[++tempR]) > 96 && nC < 123 ||
                                nC > 64 && nC < 91)
                            ;
                        --tempR;
                        where[matchI++] = tempR;  // Set to End of Word
                    }
                    else        // We Never Found A Place, Back To Normal Grep
                    {
                        normyGrepL:
                        where[matchI++] = k - strlen(match);
                        where[matchI++] = k - 1;
                    }
                }
                /* End Left Shift Grep */
                
                
                /* Start Right Shift Grep */
                else if (argMatch[2])  // If -r (Right Shift) Parameter
                {
                    // if no match: where = k - strlen(match);
                    tempK = k;
                    temp = k + argMatch[7];
                    
                    /* Finding Good Starting Spot */
                    for (tempR = 0 ; temp >= ++tempK ; )
                    {
                        if ((nC = buffer[tempK]) == '\n')
                            goto normyGrepR;
                        
                        if ((nC = buffer[tempK]) > 96 && nC < 123 || 
                                nC > 64 && nC < 91)
                            tempR = tempK;

                    }
                    
                    if (tempR)  // If We Have A Good Place, Let's Evaluate Word
                    {
                        while((nC = buffer[--tempR]) > 96 && nC < 123 || 
                                nC > 64 && nC < 91)
                            ;
                        ++tempR;
                        where[matchI++] = tempR;  // Set to Beginning of Word
                        
                        while((nC = buffer[++tempR]) > 96 && nC < 123 ||
                                nC > 64 && nC < 91)
                            ;
                        --tempR;
                        where[matchI++] = tempR;  // Set to End of Word
                    }
                    else        // We Never Found A Place, Back To Normal Grep
                    {
                        normyGrepR:
                        where[matchI++] = k - strlen(match);
                        where[matchI++] = k - 1;
                    }
                }
                /* End Right Shift Grep */
                
                /* Start Normal Grep */
                else  
                {
                    where[matchI++] = k - strlen(match);
                    where[matchI++] = k - 1;
                }
                /* End Normal Grep */
                
                /* End of If Statement Happenings*/
                
                ++matched;
                m = 0;
                
                /* End of If Statement */
            }
            if (c == match[m])  // Condition 2: Part of the Match is Met
            {
                ++m;
                continue;
            }
            else  // Condition 3: Nothing is Met
            {
                m = 0;
            }
            if (c == '\n')
                break;
        }
        ++k;  // Otherwise Program will get caught on "\n"
        lines[j] += matched;  
    }
    
}

void totalMatches(void)
{
    int i;
    for (i = 0; i < tL; ++i)
    {
        tM += lines[i];
    }
}


void formatGrep(char *pattern)
{
    /* Count Effective Lines */
    int i, tempS, tempE;
    int j = 0;
    int matchI = 0;
    int strI;
    char c;
    
    int nCounter = 0;
    for (i = 0; i < tL; ++i)
    {
        if (lines[i])  // If This is a Line We Want to Include
        {
            printf("Line %d: ", i + 1);
            for ( ; (c = buffer[j]) != '\n'; ++j) 
            {
                if (j == (tempS = where[matchI]))  // If the Index is the Same as Match
                {
                    tempE = where[++matchI];
                    for ( ; tempS <= tempE ; ++tempS, ++j) 
                    {
                        printf("\033[38;5;206m%c\033[0m", buffer[j]);  // PINK
                    }
                    --j, ++matchI;  // For Next Iteration
                }
                else
                    printf("%c", c);  // If it's Not the Match, Print Normal
            }
            printf("\n");
            ++j;  // Avoid Program Getting Stuck on "\n"
        }
        else  // If it's Not a Line We're Interested in...
        {
            while(buffer[j++] != '\n')  // Just Skip Everything Until Next Line
                ;
        }
    }
    printf("\n");
    return;
}