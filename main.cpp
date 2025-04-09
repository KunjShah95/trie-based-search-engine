#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <ctime>

using namespace std;

// Constants
const int ALPHABET_SIZE = 26;
const int MAX_FILES = 100;
const int MAX_WORD_LENGTH = 100;
const int MAX_SUGGESTIONS = 50;
const int MAX_RESULTS = 100;
const int MAX_HISTORY = 20;
const int MAX_EDIT_DISTANCE = 2; // For spell checking

// Forward declaration
void processFile(const string &filename, class Trie &trie);

// Spell checker and search history structures
struct SearchHistory
{
    char queries[MAX_HISTORY][MAX_WORD_LENGTH];
    int count;

    SearchHistory() : count(0) {}

    void addQuery(const char *query)
    {
        // Don't add duplicates of the last query
        if (count > 0 && strcmp(queries[count - 1], query) == 0)
        {
            return;
        }

        // Shift if history is full
        if (count >= MAX_HISTORY)
        {
            for (int i = 0; i < MAX_HISTORY - 1; i++)
            {
                strcpy(queries[i], queries[i + 1]);
            }
            count = MAX_HISTORY - 1;
        }

        // Add new query
        strncpy(queries[count++], query, MAX_WORD_LENGTH);
    }

    void display()
    {
        if (count == 0)
        {
            cout << "No search history.\n";
            return;
        }

        cout << "Search History:\n";
        for (int i = count - 1; i >= 0; i--)
        {
            cout << (count - i) << ". " << queries[i] << endl;
        }
    }
};

struct FileInfo
{
    int fileId;
    int frequency;
    int positions[MAX_WORD_LENGTH]; // Store word positions within document
};

struct TrieNode
{
    TrieNode *children[ALPHABET_SIZE]; // Use array instead of unordered_map
    bool isEndOfWord;
    char word[MAX_WORD_LENGTH]; // Use char array instead of string for better memory management
    FileInfo fileInfo[MAX_FILES];
    int fileInfoCount;

    TrieNode() : isEndOfWord(false), fileInfoCount(0)
    {
        for (int i = 0; i < ALPHABET_SIZE; i++)
        {
            children[i] = nullptr;
        }
        memset(word, 0, sizeof(word));
    }

    ~TrieNode()
    {
        // No need to manually free memory as char array handles it
    }
};

class Trie
{
private:
    TrieNode *root;
    char fileList[MAX_FILES][MAX_WORD_LENGTH];
    int fileListCount;

    void destroyTrie(TrieNode *node)
    {
        if (!node)
            return;
        for (int i = 0; i < ALPHABET_SIZE; i++)
        {
            destroyTrie(node->children[i]);
        }
        delete node;
    }

    void collectWords(TrieNode *node, char suggestions[MAX_SUGGESTIONS][MAX_WORD_LENGTH], int &suggestionCount)
    {
        if (node->isEndOfWord)
        {
            strncpy(suggestions[suggestionCount++], node->word, MAX_WORD_LENGTH);
        }

        for (int i = 0; i < ALPHABET_SIZE; i++)
        {
            if (node->children[i])
            {
                collectWords(node->children[i], suggestions, suggestionCount);
            }
        }
    }

    int findFileId(const char *filename)
    {
        for (int i = 0; i < fileListCount; i++)
        {
            if (strcmp(fileList[i], filename) == 0)
            {
                return i;
            }
        }
        return -1;
    }

    int findFileIdInFileInfo(TrieNode *node, int fileId)
    {
        for (int i = 0; i < node->fileInfoCount; i++)
        {
            if (node->fileInfo[i].fileId == fileId)
            {
                return i;
            }
        }
        return -1;
    }

    // Calculate Levenshtein distance (edit distance) for spell checking
    int editDistance(const char *s1, const char *s2)
    {
        int len1 = strlen(s1);
        int len2 = strlen(s2);

        // Create a 2D array to store distances
        // We only need two rows, the current and previous
        int dp[2][MAX_WORD_LENGTH + 1];

        // Initialize the first row
        for (int j = 0; j <= len2; j++)
        {
            dp[0][j] = j;
        }

        // Fill the dp table
        for (int i = 1; i <= len1; i++)
        {
            for (int j = 0; j <= len2; j++)
            {
                if (j == 0)
                {
                    dp[i % 2][j] = i;
                }
                else if (s1[i - 1] == s2[j - 1])
                {
                    dp[i % 2][j] = dp[(i - 1) % 2][j - 1];
                }
                else
                {
                    dp[i % 2][j] = 1 + min(
                                           dp[(i - 1) % 2][j], // Delete
                                           min(
                                               dp[i % 2][j - 1],      // Insert
                                               dp[(i - 1) % 2][j - 1] // Replace
                                               ));
                }
            }
        }

        return dp[len1 % 2][len2];
    }

    void findAllWords(TrieNode *node, char allWords[MAX_SUGGESTIONS][MAX_WORD_LENGTH], int &count)
    {
        if (!node)
            return;

        if (node->isEndOfWord && count < MAX_SUGGESTIONS)
        {
            strncpy(allWords[count++], node->word, MAX_WORD_LENGTH);
        }

        for (int i = 0; i < ALPHABET_SIZE; i++)
        {
            if (node->children[i])
            {
                findAllWords(node->children[i], allWords, count);
                if (count >= MAX_SUGGESTIONS)
                    break;
            }
        }
    }

public:
    Trie() : fileListCount(0)
    {
        root = new TrieNode();
    }

    ~Trie()
    {
        destroyTrie(root);
    }

    // Improved stemming algorithm
    void stem(char *word)
    {
        int length = strlen(word);

        // Basic Porter stemming algorithm (simplified)
        if (length > 3)
        {
            // Handle -ing suffix
            if (strcmp(word + length - 3, "ing") == 0)
            {
                word[length - 3] = '\0';
                // If word ends with double letter after removing 'ing', remove one
                length = strlen(word);
                if (length >= 2 && word[length - 1] == word[length - 2])
                {
                    word[length - 1] = '\0';
                }
            }
            // Handle -ed suffix
            else if (length > 2 && strcmp(word + length - 2, "ed") == 0)
            {
                word[length - 2] = '\0';
                // If word ends with double letter after removing 'ed', remove one
                length = strlen(word);
                if (length >= 2 && word[length - 1] == word[length - 2])
                {
                    word[length - 1] = '\0';
                }
            }
            // Handle -ies suffix (plurals like "stories" -> "story")
            else if (length > 3 && strcmp(word + length - 3, "ies") == 0)
            {
                word[length - 3] = 'y';
                word[length - 2] = '\0';
            }
            // Handle -es suffix
            else if (length > 2 && strcmp(word + length - 2, "es") == 0)
            {
                word[length - 2] = '\0';
            }
            // Handle regular plural -s
            else if (length > 1 && word[length - 1] == 's' &&
                     word[length - 2] != 's' && word[length - 2] != 'i')
            {
                word[length - 1] = '\0';
            }
        }
    }

    bool isStopWord(const char *word)
    {
        const char *stopWords[] = {"the", "a", "is", "of", "and", "in", "to", "it", "that", "for"};
        for (int i = 0; i < 10; i++)
        {
            if (strcmp(stopWords[i], word) == 0)
            {
                return true;
            }
        }
        return false;
    }

    void insert(const char *word, int fileId)
    {
        TrieNode *current = root;

        for (int i = 0; word[i]; i++)
        {
            char ch = tolower(word[i]);
            if (!isalpha(ch))
            {
                continue;
            }
            int index = ch - 'a';
            if (!current->children[index])
            {
                current->children[index] = new TrieNode();
                if (current->children[index] == nullptr)
                {
                    cerr << "Memory allocation failed!" << endl;
                    exit(1);
                }
            }
            current = current->children[index];
        }

        current->isEndOfWord = true;
        if (strlen(current->word) == 0)
        {
            strncpy(current->word, word, MAX_WORD_LENGTH);
        }

        int fileIndex = findFileIdInFileInfo(current, fileId);
        if (fileIndex == -1)
        {
            current->fileInfo[current->fileInfoCount++] = {fileId, 1};
        }
        else
        {
            current->fileInfo[fileIndex].frequency++;
        }
    }

    bool search(const char *word)
    {
        char wordCopy[MAX_WORD_LENGTH];
        strncpy(wordCopy, word, MAX_WORD_LENGTH);

        // Apply stemming to the search word
        stem(wordCopy);

        TrieNode *current = root;

        for (int i = 0; wordCopy[i] != '\0'; i++)
        {
            char ch = tolower(wordCopy[i]);
            if (!isalpha(ch))
                return false;

            int index = ch - 'a';
            if (!current->children[index])
            {
                return false;
            }
            current = current->children[index];
        }

        return current != nullptr && current->isEndOfWord;
    }

    int addFile(const char *filename)
    {
        for (int i = 0; i < fileListCount; i++)
        {
            if (strcmp(fileList[i], filename) == 0)
            {
                return i;
            }
        }
        strncpy(fileList[fileListCount++], filename, MAX_WORD_LENGTH);
        return fileListCount - 1;
    }

    TrieNode *getRootNode()
    {
        return root;
    }

    const char *getFilename(int fileId)
    {
        return fileList[fileId];
    }

    // Get word details including file occurrences
    void getWordDetails(const char *word, char result[MAX_RESULTS][MAX_WORD_LENGTH], int &resultCount)
    {
        resultCount = 0;
        char wordCopy[MAX_WORD_LENGTH];
        strncpy(wordCopy, word, MAX_WORD_LENGTH);
        stem(wordCopy);

        TrieNode *current = root;
        for (int i = 0; wordCopy[i] != '\0'; i++)
        {
            char ch = tolower(wordCopy[i]);
            if (!isalpha(ch))
                return;

            int index = ch - 'a';
            if (!current->children[index])
            {
                return;
            }
            current = current->children[index];
        }

        if (current && current->isEndOfWord)
        {
            // Format: "Original word: [stemmed form]"
            sprintf(result[resultCount++], "Word: %s [stemmed: %s]", word, current->word);

            // Format: "Total occurrences: X"
            int totalCount = 0;
            for (int i = 0; i < current->fileInfoCount; i++)
            {
                totalCount += current->fileInfo[i].frequency;
            }
            sprintf(result[resultCount++], "Total occurrences: %d", totalCount);

            // List occurrences by file
            sprintf(result[resultCount++], "Occurrences by file:");
            for (int i = 0; i < current->fileInfoCount; i++)
            {
                sprintf(result[resultCount++], "  - %s: %d times",
                        fileList[current->fileInfo[i].fileId],
                        current->fileInfo[i].frequency);
            }
        }
    }

    // Partial search functionality
    bool partialSearch(const char *word, char results[MAX_RESULTS][MAX_WORD_LENGTH], int &resultCount)
    {
        resultCount = 0;
        char wordCopy[MAX_WORD_LENGTH];
        strncpy(wordCopy, word, MAX_WORD_LENGTH);

        TrieNode *current = root;
        for (int i = 0; wordCopy[i] != '\0'; i++)
        {
            char ch = tolower(wordCopy[i]);
            if (!isalpha(ch))
                return false;

            int index = ch - 'a';
            if (!current->children[index])
            {
                return false;
            }
            current = current->children[index];
        }

        // Now collect all words from this point in the trie
        collectWords(current, results, resultCount);
        return resultCount > 0;
    }

    // Autocomplete functionality
    bool autocomplete(const char *prefix, char suggestions[MAX_SUGGESTIONS][MAX_WORD_LENGTH], int &suggestionCount)
    {
        suggestionCount = 0;

        TrieNode *current = root;
        for (int i = 0; prefix[i] != '\0'; i++)
        {
            char ch = tolower(prefix[i]);
            if (!isalpha(ch))
                return false;

            int index = ch - 'a';
            if (!current->children[index])
            {
                return false;
            }
            current = current->children[index];
        }

        // Collect all words from this point in the trie
        collectWords(current, suggestions, suggestionCount);
        return suggestionCount > 0;
    }

    // Advanced search with phrase matching
    bool advancedSearch(const char *query, char results[MAX_RESULTS][MAX_WORD_LENGTH], int &resultCount)
    {
        resultCount = 0;

        // Split the query into words
        char queryCopy[MAX_WORD_LENGTH];
        strncpy(queryCopy, query, MAX_WORD_LENGTH);

        char *words[MAX_SUGGESTIONS];
        int wordCount = 0;

        char *token = strtok(queryCopy, " ");
        while (token != NULL && wordCount < MAX_SUGGESTIONS)
        {
            words[wordCount++] = token;
            token = strtok(NULL, " ");
        }

        // For advanced search, we'll implement a basic AND search (all terms must be present)
        if (wordCount == 0)
            return false;

        // First, find file occurrences for the first word
        int fileMatches[MAX_FILES] = {0}; // Track matched files

        for (int w = 0; w < wordCount; w++)
        {
            char *word = words[w];
            stem(word);

            TrieNode *current = root;
            // This is the fixed loop condition
            for (int i = 0; word[i] != '\0'; i++)
            {
                char ch = tolower(word[i]);
                if (!isalpha(ch))
                    continue;

                int index = ch - 'a';
                if (!current->children[index])
                {
                    // Word doesn't exist, so no matches
                    return false;
                }
                current = current->children[index];
            }

            if (!current->isEndOfWord)
            {
                // Word doesn't exist
                return false;
            }

            // Mark files that contain this word
            for (int i = 0; i < current->fileInfoCount; i++)
            {
                int fileId = current->fileInfo[i].fileId;
                if (w == 0 || fileMatches[fileId] == w)
                {
                    fileMatches[fileId]++;
                }
            }
        }

        // Check which files contain all words
        for (int i = 0; i < MAX_FILES; i++)
        {
            if (fileMatches[i] == wordCount)
            {
                // This file contains all words
                sprintf(results[resultCount++], "%s", fileList[i]);
                if (resultCount >= MAX_RESULTS)
                    break;
            }
        }

        return resultCount > 0;
    }

    // Find words similar to the misspelled word
    bool spellCheck(const char *word, char suggestions[MAX_SUGGESTIONS][MAX_WORD_LENGTH], int &suggestionCount)
    {
        suggestionCount = 0;

        // Get all words in the dictionary
        char allWords[MAX_SUGGESTIONS][MAX_WORD_LENGTH];
        int wordCount = 0;
        findAllWords(root, allWords, wordCount);

        // Create an array to store words with their edit distances
        struct WordDistance
        {
            char word[MAX_WORD_LENGTH];
            int distance;
        };

        WordDistance distances[MAX_SUGGESTIONS];
        int distanceCount = 0;

        // Calculate edit distance for each word
        for (int i = 0; i < wordCount; i++)
        {
            int distance = editDistance(word, allWords[i]);
            if (distance <= MAX_EDIT_DISTANCE)
            {
                strncpy(distances[distanceCount].word, allWords[i], MAX_WORD_LENGTH);
                distances[distanceCount].distance = distance;
                distanceCount++;

                if (distanceCount >= MAX_SUGGESTIONS)
                    break;
            }
        }

        // Sort by distance (simple bubble sort)
        for (int i = 0; i < distanceCount - 1; i++)
        {
            for (int j = 0; j < distanceCount - i - 1; j++)
            {
                if (distances[j].distance > distances[j + 1].distance)
                {
                    WordDistance temp = distances[j];
                    distances[j] = distances[j + 1];
                    distances[j + 1] = temp;
                }
            }
        }

        // Copy suggestions
        suggestionCount = distanceCount;
        for (int i = 0; i < distanceCount; i++)
        {
            strncpy(suggestions[i], distances[i].word, MAX_WORD_LENGTH);
        }

        return suggestionCount > 0;
    }

    // Word proximity search
    bool proximitySearch(const char *word1, const char *word2, int maxDistance,
                         char results[MAX_RESULTS][MAX_WORD_LENGTH], int &resultCount)
    {
        resultCount = 0;

        char stemmed1[MAX_WORD_LENGTH];
        char stemmed2[MAX_WORD_LENGTH];

        strncpy(stemmed1, word1, MAX_WORD_LENGTH);
        strncpy(stemmed2, word2, MAX_WORD_LENGTH);

        stem(stemmed1);
        stem(stemmed2);

        // Find positions of both words in each file
        for (int fileId = 0; fileId < fileListCount; fileId++)
        {
            int positions1[MAX_WORD_LENGTH] = {0};
            int positions2[MAX_WORD_LENGTH] = {0};
            int pos1Count = 0, pos2Count = 0;

            // Find positions for word1
            TrieNode *current = root;
            for (int i = 0; stemmed1[i] != '\0'; i++)
            {
                int index = tolower(stemmed1[i]) - 'a';
                if (index < 0 || index >= ALPHABET_SIZE || !current->children[index])
                {
                    break;
                }
                current = current->children[index];
            }

            if (current && current->isEndOfWord)
            {
                for (int i = 0; i < current->fileInfoCount; i++)
                {
                    if (current->fileInfo[i].fileId == fileId)
                    {
                        // Copy positions
                        for (int j = 0; j < MAX_WORD_LENGTH && current->fileInfo[i].positions[j] != 0; j++)
                        {
                            positions1[pos1Count++] = current->fileInfo[i].positions[j];
                        }
                        break;
                    }
                }
            }

            // Find positions for word2
            current = root;
            for (int i = 0; stemmed2[i] != '\0'; i++)
            {
                int index = tolower(stemmed2[i]) - 'a';
                if (index < 0 || index >= ALPHABET_SIZE || !current->children[index])
                {
                    break;
                }
                current = current->children[index];
            }

            if (current && current->isEndOfWord)
            {
                for (int i = 0; i < current->fileInfoCount; i++)
                {
                    if (current->fileInfo[i].fileId == fileId)
                    {
                        // Copy positions
                        for (int j = 0; j < MAX_WORD_LENGTH && current->fileInfo[i].positions[j] != 0; j++)
                        {
                            positions2[pos2Count++] = current->fileInfo[i].positions[j];
                        }
                        break;
                    }
                }
            }

            // Check if words appear close to each other
            bool foundProximity = false;
            for (int i = 0; i < pos1Count; i++)
            {
                for (int j = 0; j < pos2Count; j++)
                {
                    int distance = abs(positions1[i] - positions2[j]);
                    if (distance <= maxDistance)
                    {
                        sprintf(results[resultCount++], "%s (distance: %d)", fileList[fileId], distance);
                        foundProximity = true;
                        break;
                    }
                }
                if (foundProximity)
                    break;
            }
        }

        return resultCount > 0;
    }

    // Export search results to file
    bool exportResults(const char *filename, const char results[MAX_RESULTS][MAX_WORD_LENGTH], int resultCount)
    {
        // Ensure the file has .txt extension
        string txtFilename = filename;
        if (txtFilename.length() < 4 || txtFilename.substr(txtFilename.length() - 4) != ".txt")
        {
            txtFilename += ".txt";
        }

        FILE *file = fopen(txtFilename.c_str(), "w");
        if (!file)
        {
            return false;
        }

        // Get the current date and time
        time_t now = time(0);
        struct tm *timeinfo = localtime(&now);
        char timeStr[80];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);

        fprintf(file, "Search Results - %s\n", timeStr);
        fprintf(file, "--------------------------------\n");

        for (int i = 0; i < resultCount; i++)
        {
            fprintf(file, "%d. %s\n", i + 1, results[i]);
        }

        fprintf(file, "--------------------------------\n");
        fprintf(file, "Total Results: %d\n", resultCount);

        fclose(file);
        return true;
    }

    // Export search results to CSV
    bool exportToCSV(const char *filename, const char results[MAX_RESULTS][MAX_WORD_LENGTH], int resultCount)
    {
        // Ensure the file has .csv extension
        string csvFilename = filename;
        if (csvFilename.length() < 4 || csvFilename.substr(csvFilename.length() - 4) != ".csv")
        {
            csvFilename += ".csv";
        }

        FILE *file = fopen(csvFilename.c_str(), "w");
        if (!file)
        {
            return false;
        }

        // Get the current date and time
        time_t now = time(0);
        struct tm *timeinfo = localtime(&now);
        char timeStr[80];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);

        // Write CSV header
        fprintf(file, "Index,Result,Timestamp\n");

        // Write data rows
        for (int i = 0; i < resultCount; i++)
        {
            // Escape any commas in the result
            char escapedResult[MAX_WORD_LENGTH * 2];
            int j = 0, k = 0;
            while (results[i][j] != '\0')
            {
                if (results[i][j] == ',')
                {
                    escapedResult[k++] = '"';
                    escapedResult[k++] = results[i][j];
                    escapedResult[k++] = '"';
                }
                else
                {
                    escapedResult[k++] = results[i][j];
                }
                j++;
            }
            escapedResult[k] = '\0';

            fprintf(file, "%d,%s,%s\n", i + 1, escapedResult, timeStr);
        }

        fclose(file);
        return true;
    }

    // Export word details to PDF-like text file (simplified PDF representation)
    bool exportToPDF(const char *filename, const char results[MAX_RESULTS][MAX_WORD_LENGTH], int resultCount)
    {
        // Ensure the file has .pdf extension
        string pdfFilename = filename;
        if (pdfFilename.length() < 4 || pdfFilename.substr(pdfFilename.length() - 4) != ".pdf")
        {
            pdfFilename += ".pdf";
        }

        FILE *file = fopen(pdfFilename.c_str(), "wb"); // Use binary mode for better compatibility
        if (!file)
        {
            return false;
        }

        // Get the current date and time
        time_t now = time(0);
        struct tm *timeinfo = localtime(&now);
        char timeStr[80];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);

        // Basic PDF structure (simplified)
        fprintf(file, "%%PDF-1.4\n");

        // Object 1: Catalog
        long obj1Pos = ftell(file);
        fprintf(file, "1 0 obj\n");
        fprintf(file, "<<\n");
        fprintf(file, "/Type /Catalog\n");
        fprintf(file, "/Pages 2 0 R\n");
        fprintf(file, ">>\n");
        fprintf(file, "endobj\n");

        // Object 2: Pages
        long obj2Pos = ftell(file);
        fprintf(file, "2 0 obj\n");
        fprintf(file, "<<\n");
        fprintf(file, "/Type /Pages\n");
        fprintf(file, "/Kids [3 0 R]\n");
        fprintf(file, "/Count 1\n");
        fprintf(file, ">>\n");
        fprintf(file, "endobj\n");

        // Object 3: Page
        long obj3Pos = ftell(file);
        fprintf(file, "3 0 obj\n");
        fprintf(file, "<<\n");
        fprintf(file, "/Type /Page\n");
        fprintf(file, "/Parent 2 0 R\n");
        fprintf(file, "/Resources <<\n");
        fprintf(file, "  /Font <<\n");
        fprintf(file, "    /F1 4 0 R\n");
        fprintf(file, "  >>\n");
        fprintf(file, ">>\n");
        fprintf(file, "/MediaBox [0 0 612 792]\n");
        fprintf(file, "/Contents 5 0 R\n");
        fprintf(file, ">>\n");
        fprintf(file, "endobj\n");

        // Object 4: Font
        long obj4Pos = ftell(file);
        fprintf(file, "4 0 obj\n");
        fprintf(file, "<<\n");
        fprintf(file, "/Type /Font\n");
        fprintf(file, "/Subtype /Type1\n");
        fprintf(file, "/BaseFont /Helvetica\n");
        fprintf(file, "/Encoding /WinAnsiEncoding\n");
        fprintf(file, ">>\n");
        fprintf(file, "endobj\n");

        // Build content string
        string content = "BT\n";
        content += "/F1 14 Tf\n";
        content += "50 750 Td\n";
        content += "(Mini Search Engine - Export Results) Tj\n";
        content += "0 -25 Td\n";
        content += "/F1 10 Tf\n";
        content += "(Generated on: " + string(timeStr) + ") Tj\n";
        content += "0 -30 Td\n";

        // Set up a table-like format for results
        int yPos = 680; // Starting Y position for results

        for (int i = 0; i < resultCount && i < MAX_RESULTS; i++)
        {
            // Check if we need to start a new page (simplified - not implementing multi-page)
            if (yPos < 100)
            {
                break;
            }

            // Sanitize the content for PDF (escape special characters)
            string line = "";
            line += "50 " + to_string(yPos) + " Td\n";
            line += "(";

            // Create a formatted row
            string rowText = to_string(i + 1) + ". ";
            for (int j = 0; results[i][j] != '\0'; j++)
            {
                char ch = results[i][j];
                if (ch == '(' || ch == ')' || ch == '\\')
                {
                    rowText += "\\";
                }
                rowText += ch;
            }
            line += rowText;
            line += ") Tj\n";

            content += line;
            yPos -= 20; // Move down for next line
        }

        // Add total results count at the bottom
        content += "50 50 Td\n";
        content += "(Total Results: " + to_string(resultCount) + ") Tj\n";
        content += "ET";

        // Object 5: Content Stream
        long obj5Pos = ftell(file);
        fprintf(file, "5 0 obj\n");
        fprintf(file, "<<\n");
        fprintf(file, "/Length %zu\n", content.length());
        fprintf(file, ">>\n");
        fprintf(file, "stream\n");
        fprintf(file, "%s\n", content.c_str());
        fprintf(file, "endstream\n");
        fprintf(file, "endobj\n");

        // Cross-reference table
        long xref_pos = ftell(file);
        fprintf(file, "xref\n");
        fprintf(file, "0 6\n");
        fprintf(file, "0000000000 65535 f\n");
        fprintf(file, "%010ld 00000 n\n", obj1Pos);
        fprintf(file, "%010ld 00000 n\n", obj2Pos);
        fprintf(file, "%010ld 00000 n\n", obj3Pos);
        fprintf(file, "%010ld 00000 n\n", obj4Pos);
        fprintf(file, "%010ld 00000 n\n", obj5Pos);

        // Trailer
        fprintf(file, "trailer\n");
        fprintf(file, "<<\n");
        fprintf(file, "/Size 6\n");
        fprintf(file, "/Root 1 0 R\n");
        fprintf(file, ">>\n");
        fprintf(file, "startxref\n");
        fprintf(file, "%ld\n", xref_pos);
        fprintf(file, "%%%%EOF\n");

        fclose(file);
        return true;
    }
};

// File processing functions
void processFile(const string &filename, Trie &trie)
{
    FILE *file = fopen(filename.c_str(), "r");
    if (!file)
    {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    // Add file to file list and get file ID
    int fileId = trie.addFile(filename.c_str());

    char word[MAX_WORD_LENGTH];
    int position = 1;
    while (fscanf(file, "%99s", word) == 1)
    {
        // Improved word cleanup
        char cleanedWord[MAX_WORD_LENGTH] = {0};
        int j = 0;
        for (int i = 0; word[i]; i++)
        {
            if (isalpha(word[i]))
            {
                cleanedWord[j++] = tolower(word[i]);
            }
        }

        // Stop word removal
        if (trie.isStopWord(cleanedWord))
        {
            continue;
        }

        // Stemming
        trie.stem(cleanedWord);

        if (strlen(cleanedWord) > 0)
        {
            trie.insert(cleanedWord, fileId);
        }

        position++;
    }

    fclose(file);
}

// Menu system
void displayMenu()
{
    cout << "\nMini Search Engine\n";
    cout << "1. Search Word\n";
    cout << "2. Partial Search\n";
    cout << "3. Autocomplete\n";
    cout << "4. Show Word Details\n";
    cout << "5. Advanced Search\n";
    cout << "6. Spell Check\n";
    cout << "7. Proximity Search\n";
    cout << "8. Search History\n";
    cout << "9. Export Results\n";
    cout << "10. Exit\n";
    cout << "Choice: ";
}

int main()
{
    Trie trie;
    SearchHistory history;
    string filename;
    char fileListInput[MAX_FILES][MAX_WORD_LENGTH];
    int fileCount = 0;

    cout << "==== Mini Search Engine ====\n";
    cout << "Enter files to index (space separated): ";
    while (cin >> filename && fileCount < MAX_FILES)
    {
        strncpy(fileListInput[fileCount++], filename.c_str(), MAX_WORD_LENGTH);
        if (cin.peek() == '\n')
            break;
    }

    // Show processing message
    cout << "Indexing files...\n";
    for (int i = 0; i < fileCount; i++)
    {
        cout << "Processing: " << fileListInput[i] << "...\n";
        processFile(fileListInput[i], trie);
    }
    cout << "Indexing complete! " << fileCount << " files processed.\n";

    while (true)
    {
        displayMenu();
        int choice;
        if (!(cin >> choice))
        {
            cerr << "Invalid input. Please enter a number.\n";
            cin.clear();
            // Use a fixed size instead of numeric_limits
            cin.ignore(1000, '\n');
            continue;
        }

        if (choice == 10)
            break;

        string input;
        switch (choice)
        {
        case 1: // Search Word
            cout << "Enter word to search: ";
            cin >> input;
            history.addQuery(input.c_str());
            if (trie.search(input.c_str()))
            {
                cout << "Word found!\n";

                // Show basic word details
                char result[MAX_RESULTS][MAX_WORD_LENGTH];
                int resultCount = 0;
                trie.getWordDetails(input.c_str(), result, resultCount);
                for (int i = 0; i < resultCount && i < 2; i++)
                { // Just show first two lines
                    cout << result[i] << endl;
                }
            }
            else
            {
                cout << "Word not found.\n";
                cout << "Try using autocomplete to find similar words.\n";
            }
            break;

        case 2: // Partial Search
            cout << "Enter partial word to search: ";
            cin >> input;
            history.addQuery(input.c_str());
            {
                char partialResults[MAX_RESULTS][MAX_WORD_LENGTH];
                int partialResultCount = 0;
                if (trie.partialSearch(input.c_str(), partialResults, partialResultCount))
                {
                    cout << "Found " << partialResultCount << " partial matches:\n";
                    for (int i = 0; i < partialResultCount; i++)
                    {
                        cout << (i + 1) << ". " << partialResults[i] << endl;
                        if (i >= 9)
                        { // Limit to 10 results
                            cout << "... and " << (partialResultCount - 10) << " more matches\n";
                            break;
                        }
                    }
                }
                else
                {
                    cout << "No partial matches found.\n";
                }
            }
            break;

        case 3: // Autocomplete
            cout << "Enter prefix for autocomplete: ";
            cin >> input;
            history.addQuery(input.c_str());
            {
                char autocompleteSuggestions[MAX_SUGGESTIONS][MAX_WORD_LENGTH];
                int suggestionCount = 0;
                if (trie.autocomplete(input.c_str(), autocompleteSuggestions, suggestionCount))
                {
                    cout << "Autocomplete suggestions:\n";
                    for (int i = 0; i < suggestionCount; i++)
                    {
                        cout << (i + 1) << ". " << autocompleteSuggestions[i] << endl;
                        if (i >= 9)
                        { // Limit to 10 results
                            cout << "... and " << (suggestionCount - 10) << " more suggestions\n";
                            break;
                        }
                    }
                }
                else
                {
                    cout << "No autocomplete suggestions found.\n";
                }
            }
            break;

        case 4: // Word Details
            cout << "Enter word to show details: ";
            cin >> input;
            history.addQuery(input.c_str());
            {
                char detailResults[MAX_RESULTS][MAX_WORD_LENGTH];
                int detailCount = 0;
                trie.getWordDetails(input.c_str(), detailResults, detailCount);
                if (detailCount > 0)
                {
                    cout << "=== Word Details ===\n";
                    for (int i = 0; i < detailCount; i++)
                    {
                        cout << detailResults[i] << endl;
                    }
                    cout << "==================\n";
                }
                else
                {
                    cout << "Word not found. Try using autocomplete to find similar words.\n";
                }
            }
            break;

        case 5: // Advanced Search
            cout << "Enter phrase for advanced search (multiple words): ";
            cin.ignore();
            getline(cin, input);
            history.addQuery(input.c_str());
            {
                if (input.empty())
                {
                    cout << "Empty search phrase. Please try again.\n";
                    break;
                }

                char advancedResults[MAX_RESULTS][MAX_WORD_LENGTH];
                int advancedResultCount = 0;
                if (trie.advancedSearch(input.c_str(), advancedResults, advancedResultCount))
                {
                    cout << "Found " << advancedResultCount << " files containing all words in: \"" << input << "\"\n";
                    for (int i = 0; i < advancedResultCount; i++)
                    {
                        cout << (i + 1) << ". " << advancedResults[i] << endl;
                    }
                }
                else
                {
                    cout << "No files found containing all words in the phrase.\n";
                    cout << "Try a simpler search with fewer terms.\n";
                }
            }
            break;

        case 6: // Spell Check
            cout << "Enter word to check spelling: ";
            cin >> input;
            history.addQuery(input.c_str());
            {
                char spellSuggestions[MAX_SUGGESTIONS][MAX_WORD_LENGTH];
                int suggestionCount = 0;
                if (trie.spellCheck(input.c_str(), spellSuggestions, suggestionCount))
                {
                    cout << "Did you mean:\n";
                    for (int i = 0; i < suggestionCount; i++)
                    {
                        cout << (i + 1) << ". " << spellSuggestions[i] << endl;
                    }
                }
                else
                {
                    cout << "No suggestions found for the word.\n";
                }
            }
            break;

        case 7: // Proximity Search
        {
            cout << "Enter two words for proximity search: ";
            string word1, word2;
            cin >> word1 >> word2;
            history.addQuery((word1 + " " + word2).c_str());
            {
                int maxDistance;
                cout << "Enter maximum distance between words: ";
                cin >> maxDistance;

                char proximityResults[MAX_RESULTS][MAX_WORD_LENGTH];
                int proximityResultCount = 0;
                if (trie.proximitySearch(word1.c_str(), word2.c_str(), maxDistance, proximityResults, proximityResultCount))
                {
                    cout << "Found " << proximityResultCount << " results:\n";
                    for (int i = 0; i < proximityResultCount; i++)
                    {
                        cout << (i + 1) << ". " << proximityResults[i] << endl;
                    }
                }
                else
                {
                    cout << "No proximity matches found.\n";
                }
            }
            break;
        }

        case 8: // Search History
            history.display();
            break;

        case 9: // Export Results
        {
            string exportType;
            cout << "What would you like to export (last/history/word)? ";
            cin >> exportType;

            if (exportType == "last" && history.count > 0)
            {
                // Export results for last search
                string lastQuery = history.queries[history.count - 1];
                cout << "Exporting results for: " << lastQuery << endl;

                char exportData[MAX_RESULTS][MAX_WORD_LENGTH];
                int exportCount = 0;

                // Get results based on last query
                char lastWord[MAX_WORD_LENGTH];
                strncpy(lastWord, lastQuery.c_str(), MAX_WORD_LENGTH);

                // Simple word search export
                trie.getWordDetails(lastWord, exportData, exportCount);

                if (exportCount == 0)
                {
                    // Try partial search if exact match failed
                    trie.partialSearch(lastWord, exportData, exportCount);
                }

                string filename;
                cout << "Enter filename to export results (without extension): ";
                cin >> filename;

                string format;
                cout << "Export format (txt/csv/pdf): ";
                cin >> format;

                // Generate export path based on format
                string exportPath = filename;
                bool exportSuccess = false;

                if (format == "csv")
                {
                    // Add .csv extension if not present
                    if (exportPath.length() < 4 || exportPath.substr(exportPath.length() - 4) != ".csv")
                    {
                        exportPath += ".csv";
                    }
                    exportSuccess = trie.exportToCSV(exportPath.c_str(), exportData, exportCount);
                }
                else if (format == "pdf")
                {
                    // Add .pdf extension if not present
                    if (exportPath.length() < 4 || exportPath.substr(exportPath.length() - 4) != ".pdf")
                    {
                        exportPath += ".pdf";
                    }
                    exportSuccess = trie.exportToPDF(exportPath.c_str(), exportData, exportCount);
                }
                else
                {
                    // Default to txt format
                    if (exportPath.length() < 4 || exportPath.substr(exportPath.length() - 4) != ".txt")
                    {
                        exportPath += ".txt";
                    }
                    exportSuccess = trie.exportResults(exportPath.c_str(), exportData, exportCount);
                }

                if (exportSuccess)
                {
                    cout << "Results exported successfully to: " << exportPath << endl;
                    cout << "File is ready to be downloaded in " << format << " format." << endl;
                }
                else
                {
                    cout << "Failed to export results. Please check file permissions or disk space." << endl;
                }
            }
            else if (exportType == "history")
            {
                // Export search history
                string filename;
                cout << "Enter filename to export history (without extension): ";
                cin >> filename;

                string format;
                cout << "Export format (txt/csv/pdf): ";
                cin >> format;

                // Generate export path based on format
                string exportPath = filename;
                if (format == "csv" && (exportPath.length() < 4 || exportPath.substr(exportPath.length() - 4) != ".csv"))
                {
                    exportPath += ".csv";
                }
                else if (format == "pdf" && (exportPath.length() < 4 || exportPath.substr(exportPath.length() - 4) != ".pdf"))
                {
                    exportPath += ".pdf";
                }
                else if (format != "csv" && format != "pdf" && (exportPath.length() < 4 || exportPath.substr(exportPath.length() - 4) != ".txt"))
                {
                    exportPath += ".txt";
                }

                // Prepare history data for export
                char historyData[MAX_RESULTS][MAX_WORD_LENGTH];
                int historyCount = 0;

                for (int i = history.count - 1; i >= 0 && historyCount < MAX_RESULTS; i--)
                {
                    sprintf(historyData[historyCount++], "Query %d: %s", history.count - i, history.queries[i]);
                }

                bool exportSuccess = false;
                if (format == "csv")
                {
                    exportSuccess = trie.exportToCSV(exportPath.c_str(), historyData, historyCount);
                }
                else if (format == "pdf")
                {
                    exportSuccess = trie.exportToPDF(exportPath.c_str(), historyData, historyCount);
                }
                else
                {
                    // Fallback to regular text file export
                    FILE *file = fopen(exportPath.c_str(), "w");
                    if (file)
                    {
                        fprintf(file, "Search History\n");
                        fprintf(file, "--------------------------------\n");

                        for (int i = history.count - 1; i >= 0; i--)
                        {
                            fprintf(file, "%d. %s\n", history.count - i, history.queries[i]);
                        }

                        fclose(file);
                        exportSuccess = true;
                    }
                }

                if (exportSuccess)
                {
                    cout << "History exported successfully to: " << exportPath << endl;
                    cout << "File is ready to be downloaded in " << format << " format." << endl;
                }
                else
                {
                    cout << "Failed to export history. Please check file permissions or disk space." << endl;
                }
            }
            else if (exportType == "word")
            {
                // Export details for a specific word
                cout << "Enter word to export details: ";
                string wordToExport;
                cin >> wordToExport;

                char wordDetails[MAX_RESULTS][MAX_WORD_LENGTH];
                int detailCount = 0;
                trie.getWordDetails(wordToExport.c_str(), wordDetails, detailCount);

                if (detailCount == 0)
                {
                    cout << "No details found for the word '" << wordToExport << "'. Nothing to export." << endl;
                    break;
                }

                string filename;
                cout << "Enter filename for export (without extension): ";
                cin >> filename;

                string format;
                cout << "Export format (txt/csv/pdf): ";
                cin >> format;

                // Generate export path based on format
                string exportPath = filename;
                if (format == "csv" && (exportPath.length() < 4 || exportPath.substr(exportPath.length() - 4) != ".csv"))
                {
                    exportPath += ".csv";
                }
                else if (format == "pdf" && (exportPath.length() < 4 || exportPath.substr(exportPath.length() - 4) != ".pdf"))
                {
                    exportPath += ".pdf";
                }
                else if (format != "csv" && format != "pdf" && (exportPath.length() < 4 || exportPath.substr(exportPath.length() - 4) != ".txt"))
                {
                    exportPath += ".txt";
                }

                bool exportSuccess = false;
                if (format == "csv")
                {
                    exportSuccess = trie.exportToCSV(exportPath.c_str(), wordDetails, detailCount);
                }
                else if (format == "pdf")
                {
                    exportSuccess = trie.exportToPDF(exportPath.c_str(), wordDetails, detailCount);
                }
                else
                {
                    exportSuccess = trie.exportResults(exportPath.c_str(), wordDetails, detailCount);
                }

                if (exportSuccess)
                {
                    cout << "Word details for '" << wordToExport << "' exported successfully to: " << exportPath << endl;
                    cout << "File is ready to be downloaded in " << format << " format." << endl;
                }
                else
                {
                    cout << "Failed to export word details. Please check file permissions or disk space." << endl;
                }
            }
            else
            {
                cout << "Invalid export type or no search history available.\n";
            }
        }
        break;

        default:
            cout << "Invalid choice. Please select an option from 1-10.\n";
        }

        // Add a pause before showing menu again
        cout << "\nPress Enter to continue...";
        cin.ignore(1000, '\n');
        cin.get();

        // Clear screen (simple approach)
        for (int i = 0; i < 50; i++)
            cout << endl;
    }

    cout << "Thank you for using Mini Search Engine!\n";
    return 0;
}
