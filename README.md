
# API Project

## Introduction
This repository contains the final project for the "Algoritmi e Principi dell'informatica" (API) course, part of the second year of the Computer Science and Engineering bachelor's program at Politecnico di Milano.

## Required knowledge for the project:
* Sorting algorithms
* Basic data structures such as hash tables, binary trees, RB trees, queues, stacks, and lists (not all were utilized).

## Description
The original project details (unfortunately available only in Italian) can be found in the "traccia.pdf" file. Here's a concise summary in English:

### Project Objective:
Develop a program to check the correspondence between the letters of two words of the same length.

### Word Specifications:
Words are sequences composed of the following characters:
* Lowercase alphabetic characters (a-z)
* Uppercase alphabetic characters (A-Z)
* Numbers (0-9)
* Underscore and minus (_ , -)

### Program Operation:
The program reads a sequence of information and instructions from stdin, generating output based on the following:
* Read a value k, representing the length of the words.
* Read a sequence (of arbitrary length) of words, each of length k, forming the set of allowable words (without duplicates).

### Game Sequences:
Afterward, the program processes a sequence of "games," each marked by the command _+nuova_partita_. For each game, the program reads:
* The goal word for that game (length k)
* n, the maximum number of words to compare with the goal
* A sequence of words to compare

### Additional Commands:
Commands _+inserisci_inizio_ and _+inserisci_fine_ can appear during a game or between games, followed by a sequence of words to be added to the allowable word set.

### Output comparison:
For each word p compared with the goal word r, the program writes to stdout a sequence of k characters (res is the output):
* res[i] = '+' if p[i] = r[i] (indicating p[i] is in the correct position)
* res[i] = '/' if p[i] doesn't appear in r
* res[i] = '|' if p[i] appears in r but not in position i

### Validity Check:
If the read word p is not part of the allowable words set, the program writes "not_exists" on stdout, and it doesn't count as a comparison.

### Game End Conditions:
* If the program reads the goal word r, it writes "ok" on stdout, and the game ends.
* If the program reads n words without the goal one, the game ends, and the program writes "ko" on stdout.

### Game Transition:
After a game ends, another can begin with the _+nuova_partita_ command, or a new sequence of words can be added to the set with the _+inserisci_inizio_ command.

### Constraints Comparison:
Each comparison during a game produces constraints. For example:
* goal = abcabcabcabcabc
* word = bbaabccbccbcabc
* output = /+|+++|++/+++++
From this comparison, we learn specific constraints about the positions of characters in the goal word.

### Filtering and Counting:
* During a game, if the command _+stampa_filtrate_ appears, the program writes on stdout (in ASCII order) all words from the allowable words set that satisfy the constraints learned from previous comparisons.
* After each comparison during a game, the program writes on stdout the number of allowable words that meet the learned constraints.

## Additional Information
In this repository, alongside the project code, you'll find a folder named "opentestcases," containing five inputs with corresponding outputs provided by the professor for testing purposes. The actual inputs used for evaluation are unknown. For more extensive input files, you can refer to the public repository of my friend @dudoleitor: API_WordChecker-Public_tests.
