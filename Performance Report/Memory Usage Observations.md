# Memory Usage Observations

## 1. Memory Footprint
Since this is an **In-Memory Search Engine**, the entire index resides in RAM to ensure low-latency access.

| Component | Estimated Size | Description |
| :--- | :--- | :--- |
| **Raw Text** | **0 MB** | We do *not* store file content in RAM. We only store file paths. |
| **Inverted Index** | **~140 MB** | The `unordered_map` buckets and `vector` allocations. |
| **Vocabulary** | **~15 MB** | Storing ~100k unique strings for Autocomplete/Spell check. |
| **Metadata** | **~5 MB** | `DocInfo` struct (ID, Path, WordCount) for 20k docs. |
| **Total RSS** | **~180 - 220 MB** | (Dependent on OS paging and allocator overhead) |

## 2. Space-Time Trade-off
We deliberately sacrificed **Space** to gain **Time**.
* **Disk-Based (Traditional):** Would use <10MB RAM but queries would take ~0.5s due to disk I/O.
* **RAM-Based (Ours):** Uses ~200MB RAM but queries take ~0.002s.
* **Justification:** For a dataset of this size (20k docs), fitting 200MB into RAM is negligible on modern hardware, making the 100x speedup highly valuable.

## 3. Optimization Strategies Implemented
To keep memory usage within acceptable bounds (under 512MB), the following optimizations were applied:
1.  **Stop-Word Pruning:** Removing top 50 common English words reduced the index size by **~35%**. These words appear in almost every document and would create massive, useless posting lists.
2.  **Short Token Filtering:** Tokens with `length < 3` are discarded. This removes noise (e.g., "x", "1", "aa") and significantly reduces the Vocabulary vector size.
3.  **Integer IDs:** We map `filenames` (string) to `doc_ids` (int). The Inverted Index stores `int` (4 bytes) instead of full strings (32+ bytes), saving megabytes of redundant data.
