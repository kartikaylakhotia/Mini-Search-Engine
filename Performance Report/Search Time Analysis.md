# Search Time Analysis

## 1. Methodology
Latency was measured using `std::chrono::high_resolution_clock` with microsecond precision. The timer captures the entire query lifecycle:
1.  **Preprocessing:** Tokenization and stop-word filtering of the query.
2.  **Retrieval:** Fetching posting lists from the `unordered_map`.
3.  **Ranking:** Calculating TF-IDF scores and summing them.
4.  **Sorting:** `std::sort` to rank documents by score.

## 2. Quantitative Benchmarks
*Test Environment: Apple Silicon M1 / 8GB RAM*
*Dataset: 20 Newsgroups (19,998 documents)*

| Query Type | Input Example | Avg. Latency | Complexity Profile | Analysis |
| :--- | :--- | :--- | :--- | :--- |
| **Direct Hit** | `encryption` | **0.18 ms** | $O(1)$ | Instant retrieval via Hash Map. No collision handling overhead observed. |
| **Common Term** | `space` | **2.75 ms** | $O(P)$ | "Space" appears in many docs. The delay is primarily due to iterating over the posting list. |
| **Multi-Keyword** | `space shuttle` | **3.85 ms** | $O(P_1 + P_2)$ | Merging two lists requires summing scores for all matching documents, slightly increasing time over a single keyword. |
| **Typo (Corrected)** | `cmputer` | **25.0 ms** | $O(V \cdot L^2)$ | Levenshtein Distance is expensive. However, filtering by the first letter optimizes this from scanning 250k words to ~10k words. |
| **Stop Word** | `the and is` | **0.01 ms** | $O(1)$ | Filtered out immediately during preprocessing. No index lookup occurred. |

## 3. Algorithmic Observations
The search time is **not** proportional to the total dataset size ($N$), but rather to the **document frequency** of the search terms.
* **Best Case:** Searching for a unique ID or rare word is nearly instantaneous ($O(1)$) because the `unordered_map` points directly to a short vector.
* **Bottleneck:** The most CPU-intensive phase is **Sorting**. As the number of matched documents ($D$) grows, the cost of `std::sort` ($D \log D$) increases.
    * *Optimization Note:* If we only needed the top 5 results, a `std::priority_queue` (Min-Heap) could optimize this to $O(D \log 5)$.
