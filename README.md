# Mini Search Engine

A high-performance C++ implementation of a text search engine using Trie data structure.

## Features

- **Word Search**: Find exact word matches in indexed documents
- **Partial Search**: Find all words starting with a given prefix
- **Autocomplete**: Get word suggestions based on prefix
- **Word Details**: View detailed word information including occurrence counts by file
- **Advanced Search**: Search for phrases with multiple words (AND-based search)
- **Spell Checking**: Get word suggestions for misspelled words
- **Proximity Search**: Find files where two words appear within a specific distance
- **Search History**: Track and review past searches
- **Export Results**: Save search results to files for later reference

## Export Functionality

The search engine supports exporting search results, word details, and search history in multiple formats:

- **Text Files (.txt)**: Standard text format for easy reading
- **CSV Files (.csv)**: Comma-separated values format for spreadsheet applications
- **PDF Files (.pdf)**: Structured document format with proper headers and formatting

All export functions automatically ensure the correct file extension is used based on the selected format.

## Implementation Details

- Uses optimized Trie data structure for fast lookups
- Custom stemming algorithm to handle word variations
- Stop word filtering to improve search relevance
- Memory-efficient C++ implementation without external dependencies

## Usage

1. Compile the code:
```
g++ main.cpp -o search_engine
```

2. Run the program:
```
./search_engine
```

3. Enter the files you want to index when prompted
4. Use the menu system to perform various search operations

## Example

```
==== Mini Search Engine ====
Enter files to index (space separated): sample.txt documents.txt
Indexing files...
Processing: sample.txt...
Processing: documents.txt...
Indexing complete! 2 files processed.

Mini Search Engine
1. Search Word
2. Partial Search
3. Autocomplete
4. Show Word Details
5. Advanced Search
6. Spell Check
7. Proximity Search
8. Search History
9. Export Results
10. Exit
Choice:
```

## Export Example

To export search results:
1. Choose option 9 from the menu
2. Select what to export (last search, history, or word details)
3. Enter the filename (without extension)
4. Choose the format (txt, csv, or pdf)

The system will generate the file with the appropriate extension and formatting.

## Project Structure

- `main.cpp`: Core implementation including Trie data structure and search algorithms
- `documents.txt`: Sample document for testing
- `sample.txt`: Sample document for testing

## License

This project is open source and available under the MIT License.
