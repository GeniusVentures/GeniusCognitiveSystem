---
title: GNUS-NEO-SWARM/gnus-poc/data/scripts/extract_source_niches.py

---

# GNUS-NEO-SWARM/gnus-poc/data/scripts/extract_source_niches.py





## Namespaces

| Name           |
| -------------- |
| **[scripts](/python-reference/Namespaces/df/d75/namespacescripts/)**  |
| **[scripts::extract_source_niches](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/)**  |

## Functions

|                | Name           |
| -------------- | -------------- |
| | **[extract_source_based_niches](/python-reference/Files/d7/dbe/extract__source__niches_8py/#function-extract_source_based_niches)**(sample_size sample_size =50000) |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[PROJECT_ROOT](/python-reference/Files/d7/dbe/extract__source__niches_8py/#variable-project_root)**  |
| | **[exist_ok](/python-reference/Files/d7/dbe/extract__source__niches_8py/#variable-exist_ok)**  |
| dict | **[TARGET_NICHES](/python-reference/Files/d7/dbe/extract__source__niches_8py/#variable-target_niches)**  |
| | **[viable_niches](/python-reference/Files/d7/dbe/extract__source__niches_8py/#variable-viable_niches)**  |
| | **[source_counts](/python-reference/Files/d7/dbe/extract__source__niches_8py/#variable-source_counts)**  |
| | **[all_niche_samples](/python-reference/Files/d7/dbe/extract__source__niches_8py/#variable-all_niche_samples)**  |
| | **[sample_size](/python-reference/Files/d7/dbe/extract__source__niches_8py/#variable-sample_size)**  |
| dict | **[output_data](/python-reference/Files/d7/dbe/extract__source__niches_8py/#variable-output_data)**  |
| | **[output_path](/python-reference/Files/d7/dbe/extract__source__niches_8py/#variable-output_path)**  |
| | **[f](/python-reference/Files/d7/dbe/extract__source__niches_8py/#variable-f)**  |
| | **[indent](/python-reference/Files/d7/dbe/extract__source__niches_8py/#variable-indent)**  |


## Functions Documentation

### function extract_source_based_niches

```python
extract_source_based_niches(
    sample_size sample_size =50000
)
```




```
Extract niches directly from source labels```



## Attributes Documentation

### variable PROJECT_ROOT

```python
PROJECT_ROOT =  Path(__file__).resolve().parent.parent.parent;
```


### variable exist_ok

```python
exist_ok;
```


### variable TARGET_NICHES

```python
dict TARGET_NICHES;
```


### variable viable_niches

```python
viable_niches;
```


### variable source_counts

```python
source_counts;
```


### variable all_niche_samples

```python
all_niche_samples;
```


### variable sample_size

```python
sample_size;
```


### variable output_data

```python
dict output_data =  {
        'viable_niches': viable_niches,
        'all_sources': source_counts,
        'extraction_config': {
            'sample_size': 50000,
            'target_niches': TARGET_NICHES
        }
    };
```


### variable output_path

```python
output_path =  str(PROJECT_ROOT / 'data' / 'analysis' / 'source_based_niches.json');
```


### variable f

```python
f;
```


### variable indent

```python
indent;
```



## Source code

```python
"""
Source-based niche extraction from Common Pile
More reliable than clustering for creating distinct specialists
"""

import json
from datasets import load_dataset
from collections import Counter, defaultdict
import os

# Create output directory
from pathlib import Path
PROJECT_ROOT = Path(__file__).resolve().parent.parent.parent
os.makedirs(str(PROJECT_ROOT / 'data' / 'analysis'), exist_ok=True)

# Target niches based on Common Pile sources
TARGET_NICHES = {
    'medical': {
        'sources': ['PubMed Abstracts', 'PubMed Central', 'NIH ExPorter'],
        'min_samples': 5000,
        'description': 'Medical research, clinical studies, biomedical science'
    },
    'patents': {
        'sources': ['USPTO Backgrounds'],
        'min_samples': 3000,  # Lowered based on cluster data
        'description': 'Patent applications, inventions, technical innovations'
    },
    'code': {
        'sources': ['Github'],
        'min_samples': 2000,  # Lowered - code is valuable even in smaller amounts
        'description': 'Software code, programming, technical documentation'
    },
    'qa_technical': {
        'sources': ['StackExchange'],
        'min_samples': 3000,
        'description': 'Technical Q&A, problem-solving, community knowledge'
    },
    'encyclopedic': {
        'sources': ['Wikipedia (en)'],
        'min_samples': 3000,
        'description': 'General knowledge, encyclopedic content'
    },
    'legal': {
        'sources': ['FreeLaw'],
        'min_samples': 2000,
        'description': 'Legal documents, court cases, legal reasoning'
    },
    'books': {
        'sources': ['Gutenberg (PG-19)', 'BookCorpus2'],
        'min_samples': 2000,
        'description': 'Literature, narrative text, creative writing'
    }
}


def extract_source_based_niches(sample_size=50000):
    """Extract niches directly from source labels"""

    print("Loading Common Pile with source labels...")
    print("(This will take 10-15 minutes for 50k samples)\n")

    try:
        dataset = load_dataset(
            "monology/pile-uncopyrighted",
            split="train",
            streaming=True,
            trust_remote_code=True
        )
    except Exception as e:
        print(f"Trying alternative dataset due to: {e}")
        dataset = load_dataset(
            "EleutherAI/pile",
            split="train",
            streaming=True,
            trust_remote_code=True
        )

    niche_samples = defaultdict(list)
    source_counts = Counter()
    total_processed = 0

    for i, example in enumerate(dataset):
        if i >= sample_size:
            break

        if i % 2500 == 0:
            print(f"Processed {i}/{sample_size}... ({i / sample_size * 100:.0f}%)")

        # Extract source from metadata
        meta = example.get('meta', {})
        source = meta.get('pile_set_name', 'unknown')

        # Handle different metadata formats
        if source == 'unknown' and isinstance(meta, dict):
            # Try alternative metadata keys
            source = meta.get('source', meta.get('dataset', 'unknown'))

        source_counts[source] += 1
        total_processed += 1

        # Assign to niche
        text = example.get('text', example.get('content', ''))

        if len(text) >= 100:  # Only keep substantial texts
            for niche_name, niche_config in TARGET_NICHES.items():
                if source in niche_config['sources']:
                    niche_samples[niche_name].append({
                        'text': text,
                        'source': source,
                        'length': len(text),
                        'index': i
                    })
                    break

    print(f"\n✓ Processed {total_processed} documents\n")

    print("=" * 80)
    print("SOURCE DISTRIBUTION IN COMMON PILE:")
    print("=" * 80)
    for source, count in source_counts.most_common(25):
        print(f"  {source:<35} {count:>6} ({count / total_processed * 100:>5.1f}%)")

    print("\n" + "=" * 80)
    print("NICHE EXTRACTION RESULTS:")
    print("=" * 80)

    viable_niches = []
    for niche_name, samples in sorted(niche_samples.items(), key=lambda x: len(x[1]), reverse=True):
        config = TARGET_NICHES[niche_name]
        size = len(samples)

        if size >= config['min_samples']:
            viable = True
            status = "✓ VIABLE FOR SPECIALIST"

            viable_niches.append({
                'name': niche_name,
                'size': size,
                'percentage': size / total_processed * 100,
                'description': config['description'],
                'sources': config['sources'],
                'avg_length': int(sum(s['length'] for s in samples) / len(samples)),
                'samples': [s['text'][:400] + "..." for s in samples[:3]]
            })
        else:
            viable = False
            status = f"✗ Too small (need {config['min_samples']}, got {size})"

        print(f"\n{niche_name.upper()}: {size:,} samples ({size / total_processed * 100:.1f}%)")
        print(f"  {config['description']}")
        print(f"  Sources: {', '.join(config['sources'])}")
        print(f"  Status: {status}")

        if viable and samples:
            print(f"  Avg length: {sum(s['length'] for s in samples) / len(samples):.0f} chars")
            print(f"  Sample: {samples[0]['text'][:200]}...")

    return viable_niches, dict(source_counts.most_common()), niche_samples


if __name__ == "__main__":
    print("GNUS.AI Source-Based Niche Extraction")
    print("=" * 80 + "\n")

    viable_niches, source_counts, all_niche_samples = extract_source_based_niches(sample_size=50000)

    print("\n" + "=" * 80)
    print("FINAL RECOMMENDATIONS FOR GNUS.AI SPECIALISTS")
    print("=" * 80)

    if len(viable_niches) >= 3:
        print(f"\n✓ Found {len(viable_niches)} viable niches for specialist training!\n")

        for i, niche in enumerate(viable_niches, 1):
            print(f"{i}. {niche['name'].upper()} Specialist")
            print(f"   Training samples: {niche['size']:,}")
            print(f"   Coverage: {niche['percentage']:.1f}% of dataset")
            print(f"   Avg length: {niche['avg_length']:,} chars")
            print(f"   Focus: {niche['description']}")
            print(f"   Sample text:")
            print(f"     {niche['samples'][0][:250]}...")
            print()
    else:
        print(f"\n⚠ Only found {len(viable_niches)} viable niches.")
        print("Recommendations:")
        print("  1. Increase sample_size to 100k for more coverage")
        print("  2. Lower min_samples thresholds in TARGET_NICHES")
        print("  3. Check if dataset source labels are available")

    # Save detailed results
    output_data = {
        'viable_niches': viable_niches,
        'all_sources': source_counts,
        'extraction_config': {
            'sample_size': 50000,
            'target_niches': TARGET_NICHES
        }
    }

    output_path = str(PROJECT_ROOT / 'data' / 'analysis' / 'source_based_niches.json')
    with open(output_path, 'w') as f:
        json.dump(output_data, f, indent=2)

    print("=" * 80)
    print(f"✓ Results saved to {output_path}")
    print("\nNext steps:")
    print("  1. Review the viable niches above")
    print("  2. Select 3-5 for specialist training")
    print("  3. Run prepare_datasets.py to create training splits")
    print("=" * 80)
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700
