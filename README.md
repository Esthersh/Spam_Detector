Spam Detector for messages, written in C++. Uses a variant of the Hashmap data structure that I implemented.

Part of the course "Programming workshop in C++", at The Hebrew University Of Jerusalem.


includes 2 file's:

HashMap.hpp - 
A simple implementation of the Hashmap data structure. I used vectors for the table, and allocated memory for bauckets (based on the open hashing method, https://en.wikipedia.org/wiki/Hash_table#Open_addressing). I built an iterator for the data structure, simply using the indexes in the vector and and in the buckets, and skipping over the empty spots. 


SpamDetector.cpp - 
Reads two files and one positive value from argv. The first is a CSV file, listing pairs of words and positive values. We will refer to these words as "Bad words" and the values are the amount of "Bad points" for each word, I'll later explain how we use the list of bad words and bad points to decide if an Email should be titled as spam. The second file is an Email, and it's extension is TXT. The third argument is the threshold value for determining whether an Email is spam.

How does the program use the bad words and bad points to recognize spam messages?
We hold a counter initiated to 0, this counter keeps track of the number of bad points calculated so far. The SpamDetector reads through the Email and counts the number of bad words that are mentioned in it. For each bad word that was used, the number of bad points to match it are added to the counter. After reading the Email, if the value of the counter is larger than the threshold value or equal to it, than the Email will be titled as spam.

