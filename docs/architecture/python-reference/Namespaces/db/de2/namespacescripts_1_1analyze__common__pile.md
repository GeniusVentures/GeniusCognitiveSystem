---
title: scripts::analyze_common_pile

---

# scripts::analyze_common_pile



 [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| str | **[clean_text](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#function-clean_text)**(str text) |
| List[str] | **[extract_keywords](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#function-extract_keywords)**(str text, int top_n =10) |
| Tuple[List[str], List[Dict]] | **[load_and_sample_common_pile](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#function-load_and_sample_common_pile)**(int sample_size =[SAMPLE_SIZE](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#variable-sample_size)) |
| Tuple[np.ndarray, TfidfVectorizer, MiniBatchKMeans] | **[cluster_documents](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#function-cluster_documents)**(List texts[str], int n_clusters =[N_CLUSTERS](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#variable-n_clusters)) |
| List[Dict] | **[analyze_clusters](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#function-analyze_clusters)**(List texts[str], np.ndarray labels, List metadata[Dict], TfidfVectorizer vectorizer) |
| List[Dict] | **[suggest_niche_names](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#function-suggest_niche_names)**(List niches[Dict]) |
| | **[save_analysis](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#function-save_analysis)**(List niches[Dict], List texts[str], np.ndarray labels) |
| | **[print_recommendations](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#function-print_recommendations)**(List niches[Dict]) |
| | **[main](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#function-main)**() |

## Attributes

|                | Name           |
| -------------- | -------------- |
| int | **[SAMPLE_SIZE](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#variable-sample_size)**  |
| int | **[N_CLUSTERS](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#variable-n_clusters)**  |
| int | **[MIN_NICHE_SIZE](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#variable-min_niche_size)**  |
| int | **[MAX_FEATURES](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#variable-max_features)**  |
| int | **[RANDOM_SEED](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#variable-random_seed)**  |
| | **[PROJECT_ROOT](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#variable-project_root)**  |
| | **[OUTPUT_DIR](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#variable-output_dir)**  |
| | **[exist_ok](/python-reference/Namespaces/db/de2/namespacescripts_1_1analyze__common__pile/#variable-exist_ok)**  |

## Detailed Description




```
Common Pile Niche Discovery Script
Analyzes Common Pile dataset to identify viable niches for GNUS.ai specialists

This script:
1. Streams Common Pile to avoid memory issues
2. Extracts topics using TF-IDF + clustering
3. Identifies niches with sufficient data (>10k samples recommended)
4. Outputs niche recommendations with sample texts
```


## Functions Documentation

### function clean_text

```python
str clean_text(
    str text
)
```




```
Clean and normalize text for analysis```


### function extract_keywords

```python
List[str] extract_keywords(
    str text,
    int top_n =10
)
```




```
Extract potential domain keywords from text```


### function load_and_sample_common_pile

```python
Tuple[List[str], List[Dict]] load_and_sample_common_pile(
    int sample_size =SAMPLE_SIZE
)
```




```
Load Common Pile and extract representative sample
Returns: (texts, metadata)
```


### function cluster_documents

```python
Tuple[np.ndarray, TfidfVectorizer, MiniBatchKMeans] cluster_documents(
    List texts[str],
    int n_clusters =N_CLUSTERS
)
```




```
Cluster documents using TF-IDF + MiniBatchKMeans
Returns: (cluster_labels, vectorizer, clustering_model)
```


### function analyze_clusters

```python
List[Dict] analyze_clusters(
    List texts[str],
    np.ndarray labels,
    List metadata[Dict],
    TfidfVectorizer vectorizer
)
```




```
Analyze each cluster to identify niche characteristics
Returns: List of niche descriptions
```


### function suggest_niche_names

```python
List[Dict] suggest_niche_names(
    List niches[Dict]
)
```




```
Suggest human-readable names for niches based on top terms
```


### function save_analysis

```python
save_analysis(
    List niches[Dict],
    List texts[str],
    np.ndarray labels
)
```




```
Save analysis results for later use```


### function print_recommendations

```python
print_recommendations(
    List niches[Dict]
)
```




```
Print top niche recommendations```


### function main

```python
main()
```




```
Main execution```



## Attributes Documentation

### variable SAMPLE_SIZE

```python
int SAMPLE_SIZE =  50000;
```


### variable N_CLUSTERS

```python
int N_CLUSTERS =  20;
```


### variable MIN_NICHE_SIZE

```python
int MIN_NICHE_SIZE =  5000;
```


### variable MAX_FEATURES

```python
int MAX_FEATURES =  5000;
```


### variable RANDOM_SEED

```python
int RANDOM_SEED =  42;
```


### variable PROJECT_ROOT

```python
PROJECT_ROOT =  Path(__file__).resolve().parent.parent.parent;
```


### variable OUTPUT_DIR

```python
OUTPUT_DIR =  str(PROJECT_ROOT / "data" / "analysis");
```


### variable exist_ok

```python
exist_ok;
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700