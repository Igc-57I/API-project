#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// var used to temp store input (global because is easy to use)
char input[18];

// store the dictionary
char* dictionary;
u_int32_t hash_dim; // dim of the table
float count = 0; // element in the table
float load_factor = 0; // hash table load factor

// variable used to store the filtered words
char* filtered_words = NULL;
u_int32_t end_filt_word = 0;
u_int32_t filt_words_num = 0;
bool need_to_order;

// variables used to store the constrains learned
struct char_constrains
{
    bool not_present;
    u_int64_t must_be_here;
    u_int64_t not_here;
    u_int32_t num_min;
    u_int32_t num_max;
};
struct char_constrains constrains[64];
u_int64_t constrains_modified = 0;

// FUNCTIONS PROTOTYPES
// hash function
u_int64_t hash(char* word, u_int32_t len, u_int32_t step);

// function to reallocate a larger hash table
u_int32_t rehash(u_int32_t w_len);

// allocate hash table for dictionary and inizialize it
u_int32_t initialize_dictionary(u_int32_t w_len);

// research in the dictionary
bool find(char* word, u_int32_t len);

// function to reset the global values to default
void reset_game(u_int32_t len);

// function used to convert a char to the corresponding int index for arrays
u_int32_t char_map(char c);

// reverse of char map
char reverse_char_map(u_int32_t index);

// function to play a new game
bool new_game(u_int32_t new_game_len);

// function used to apply filter at the filtered array
u_int32_t word_filter(u_int32_t len);

// merge function for merge sort
void merge(u_int32_t first, u_int32_t mid, u_int32_t last, u_int32_t len);

// merge sort to order the filtered words array
void merge_sort(u_int32_t first, u_int32_t last, u_int32_t len);

// function to insert a word in the filtered array
void insert_filtered(char* word, u_int32_t len);

// function to remove a word in the filtered array
void remove_filtered(u_int32_t pos, u_int32_t len);

// function to print filtered word
void stampa_filtrate(u_int32_t len);

// function to insert new words in the dictionary
void inserisci_inizio(u_int32_t len);

// function used to call the right function based on the current input
void caller(char curr_input[18], u_int32_t new_game_len);


int main(int argc, char const *argv[])
{
    u_int32_t len;

    // getting word lenght
    if (fscanf(stdin, "%d", &len) == EOF)
    {
        return 0;
    }
    
    // creating dictionary
    hash_dim = initialize_dictionary(len);

    // initialize filters
    for (size_t i = 0; i < 64; i++)
    {
        constrains[i].not_present = false;
        constrains[i].must_be_here = 0;
        constrains[i].not_here = 0;
        constrains[i].num_min = 0;
        constrains[i].num_max = __UINT32_MAX__;
    }

    // printf("%d", hash_dim);

    // printf("\nDOCTIONARY INITIALIZED\n");
    // for (size_t i = 0; i < hash_dim; i++)
    // {
    //     if (dictionary[i * (len + 1)] != 0)
    //     {
    //         printf("%s\n", &dictionary[i * (len + 1)]);
    //     }
    //     else
    //     {
    //         printf("empty\n");
    //     }
    // }

    // select the proper function for the command
    caller(input, len);
    while (fscanf(stdin, "%s", input) != EOF)
    {
        caller(input, len);
    }

    // free all
    free(dictionary);

    return 0;
}

// hash function
u_int64_t hash(char* word, u_int32_t len, u_int32_t step)
{
    u_int64_t h1 = 0;
    u_int64_t h2 = 0;
    const int m = 1e9 + 9;
    u_int64_t pow = 1;

    // calculating first part of hash function (polynomial rolling p = 67 m = 10^9 + 9)
    for (size_t i = 0; i < len; i++)
    {
        h1 = (h1 + (word[i] - '-' + 1) * pow) % m;
        pow = (pow * 67) % m;
    }
    
    // only if step != 0
    if (step != 0)
    {
        // calculating second part of hash function (polynomial rolling p = 61 m = 10^9 + 9)
        pow = 1;
        for (size_t i = 0; i < len; i++)
        {
            h2 = (h2 + (word[i] - '-' + 1) * pow) % m;
            pow = (pow * 61) % m;
        }
        if (h2 % 2 == 0)
        {
            h2++;
        }
    }

    return (h1 + step * h2) % hash_dim;
}

// function to reallocate the hash table if load factor >= 0.75
u_int32_t rehash(u_int32_t w_len)
{
    char* word_stack;
    u_int32_t stack_el_count = 0;
    u_int32_t end_stack = 0;

    word_stack = (char*)malloc(hash_dim * (w_len + 1) * sizeof(char));
    memset(word_stack, 0, hash_dim * (w_len + 1));
    u_int32_t hash_value;

    // creating the list of element to be insert in the new table
    for (size_t i = 0; i < hash_dim; i++)
    {
        if (dictionary[i * (w_len + 1)] != 0)
        {
            if(strcpy(&word_stack[end_stack], &dictionary[i * (w_len + 1)]) == 0)
            {
                printf("error/n");
            }

            stack_el_count++;
            end_stack += (w_len + 1);
        }
    }

    // duplicate table dimension
    hash_dim = hash_dim<<1;

    // reallocating hash table
    dictionary = (char*)realloc(dictionary, sizeof(char) * hash_dim * (w_len + 1));
    memset(dictionary, 0, sizeof(char) * hash_dim * (w_len + 1));

    for (size_t i = 0; i < stack_el_count; i++)
    {
        u_int32_t j = 0;
        // for each word find a free place in the table
        hash_value = hash(&word_stack[i * (w_len + 1)], w_len, j);
        while (dictionary[hash_value * (w_len + 1)] != 0)
        {
            j++;
            hash_value = hash(&word_stack[i * (w_len + 1)], w_len, j);
        }

        if(strcpy(&dictionary[hash_value * (w_len + 1)], &word_stack[i * (w_len + 1)]) == 0)
        {
            printf("error/n");
        }
    }
    
    // free of the support stack
    free(word_stack);

    // return the new dimension
    return hash_dim;
}

u_int32_t initialize_dictionary(u_int32_t w_len)
{
    // calculating dim
    hash_dim = 1<<17;

    // allocating table
    dictionary = (char*)malloc(sizeof(char) * hash_dim * (w_len + 1));
    memset(dictionary, 0, sizeof(char) * hash_dim * (w_len + 1));
    
    // getting words and putting in table
    u_int64_t hash_value, i;
    while (fscanf(stdin, "%s", input) != EOF && input[0] != '+') // getting word till the first command starting with '+'
    {
        i = 0;

        // if hash table is too full rehash
        if (load_factor >= 0.75f)
        {
            hash_dim = rehash(w_len);
        }

        // for each word find a free place in the table
        hash_value = hash(input, w_len, i);
        while (dictionary[hash_value * (w_len + 1)] != 0)
        {
            i++;
            hash_value = hash(input, w_len, i);
        }

        // printf("word = %s, step = %d\n", input, i);
        
        // allocating the space and insert
        if(strcpy(&dictionary[hash_value * (w_len + 1)], input) == 0)
        {
            printf("error/n");
        }

        // printf("inserita = %s\n", *(dictionary + hash_value));

        // load factor update
        count++;
        load_factor = count / hash_dim;

        // printf("count = %f, load_f = %f\n\n", count, load_factor);
    }
    
    return hash_dim;
}

// research in the dictionary
bool find(char* word, u_int32_t len)
{
    u_int32_t i = 0;
    u_int32_t hash_val = hash(word, len, i);
    // printf("ora %s\n", &dictionary[hash_val * (len +1)]);
    while (dictionary[hash_val * (len + 1)] != 0)
    {
        // printf("dic %s, input %s\n", &dictionary[hash_val * (len +1)], word);
        if (strcmp(&dictionary[hash_val * (len + 1)], word) == 0)
        {
            return true;
        }

        i++;
        hash_val = hash(word, len, i);
    }
    
    return false;
}

// function to reset the global values to default
void reset_game(u_int32_t len)
{
    // resetting the filtered word array for the new game
    for (size_t i = 0; i < 64; i++)
    {
        constrains[i].not_present = false;
        constrains[i].must_be_here = 0;
        constrains[i].not_here = 0;
        constrains[i].num_min = 0;
        constrains[i].num_max = __UINT32_MAX__;
    }
    constrains_modified = 0;

    // resetting filtered word array
    free(filtered_words);
    filtered_words = NULL;
    end_filt_word = 0;
    filt_words_num = 0;
    need_to_order = true;
}

// function used to convert a char to the corresponding int index for arrays
u_int32_t char_map(char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return (c - 'a' + 38);
    }
    else if (c >= 'A' && c <= 'Z')
    {
        return (c - 'A' + 12);
    }
    else if (c >= '0' && c <= '9')
    {
        return (c - '0' + 2);
    }
    else if (c == '_')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// reverse of char map
char reverse_char_map(u_int32_t index)
{
    if (index >= 38 && index <= 63)
    {
        return (index + 'a' - 38);
    }
    else if (index >= 12 && index <= 37)
    {
        return (index + 'A' - 12);
    }
    else if (index >= 2 && index <= 11)
    {
        return (index + '0' - 2);
    }
    else if (index == 1)
    {
        return '_';
    }
    else
    {
        return '-';
    }
}

// function to play a new game
bool new_game(u_int32_t new_game_len)
{
    // storing the goal word & the number of chars
    char goal[new_game_len + 1];
    u_int32_t goal_chars_num[64];
    u_int32_t const_goal_chars_num[64];
    memset(const_goal_chars_num, 0, 64 * sizeof(u_int32_t));
    memset(goal_chars_num, 0, 64 * sizeof(u_int32_t));
    
    // used to store chars num of the current word
    u_int32_t curr_word_num[64];
    memset(curr_word_num, 0, 64 * sizeof(u_int32_t));

    // storing the answer to print to the current input word
    char answer[new_game_len + 1];
    memset(answer, 0, (new_game_len + 1) * sizeof(char));
    answer[new_game_len] = '\0';

    // used to make bit logical operetions
    u_int64_t mask = 0;

    // used as index for arrays
    u_int32_t index;

    // storing the goal word for the current game
    if (fscanf(stdin, "%s", goal) == EOF)
    {
        printf("error\n");
    }
    // setting const goal chars num (used to update filters)
    for (size_t i = 0; i < new_game_len; i++)
    {
        const_goal_chars_num[char_map(goal[i])]++;
    }

    // printf("constgoalcharnum\n");
    // for (size_t i = 0; i < 64; i++)
    // {
    //     printf("%ld: %d | ", i, const_goal_chars_num[i]);
    // }
    // printf("\n");
    

    // storing the max number of try
    u_int32_t max_try;
    if (fscanf(stdin, "%d", &max_try) == EOF)
    {
        printf("error\n");
    }

    // printf("goal %s, max try %d\n", goal, max_try);

    // looping for max_try times
    for (size_t i = 0; i < max_try; i++)
    {
        // getting the word
        if (fscanf(stdin, "%s", input) == EOF)
        {
            return false;
        }

        // printf("curr %s\n", input);

        if (input[0] != '+') // input is not a command
        {
            // if the input is not part of the dictionary
            if (find(input, new_game_len) == false)
            {
                printf("not_exists\n");
                i--;
            }
            // the input is the goal word
            else if (strcmp(input, goal) == 0)
            {
                printf("ok\n");
                reset_game(new_game_len);
                return true;
            }
            else
            {
                memset(answer, 0, (new_game_len + 1) * sizeof(char));
                answer[new_game_len] = '\0';
                memset(curr_word_num, 0, 64 * sizeof(u_int32_t));
                memset(goal_chars_num, 0, 64 * sizeof(u_int32_t));
                constrains_modified = 0;
                
                // setting goal chars num (used to update filters)
                for (size_t i = 0; i < new_game_len; i++)
                {
                    goal_chars_num[char_map(goal[i])]++;
                }

                // // print goal chars num
                // for (size_t i = 0; i < 64; i++)
                // {
                //     printf("%ld: %d | ", i, goal_chars_num[i]);
                // }
                // printf("\n");
                

                // looping through the input word for find correct word in correct place
                for (size_t i = 0; i < new_game_len; i++)
                {
                    if (input[i] == goal[i])
                    {
                        answer[i] = '+';
                        mask = 1;
                        mask<<=i;
                        index = char_map(input[i]);
                        if ((constrains[index].must_be_here&mask) == 0)
                        {
                            constrains[index].must_be_here = constrains[index].must_be_here^mask;
                        }
                        goal_chars_num[index]--;
                        curr_word_num[index]++;
                        mask = 1;
                        mask<<=index;
                        if ((constrains_modified&mask) == 0)
                        {
                            constrains_modified = constrains_modified^mask;
                        }
                    }
                    else
                    {
                        mask = 1;
                        mask<<=i;
                        index = char_map(input[i]);
                        if ((constrains[index].not_here&mask) == 0)
                        {
                            constrains[index].not_here = constrains[index].not_here^mask;
                        }
                        curr_word_num[index]++;
                        mask = 1;
                        mask<<=index;
                        if ((constrains_modified&mask) == 0)
                        {
                            constrains_modified = constrains_modified^mask;
                        }
                    }
                }

                // looping through the word to look for other letters
                for (size_t i = 0; i < new_game_len; i++)
                {
                    if (answer[i] != '+')
                    {
                        index = char_map(input[i]);
                        if (goal_chars_num[index] > 0)
                        {
                            goal_chars_num[index]--;
                            answer[i] = '|';
                        }  
                        else
                        {
                            answer[i] = '/';
                        }
                    }
                }

                // // print curr word num
                // for (size_t i = 0; i < 64; i++)
                // {
                //     printf("%ld: %d | ", i, curr_word_num[i]);
                // }
                // printf("\n");

                // looking for the number of chars in the word
                for (size_t i = 0; i < 64; i++)
                {
                    // this char is present in the input
                    if (curr_word_num[i] != 0)
                    {
                        // less amount of the char in the input than in the goal
                        if (curr_word_num[i] > constrains[i].num_min && curr_word_num[i] <= const_goal_chars_num[i])
                        {
                            constrains[i].num_min = curr_word_num[i];
                            mask = 1;
                            mask<<=i;
                            if ((constrains_modified&mask) == 0)
                            {
                                constrains_modified = constrains_modified^mask;
                            }
                        }
                        // more chars than the goal
                        else if (curr_word_num[i] > const_goal_chars_num[i])
                        {
                            constrains[i].num_min = const_goal_chars_num[i];
                            constrains[i].num_max = const_goal_chars_num[i];
                            if (const_goal_chars_num[i] == 0)
                            {
                                constrains[i].not_present = true;
                                for (size_t j = 0; j < new_game_len; j++)
                                {
                                    mask = 1;
                                    mask<<=j;
                                    if ((constrains[i].not_here&mask) == 0)
                                    {
                                        constrains[i].not_here = constrains[i].not_here^mask;
                                    }
                                }
                                
                            }
                            
                            mask = 1;
                            mask<<=i;
                            if ((constrains_modified&mask) == 0)
                            {
                                constrains_modified = constrains_modified^mask;
                            }
                        }
                    }
                }

                // print constrains
                // for (size_t i = 0; i < 64; i++)
                // {
                //     printf("%ld, %c:\nnot present = %d | num min = %d | num max = %d\n", i, reverse_char_map(i), constrains[i].not_present, constrains[i].num_min, constrains[i].num_max);
                //     for (size_t j = 0; j < new_game_len; j++)
                //     {
                //         printf("pos = %ld: ", j);
                //         mask = 1;
                //         mask<<=j;
                //         if ((constrains[i].must_be_here&mask) != 0)
                //         {
                //             printf("must be here = 1 | ");
                //         }
                //         else
                //         {
                //             printf("must be here = 0 | ");
                //         }

                //         if ((constrains[i].not_here&mask) != 0)
                //         {
                //             printf("not here = 1\n");
                //         }
                //         else
                //         {
                //             printf("not here = 0\n");
                //         }
                //     }
                    
                // }
                // printf("constrains modified\n");
                // for (size_t i = 0; i < 64; i++)
                // {
                //     mask = 1;
                //     mask<<=i;
                //     if ((constrains_modified&mask) != 0)
                //     {
                //         printf("modified %ld = %c | ", i, reverse_char_map(i));
                //     }
                    
                // }
                // printf("\n");
                
                

                // print the answer
                printf("%s\n%d\n", answer, word_filter(new_game_len));
            }
        }
        else if (input[1] == 's') // input is '+stmpa_filtrate'
        {
            i--;
            caller(input, new_game_len);
        }
        else // input is '+inserisci_inizio'
        {
            i--;
            caller(input, new_game_len);
        }        
    }

    reset_game(new_game_len);

    return false;
}

u_int32_t word_filter(u_int32_t len)
{
    bool insert = true;
    bool remove = false;

    // variable used for bitwise opertions
    u_int64_t mask = 0;

    // used to store the occurrencies of the current chars
    u_int32_t num;

    // if there arent modified contrains that mean that all tha current word are valid
    if (constrains_modified == 0)
    {
        return filt_words_num;
    }

    // not yet filtered words
    if (filtered_words == NULL)
    {
        // allocating spaces for the filtered words
        filtered_words = (char*)malloc(hash_dim * (len + 1) * sizeof(char));
        memset(filtered_words, 0, hash_dim * (len + 1) * sizeof(char));

        // inserting all words in the hash in the filtered array
        for (size_t i = 0; i < hash_dim; i++)
        {
            if (dictionary[i * (len + 1)] != 0)
            {
                insert = true;

                // cicling on the filters
                for (size_t j = 0; j < 64; j++)
                {
                    mask = 1;
                    mask<<=j;
                    if ((constrains_modified&mask) != 0)
                    {
                        num = 0;
                        for (size_t k = 0; k < len; k++)
                        {
                            mask = 1;
                            mask<<=k;
                            if (dictionary[i * (len + 1) + k] == reverse_char_map(j))
                            {
                                if (constrains[j].not_present == true)
                                {
                                    insert = false;
                                    // printf("1: ");
                                    // printf("char = %c: ", reverse_char_map(j)); 
                                    break;
                                }
                                if ((constrains[j].not_here&mask) != 0)
                                {
                                    insert = false;
                                    // printf("2: ");
                                    // printf("char = %c: ", reverse_char_map(j)); 
                                    break;
                                }
                                num++;
                            }
                            else if (dictionary[i * (len + 1) + k] != reverse_char_map(j)
                                    && (constrains[j].must_be_here&mask) != 0)
                            {
                                insert = false;
                                // printf("3: ");
                                // printf("char = %c: ", reverse_char_map(j)); 
                                break;
                            }
                        }

                        if (insert == false)
                        {
                            // printf("4: ");
                            // printf("char = %c: ", reverse_char_map(j)); 
                            break;
                        }
                        
                        if (num < constrains[j].num_min || num > constrains[j].num_max)
                        {
                            insert = false;
                            // printf("5: ");
                            // printf("char = %c: ", reverse_char_map(j)); 
                            break;
                        }
                    }
                    
                }

                // if (insert == false)
                // {
                //     printf("not inserting %s\n", &dictionary[i * (len + 1)]);
                // }
                

                // inserting
                if (insert == true)
                {
                    // printf("inserting %s\n", &dictionary[i * (len + 1)]);
                    insert_filtered(&dictionary[i * (len + 1)], len);
                }
            }
        }
    }
    else
    {
        for (size_t i = 0; i <= end_filt_word; i++)
        {
            if (filtered_words[i * (len + 1)] != 0)
            {
                remove = false;

                // cicling on the filters
                for (size_t j = 0; j < 64; j++)
                {
                    mask = 1;
                    mask<<=j;
                    if ((constrains_modified&mask) != 0)
                    {
                        num = 0;
                        for (size_t k = 0; k < len; k++)
                        {
                            mask = 1;
                            mask<<=k;                            
                            if (filtered_words[i * (len + 1) + k] == reverse_char_map(j))
                            {
                                if (constrains[j].not_present == true)
                                {
                                    // printf("1: ");
                                    // printf("char = %c: ", reverse_char_map(j)); 
                                    remove = true;
                                    break;
                                }
                                if ((constrains[j].not_here&mask) != 0)
                                {
                                    // printf("2: ");
                                    // printf("char = %c: ", reverse_char_map(j)); 
                                    remove = true;
                                    break;
                                }
                                num++;
                            }
                            else if (filtered_words[i * (len + 1) + k] != reverse_char_map(j)
                                    && (constrains[j].must_be_here&mask) != 0)
                            {
                                // printf("3: ");
                                // printf("char = %c: ", reverse_char_map(j)); 
                                remove = true;
                                break;
                            }
                        }

                        if (remove == true)
                        {
                            // printf("4: ");
                            // printf("char = %c: ", reverse_char_map(j)); 
                            break;
                        }
                        
                        if (num < constrains[j].num_min || num > constrains[j].num_max)
                        {
                            // printf("5: ");
                            // printf("char = %c: ", reverse_char_map(j)); 
                            remove = true;
                            break;
                        }
                    }
                }

                // if (remove == false)
                // {
                //     printf("not removing %s\n", &filtered_words[i * (len + 1)]);
                // }

                // removing
                if (remove == true)
                {
                    // printf("removing %s\n", &filtered_words[i * (len + 1)]);
                    remove_filtered(i, len);
                }
            }
        }
    }

    // returning number of filtered array elements
    return filt_words_num;
}

// merge function for merge sort
void merge(u_int32_t first, u_int32_t mid, u_int32_t last, u_int32_t len)
{
    // calculating dim of the 2 subarrays
    u_int32_t n1 = mid - first + 1;
    u_int32_t n2 = last - mid;

    // printf("MERGE: first %d, mid %d, last %d | n1 %d, n2 %d\n", first, mid, last, n1, n2);

    // allocating and inizializing support array
    char* support = (char*)malloc(((last - first + 1) * (len + 1) + 2) * sizeof(char));
    memset(support, 0, ((last - first + 1) * (len + 1) + 2) * sizeof(char));
    for (size_t i = 0; i < n1; i++)
    {
        if (strcpy(&support[i * (len + 1)], &filtered_words[(first + i) * (len + 1)]) == 0)
        {
            printf("error\n");
        }
    }
    for (size_t i = 0; i < n2; i++)
    {
        if (strcpy(&support[(n1 * (len + 1) + 1) + i * (len + 1)], &filtered_words[(mid + 1 + i) * (len + 1)]) == 0)
        {
            printf("error\n");
        }
    }

    // printf("LEFT\n");
    // for (size_t i = 0; i < n1; i++)
    // {
    //     printf("%s\n", &support[i * (len + 1)]);
    // }
    // printf("RIGHT\n");
    // for (size_t i = 0; i < n2; i++)
    // {
    //     printf("%s\n", &support[(n1 * (len + 1) + 1) + i * (len + 1)]);
    // }
    
    u_int32_t i = 0;
    u_int32_t j = n1 * (len + 1) + 1;

    for (size_t k = first; k <= last; k++)
    {
        // printf("i = %d, j = %d\n", i, j);
        if (support[i] == 0 && support[j] == 0)
        {
            free(support);
            return;
        }
        else if (support[i] == 0)
        {
            if (strcpy(&filtered_words[k * (len + 1)], &support[j]) == 0)
            {
                printf("error\n");
            }
            j+=(len + 1);
        }
        else if (support[j] == 0)
        {
            if (strcpy(&filtered_words[k * (len + 1)], &support[i]) == 0)
            {
                printf("error\n");
            }
            i+=(len + 1);
        }
        else if (strcmp(&support[i], &support[j]) < 0)
        {
            if (strcpy(&filtered_words[k * (len + 1)], &support[i]) == 0)
            {
                printf("error\n");
            }
            i+=(len + 1);
        }
        else
        {
            if (strcpy(&filtered_words[k * (len + 1)], &support[j]) == 0)
            {
                printf("error\n");
            }
            j+=(len + 1);
        }
    }

    free(support);
}

// merge sort to order the filtered words array
void merge_sort(u_int32_t first, u_int32_t last, u_int32_t len)
{
    if (last == first)
    {
        return;
    }
    else
    {   
        u_int32_t mid = (last + first)/2;
        // printf("sort: first %d, mid %d. last %d\n", first, mid, last);
        merge_sort(first, mid, len);
        merge_sort(mid + 1, last, len);

        merge(first, mid, last, len);
    }
}

// function to insert a word in the filtered array
void insert_filtered(char* word, u_int32_t len)
{
    // first element
    if (filtered_words[0] == 0)
    {
        if (strcpy(&filtered_words[0], word) == 0)
        {
            printf("error\n");
        }
    }
    // for any other element increase end_filt and insert
    else
    {
        end_filt_word++;
        if (strcpy(&filtered_words[end_filt_word * (len + 1)], word) == 0)
        {
            printf("error\n");
        }
    }

    filt_words_num++;
    need_to_order = true;
}

// function to remove a word in the filtered array
void remove_filtered(u_int32_t pos, u_int32_t len)
{
    // remove the word
    memset(&filtered_words[pos * (len + 1)], 0, len * sizeof(char));
    filt_words_num--;
}

// function to print filtered word
void stampa_filtrate(u_int32_t len)
{   
    // printf("\nstampa filtrate\n");
    // for (size_t i = 0; i <= end_filt_word; i++)
    // {
    //     if (filtered_words[i * (len + 1)] != 0)
    //     {
    //         printf("%s\n", &filtered_words[i * (len + 1)]);
    //     }
    // }

    // not yet filtered words
    if (filtered_words == NULL)
    {
        // allocating spaces for the filtered words
        filtered_words = (char*)malloc(hash_dim * (len + 1) * sizeof(char));
        memset(filtered_words, 0, hash_dim * (len + 1) * sizeof(char));

        // inserting all words in the hash in the filtered array
        for (size_t i = 0; i < hash_dim; i++)
        {
            if (dictionary[i * (len + 1)] != 0)
            {
                insert_filtered(&dictionary[i * (len + 1)], len);
            }
        }
    }
    else
    {
        // eliminating empty spaces in the filtered word array to sort it easily
        if (filt_words_num != end_filt_word + 1)
        {
            u_int32_t j = 0;
            for (size_t i = 0; i <= end_filt_word; i++)
            {
                if (filtered_words[i * (len + 1)] != 0)
                {
                    if (strcmp(&filtered_words[j * (len + 1)], &filtered_words[i * (len + 1)]) != 0)
                    {
                        if (strcpy(&filtered_words[j * (len + 1)], &filtered_words[i * (len + 1)]) == 0)
                        {
                            printf("error\n");
                        }
                    }
                    j++;
                }
            }
            end_filt_word = filt_words_num - 1;
        }
    }

    // printf("preorder\n");
    // for (size_t i = 0; i <= end_filt_word; i++)
    // {
    //     if (filtered_words[i * (len + 1)] != 0)
    //     {
    //         printf("%s\n", &filtered_words[i * (len + 1)]);
    //     }
    //     else
    //     {
    //         printf("BRUTTO\n");
    //     }
        
    // }

    // sorting the filtered words array
    if (need_to_order == true)
    {
        merge_sort(0, end_filt_word, len);
        need_to_order = false;
    }
    
    // printf("postorder\n");
    // printing the filtered words in lessicographic order
    for (size_t i = 0; i <= end_filt_word; i++)
    {
        if (filtered_words[i * (len + 1)] != 0)
        {
            printf("%s\n", &filtered_words[i * (len + 1)]);
        }
    }
}

// function to insert new words in the dictionary
void inserisci_inizio(u_int32_t len)
{
    u_int32_t i, num, hash_value; // hash step & hash value
    bool insert = true; // used to decide if the word must be inserted in the filtered array
    u_int64_t mask;


    while (fscanf(stdin, "%s", input) != EOF && input[0] != '+') // getting word till the first command starting with '+'
    {
        i = 0;

        // if table is almost full rehash
        if (load_factor >= 0.75f)
        {
            hash_dim = rehash(len);
        }

        // for each word find a free place in the table
        hash_value = hash(input, len, i);
        while (dictionary[hash_value * (len +1)] != 0)
        {
            i++;
            hash_value = hash(input, len, i);
        }

        // insert the new word in the table
        if(strcpy(&dictionary[hash_value * (len + 1)], input) == 0)
        {
            printf("error/n");
        }

        // load factor updating
        count++;
        load_factor = count / hash_dim;

        if (filtered_words != NULL)
        {
            insert = true;

            // cicling on the filters
            for (size_t j = 0; j < 64; j++)
            {
                mask = 1;
                mask<<=j;
                
                num = 0;
                for (size_t k = 0; k < len; k++)
                {
                    mask = 1;
                    mask<<=k;
                    if (input[k] == reverse_char_map(j))
                    {
                        if (constrains[j].not_present == true)
                        {
                            // printf("1: ");
                            // printf("char = %c: ", reverse_char_map(j)); 
                            insert = false;
                            break;
                        }
                        if ((constrains[j].not_here&mask) != 0)
                        {
                            // // printf("2: ");
                            // printf("char = %c: ", reverse_char_map(j)); 
                            insert = false;
                            break;
                        }
                        num++;
                    }                       
                    else if (input[k] != reverse_char_map(j)
                            && (constrains[j].must_be_here&mask) != 0)
                    {
                        // printf("3: ");
                        // printf("char = %c: ", reverse_char_map(j)); 
                        insert = false;
                        break;
                    }
                }
                
                if (insert == false)
                {
                    // printf("4: ");
                    // printf("char = %c: ", reverse_char_map(j)); 
                    break;
                }

                // printf("char = %c, num = %d, max = %d, min = %d\n", reverse_char_map(j), num, constrains[j].num_max, constrains[j].num_min);

                if (num < constrains[j].num_min || num > constrains[j].num_max)
                {
                    // printf("5: ");
                    // printf("char = %c: min = %d, mac = %d, curr = %d | ", reverse_char_map(j), constrains[j].num_min, constrains[j].num_max, num); 
                    insert = false;
                    break;
                }
            }

            // if (insert == false)
            // {
            //     printf("not inserting inizio %s\n", input);
            // }

            // if the word meet the criteria insert in the filtered array
            if (insert == true)
            {
                // printf("inserting inizio %s\n", input);
                // insert
                insert_filtered(input, len);
            }
        }
        
    }
}

// function used to call the right function based on the current input
void caller(char curr_input[18], u_int32_t new_game_len) 
{
    if (curr_input[1] == 'n') // '+nuova_partita'
    {
        if(new_game(new_game_len) == false)
        {
            printf("ko\n");
        }
    }
    else if (curr_input[1] == 's') // '+stampa_filtrate'
    {
        stampa_filtrate(new_game_len);
    }
    else if (curr_input[1] == 'i') // '+inserisci inizio'
    {
        inserisci_inizio(new_game_len);
    }    
}