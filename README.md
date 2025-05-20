# Efficient cycle enumeration on temporal networks

## Introduction
Temporal cycles are fundamental patterns in graphs with wide applications in finance, security, and neuroscience. We address the Simple Temporal Cycle Enumeration (STCE) problem: enumerating all simple cycles with strictly increasing timestamps within a time window. Existing methods, such as {\scent}, suffer from redundant checks and expensive preprocessing, making them impractical for large or evolving graphs. We propose a novel edge-centric framework that treats temporal edges as the primary exploration units. By computing edge offsets in linear time, our method depends on a constraint-based DFS that avoids unnecessary traversal and achieves polynomial delay, eliminating both time-order verification and source detection preprocessing. To support dynamic scenarios, we introduce an efficient update algorithm that incrementally explores only the affected paths via a DFS with breakpoint handling. Experiments on 16 real-world datasets show that our approach outperforms the state-of-the-art method by up to two orders of magnitude and handles updates within 1ms.

## Data Source:

FR, MS, and TR are from: https://dataverse.harvard.edu/dataset.xhtml?persistentId=doi:10.7910/DVN/V6AJRV

NL is from: http://konect.cc/networks/link-dynamic-nlwiki/

AM is from: https://www.kaggle.com/datasets/ealtman2019/ibm-transactions-for-anti-money-laundering-aml?select=HI-Large_Trans.csv

Other data are from: https://snap.stanford.edu/data/

The graph files need to follow this rule:

1. The EdgeNum lines' format: <sourceID, targetID, Timestamp>
2. Vertex ID is [1, N]
3. The edges are ordered by their timestamps 

We give two data examples CO (CollegeMsg) and SM (SMS) here.

para.csv gives the details of graphs, such as vertices number (the second line), edges number (the third line), etc.

## Code:

Use the command “ulimit -s 102400” to increase the space of the stack.

The codes are in the Folder Code.

### Run our data:

#### To compile: 

cd build

make

#### To run: 

./DynamicCycle -i {Graph_file_path} -n {vertices number} -m {edge number} -t {time type} -w {time window} -a {algorithm type} -e {result_file_path}

#### Algorithm Type :

1: F-DFS 2: B-DFS 3: IB-DFS 4: B-DFS for dynamic search

#### Time Type:

0: hours; 1: minutes; 2: seconds

### The Baseline Solution and 2SCENT code:

https://github.com/rohit13k/CycleDetection
