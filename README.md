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

## Project Structure

- `main.cpp`: Core implementation including Trie data structure and search algorithms
- `documents.txt`: Sample document for testing
- `sample.txt`: Sample document for testing

## License

This project is open source and available under the MIT License.
