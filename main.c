#include <stdio.h>
#include <stdlib.h>

#define MAXLINE 30 //Maximum input line - needed to initialize input_line[] array
#define MAXLEN 15 //Maximum length of output line, that we print on the screen, output_line[] array length

int getline(char input_line[], char output_line[]);
int copy(char from[], char to[], int start, int stop);
int check_spaces(char line[], int j);
int search_last_space(char line[], int last_char_before_new_string, int line_len);
void fold_line(char input_line[], char output_line[], int length, int finish_string);

int current_pos = 0;
char buffer[MAXLEN];
int buffer_len = 0;

int main()
{
    int len = 0;
    char input_line[MAXLINE];
    char output_line[MAXLEN+2];

    //for debugging purposes:
    printf("length=\n01234567890123456789012345678900123456789012345678901234567890\n");

    while ((len = getline(input_line, output_line)) > 0) {
        fold_line(input_line, output_line, len, 1);
    }
    return 0;
}

int getline(char input_line[], char output_line[]){
    int i = 0;
    int c = 0;
    extern int buffer_len;
    extern char buffer[];
    while ((c = getchar()) != EOF && c != '\n'){
        if (i == MAXLINE) {
            input_line[i] = c;
            fold_line(input_line, output_line, MAXLINE+1, 0); //intermediate folding for very long strings (more than the size of an array)
            i = 0;

			//if intermediate folding finished with some buffer (characters of one word that started in one input_line[] and didn't end there)
			//we put those characters in a new input_line[] first
            if (buffer_len > 0){
                for (i = 0; i < buffer_len; i++){
                    input_line[i] = buffer[i];
                }
                buffer_len = 0;
            }
        } else {
            input_line[i]=c;
            i++;
        }
    }

    if (c == '\n'){
        input_line[i] = c;
        i++;
    }
    if (c == EOF && i == 0){
        return 0;
    }
    input_line[i] = '\0';
    return i+1;
}

// We actually don't use this function anymore. Because we needed different
// variations of this function inside fold_line() function
int copy(char from[], char to[], int start_point, int end_point) {
    int i = 0;
    int j = 0;
    for (i = start_point; i < end_point && from[i] != '\0'; i++) {
        to[j] = from[i];
        if (from[i] == ' ')
            to[j] = '_';
        j++;
    }
    current_pos = j; // Last position of the cursor on the display;
    return j;
}

void fold_line(char input_line[], char output_line[], int len, int finish_string) {
    int skip_copying_to_output;
    int last_char_before_new_string = 0;
    int prev_point = 0;
    int i, k;
    extern int current_pos;
    int j = current_pos;
    int last_space;
    extern int buffer_len;
    extern char buffer[];

    while ((last_char_before_new_string + MAXLEN) < len){
        skip_copying_to_output = 0;
        prev_point = last_char_before_new_string;

        // The main and the simplest part of the fold_line() function. We're searching for the last ' ' every MAXLEN characters
        // j and buffer_len - are used when there are multiple input_line[] arrays and we need to concatenate last words from one
        // input_line[] and first words from another input_line[] IN ONE OUTPUT line :)
        for (i = prev_point; i < (prev_point + MAXLEN - j - buffer_len) && i < len; i++) {
            if (input_line[i] == ' '){
                last_char_before_new_string = i + 1;
            }
        }
        // In case we got no words (string of blanks for example) or if one word fills all of the line (and more)
        // we just pass to the output full length line (= MAXLEN)
        // otherwise we are getting infinite while loop

        // But! We can have very special case if we're in the process of folding a line from TWO different arrays
        // caused by very long input string.
        // j > 0 will point out on that case.
        if (last_char_before_new_string - prev_point == 0){
            if (j == 0){
                last_char_before_new_string += MAXLEN; // can be optimized
            } else if (j > 0){
                if (check_spaces(output_line, j) == 1){ // There were blanks in the last line of the prev. input_line but
                // the first word in new input_line[] is too long (no new blanks) and we need to pass it to a new line
                    output_line[j] = '\n';
                    output_line[j+1] = '\0';
                    j = 0;
                    printf("%s", output_line);
                    skip_copying_to_output = 1;
                } else {
                    last_char_before_new_string += (MAXLEN - j); // can be optimized
                }
            }
        }

    //Copying part of the input_line to the output_line and printing it out
        if (skip_copying_to_output == 0){
            for (i = prev_point; i < last_char_before_new_string; i++) {
                if (input_line[i] == ' ') {
                    output_line[j] = '_';
                } else {
                    output_line[j] = input_line[i];
                }
                j++;
            }
            output_line[j] = '\n';
            output_line[j + 1] = '\0';
            j = 0;
            printf("%s", output_line);
        }
    }

    // Last line or the only one line case
    // We must search for the last ' ' in current input_line. Because we can have a word that starts
    // in one input_line[] and ends in the next input_line. The exception is when this is the LAST input_line
    // and we fold it with argument (finish_string = 1) - then we take all remained characters of input_line
    if (finish_string == 1){
        last_space = len;
    } else {
        last_space = search_last_space(input_line, last_char_before_new_string, len);
    }

    for (i = last_char_before_new_string; i < last_space; i++) {
        output_line[j] = input_line[i];
        if (input_line[i] == ' ')
            output_line[j] = '_';
        j++;
    }

    //Buffer array that holds characters of word that starts in one input_line[] and ends in another input_line[]
    if (last_space < len){
        k = 0;
        for (i = last_space; i < len; i++){
            buffer[k] = input_line[i];
            k++;
        }
        buffer_len = k;
    }
// This strange part of the code with this function argument "finish_string" is needed only because
// we have to deal with very large input strings, which can full our input_line[] array multiple times.

// When we're folding intermediate input_line[] array (not the last one), we don't print our last output_line[]
// of this array (i.e. this part of the whole input string), but we remember last cursor position after printer character
// for the next part of input string (i.e. next input_line[]) to continue from that place

// When we're calling fold_line() function for the last time (from the main() function) then we pass finish_string=1 argument
// to the fold_line() function, so it knows that this time last string must be closed with '\n' and '\0' and it must be printed
    if (finish_string == 1) {
        output_line[j] = '\n';
        output_line[j+1] = '\0';
        j = 0;
        printf("%s", output_line);
    }
    current_pos = j; // Last position of the cursor on the display (after printed character!);
}

int check_spaces(char line[], int j){
    int i;
    for (i = (j-1); i >= 0; i--){
        if (line[i] == '_' || line[i] == '_'){
            return 1;
        }
    }
    return 0;
}

int search_last_space(char line[], int last_char_before_new_string, int line_len){
    int last_space = line_len;
    int i;
    for (i = last_char_before_new_string; i < line_len; i++) {
        if (line[i] == ' '){
            last_space = i + 1;
        }
    }
    return last_space;
}
