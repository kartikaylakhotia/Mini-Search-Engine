# Mini-Search-Engine

## Project Overview

The **Mini Search Engine** is a latency-optimized, memory-resident information retrieval system engineered from first principles in C++. Designed to parse, index, and query the **20 Newsgroups Dataset**, this project demonstrates a deep understanding of core data structures and algorithmic efficiency without reliance on external indexing libraries, database engines, or machine learning frameworks.

The system implements a **Vector Space Model** utilizing **TF-IDF (Term Frequency-Inverse Document Frequency)** weighting to rank documents by relevance, ensuring that users receive semantically significant results rather than simple frequency matches. Additionally, it features query enhancement mechanisms including **prefix-based autocomplete** and **Levenshtein edit-distance spell correction**.

---

## Approach & Design

The architecture of the search engine is divided into three distinct phases: **Ingestion & Canonicalization**, **Index Construction**, and **Query Processing**.

### 1. Ingestion & Lexical Analysis (Preprocessing)
Raw textual data is noisy and unstructured. To ensure index quality, we implement a rigorous preprocessing pipeline:
* **Tokenization:** The raw byte stream is segmented into atomic units (tokens) based on whitespace delimiters.
* **Canonicalization:** All tokens are normalized to lowercase to enforce case-insensitivity, ensuring `Apple` and `apple` are treated as identical entities.
* **Noise Filtration:** Punctuation and non-alphanumeric characters are stripped. Furthermore, a stop-list logic (using `std::unordered_set`) filters out ubiquitous, low-information terms (e.g., *the, is, at*) which would otherwise bloat the index and skew relevance scoring.

### 2. The Inverted Index Construction
The backbone of the system is an **Inverted Index**, a sparse matrix representation mapping unique lexicon terms to their occurrences across the corpus.
* Unlike a forward index (Document $\to$ Words), which requires linear scanning ($O(N)$) for search, the Inverted Index (Word $\to$ Documents) enables near-instantaneous ($O(1)$) retrieval of relevant document identifiers.
* We utilize a **Postings List** structure where each term points to a vector of `(DocumentID, Frequency)` tuples, facilitating rapid TF-IDF calculation during query time.

### 3. Relevance Ranking (TF-IDF)
To transcend simple boolean retrieval, we calculate a relevance score for each document $d$ against a query $q$. The score is the summation of TF-IDF weights:

$$Score(q, d) = \sum_{t \in q} tf(t, d) \times idf(t)$$

* **Term Frequency ($tf$):** Quantifies the local importance of a term within a specific document.
    $$tf(t, d) = \frac{\text{count of } t \text{ in } d}{\text{total words in } d}$$
* **Inverse Document Frequency ($idf$):** Quantifies the global specificity of a term. Rare terms (e.g., "microprocessor") are weighted heavily, while common terms (e.g., "subject") are penalized.
    $$idf(t) = \log(\frac{\text{Total Documents}}{\text{Documents containing } t})$$

---

## Data Structures Used

The system leverages the C++ Standard Template Library (STL) to balance memory overhead with execution speed.

| Component | C++ Structure | Algorithmic Rationale |
|-----------|---------------|-----------------------|
| **Inverted Index** | `std::unordered_map<string, vector<pair<int, int>>>` | The hash map provides **amortized $O(1)$ access** to posting lists. The vector stores pairs of `{docID, freq}` allowing for cache-friendly iteration during scoring. |
| **Document Metadata** | `std::unordered_map<int, DocInfo>` | Maps integer IDs to file paths and lengths. Using integer keys maximizes hashing efficiency compared to storing full string paths in the main index. |
| **Stop Word Filter** | `std::unordered_set<string>` | A hash set allows for $O(1)$ lookups to instantly determine if a token should be discarded, significantly faster than `std::vector` or `std::set` ($O(\log N)$). |
| **Vocabulary Store** | `std::vector<string>` | A sorted dynamic array storing unique terms. This structure is specifically chosen to enable **Binary Search** ($O(\log V)$) for the autocomplete feature. |
| **Ranking Buffer** | `std::vector<pair<double, int>>` | Used to aggregate final scores. We utilize `std::sort` on this vector to produce the final ranked output. |

---

## Time & Space Complexity Analysis

Let:
* $N$ = Total number of documents.
* $W$ = Total number of words in the corpus.
* $V$ = Vocabulary size (number of unique terms).
* $L_{avg}$ = Average document length.
* $Q$ = Number of terms in a search query.

### Time Complexity
1.  **Index Construction:** **$O(W)$**
    * We iterate through every word in the corpus exactly once to tokenize and insert into the hash map.
2.  **Query Processing:** **$O(Q \cdot \bar{P} + D \log D)$**
    * Where $\bar{P}$ is the average size of a posting list and $D$ is the number of relevant documents. Retrieving posting lists takes $O(Q)$; traversing them takes proportional time to the term frequency. Sorting the results dominates the tail end of the process.
3.  **Autocomplete:** **$O(\log V + K)$**
    * Binary search finds the prefix start index ($O(\log V)$), and we linearly extract $K$ suggestions.
4.  **Spell Correction (Levenshtein):** **$O(V \cdot L_{word}^2)$**
    * In the worst case (searching the whole vocabulary), this is expensive. We optimize by checking length differentials first to prune the search space.

### Space Complexity
* **Overall:** **$O(W)$** (bounded by the total character count of the corpus).
* The Inverted Index is the primary consumer of RAM. By using 32-bit integers for IDs and frequencies, we minimize the memory footprint compared to storing raw strings.

---

## Build & Execution Instructions

### Prerequisites
* C++ Compiler (GCC, Clang, or MSVC) supporting **C++17** or later.
* **20 Newsgroups Dataset** extracted and located in the project root directory.

### Compilation
Open your terminal in the project directory and run:

```bash 
g++ -std=c++17 -O3 engine.cpp -o engine
```

### Execution
*Run the compiled executable:

```bash 
./engine
```

## Usage Example
* Upon running the engine, it will first index the documents. Once ready, you can perform searches. Here is an actual runtime session:

```bash 
Indexing documents from: 20_newsgroups ...
Indexing Complete.
Total Documents: 19998
Total Unique Terms: 257715
Index Build Time: 6.87711 seconds
Memory usage: 80.5625 MB
------------------------------------------------

Enter search query (or type 'exit' to quit): space
Autocomplete: space space1999 spaceacronyms spaceacronyms731394007gzacom spaceacronyms733982404gzacom 

Results found: 1032
Search time: 0.002749 seconds
Memory usage: 80.578125 MB
Top 5 results:
[1] Score: 0.097123 | File: 20_newsgroups/sci.space/62126
[2] Score: 0.095325 | File: 20_newsgroups/sci.space/61352
[3] Score: 0.095325 | File: 20_newsgroups/sci.space/60949
[4] Score: 0.094624 | File: 20_newsgroups/sci.space/60859
[5] Score: 0.087247 | File: 20_newsgroups/sci.space/60904

Related Keywords: jennifer huntsville camp
```
